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
	// list of currently-active child gizmos
	UPROPERTY()
	TArray<UInteractiveGizmo*> ActiveGizmos;

	void CreateSubGizmos(const FBoxSphereBounds& InputBounds);
	void DestroySubGizmos();
	void CreateElevationGizmo(const FBoxSphereBounds& InputBounds);
	void CreateCornerGizmo(const FBoxSphereBounds& InputBounds, bool bPositiveX, bool bPositiveY);

	//
	// Bounds
	//
public:
	FBoxSphereBounds GetBounds() const { return Bounds; }

protected:
	UPROPERTY(Transient, NonTransactional)
	FBoxSphereBounds Bounds;

	void RecreateBounds();
	void RecreateBoundsFromCorner(bool bPositiveX, bool bPositiveY);
	void SyncComponentsFromCorner(bool bPositiveX, bool bPositiveY);

	//
	// Helper Functions
	//
protected:
	FVector GetPlanCornerLocation(const FBoxSphereBounds& InBounds, bool bPositiveX, bool bPositiveY) const;
};
