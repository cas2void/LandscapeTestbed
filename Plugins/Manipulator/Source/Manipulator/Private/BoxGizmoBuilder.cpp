// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmoBuilder.h"

#include "InteractiveGizmoManager.h"

#include "BoxGizmo.h"

const FString UBoxGizmoBuilder::BuilderIdentifier = TEXT("BoxGizmoType");

UInteractiveGizmo* UBoxGizmoBuilder::BuildGizmo(const FToolBuilderState& SceneState) const
{
    UInteractiveGizmoManager* GizmoManager = SceneState.GizmoManager;
    UBoxGizmo* NewGizmo = NewObject<UBoxGizmo>(GizmoManager);
    if (NewGizmo)
    {
        NewGizmo->SetWorld(SceneState.World);
    }
    return NewGizmo;
}
