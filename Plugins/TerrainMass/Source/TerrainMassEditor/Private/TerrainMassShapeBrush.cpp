#include "TerrainMassShapeBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeInfo.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "Editor.h"

#include "TerrainMassHandleComponent.h"
#include "TerrainMassSplineComponent.h"
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

    ArrowComponent = CreateDefaultSubobject<UTerrainMassHandleComponent>(FName(TEXT("Arrow")));
    ArrowComponent->SetupAttachment(RootComponent);
    ArrowComponent->SetArrowColor(FLinearColor::Yellow);
    ArrowComponent->ArrowSize = 40.0f;
    ArrowComponent->ArrowLength = 25.0f;
    SetRootComponent(ArrowComponent);

    SplineComponent = CreateDefaultSubobject<UTerrainMassSplineComponent>(FName(TEXT("Spline")));
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
    // Brush Rendering
    //

    //
    // Shape
    //
    if (IsDirty(EShapeBrushDirtyLevel::ShapeData))
    {
        // Vertex Buffer
        ShapeVertices.Empty();
        for (float Time = 0.0f; Time < SplineComponent->Duration; Time += 0.01f)
        {
            FVector LocalLocation = SplineComponent->GetLocationAtTime(Time, ESplineCoordinateSpace::Local);
            ShapeVertices.Emplace(LocalLocation);
        }

        if (ShapeVertices.Num() < 3)
        {
            return nullptr;
        }

        // Index Buffer
        ShapeIndices.Empty();
        const int32 NumPrimitives = ShapeVertices.Num() - 2;
        const int32 NumIndices = NumPrimitives * 3;
        ShapeIndices.AddUninitialized(NumIndices);
        for (int32 Index = 0; Index < NumPrimitives; Index++)
        {
            ShapeIndices[Index * 3 + 0] = 0;
            ShapeIndices[Index * 3 + 1] = Index + 1;
            ShapeIndices[Index * 3 + 2] = Index + 2;
        }

        //UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::ShapeData"));
        ResetDirty(EShapeBrushDirtyLevel::ShapeData);
    }

    if (IsDirty(EShapeBrushDirtyLevel::ShapeRT))
    {
        FTerrainMassShapeShaderParameter ShapeShaderParams;

        // Transform vertex from world to uv space
        int32 MinX, MinY, MaxX, MaxY;
        Landscape->GetLandscapeInfo()->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
        FVector2D LandscapeSize(MaxX - MinX, MaxY - MinY);

        FTransform HeightmapTransform(FTransform::Identity);
        FVector HeightmapScale = FVector(LandscapeSize, LANDSCAPE_ZSCALE);
        HeightmapTransform.SetScale3D(FVector(1.0f) / HeightmapScale);

        FTransform UVScaleTransform(FTransform::Identity);
        if (bUVOffset)
        {
            FVector2D LandscapeUVOffset = LandscapeSize * InvTextureSize;
            UVScaleTransform.SetScale3D(FVector(LandscapeUVOffset, 1.0f));
        }

        FTransform HalfPixelTransform(FTransform::Identity);
        FVector2D HalfPixelOffset = InvTextureSize * 0.5f;
        HalfPixelTransform.SetLocation(FVector(HalfPixelOffset, 0.0f));

        ShapeShaderParams.Local2UV = SplineComponent->GetComponentTransform().ToMatrixWithScale() * Landscape->GetActorTransform().ToMatrixWithScale().Inverse() * 
            HeightmapTransform.ToMatrixWithScale() * UVScaleTransform.ToMatrixWithScale() * HalfPixelTransform.ToMatrixNoScale();

        UKismetRenderingLibrary::ClearRenderTarget2D(this, ShapeRT);
        FTerrainMassShapeShader::Render(ShapeRT, ShapeVertices, ShapeIndices, ShapeShaderParams);

        //UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::ShapeRT"));
        ResetDirty(EShapeBrushDirtyLevel::ShapeRT);
    }

    //
    // Jump Flooding
    //
    if (IsDirty(EShapeBrushDirtyLevel::JumpFlooding))
    {
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

        //UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::JumpFlooding"));
        ResetDirty(EShapeBrushDirtyLevel::JumpFlooding);
    }

    if (IsDirty(EShapeBrushDirtyLevel::DistanceField))
    {
        FTerrainMassDistanceFieldShaderParameter DistanceFieldShaderParams;
        DistanceFieldShaderParams.InvTextureSize = InvTextureSize;
        DistanceFieldShaderParams.Width = FVector2D(Width) / FVector2D(Landscape->GetActorScale());

        FTerrainMassJumpFloodingShader::DistanceField(DistanceFieldRT, JumpFloodingRTs[OutputIndex], DistanceFieldShaderParams);

        //UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::DistanceField"));
        ResetDirty(EShapeBrushDirtyLevel::DistanceField);
    }

    //
    // Blur
    //
    if (IsDirty(EShapeBrushDirtyLevel::Blur))
    {
        if (bBlur)
        {
            FTerrainMassGaussianBlurShaderParameter BlurShaderParams;
            BlurShaderParams.InvTextureSize = InvTextureSize;
            BlurShaderParams.KernelSize = KernelSize;
            BlurShaderParams.Sigma = Sigma;

            FTerrainMassGaussianBlurShader::Render(BlurRT, DistanceFieldRT, BlurIntermediateRT, BlurShaderParams);
        }

        //UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::Blur"));
        ResetDirty(EShapeBrushDirtyLevel::Blur);
    }

    //
    // Composition
    //
    {
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
    }

    return OutputRT;
}

void ATerrainMassShapeBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
    //UE_LOG(LogTemp, Warning, TEXT("ATerrainMassShapeBrush::Initialize_Native"));
    
    DirtyFlags.Empty();
    DirtyFlags.Init(true, static_cast<int32>(EShapeBrushDirtyLevel::Max));

    SplineComponent->OnSplineUpdated().RemoveAll(this);
    SplineComponent->OnSplineUpdated().AddUObject(this, &ATerrainMassShapeBrush::OnSplineUpdated);

    const float MinFloat = TNumericLimits<float>::Lowest();
    PreviousLocation = FVector(MinFloat);
    PreviousRotation = FQuat(MinFloat, MinFloat, MinFloat, MinFloat);
    PreviousScale = FVector(MinFloat);

    GetRootComponent()->TransformUpdated.RemoveAll(this);
    GetRootComponent()->TransformUpdated.AddUObject(this, &ATerrainMassShapeBrush::OnTransformUpdated);
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
    else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, bUVOffset))
    {
        MarkDirty(EShapeBrushDirtyLevel::ShapeRT);
    }
    else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, bSetIteration) ||
        PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, NumIteration))
    {
        MarkDirty(EShapeBrushDirtyLevel::JumpFlooding);
    }
    else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, Width))
    {
        MarkDirty(EShapeBrushDirtyLevel::DistanceField);
    }
    else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, bBlur) ||
        PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, KernelSize) ||
        PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassShapeBrush, Sigma))
    {
        MarkDirty(EShapeBrushDirtyLevel::Blur);
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
}

void ATerrainMassShapeBrush::MarkDirty(EShapeBrushDirtyLevel DirtyLevel)
{
    int32 LevelAsIndex = static_cast<int32>(DirtyLevel);
    if (DirtyFlags.IsValidIndex(LevelAsIndex))
    {
        for (int32 Index = LevelAsIndex; Index < DirtyFlags.Num(); Index++)
        {
            DirtyFlags[Index] = true;
        }
    }
}

bool ATerrainMassShapeBrush::IsDirty(EShapeBrushDirtyLevel DirtyLevel) const
{
    bool Result = false;
    int32 LevelAsIndex = static_cast<int32>(DirtyLevel);
    if (DirtyFlags.IsValidIndex(LevelAsIndex))
    {
        Result = DirtyFlags[LevelAsIndex];
    }
    return Result;
}

void ATerrainMassShapeBrush::ResetDirty(EShapeBrushDirtyLevel DirtyLevel)
{
    int32 LevelAsIndex = static_cast<int32>(DirtyLevel);
    if (DirtyFlags.IsValidIndex(LevelAsIndex))
    {
        DirtyFlags[LevelAsIndex] = false;
    }
}

void ATerrainMassShapeBrush::OnTransformUpdated(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
    FVector CurrentLocation = UpdatedComponent->GetRelativeTransform().GetLocation();
    FQuat CurrentRotation = UpdatedComponent->GetRelativeTransform().GetRotation();
    FVector CurrentScale = UpdatedComponent->GetRelativeTransform().GetScale3D();

    if (CurrentRotation != PreviousRotation || CurrentScale != PreviousScale || CurrentLocation.X != PreviousLocation.X || CurrentLocation.Y != PreviousLocation.Y)
    {
        MarkDirty(EShapeBrushDirtyLevel::ShapeRT);
    }

    PreviousLocation = CurrentLocation;
    PreviousRotation = CurrentRotation;
    PreviousScale = CurrentScale;
}

void ATerrainMassShapeBrush::OnSplineUpdated()
{
    MarkDirty(EShapeBrushDirtyLevel::ShapeData);
}
