#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"

struct FTerrainMassShapeShaderParameter
{
	TWeakObjectPtr<UTexture2DDynamic> SideFalloffTexture;
	FVector2D InvTextureSize;
};

class TERRAINMASSSHADER_API FTerrainMassShapeShader
{
public:
	static void Render(UTextureRenderTarget2D* OutputRT, const TArray<FVector>& ShapePoints, const FTerrainMassShapeShaderParameter& ShaderParams);
};