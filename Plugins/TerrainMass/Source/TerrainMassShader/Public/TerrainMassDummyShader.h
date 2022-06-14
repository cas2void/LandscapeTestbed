#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"

struct FTerrainMassDummyShaderParameter
{
	TWeakObjectPtr<UTexture2DDynamic> SideFalloffTexture;
	FVector2D InvTextureSize;
	FVector Center;
	float Radius;

#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
	FVector4 PosScaleBias;
	FVector4 UVScaleBias;
	FVector4 InvTargetSizeAndTextureSize;
#endif
};

class TERRAINMASSSHADER_API FTerrainMassDummyShader
{
public:
	static void Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* OutputRT, const FTerrainMassDummyShaderParameter& ShaderParams);
};