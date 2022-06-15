#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"

struct FTerrainMassGaussianBlurShaderParameter
{
	FVector2D InvTextureSize;
	int KernelSize;
};

class TERRAINMASSSHADER_API FTerrainMassGaussianBlurShader
{
public:
	static void Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* IntermediateRT, 
		const FTerrainMassGaussianBlurShaderParameter& ShaderParams);
};