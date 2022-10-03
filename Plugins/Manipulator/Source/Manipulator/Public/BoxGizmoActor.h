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

	//
	// Bounds Group
	//
public:
	USceneComponent* GetBoundsGroupComponent() { return BoundsGroupComponent; }
	UPrimitiveComponent* GetElevationComponent() { return ElevationComponent; }
	UPrimitiveComponent* GetPlanTopLeftComponent() { return PlanTopLeftComponent; }
	UPrimitiveComponent* GetPlanTopRightComponent() { return PlanTopRightComponent; }
	UPrimitiveComponent* GetPlanBottomRightComponent() { return PlanBottomRightComponent; }
	UPrimitiveComponent* GetPlanBottomLeftComponent() { return PlanBottomLeftComponent; }

	TArray<UPrimitiveComponent*> GetBoundsSubComponents();

	// 0: TL, 1: TR, 2: BR, 3: BL
	UPrimitiveComponent* GetPlanCornerComponent(int32 CornerIndex);
	int32 GetPlanCornerDiagonalIndex(int32 CornerIndex) const;
	TArray<int32> GetPlanCornerNeighborIndices(int32 CornerIndex) const;

protected:
	UPROPERTY()
	USceneComponent* BoundsGroupComponent;

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

	//
	// Rotation Group
	//
public:
	USceneComponent* GetRotationGroupComponent() { return RotationGroupComponent; }
	UPrimitiveComponent* GetRotateXComponent() { return RotateXComponent; }

	// 0: X, 1: Y, 2: Z
	UPrimitiveComponent* GetRotationAxisComponent(int32 AxisIndex);

protected:
	UPROPERTY()
	USceneComponent* RotationGroupComponent;

	UPROPERTY()
	UPrimitiveComponent* RotateXComponent;

	UPROPERTY()
	UPrimitiveComponent* RotateYComponent;

	UPROPERTY()
	UPrimitiveComponent* RotateZComponent;

	UPROPERTY()
	USceneComponent* TargetProxy;
};
