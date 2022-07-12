// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "TerrainMassRingComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TERRAINMASS_API UTerrainMassRingComponent : public USplineComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTerrainMassRingComponent();

	//
	// Handle
	//
protected:
	UPROPERTY(EditAnywhere)
	FVector HandleLocation;
};
