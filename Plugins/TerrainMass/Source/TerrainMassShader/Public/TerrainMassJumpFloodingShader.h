#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"

struct FTerrainMassJumpFloodingShaderParameter
{
	FVector2D InvTextureSize;
};

struct FTerrainMassDistanceFieldShaderParameter
{
	FVector2D InvTextureSize;
	FVector2D Width;
};

class TERRAINMASSSHADER_API FTerrainMassJumpFloodingShader
{
public:
	static void Encode(UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* InputRT);

	static void Flood(UTextureRenderTarget2D* OutputRTs[], int32& OutputIndex, int32 InputIndex, int32 NumIteration, const FTerrainMassJumpFloodingShaderParameter& ShaderParams);

	static void Flood(UTextureRenderTarget2D* OutputRTs[], int32& OutputIndex, int32 InputIndex, const FTerrainMassJumpFloodingShaderParameter& ShaderParams);

	static void DistanceField(UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* InputRT, const FTerrainMassDistanceFieldShaderParameter& ShaderParams);
};