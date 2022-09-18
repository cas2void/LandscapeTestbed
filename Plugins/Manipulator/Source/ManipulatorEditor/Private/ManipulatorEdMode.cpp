// Fill out your copyright notice in the Description page of Project Settings.


#include "ManipulatorEdMode.h"

#include "Tools/Modes.h"
#include "EditorModeManager.h"
#include "Engine/Selection.h"
#include "InteractiveGizmo.h"
#include "EdModeInteractiveToolsContext.h"
#include "BaseGizmos/TransformGizmo.h"

#include "Manipulable.h"

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
    FEditorModeTools* ModeManager = GetModeManager();
    if (ModeManager)
    {
        bool bCreateGizmo = true;
        USelection* Selection = ModeManager->GetSelectedActors();
        if (Selection)
        {
            TArray<AActor*> SelectedActors;
            Selection->GetSelectedObjects<AActor>(SelectedActors);

            for (const auto& Actor : SelectedActors)
            {
                if (!Actor->Implements<UManipulable>())
                {
                    bCreateGizmo = false;
                    break;
                }
            }
        }

        if (bCreateGizmo)
        {
            ModeManager->SetShowWidget(false);
            RecreateGizmo();
        }
        else
        {
            DestroyGizmo();
            ModeManager->SetShowWidget(true);
        }
    }
}

void UManipulatorEdMode::Enter()
{
    if (!GetModeManager() || !GetModeManager()->HasToolkitHost())
    {
        return;
    }

    Super::Enter();

    WidgetModeChangedHandle =
        GetModeManager()->OnWidgetModeChanged().AddLambda([this](FWidget::EWidgetMode) { RecreateGizmo(); });
}

void UManipulatorEdMode::Exit()
{
    if (!ToolsContext)
    {
        return;
    }

    GetModeManager()->OnWidgetModeChanged().Remove(WidgetModeChangedHandle);
    WidgetModeChangedHandle.Reset();

    Super::Exit();
}

void UManipulatorEdMode::RecreateGizmo()
{
    DestroyGizmo();

    ETransformGizmoSubElements Elements = ETransformGizmoSubElements::None;
    bool bUseContextCoordinateSystem = true;
    FWidget::EWidgetMode WidgetMode = GetModeManager()->GetWidgetMode();
    switch (WidgetMode)
    {
    case FWidget::EWidgetMode::WM_Translate:
        Elements = ETransformGizmoSubElements::TranslateAllAxes | ETransformGizmoSubElements::TranslateAllPlanes;
        break;
    case FWidget::EWidgetMode::WM_Rotate:
        Elements = ETransformGizmoSubElements::RotateAllAxes;
        break;
    case FWidget::EWidgetMode::WM_Scale:
        Elements = ETransformGizmoSubElements::ScaleAllAxes | ETransformGizmoSubElements::ScaleAllPlanes;
        bUseContextCoordinateSystem = false;
        break;
    case FWidget::EWidgetMode::WM_2D:
        Elements = ETransformGizmoSubElements::RotateAxisY | ETransformGizmoSubElements::TranslatePlaneXZ;
        break;
    default:
        Elements = ETransformGizmoSubElements::FullTranslateRotateScale;
        break;
    }
    UTransformGizmo* TransformGizmo = ToolsContext->GizmoManager->CreateCustomTransformGizmo(Elements);
    TransformGizmo->bUseContextCoordinateSystem = bUseContextCoordinateSystem;

    TArray<AActor*> SelectedActors;
    GetModeManager()->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);

    UTransformProxy* TransformProxy = NewObject<UTransformProxy>();
    for (auto Actor : SelectedActors)
    {
        USceneComponent* SceneComponent = Actor->GetRootComponent();
        TransformProxy->AddComponent(SceneComponent);
    }
    TransformGizmo->SetActiveTarget(TransformProxy);
    TransformGizmo->SetVisibility(SelectedActors.Num() > 0);

    Gizmo = TransformGizmo;
}

void UManipulatorEdMode::DestroyGizmo()
{
    check(ToolsContext);
    check(ToolsContext->GizmoManager);
    if (Gizmo)
    {
        ToolsContext->GizmoManager->DestroyGizmo(Gizmo);
    }
}

#undef LOCTEXT_NAMESPACE