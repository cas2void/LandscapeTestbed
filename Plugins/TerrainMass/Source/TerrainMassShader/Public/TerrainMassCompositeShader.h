#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"

struct FTerrainMassCompositeShaderParameter
{
	FVector2D InvTextureSize;

#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
	FVector4 PosScaleBias;
	FVector4 UVScaleBias;
	FVector4 InvTargetSizeAndTextureSize;
#endif
};

class TERRAINMASSSHADER_API FTerrainMassCompositeShader
{
public:
	static void Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* CanvasRT, UTextureRenderTarget2D* BlendRT, UTextureRenderTarget2D* OutputRT, 
		const FIntPoint& Size, const FTerrainMassCompositeShaderParameter& ShaderParams);
};