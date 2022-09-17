// Fill out your copyright notice in the Description page of Project Settings.


#include "ManipulatorEdMode.h"

#include "Tools/Modes.h"
#include "EditorModeManager.h"

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
    UE_LOG(LogTemp, Warning, TEXT("UManipulatorEdMode::ActorSelectionChangeNotify"));
}

void UManipulatorEdMode::Enter()
{
    if (!GetModeManager() || !GetModeManager()->HasToolkitHost())
    {
        return;
    }
    Super::Enter();
}

void UManipulatorEdMode::Exit()
{
    if (!ToolsContext)
    {
        return;
    }
    Super::Exit();
}

#undef LOCTEXT_NAMESPACE