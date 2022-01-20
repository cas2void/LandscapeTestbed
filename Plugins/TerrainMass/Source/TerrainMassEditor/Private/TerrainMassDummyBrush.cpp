#include "TerrainMassDummyBrush.h"

#include "Landscape.h"

#include "TerrainMassLibrary.h"
#include "TerrainMassDummyShader.h"

ATerrainMassDummyBrush::ATerrainMassDummyBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);
}

UTextureRenderTarget2D* ATerrainMassDummyBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);
    InputRT = InCombinedResult;

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);
    if (!UTerrainMassLibrary::CreateOrUpdateRenderTarget(GetTransientPackage(), CanvasRT, RenderTargetSize, RTF_R16f, true))
    {
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
    
    FTerrainMassDummyShaderParameter ShaderParams;
    ShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);
    ShaderParams.Center = Center;
    ShaderParams.Radius = Radius * RadiusScale / RenderTargetSize.X;

    ENQUEUE_RENDER_COMMAND(TerranMassDummyBrush)(
        [this, RenderTargetSize, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (CanvasRT && CanvasRT->GetRenderTargetResource() && CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                FTerrainMassDummyShader::Render(RHICmdList, CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture(), RenderTargetSize, ShaderParams);
            }
        }
    );

    // LandscapeEditLayers.cpp - ALandscape::RegenerateLayersHeightmaps
    //
    // if Brush::Render_Native() return non-null value, returned RT will be copy to InCombinedResult by ExecuteCopyLayersTexture(),
    // otherwise, InCombinedResult won't be touched.
    //
    // Side note: 
    // if returned RT's format is different to InCombinedResult, copy operation will be excecuted, but the content of destination does not changed.
    return nullptr;
}

void ATerrainMassDummyBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
}
