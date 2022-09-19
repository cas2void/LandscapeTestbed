// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmo.h"

#include "BoxGizmoActor.h"

void UBoxGizmo::Setup()
{
    Super::Setup();

	if (World)
	{
		ABoxGizmoActor* NewActor = World->SpawnActor<ABoxGizmoActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (NewActor)
		{
			GizmoActor = NewActor;
		}
	}
}

void UBoxGizmo::Shutdown()
{
	if (GizmoActor)
	{
		GizmoActor->Destroy();
		GizmoActor = nullptr;
	}

    Super::Shutdown();
}
