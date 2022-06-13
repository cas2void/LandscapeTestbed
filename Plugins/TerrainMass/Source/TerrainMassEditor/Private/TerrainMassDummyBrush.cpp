#include "TerrainMassDummyBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetRenderingLibrary.h"

#include "ScalarRamp.h"
#include "TerrainMassDummyShader.h"
#include "TerrainMassCompositeShader.h"

ATerrainMassDummyBrush::ATerrainMassDummyBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);
}

UTextureRenderTarget2D* ATerrainMassDummyBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);

    if (!CanvasRT || CanvasRT->SizeX != RenderTargetSize.X || CanvasRT->SizeY != RenderTargetSize.Y)
    {
        CanvasRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_RG8);
    }

    if (!BlendRT || BlendRT->SizeX != RenderTargetSize.X || BlendRT->SizeY != RenderTargetSize.Y)
    {
        BlendRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_R8);
    }

    if (!OutputRT || OutputRT->SizeX != RenderTargetSize.X || OutputRT->SizeY != RenderTargetSize.Y)
    {
        OutputRT = UKismetRenderingLibrary::CreateRenderTarget2D(this, RenderTargetSize.X, RenderTargetSize.Y, RTF_RGBA8);
    }

    if (!CanvasRT || !BlendRT || !OutputRT)
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

    FVector Center = GetActorLocation();
    float RadiusScale = 1.0f;
    if (GetOwningLandscape())
    {
        FTransform BrushRelativeTransform = GetActorTransform().GetRelativeTransform(GetOwningLandscape()->GetActorTransform());
        Center = BrushRelativeTransform.GetLocation();
        RadiusScale = BrushRelativeTransform.GetScale3D().X;
    }
    Center.Z *= LANDSCAPE_INV_ZSCALE;

    FTerrainMassDummyShaderParameter ShaderParams;
    ShaderParams.SideFalloffTexture = SideFalloffTexture;
    ShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);
    ShaderParams.Center = Center;
    ShaderParams.Radius = Radius * RadiusScale / RenderTargetSize.X;
#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
    ShaderParams.PosScaleBias = FVector4(RenderTargetSize.X, RenderTargetSize.Y, 0.0f, 0.0f);
    ShaderParams.UVScaleBias = FVector4(1.0f, 1.0f, 0.0f, 0.0f);
    ShaderParams.InvTargetSizeAndTextureSize = FVector4(1.0f / RenderTargetSize.X, 1.0f / RenderTargetSize.Y, 1.0f, 1.0f);
#endif

    FTerrainMassDummyShader::Render(CanvasRT, RenderTargetSize, ShaderParams);

    FTerrainMassCompositeShaderParameter CompositeShaderParams;
    CompositeShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);
#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
    CompositeShaderParams.PosScaleBias = FVector4(RenderTargetSize.X, RenderTargetSize.Y, 0.0f, 0.0f);
    CompositeShaderParams.UVScaleBias = FVector4(1.0f, 1.0f, 0.0f, 0.0f);
    CompositeShaderParams.InvTargetSizeAndTextureSize = FVector4(1.0f / RenderTargetSize.X, 1.0f / RenderTargetSize.Y, 1.0f, 1.0f);
#endif

    FTerrainMassCompositeShader::Render(InCombinedResult, CanvasRT, BlendRT, OutputRT, RenderTargetSize, CompositeShaderParams);

    return OutputRT;
}

void ATerrainMassDummyBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
    UE_LOG(LogTemp, Warning, TEXT("ATerrainMassDummyBrush::Initialize_Native"));
}

#if WITH_EDITOR
void ATerrainMassDummyBrush::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassDummyBrush, SideFalloffRamp) ||
        PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ATerrainMassDummyBrush, SideFalloffRamp))
    {
        SideFalloffRamp.WriteTexture(SideFalloffTexture);
    }
}
#endif

void ATerrainMassDummyBrush::PostRegisterAllComponents()
{
    Super::PostRegisterAllComponents();

    if (!SideFalloffTexture)
    {
        SideFalloffTexture = FScalarRamp::CreateTexture(256);
        SideFalloffRamp.WriteTexture(SideFalloffTexture);
    }
}