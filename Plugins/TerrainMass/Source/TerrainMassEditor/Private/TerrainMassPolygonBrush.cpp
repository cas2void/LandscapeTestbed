#include "TerrainMassPolygonBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"

#include "TerrainMassLibrary.h"
#include "TerrainMassPolygonShader.h"

ATerrainMassPolygonBrush::ATerrainMassPolygonBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);
}

UTextureRenderTarget2D* ATerrainMassPolygonBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);
    if (!UTerrainMassLibrary::CreateOrUpdateRenderTarget2D(GetTransientPackage(), CanvasRT, RenderTargetSize, RTF_RGBA8, true))
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

    FTerrainMassPolygonShaderParameter ShaderParams;
    ShaderParams.SourceTexture = InCombinedResult;
    ShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);

    ENQUEUE_RENDER_COMMAND(TerranMassPolygonBrush)(
        [this, InCombinedResult, RenderTargetSize, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (InCombinedResult->GetRenderTargetResource() && InCombinedResult->GetRenderTargetResource()->GetRenderTargetTexture() &&
                CanvasRT->GetRenderTargetResource() && CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                FTerrainMassPolygonShader::Render(RHICmdList,
                    InCombinedResult->GetRenderTargetResource()->GetRenderTargetTexture(),
                    CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    RenderTargetSize, ShaderParams);
            }
        }
    );

    return CanvasRT;
}

void ATerrainMassPolygonBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
}

#if WITH_EDITOR
void ATerrainMassPolygonBrush::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif