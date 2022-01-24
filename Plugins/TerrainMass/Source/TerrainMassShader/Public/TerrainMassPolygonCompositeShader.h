#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RHICommandList.h"
#include "RHIResources.h"

struct FTerrainMassPolygonCompositeShaderParameter
{
    UTextureRenderTarget2D* SourceTexture = nullptr;
    UTextureRenderTarget2D* CanvasTexture = nullptr;
    FVector2D InvTextureSize;
};

class TERRAINMASSSHADER_API FTerrainMassPolygonCompositeShader
{
public:
    static void Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassPolygonCompositeShaderParameter& ShaderParams);
};
