#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"

struct FTerrainMassShapeCompositionShaderParameter
{
	TWeakObjectPtr<UTexture2DDynamic> SideFalloffTexture;
	FVector2D InvTextureSize;
	float Elevation;
};

class TERRAINMASSSHADER_API FTerrainMassShapeCompositionShader
{
public:
	static void Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* BlendRT, UTextureRenderTarget2D* OutputRT, const FTerrainMassShapeCompositionShaderParameter& ShaderParams);
};