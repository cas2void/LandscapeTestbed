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
	UPrimitiveComponent* GetRotateXComponent() { return RotateXComponent; }

	TArray<UPrimitiveComponent*> GetGizmoComponents();
	// 0: TL, 1: TR, 2: BR, 3: BL
	UPrimitiveComponent* GetPlanCornerComponent(int32 CornerIndex);
	// 0: X, 1: Y, 2: Z
	UPrimitiveComponent* GetRotationComponent(int32 AxisIndex);

	int32 GetPlanCornerDiagonalIndex(int32 CornerIndex) const;
	TArray<int32> GetPlanCornerNeighborIndices(int32 CornerIndex) const;

protected:
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

	UPROPERTY()
	UPrimitiveComponent* RotateXComponent;

	UPROPERTY()
	UPrimitiveComponent* RotateYComponent;

	UPROPERTY()
	UPrimitiveComponent* RotateZComponent;
};
