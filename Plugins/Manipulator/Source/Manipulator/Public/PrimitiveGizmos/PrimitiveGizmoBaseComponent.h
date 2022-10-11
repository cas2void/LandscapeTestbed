// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "PrimitiveGizmoBaseComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UPrimitiveGizmoBaseComponent : public UGizmoBaseComponent
{
	GENERATED_BODY()

public:
	void UpdateInteractionState(bool bValue)
	{
		if (bValue != bInteracting)
		{
			bInteracting = bValue;
		}
	}

protected:
	// Interaction state (dragging)
	bool bInteracting = false;
};
