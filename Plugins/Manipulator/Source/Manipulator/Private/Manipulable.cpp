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
        Result.bValid = true;
        Result.Transform = Actor->GetActorTransform();
    }

    return Result;
}

FManipulableBounds IManipulable::GetBounds() const
{
    FManipulableBounds Result;

    const AActor* Actor = Cast<const AActor>(this);
    if (Actor)
    {
        FBox BoundingBox = Actor->CalculateComponentsBoundingBoxInLocalSpace();
        Result.bValid = true;
        Result.Bounds = FBoxSphereBounds(BoundingBox);
    }

    return Result;
}

void IManipulable::OnBoundsModified(const FBoxSphereBounds& InBounds, const FTransform& InBoundsTransform)
{
    AActor* Actor = Cast<AActor>(this);
    if (Actor)
    {
        const FVector BoundsWorldCenter = InBoundsTransform.TransformPosition(InBounds.Origin);
        Actor->SetActorLocation(BoundsWorldCenter);

        //FBox BoundingBox = Actor->GetComponentsBoundingBox();
        //FBoxSphereBounds CurrentBounds(BoundingBox);
        //FVector BoxScale = InBounds.BoxExtent / CurrentBounds.BoxExtent;
        //FVector NewScale = Actor->GetActorScale() * BoxScale;
        //Actor->SetActorScale3D(NewScale);
    }
}

void IManipulable::OnRotationModified(const FQuat& InRotation)
{
    AActor* Actor = Cast<AActor>(this);
    if (Actor)
    {
        Actor->SetActorRotation(InRotation);
    }
}
