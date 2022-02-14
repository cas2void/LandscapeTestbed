#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "RHICommandList.h"
#include "RHIResources.h"

struct FTerrainMassPolygonShaderParameter
{
    FVector2D RenderTargetSize;
    float Width;
    float SideFalloff;
    float EndFalloff;
    FVector StartPosition;
    FVector EndPosition;
    int32 NumSegments;
    UTexture2D* StartSidefFalloffTexture;
    UTexture2D* EndSideFalloffTexture;
    FTransform WorldToCanvasTransform;
};

class TERRAINMASSSHADER_API FTerrainMassPolygonShader
{
public:
    static void Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FTerrainMassPolygonShaderParameter& ShaderParams);
};
