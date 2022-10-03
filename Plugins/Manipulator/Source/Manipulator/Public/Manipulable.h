// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Manipulable.generated.h"

USTRUCT()
struct FManipulableTransform
{
	GENERATED_BODY()

	UPROPERTY()
	bool bValid = false;

	UPROPERTY()
	FTransform Transform;
};

USTRUCT()
struct FManipulableBounds
{
	GENERATED_BODY()

	UPROPERTY()
	bool bValid = false;

	UPROPERTY()
	FBoxSphereBounds Bounds;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UManipulable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MANIPULATOR_API IManipulable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FManipulableTransform GetTransform() const;
	virtual FManipulableBounds GetBounds() const;

	virtual void OnBoundsModified(const FBoxSphereBounds& InBounds, const FTransform& InBoundsTransform);
	virtual void OnRotationModified(const FQuat& InRotation);
};
