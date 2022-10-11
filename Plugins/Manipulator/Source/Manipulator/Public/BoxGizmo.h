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

	void SetActiveGizmoPrimitiveComponent();
	void ClearActiveGizmoPrimitiveComponent();

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

	//
	// Sub Gizmos
	//
protected:
	// Currently active sub gizmos and their visual counterparts
	UPROPERTY()
	TMap<class UAxisPositionGizmo*, class UPrimitiveComponent*> ActiveBoundsElevationGizmos;

	UPROPERTY()
	TMap<class UPlanePositionGizmo*, class UPrimitiveComponent*> ActiveBoundsPlanCornerGizmos;

	UPROPERTY()
	TMap<class UAxisAngleGizmo*, class UPrimitiveComponent*> ActiveRotateAxisGizmos;

	UPROPERTY()
	TMap<class UAxisPositionGizmo*, class UPrimitiveComponent*> ActiveTranslateZGizmos;

	UPROPERTY()
	TMap<class UPlanePositionGizmo*, class UPrimitiveComponent*> ActiveTranslateXYGizmos;

	void CreateSubGizmos();
	void DestroySubGizmos();

	//
	// Creation
	//
	void CreateElevationGizmo(class UGizmoComponentAxisSource* AxisSource);
	void CreatePlanCornerGizmo(class UGizmoComponentAxisSource* AxisSource, int32 CornerIndex);
	void CreateRotateAxisGizmo(int32 AxisIndex);
	void CreateTranslateZGizmo(class UGizmoComponentAxisSource* AxisSource);
	void CreateTranslateXYGizmo(class UGizmoComponentAxisSource* AxisSource);
	void InitTransformProxy();

	//
	// One sub gizmo's editing need to be synced to others
	//
	// Slave components: rotation group, translation group
	void SyncComponentsByElevation();

	// Slave components: bounds group and all subs, rotation group, translation group
	void SyncComponentsByCorner(int32 CornerIndex);

	// Slave components: bounds group and all subs, translation group
	void SyncComponentsByRotation(int32 AxisIndex);

	// Slave Components: bounds group and all subs, rotation group, translation group
	void SyncComponentsByTranslation();

	void RegulateGizmoRootTransform();
	void RegulateBoundsGroupTransform();
	void RegulateElevationTransform();
	void RegulatePlanCornerTransform(int32 CornerIndex);
	void RegulateBoundsAndSubTransform();
	void RegulateRotationGroupTransform();
	void RegulateRotateAxisTransform(int32 AxisIndex);
	void RegulateRotationAndSubTransform();
	void RegulateTranslationGroupTransform();

	//
	// Events
	//
	void NotifyBoundsModified();
	void NotifyRotationModified();
	void NotifyTranslationModified();

	//
	// Visibility
	//
	bool IsInteracting() const;

	void SetBoundsGizmoVisibility(bool bVisible);
	void SetRotationGizmoVisibility(bool bVisible);
	void SetTranslationGizmoVisibility(bool bVisible);

	//
	// Editing Constraint
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
