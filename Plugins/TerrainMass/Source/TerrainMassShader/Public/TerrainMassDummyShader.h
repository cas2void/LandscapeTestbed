#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "RHICommandList.h"
#include "RHIResources.h"

struct FTerrainMassDummyShaderParameter
{
	UTextureRenderTarget2D* SourceTexture = nullptr;
	UTexture2D* SideFalloffTexture = nullptr;
	FVector2D InvTextureSize;
	FVector Center;
	float Radius;
};

class TERRAINMASSSHADER_API FTerrainMassDummyShader
{
public:
	static void Render(FRHICommandListImmediate& RHICmdList, FRHITexture* SourceTexture, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassDummyShaderParameter& ShaderParams);
};