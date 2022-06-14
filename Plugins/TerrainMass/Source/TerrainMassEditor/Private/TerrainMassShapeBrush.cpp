#include "TerrainMassShapeBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/SplineComponent.h"

#include "ScalarRamp.h"
#include "TerrainMassShapeShader.h"

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

    //FVector Center = GetActorLocation();
    //float RadiusScale = 1.0f;
    //if (GetOwningLandscape())
    //{
    //    FTransform BrushRelativeTransform = GetActorTransform().GetRelativeTransform(GetOwningLandscape()->GetActorTransform());
    //    Center = BrushRelativeTransform.GetLocation();
    //    RadiusScale = BrushRelativeTransform.GetScale3D().X;
    //}
    //Center.Z *= LANDSCAPE_INV_ZSCALE;

    ALandscape* Landscape = GetOwningLandscape();
    if (!Landscape || Landscape->LandscapeComponents.Num() < 1 || !Landscape->LandscapeComponents[0])
    {
        return nullptr;
    }

    FIntPoint ComponentCounts = Landscape->ComputeComponentCounts();
    int32 SectionSize = Landscape->LandscapeComponents[0]->NumSubsections * Landscape->LandscapeComponents[0]->SubsectionSizeQuads;
    FIntPoint LandscapeResolution = ComponentCounts * SectionSize;
    FVector LandscapeUVScale = FVector(LandscapeResolution.X, LandscapeResolution.Y, LANDSCAPE_ZSCALE);

    if (SplineComponent->GetNumberOfSplinePoints() > 2)
    {
        FTerrainMassShapeShaderParameter ShaderParams;
        ShaderParams.SideFalloffTexture = SideFalloffTexture;
        ShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);

        TArray<FVector> ShapePoints;
        for (float Time = 0.0f; Time < SplineComponent->Duration; Time += 0.01f)
        {
            FVector WorldLocation = SplineComponent->GetLocationAtTime(Time, ESplineCoordinateSpace::World);
            FVector TextureLocation = Landscape->GetActorTransform().InverseTransformPosition(WorldLocation);
            ShapePoints.Add(TextureLocation / LandscapeUVScale);
        }

        FTerrainMassShapeShader::Render(ShapeRT, ShapePoints, ShaderParams);
    }

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