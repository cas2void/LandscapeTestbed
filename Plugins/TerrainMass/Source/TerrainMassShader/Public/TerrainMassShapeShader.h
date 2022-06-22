#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"
#include "TerrainMassShapeVertex.h"

struct FTerrainMassShapeShaderParameter
{
	FMatrix Local2UV;
};

class TERRAINMASSSHADER_API FTerrainMassShapeShader
{
public:
	static void Render(UTextureRenderTarget2D* OutputRT, const TArray<FTerrainMassShapeVertex>& ShapeVertices, const TArray<uint16>& ShapeIndices, const FTerrainMassShapeShaderParameter& ShaderParams);
};