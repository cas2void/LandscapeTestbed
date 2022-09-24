// Fill out your copyright notice in the Description page of Project Settings.


#include "ManipulatorEdMode.h"

#include "Tools/Modes.h"
#include "EditorModeManager.h"
#include "Engine/Selection.h"
#include "InteractiveGizmo.h"
#include "EdModeInteractiveToolsContext.h"
#include "BaseGizmos/TransformGizmo.h"

#include "Manipulable.h"
#include "BoxGizmoBuilder.h"
#include "BoxGizmo.h"

#define LOCTEXT_NAMESPACE "UManipulatorEdMode"

UManipulatorEdMode::UManipulatorEdMode()
{
	Info = FEditorModeInfo(
		FName(TEXT("ManipulatorMode")),
		LOCTEXT("ModeName", "Manipulator"),
		FSlateIcon(),
		false,
		600
	);

	SettingsClass = UManipulatorEdModeSettings::StaticClass();
}

void UManipulatorEdMode::Initialize()
{
}

bool UManipulatorEdMode::IsCompatibleWith(FEditorModeID OtherModeID) const
{
	return true;
}

void UManipulatorEdMode::ActorSelectionChangeNotify()
{
	SwitchGizmo();
}

void UManipulatorEdMode::Enter()
{
	if (!GetModeManager() || !GetModeManager()->HasToolkitHost())
	{
		return;
	}

	Super::Enter();

	GetModeManager()->SetShowWidget(true);

	ToolsContext->GizmoManager->RegisterGizmoType(UBoxGizmoBuilder::BuilderIdentifier, NewObject<UBoxGizmoBuilder>());

	WidgetModeChangedHandle =
		GetModeManager()->OnWidgetModeChanged().AddLambda([this](FWidget::EWidgetMode) { SwitchGizmo(); });
}

void UManipulatorEdMode::Exit()
{
	if (!ToolsContext || !GetModeManager())
	{
		return;
	}

	GetModeManager()->OnWidgetModeChanged().Remove(WidgetModeChangedHandle);
	WidgetModeChangedHandle.Reset();

	ToolsContext->GizmoManager->DeregisterGizmoType(UBoxGizmoBuilder::BuilderIdentifier);

	Super::Exit();
}

void UManipulatorEdMode::SwitchGizmo()
{
	TArray<TScriptInterface<IManipulable>> Manipulables;

	USelection* Selection = GetModeManager()->GetSelectedActors();
	if (Selection)
	{
		TArray<AActor*> SelectedActors;
		Selection->GetSelectedObjects<AActor>(SelectedActors);

		for (const auto& Actor : SelectedActors)
		{
			if (Actor->Implements<UManipulable>())
			{
				Manipulables.Add(Actor);
			}
		}
	}

	if (Manipulables.Num() > 0)
	{
		RecreateCustomGizmo(Manipulables);
	}
	else
	{
		DestroyCustomGizmo();
	}
}

void UManipulatorEdMode::RecreateCustomGizmo(const TArray<TScriptInterface<IManipulable>>& Targets)
{
	check(Targets.Num() > 0);

	DestroyCustomGizmo();

	UBoxGizmo* BoxGizmo = Cast<UBoxGizmo>(ToolsContext->GizmoManager->CreateGizmo(UBoxGizmoBuilder::BuilderIdentifier));
	BoxGizmo->SetActiveTarget(Targets[0]);
	CustomGizmo = BoxGizmo;
}

void UManipulatorEdMode::DestroyCustomGizmo()
{
	if (CustomGizmo)
	{
		ToolsContext->GizmoManager->DestroyGizmo(CustomGizmo);
	}
}

#undef LOCTEXT_NAMESPACE