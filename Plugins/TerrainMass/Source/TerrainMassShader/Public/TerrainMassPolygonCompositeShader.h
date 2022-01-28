#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RHICommandList.h"
#include "RHIResources.h"

struct FTerrainMassPolygonCompositeShaderParameter
{
    FVector2D RenderTargetSize;
    UTextureRenderTarget2D* SourceTexture = nullptr;
    UTextureRenderTarget2D* CanvasTexture = nullptr;
};

class TERRAINMASSSHADER_API FTerrainMassPolygonCompositeShader
{
public:
    static void Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FTerrainMassPolygonCompositeShaderParameter& ShaderParams);
};
