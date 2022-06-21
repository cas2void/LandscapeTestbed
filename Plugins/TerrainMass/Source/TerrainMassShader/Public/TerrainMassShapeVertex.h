// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerrainMassShapeVertex.generated.h"

/**
 * 
 */
USTRUCT()
struct TERRAINMASSSHADER_API FTerrainMassShapeVertex
{
	GENERATED_BODY()

public:
	FVector Position;
	//FVector4 ExtraData;

	FTerrainMassShapeVertex()
		: Position(FVector::ZeroVector)
	{}

	FTerrainMassShapeVertex(FVector InPosition)
		: Position(InPosition)
	{}
};
