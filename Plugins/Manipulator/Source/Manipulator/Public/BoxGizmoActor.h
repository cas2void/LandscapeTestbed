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
	UPrimitiveComponent* GetPlanTopLeftComponent() { return PlanTopLeftComponent; }
	UPrimitiveComponent* GetPlanTopRightComponent() { return PlanTopRightComponent; }
	UPrimitiveComponent* GetPlanBottomRightComponent() { return PlanBottomRightComponent; }
	UPrimitiveComponent* GetPlanBottomLeftComponent() { return PlanBottomLeftComponent; }

	TArray<UPrimitiveComponent*> GetGizmoComponents();
	UPrimitiveComponent* GetPlanCornerComponent(bool bPositiveX, bool bPositiveY);

protected:
	/** X Axis Translation Component */
	UPROPERTY()
	UPrimitiveComponent* ElevationComponent;

	UPROPERTY()
	UPrimitiveComponent* PlanTopLeftComponent;

	UPROPERTY()
	UPrimitiveComponent* PlanTopRightComponent;

	UPROPERTY()
	UPrimitiveComponent* PlanBottomRightComponent;

	UPROPERTY()
	UPrimitiveComponent* PlanBottomLeftComponent;
};
