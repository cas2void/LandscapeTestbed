// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveGizmo.h"
#include "InteractiveToolChange.h"
#include "Engine/World.h"
#include "Manipulable.h"
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
	FVector TransformPositionWorldToConstructionFrame(const FVector& WorldPosition) const;
	FVector TransformPositionConstructionFrameToWorld(const FVector& FramePosition) const;

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

	// Bounds Group
	void CreateElevationGizmo(class UGizmoComponentAxisSource* AxisSource);
	void CreatePlanCornerGizmo(class UGizmoComponentAxisSource* AxisSource, int32 CornerIndex);

	void RegulateGizmoRootTransform();
	void RegulateBoundsGroupTransform();
	void RegulateElevationTransform();
	void RegulatePlanCornerTransform(int32 CornerIndex);

	// Rotation Group
	void CreateRotationAxisGizmo(int32 AxisIndex);

	void RegulateRotationGroupTransform();

	//
	// Bounds
	//
public:
	FBoxSphereBounds GetBounds() const { return Bounds; }

protected:
	// Bounds in construction frame
	UPROPERTY(Transient, NonTransactional)
	FBoxSphereBounds Bounds;

	// Initialize bounds from target and construction frame
	void InitBounds();

	// Recreate bounds from elevation and all plan corners
	void RecreateBoundsByElevation();

	// Recreate bounds from elvation, the specified corner and its diagonal
	void RecreateBoundsByCorner(int32 CornerIndex);

	// Slave components: rotation group
	void SyncComponentsByElevation();

	// Slave components: all plan corners, elevation, rotation group
	void SyncComponentsByCorner(int32 CornerIndex);

	// Slave components: bounds group
	void SyncComponentsByRotation();

	void NotifyBoundsModified();
	void NotifyRotationModified();

	//
	// Bounds Constraint
	//
public:
	void SetPlanSizeMin(float InPlanSizeMin) { PlanSizeMin = InPlanSizeMin; }

protected:
	bool ConstrainElevationPosition(const FVector& RawPosition, FVector& ConstrainedPosition);
	bool ConstrainCornerPosition(const FVector& RawPosition, FVector& ConstrainedPosition, int32 CornerIndex) const;

	UPROPERTY()
	float PlanSizeMin = 20.0f;

	//
	// Helper Functions
	//
protected:
	FVector GetPlanCornerLocation(const FBoxSphereBounds& InBounds, int32 CornerIndex) const;
	FBoxSphereBounds ConvertOBBToAABB(const FBoxSphereBounds& InBounds, const FTransform& InBoundsTransform, const FTransform& FrameTransform);

	//
	// Debug
	//
	UPROPERTY(Transient, NonTransactional)
	bool bDebug;

	UPROPERTY(Transient, NonTransactional)
	FBoxSphereBounds DebugAlignedBounds;
};
