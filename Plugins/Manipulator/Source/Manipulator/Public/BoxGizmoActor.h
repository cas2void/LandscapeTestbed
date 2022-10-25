// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmos/GizmoActor.h"
#include "Components/SceneComponent.h"
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
	USceneComponent* GetRotationProxyComponent() { return RotationProxyComponent; }

	// AxisIndex - 0: X, 1: Y, 2: Z
	// FaceIndex - 0: Front, 1: Back
	USceneComponent* GetRotateAxisSocketComponent(int32 AxisIndex, int32 FaceIndex);
	UPrimitiveComponent* GetRotateAxisIndicatorComponent(int32 AxisIndex, int32 FaceIndex);

protected:
	UPROPERTY()
	USceneComponent* RotationGroupComponent;

	UPROPERTY()
	USceneComponent* RotateXFrontSocketComponent;

	UPROPERTY()
	UPrimitiveComponent* RotateXFrontIndicatorComponent;

	UPROPERTY()
	USceneComponent* RotationProxyComponent;

	//
	// Translation Group
	//
public:
	USceneComponent* GetTranslationGroupComponent() { return TranslationGroupComponent; }
	UPrimitiveComponent* GetTranslateZComponent() { return TranslateZComponent; }
	USceneComponent* GetTranslationProxyComponent() { return TranslationProxyComponent; }

	UPrimitiveComponent* GetTranslateXYComponent();
	void SetTranslateXYComponent(UPrimitiveComponent* InComponent);

protected:
	UPROPERTY()
	USceneComponent* TranslationGroupComponent;

	UPROPERTY()
	UPrimitiveComponent* TranslateZComponent;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> TranslateXYComponent;

	UPROPERTY()
	USceneComponent* TranslationProxyComponent;
};
