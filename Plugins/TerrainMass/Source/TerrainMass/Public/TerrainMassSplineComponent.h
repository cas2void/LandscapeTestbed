// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "TerrainMassSplineComponent.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMASS_API UTerrainMassSplineComponent : public USplineComponent
{
	GENERATED_BODY()

	//
	// USplineComponent Interfaces
	//
public:
	virtual void UpdateSpline() override;

	DECLARE_MULTICAST_DELEGATE(FTerrainMassSplineUpdatedDelegate);
	FTerrainMassSplineUpdatedDelegate& OnSplineUpdated() { return TerrainMassSplineUpdatedDelegate; }
	const FTerrainMassSplineUpdatedDelegate& OnSplineUpdated() const { return TerrainMassSplineUpdatedDelegate; }

protected:
	FTerrainMassSplineUpdatedDelegate TerrainMassSplineUpdatedDelegate;
};
