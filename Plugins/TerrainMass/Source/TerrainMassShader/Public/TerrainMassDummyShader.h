#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "RHICommandList.h"
#include "RHIResources.h"

struct FTerrainMassDummyShaderParameter
{
	UTexture2D* SideFalloffTexture = nullptr;
	FVector2D InvTextureSize;
	FVector Center;
	float Radius;
};

class TERRAINMASSSHADER_API FTerrainMassDummyShader
{
public:
	static void Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassDummyShaderParameter& ShaderParams);
};