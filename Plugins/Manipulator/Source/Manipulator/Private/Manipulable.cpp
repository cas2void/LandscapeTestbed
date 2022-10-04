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

void IManipulable::OnBoundsModified(const FBoxSphereBounds& InBounds, const FTransform& InBoundsTransform, const FBoxSphereBounds& AlignedBounds)
{
    AActor* Actor = Cast<AActor>(this);
    if (Actor)
    {
        const FVector BoundsWorldCenter = InBoundsTransform.TransformPosition(InBounds.Origin);
        Actor->SetActorLocation(BoundsWorldCenter);

        FVector FillScaleRatio3D = InBounds.BoxExtent / AlignedBounds.BoxExtent;
        float MinFillScaleChannel = FillScaleRatio3D.GetMin();
        FVector NewScale = Actor->GetActorScale() * MinFillScaleChannel;
        Actor->SetActorScale3D(NewScale);
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
