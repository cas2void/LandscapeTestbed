// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "ManipulatorEdMode.generated.h"

UCLASS()
class MANIPULATOREDITOR_API UManipulatorEdModeSettings : public UObject
{
	GENERATED_BODY()
};

/**
 * 
 */
UCLASS()
class MANIPULATOREDITOR_API UManipulatorEdMode : public UEdMode
{
	GENERATED_BODY()
	
public:
	UManipulatorEdMode();

	//
	// UEdMode Interfaces
	//
public:
	virtual void Initialize() override;

	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override;

	virtual void ActorSelectionChangeNotify() override;

	virtual void Enter() override;

	virtual void Exit() override;

	//
	// Gizmo
	//
protected:
	UPROPERTY(VisibleAnywhere, Transient, NonTransactional)
	class UInteractiveGizmo* CustomGizmo;

	void SwitchGizmo();

	void RecreateCustomGizmo(const TArray<TScriptInterface<class IManipulable>>& Targets);
	void DestroyCustomGizmo();

	FDelegateHandle WidgetModeChangedHandle;
};
