// Fill out your copyright notice in the Description page of Project Settings.


#include "Manipulable.h"

#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/MeshComponent.h"

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

UPrimitiveComponent* IManipulable::GetPrimitiveComponent()
{
    UPrimitiveComponent* Result = nullptr;

    const AActor* Actor = Cast<const AActor>(this);
    if (Actor)
    {
        Result = Cast<UPrimitiveComponent>(Actor->FindComponentByClass<UMeshComponent>());
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

        TArray<EAxis::Type> AlignedAxis;
        for (int32 Index = 1; Index < 4; Index++)
        {
            EAxis::Type AxisType = static_cast<EAxis::Type>(Index);
            const FVector ActorAxis = Actor->GetActorTransform().GetUnitAxis(AxisType);
            const FVector BoundsAxis = InBoundsTransform.GetUnitAxis(AxisType);
            const bool AxisAligned = FMath::Abs(FVector::DotProduct(ActorAxis, BoundsAxis)) > 0.999f;
            if (AxisAligned)
            {
                AlignedAxis.Add(AxisType);
            }
        }

        FVector FillScaleRatio3D = InBounds.BoxExtent / AlignedBounds.BoxExtent;
        if (AlignedAxis.Num() < 1)
        {
            float MinFillScaleChannel3D = FillScaleRatio3D.GetMin();
            FillScaleRatio3D = FVector(MinFillScaleChannel3D);
        }
        else if (AlignedAxis.Num() == 1)
        {
            switch (AlignedAxis[0])
            {
            case EAxis::X:
            {
                float MinFillScaleChanel2D = FMath::Min<float>(FillScaleRatio3D.Y, FillScaleRatio3D.Z);
                FillScaleRatio3D.Y = MinFillScaleChanel2D;
                FillScaleRatio3D.Z = MinFillScaleChanel2D;
            }
                break;
            case EAxis::Y:
            {
                float MinFillScaleChanel2D = FMath::Min<float>(FillScaleRatio3D.Z, FillScaleRatio3D.X);
                FillScaleRatio3D.Z = MinFillScaleChanel2D;
                FillScaleRatio3D.X = MinFillScaleChanel2D;
            }
                break;
            case EAxis::Z:
            {
                float MinFillScaleChanel2D = FMath::Min<float>(FillScaleRatio3D.X, FillScaleRatio3D.Y);
                FillScaleRatio3D.X = MinFillScaleChanel2D;
                FillScaleRatio3D.Y = MinFillScaleChanel2D;
            }
                break;
            default:
                break;
            }
        }
        FVector NewScale = Actor->GetActorScale() * FillScaleRatio3D;
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

void IManipulable::OnLocationModified(const FVector& InLocation)
{
    AActor* Actor = Cast<AActor>(this);
    if (Actor)
    {
        Actor->SetActorLocation(InLocation);
    }
}
