// Fill out your copyright notice in the Description page of Project Settings.


#include "Manipulable.h"

#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"

// Add default functionality here for any IManipulable functions that are not pure virtual.

FManipulableTransform IManipulable::GetTransform() const
{
    FManipulableTransform Result;

    const AActor* Actor = Cast<const AActor>(this);
    if (Actor)
    {
        USceneComponent* RootComp = Actor->GetRootComponent();
        if (RootComp)
        {
            Result.bValid = true;
            Result.Transform = RootComp->GetComponentTransform();
        }
    }

    return Result;
}

FManipulableBounds IManipulable::GetBounds() const
{
    FManipulableBounds Result;

    const AActor* Actor = Cast<const AActor>(this);
    if (Actor)
    {
        FBox BoundingBox = Actor->GetComponentsBoundingBox();
        Result.bValid = true;
        Result.Bounds = FBoxSphereBounds(BoundingBox);
    }

    return Result;
}
