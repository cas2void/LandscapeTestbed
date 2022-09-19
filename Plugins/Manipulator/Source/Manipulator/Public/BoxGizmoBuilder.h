// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveGizmoBuilder.h"
#include "BoxGizmoBuilder.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UBoxGizmoBuilder : public UInteractiveGizmoBuilder
{
	GENERATED_BODY()
	
	//
	// UInteractiveGizmoBuilder Interfaces
	//
public:
	virtual UInteractiveGizmo* BuildGizmo(const FToolBuilderState& SceneState) const override;


	static const FString BuilderIdentifier;
};
