// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "SceneManagement.h"
#include "SceneView.h"
#include "DynamicMeshBuilder.h"
#include "TerrainMassRingComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TERRAINMASS_API UTerrainMassRingComponent : public USplineComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTerrainMassRingComponent();

#if !UE_BUILD_SHIPPING
	//
	// UPrimitiveComponent Interfaces
	//
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	//
	// USceneComponent Interfaces
	//
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
#endif

	//
	// Handle
	//
public:
	// Color to draw arrow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	FColor HandleColor;

	// Relative size to scale drawn arrow by
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	float HandleSize;

	// Total length of drawn arrow including head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	float HandleLength;

	void CreateHandleGeometry(TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices) const;

protected:
};
