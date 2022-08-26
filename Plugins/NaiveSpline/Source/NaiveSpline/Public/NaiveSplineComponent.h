// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "NaiveSplineComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class NAIVESPLINE_API UNaiveSplineComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNaiveSplineComponent();

	//
	// UPrimitiveComponent Interfaces
	//
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	//
	// USceneComponent Interfaces
	//
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};
