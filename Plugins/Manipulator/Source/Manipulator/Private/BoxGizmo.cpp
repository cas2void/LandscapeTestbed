// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmo.h"

#include "Components/SceneComponent.h"
#include "BaseGizmos/AxisSources.h"
#include "BaseGizmos/AxisPositionGizmo.h"
#include "BaseGizmos/PlanePositionGizmo.h"
#include "BaseGizmos/ParameterToTransformAdapters.h"
#include "BaseGizmos/TransformSources.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "InteractiveGizmoManager.h"

#include "BoxGizmoActor.h"

void UBoxGizmo::Setup()
{
    Super::Setup();

    if (World)
    {
        ABoxGizmoActor* NewActor = World->SpawnActor<ABoxGizmoActor>(FVector::ZeroVector, FRotator::ZeroRotator);
        if (NewActor)
        {
            GizmoActor = NewActor;
        }
    }
}

void UBoxGizmo::Shutdown()
{
    ClearActiveTarget();

    if (GizmoActor)
    {
        GizmoActor->Destroy();
        GizmoActor = nullptr;
    }

    Super::Shutdown();
}

void UBoxGizmo::SetActiveTarget(const TScriptInterface<IManipulable>& Target, IToolContextTransactionProvider* TransactionProvider)
{
    if (ActiveTarget)
    {
        ClearActiveTarget();
    }
    ActiveTarget = Target;

    // Reset gizmo location
    USceneComponent* GizmoRootComponent = GizmoActor->GetRootComponent();
    GizmoRootComponent->SetWorldLocation(FVector::ZeroVector);

    CreateSubGizmos();
}

void UBoxGizmo::ClearActiveTarget()
{
    for (UInteractiveGizmo* Gizmo : ActiveGizmos)
    {
        GetGizmoManager()->DestroyGizmo(Gizmo);
    }
    ActiveGizmos.Empty();
    ActiveComponents.Empty();

    ActiveTarget = nullptr;
}

void UBoxGizmo::CreateSubGizmos()
{
    if (ActiveTarget)
    {
        FManipulableBounds TargetBounds = ActiveTarget->GetBounds();
        if (TargetBounds.bValid)
        {
            CreateElevationGizmo(TargetBounds.Bounds);
            // Top Left
            CreateCornerGizmo(TargetBounds.Bounds, false, false);
            // Top Right
            CreateCornerGizmo(TargetBounds.Bounds, true, false);
            // Bottom Right
            CreateCornerGizmo(TargetBounds.Bounds, true, true);
            // Bottom Left
            CreateCornerGizmo(TargetBounds.Bounds, false, true);
        }
    }
}

void UBoxGizmo::CreateElevationGizmo(const FBoxSphereBounds& Bounds)
{
    UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
    if (ElevationComponent)
    {
        //
        // Move gizmo to target location
        //
        FVector ElevationLocation = Bounds.Origin + FVector(0.0f, 0.0f, Bounds.BoxExtent.Z);
        ElevationComponent->SetWorldLocation(ElevationLocation);

        //
        // Create axis-position gizmo, axis-position parameter will drive elevation position along elevation axis
        //
        UAxisPositionGizmo* ElevationGizmo = Cast<UAxisPositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisPositionBuilderIdentifier));
        check(ElevationGizmo);

        //
        // Axis source provides the translation axis for elevation
        //
        UGizmoWorldAxisSource* ElevationAxisSource = UGizmoWorldAxisSource::Construct(ElevationLocation, 2, this);
        ElevationGizmo->AxisSource = ElevationAxisSource;

        //
        // Axis-translation parameter will drive elevation position along elevation axis
        //
        UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(ElevationComponent, this);
        // Parameter source maps axis-parameter-change to translation of TransformSource's transform
        UGizmoAxisTranslationParameterSource* ParamSource = UGizmoAxisTranslationParameterSource::Construct(ElevationAxisSource, ComponentTransformSource, this);
        //ParamSource->PositionConstraintFunction = [this](const FVector& Pos, FVector& Snapped) { return PositionSnapFunction(Pos, Snapped); };
        ElevationGizmo->ParameterSource = ParamSource;

        //
        // Sub-component provides hit target
        //
        UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(ElevationComponent, this);
        HitTarget->UpdateHoverFunction = [ElevationComponent, this](bool bHovering)
        {
            if (Cast<UGizmoBaseComponent>(ElevationComponent) != nullptr)
            {
                Cast<UGizmoBaseComponent>(ElevationComponent)->UpdateHoverState(bHovering);
            }
        };
        ElevationGizmo->HitTarget = HitTarget;

        //
        // Reference the created gizmo and its counter part of primitive component
        //
        ActiveGizmos.Add(ElevationGizmo);
        ActiveComponents.Add(ElevationComponent);
    }
}

void UBoxGizmo::CreateCornerGizmo(const FBoxSphereBounds& Bounds, bool bPositiveX, bool bPositiveY)
{
    UPrimitiveComponent* CornerComponent = nullptr;
    if (bPositiveX)
    {
        if (bPositiveY)
        {
            CornerComponent = GizmoActor->GetPlaneBottomRightComponent();
        }
        else
        {
            CornerComponent = GizmoActor->GetPlaneTopRightComponent();
        }
    }
    else
    {
        if (bPositiveY)
        {
            CornerComponent = GizmoActor->GetPlaneBottomLeftComponent();
        }
        else
        {
            CornerComponent = GizmoActor->GetPlaneTopLeftComponent();
        }
    }


    if (CornerComponent)
    {
        //
        // Move gizmo to target location
        //
        const float SignX = bPositiveX ? 1.0f : -1.0f;
        const float SignY = bPositiveY ? 1.0f : -1.0f;
        FVector CornerLocation = Bounds.Origin + FVector(Bounds.BoxExtent.X * SignX, Bounds.BoxExtent.Y * SignY, -Bounds.BoxExtent.Z);
        CornerComponent->SetWorldLocation(CornerLocation);

        //
        // Create plane-position gizmo
        //
        UPlanePositionGizmo* CornerGizmo = Cast<UPlanePositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultPlanePositionBuilderIdentifier));
        check(CornerGizmo);

        //
        // Axis source provides the translation plane for corner
        //
        UGizmoWorldAxisSource* CornerAxisSource = UGizmoWorldAxisSource::Construct(CornerLocation, 2, this);
        CornerGizmo->AxisSource = CornerAxisSource;

        //
        // Plane-translation parameter will drive corner gizmo position along corner plane
        //
        UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(CornerComponent, this);
        // Parameter source maps axis-parameter-change to translation of TransformSource's transform
        UGizmoPlaneTranslationParameterSource* ParamSource = UGizmoPlaneTranslationParameterSource::Construct(CornerAxisSource, ComponentTransformSource, this);
        CornerGizmo->ParameterSource = ParamSource;

        //
        // Sub-component provides hit target
        //
        UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(CornerComponent, this);
        HitTarget->UpdateHoverFunction = [CornerComponent, this](bool bHovering)
        {
            if (Cast<UGizmoBaseComponent>(CornerComponent) != nullptr)
            {
                Cast<UGizmoBaseComponent>(CornerComponent)->UpdateHoverState(bHovering);
            }
        };
        CornerGizmo->HitTarget = HitTarget;

        //
        // Reference the created gizmo and its counter part of primitive component
        //
        ActiveGizmos.Add(CornerGizmo);
        ActiveComponents.Add(CornerComponent);
    }
}
