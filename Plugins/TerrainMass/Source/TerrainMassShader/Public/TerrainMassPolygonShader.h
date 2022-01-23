#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "RHICommandList.h"
#include "RHIResources.h"

struct FTerrainMassPolygonShaderParameter
{
    UTextureRenderTarget2D* SourceTexture = nullptr;
    FVector2D InvTextureSize;
};

class TERRAINMASSSHADER_API FTerrainMassPolygonShader
{
public:
    static void Render(FRHICommandListImmediate& RHICmdList, FRHITexture* SourceTexture, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassPolygonShaderParameter& ShaderParams);
};
