// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmos/GizmoActor.h"
#include "Components/PrimitiveComponent.h"
#include "BoxGizmoActor.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API ABoxGizmoActor : public AGizmoActor
{
	GENERATED_BODY()
	
public:
	ABoxGizmoActor();

public:
	UPrimitiveComponent* GetElevationComponent() { return ElevationComponent; }
	UPrimitiveComponent* GetPlaneTopLeftComponent() { return PlaneTopLeftComponent; }
	UPrimitiveComponent* GetPlaneTopRightComponent() { return PlaneTopRightComponent; }
	UPrimitiveComponent* GetPlaneBottomRightComponent() { return PlaneBottomRightComponent; }
	UPrimitiveComponent* GetPlaneBottomLeftComponent() { return PlaneBottomLeftComponent; }

protected:
	/** X Axis Translation Component */
	UPROPERTY()
	UPrimitiveComponent* ElevationComponent;

	UPROPERTY()
	UPrimitiveComponent* PlaneTopLeftComponent;

	UPROPERTY()
	UPrimitiveComponent* PlaneTopRightComponent;

	UPROPERTY()
	UPrimitiveComponent* PlaneBottomRightComponent;

	UPROPERTY()
	UPrimitiveComponent* PlaneBottomLeftComponent;
};
