#include "TerrainMassShapeBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeInfo.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"

#include "ScalarRamp.h"
#include "TerrainMassShapeShader.h"
#include "TerrainMassGaussianBlurShader.h"
#include "TerrainMassJumpFloodingShader.h"
#include "TerrainMassShapeCompositionShader.h"

ATerrainMassShapeBrush::ATerrainMassShapeBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(FName(TEXT("Arrow")));
    ArrowComponent->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    ArrowComponent->SetupAttachment(RootComponent);
    ArrowComponent->SetArrowColor(FLinearColor::Yellow);
    ArrowComponent->ArrowSize = 40.0f;
    ArrowComponent->ArrowLength = 25.0f;
    ArrowComponent->bIsScreenSizeScaled = true;
    SetRootComponent(ArrowComponent);

    SplineComponent = CreateDefaultSubobject<USplineComponent>(FName(TEXT("Spline")));
    SplineComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
    SplineComponent->SetupAttachment(ArrowComponent);
}

UTextureRenderTarget2D* ATerrainMassShapeBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);
    FVector2D InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);

    if (!OutputRT || OutputRT->SizeX != RenderTargetSize.X || OutputRT->SizeY != RenderTargetSize.Y)
    {
        OutputRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, InCombinedResult->RenderTargetFormat);
    }

    if (!ShapeRT || ShapeRT->SizeX != RenderTargetSize.X || ShapeRT->SizeY != RenderTargetSize.Y)
    {
        ShapeRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R16f);
    }

    for (int32 Index = 0; Index < 2; Index++)
    {
        if (!JumpFloodingRTs[Index] || JumpFloodingRTs[Index]->SizeX != RenderTargetSize.X || JumpFloodingRTs[Index]->SizeY != RenderTargetSize.Y)
        {
            JumpFloodingRTs[Index] = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_RG16f);
        }
    }

    if (!DistanceFieldRT || DistanceFieldRT->SizeX != RenderTargetSize.X || DistanceFieldRT->SizeY != RenderTargetSize.Y)
    {
        DistanceFieldRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R16f);
    }

    if (!BlurIntermediateRT || BlurIntermediateRT->SizeX != RenderTargetSize.X || BlurIntermediateRT->SizeY != RenderTargetSize.Y)
    {
        BlurIntermediateRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R16f);
    }

    if (!BlurRT || BlurRT->SizeX != RenderTargetSize.X || BlurRT->SizeY != RenderTargetSize.Y)
    {
        BlurRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R16f);
    }

    UKismetRenderingLibrary::ClearRenderTarget2D(this, ShapeRT);

    if (!OutputRT)
    {
        // LandscapeEditLayers.cpp - ALandscape::RegenerateLayersHeightmaps
        //
        // if Brush::Render_Native() return non-null value, returned RT will be copy to InCombinedResult by ExecuteCopyLayersTexture(),
        // otherwise, InCombinedResult won't be touched by the returned value.
        //
        // Side note: 
        // if returned RT's format is different to InCombinedResult, copy operation will be excecuted, but the content of destination does not change.
        return nullptr;
    }

    ALandscape* Landscape = GetOwningLandscape();
    if (!Landscape || !Landscape->GetLandscapeInfo())
    {
        return nullptr;
    }

    //
    // Shape
    //

    // Vertex Buffer
    TArray<FTerrainMassShapeVertex> ShapeVertices;
    for (float Time = 0.0f; Time < SplineComponent->Duration; Time += 0.01f)
    {
        FVector WorldLocation = SplineComponent->GetLocationAtTime(Time, ESplineCoordinateSpace::World);
        ShapeVertices.Emplace(WorldLocation);
    }

    if (ShapeVertices.Num() < 3)
    {
        return nullptr;
    }

    // Index Buffer
    TArray<uint16> ShapeIndices;
    const int32 NumPrimitives = ShapeVertices.Num() - 2;
    const int32 NumIndices = NumPrimitives * 3;
    ShapeIndices.AddUninitialized(NumIndices);
    for (int32 Index = 0; Index < NumPrimitives; Index++)
    {
        ShapeIndices[Index * 3 + 0] = 0;
        ShapeIndices[Index * 3 + 1] = Index + 1;
        ShapeIndices[Index * 3 + 2] = Index + 2;
    }

    FTerrainMassShapeShaderParameter ShapeShaderParams;

    // Transform vertex from world to uv space
    int32 MinX, MinY, MaxX, MaxY;
    Landscape->GetLandscapeInfo()->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
    FVector2D LandscapeSize(MaxX - MinX, MaxY - MinY);

    FTransform ScaleTransform(FTransform::Identity);
    FVector LandscapeUVScale = FVector(LandscapeSize, LANDSCAPE_ZSCALE);
    ScaleTransform.SetScale3D(FVector(1.0f) / LandscapeUVScale);

    FTransform UVOffsetTransform(FTransform::Identity);
    if (bUVOffset)
    {
        FVector2D LandscapeUVOffset = (LandscapeSize - FVector2D(RenderTargetSize)) * 0.5f * InvTextureSize;
        UVOffsetTransform.SetLocation(FVector(LandscapeUVOffset, 0.0f));
    }

    ShapeShaderParams.World2UV = Landscape->GetActorTransform().ToMatrixWithScale().Inverse() * ScaleTransform.ToMatrixWithScale() * UVOffsetTransform.ToMatrixNoScale();

    FTerrainMassShapeShader::Render(ShapeRT, ShapeVertices, ShapeIndices, ShapeShaderParams);

    //
    // Jump Flooding
    //

    FTerrainMassJumpFloodingShaderParameter JumpFloodingShaderParams;
    JumpFloodingShaderParams.InvTextureSize = InvTextureSize;

    int32 InputIndex = 0;
    FTerrainMassJumpFloodingShader::Encode(JumpFloodingRTs[InputIndex], ShapeRT);

    OutputIndex = 0;
    if (bSetIteration)
    {
        FTerrainMassJumpFloodingShader::Flood(JumpFloodingRTs, OutputIndex, InputIndex, NumIteration, JumpFloodingShaderParams);
    }
    else
    {
        FTerrainMassJumpFloodingShader::Flood(JumpFloodingRTs, OutputIndex, InputIndex, JumpFloodingShaderParams);
    }

    FTerrainMassDistanceFieldShaderParameter DistanceFieldShaderParams;
    DistanceFieldShaderParams.InvTextureSize = InvTextureSize;
    DistanceFieldShaderParams.Width = FVector2D(Width) / FVector2D(Landscape->GetActorScale());

    FTerrainMassJumpFloodingShader::DistanceField(DistanceFieldRT, JumpFloodingRTs[OutputIndex], DistanceFieldShaderParams);

    //
    // Blur
    //
    if (bBlur)
    {
        FTerrainMassGaussianBlurShaderParameter BlurShaderParams;
        BlurShaderParams.InvTextureSize = InvTextureSize;
        BlurShaderParams.KernelSize = KernelSize;
        BlurShaderParams.Sigma = Sigma;

        FTerrainMassGaussianBlurShader::Render(BlurRT, DistanceFieldRT, BlurIntermediateRT, BlurShaderParams);
    }
    
    //
    // Composition
    //
    FTerrainMassShapeCompositionShaderParameter CompositionShaderParams;
    CompositionShaderParams.SideFalloffTexture = SideFalloffTexture;

    FVector ArrowLocation = ArrowComponent->GetComponentLocation();
    float ElevationInHeightMap = Landscape->GetActorTransform().InverseTransformPosition(ArrowLocation).Z * LANDSCAPE_INV_ZSCALE;
    CompositionShaderParams.Elevation = ElevationInHeightMap;

    if (bBlur)
    {
        FTerrainMassShapeCompositionShader::Render(OutputRT, InCombinedResult, BlurRT, CompositionShaderParams);
    }
    else
    {
        FTerrainMassShapeCompositionShader::Render(OutputRT, InCombinedResult, DistanceFieldRT, CompositionShaderParams);
    }

    return OutputRT;
}

void ATerrainMassShapeBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
    UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::Initialize_Native"));
}

#if WITH_EDITOR
void ATerrainMassShapeBrush::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, SideFalloffRamp) ||
        PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, SideFalloffRamp))
    {
        SideFalloffRamp.WriteTexture(SideFalloffTexture);
    }
}
#endif

void ATerrainMassShapeBrush::PostRegisterAllComponents()
{
    Super::PostRegisterAllComponents();

    if (!SideFalloffTexture)
    {
        SideFalloffTexture = FScalarRamp::CreateTexture(256);
        SideFalloffRamp.WriteTexture(SideFalloffTexture);
    }

    ArrowComponent->TransformUpdated.RemoveAll(this);
    ArrowComponent->TransformUpdated.AddLambda(
        [this](USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
        {
            RequestLandscapeUpdate();
        });
}