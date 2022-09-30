// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveGizmo.h"
#include "InteractiveToolChange.h"
#include "Engine/World.h"
#include "Manipulable.h"
#include "Components/PrimitiveComponent.h"
#include "BoxGizmo.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UBoxGizmo : public UInteractiveGizmo
{
	GENERATED_BODY()
	
	//
	// UInteractiveGizmo Interfaces
	//
public:
	virtual void Setup() override;
	virtual void Shutdown() override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;

	//
	// Gizmo Actor
	//
public:
	void SetWorld(UWorld* InWorld) { World = InWorld; }

protected:
	// Current active GizmoActor that was spawned by this Gizmo. Will be destroyed when Gizmo is.
	UPROPERTY(Transient, NonTransactional)
	class ABoxGizmoActor* GizmoActor;

	// GizmoActors will be spawned in this World
	UPROPERTY(Transient, NonTransactional)
	UWorld* World;

	//
	// Construction Plane
	//
public:
	void SetConstructionPlane(const FVector& PlaneOrigin, const FQuat& PlaneOrientation);

protected:
	UPROPERTY()
	FVector ConstructionPlaneOrigin = FVector::ZeroVector;
	
	UPROPERTY()
	FQuat ConstructionPlaneOrientation = FQuat::Identity;

	void RenderConstructionPlane(IToolsContextRenderAPI* RenderAPI) const;
	FTransform GetConstructionFrame() const;
	FVector TransformWorldPositionToConstructionFrame(const FVector& WorldPosition) const;
	FVector TransformConstructionFramePositionToWorld(const FVector& FramePosition) const;

	//
	// Manipulated Target
	//
public:
	void SetActiveTarget(const TScriptInterface<IManipulable>& Target, IToolContextTransactionProvider* TransactionProvider = nullptr);
	void ClearActiveTarget();

protected:
	UPROPERTY()
	TScriptInterface<IManipulable> ActiveTarget;

	//
	// Sub Gizmos
	//
protected:
	// Currently active sub gizmos
	UPROPERTY()
	TArray<UInteractiveGizmo*> ActiveGizmos;

	void CreateSubGizmos();
	void DestroySubGizmos();
	void CreateElevationGizmo(class UGizmoConstantAxisSource* AxisSource);
	void CreateCornerGizmo(class UGizmoConstantAxisSource* AxisSource, bool bPositiveX, bool bPositiveY);

	//
	// Bounds
	//
public:
	FBoxSphereBounds GetBounds() const { return Bounds; }

protected:
	UPROPERTY(Transient, NonTransactional)
	FBoxSphereBounds Bounds;

	void InitBounds();

	// Recreate bounds from elevation and all plan corners
	void RecreateBoundsByElevation();

	// Recreate bounds from elvation, the specified corner and its diagonal
	void RecreateBoundsByCorner(bool bPositiveX, bool bPositiveY);

	// Sync other sub gizmo components to maintain rectangle shape
	void SyncComponentsByCorner(bool bPositiveX, bool bPositiveY);

	void NotifyBoundsModified();

	//
	// Bounds Constraint
	//
public:
	void SetPlanSizeMin(float InPlanSizeMin) { PlanSizeMin = InPlanSizeMin; }

protected:
	bool ConstrainCornerPosition(const FVector& RawPosition, FVector& ConstrainedPosition, bool bPositiveX, bool bPositiveY) const;

	UPROPERTY()
	float PlanSizeMin = 20.0f;

	//
	// Helper Functions
	//
protected:
	FVector GetPlanCornerLocation(const FBoxSphereBounds& InBounds, bool bPositiveX, bool bPositiveY) const;
};
