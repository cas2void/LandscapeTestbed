// Fill out your copyright notice in the Description page of Project Settings.

#include "ManipulatorEditorModule.h"

#include "Misc/CoreDelegates.h"
#include "LevelEditor.h"
#include "Editor.h"
#include "EditorModeManager.h"

#include "ManipulatorEdMode.h"

#define LOCTEXT_NAMESPACE "FManipulatorEditorModule"

void FManipulatorEditorModule::StartupModule()
{
	auto ActivateManipulatorMode = []()
	{
		if (!GLevelEditorModeTools().IsModeActive(GetDefault<UManipulatorEdMode>()->GetID()))
		{
			GLevelEditorModeTools().ActivateMode(GetDefault<UManipulatorEdMode>()->GetID());
		}
	};

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FCoreDelegates::OnPostEngineInit.AddLambda(
		[ActivateManipulatorMode]()
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.OnLevelEditorCreated().AddLambda(
				[ActivateManipulatorMode](TSharedPtr<ILevelEditor> LevelEditor)
				{
					ActivateManipulatorMode();
				}
			);
		}
	);

	FEditorDelegates::OnMapOpened.AddLambda(
		[ActivateManipulatorMode](const FString& Filename, bool bAsTemplate)
		{
			ActivateManipulatorMode();
		}
	);
}

void FManipulatorEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FManipulatorEditorModule, ManipulatorEditor)