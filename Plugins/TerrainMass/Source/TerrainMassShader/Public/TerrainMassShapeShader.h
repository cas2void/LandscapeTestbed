#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2DDynamic.h"

//
// Vertex Layout
//
struct FTerrainMassShapeVertex
{
	FVector Position;
	//FVector4 ExtraData;

	FTerrainMassShapeVertex(FVector InPosition)
		: Position(InPosition)
	{}
};

struct FTerrainMassShapeShaderParameter
{
	FVector2D InvTextureSize;
	FMatrix World2UV;
};

class TERRAINMASSSHADER_API FTerrainMassShapeShader
{
public:
	static void Render(UTextureRenderTarget2D* OutputRT, const TArray<FTerrainMassShapeVertex>& ShapeVertices, const TArray<uint16>& ShapeIndices, const FTerrainMassShapeShaderParameter& ShaderParams);
};