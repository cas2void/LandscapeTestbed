#include "TerrainMassShapeBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeInfo.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/SplineComponent.h"

#include "ScalarRamp.h"
#include "TerrainMassShapeShader.h"
#include "TerrainMassGaussianBlurShader.h"

ATerrainMassShapeBrush::ATerrainMassShapeBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);

    SplineComponent = CreateDefaultSubobject<USplineComponent>(FName("Spline"));
    SetRootComponent(SplineComponent);
}

UTextureRenderTarget2D* ATerrainMassShapeBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);

    if (!OutputRT || OutputRT->SizeX != RenderTargetSize.X || OutputRT->SizeY != RenderTargetSize.Y)
    {
        OutputRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, InCombinedResult->RenderTargetFormat);
    }

    if (!ShapeRT || ShapeRT->SizeX != RenderTargetSize.X || ShapeRT->SizeY != RenderTargetSize.Y)
    {
        ShapeRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R32f);
    }

    if (!BlurIntermediateRT || BlurIntermediateRT->SizeX != RenderTargetSize.X || BlurIntermediateRT->SizeY != RenderTargetSize.Y)
    {
        BlurIntermediateRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R32f);
    }

    if (!BlurRT || BlurRT->SizeX != RenderTargetSize.X || BlurRT->SizeY != RenderTargetSize.Y)
    {
        BlurRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R32f);
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
    TArray<FTerrainMassShapeVertex> ShapePoints;
    for (float Time = 0.0f; Time < SplineComponent->Duration; Time += 0.01f)
    {
        FVector WorldLocation = SplineComponent->GetLocationAtTime(Time, ESplineCoordinateSpace::World);
        ShapePoints.Emplace(WorldLocation);
    }

    if (ShapePoints.Num() < 3)
    {
        return nullptr;
    }

    FTerrainMassShapeShaderParameter ShapeShaderParams;
    ShapeShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);

    int32 MinX, MinY, MaxX, MaxY;
    Landscape->GetLandscapeInfo()->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
    FVector LandscapeUVScale = FVector(MaxX - MinX, MaxY - MinY, LANDSCAPE_ZSCALE);
    FTransform ScaleTransform(FTransform::Identity);
    ScaleTransform.SetScale3D(FVector(1.0f) / LandscapeUVScale);
    ShapeShaderParams.World2UV = Landscape->GetActorTransform().ToMatrixWithScale().Inverse() * ScaleTransform.ToMatrixWithScale();

    FTerrainMassShapeShader::Render(ShapeRT, ShapePoints, ShapeShaderParams);

    //
    // Blur
    //
    FTerrainMassGaussianBlurShaderParameter BlurShaderParams;
    BlurShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);
    BlurShaderParams.KernelSize = KernelSize;

    FTerrainMassGaussianBlurShader::Render(ShapeRT, BlurRT, BlurIntermediateRT, BlurShaderParams);

    return nullptr;
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
}