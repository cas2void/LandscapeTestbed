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

    if (ActiveTarget)
    {
        FManipulableBounds TargetBounds = ActiveTarget->GetBounds();
        if (TargetBounds.bValid)
        {
            // Init bounds by input bounds from IManipulable
            Bounds = TargetBounds.Bounds;

            CreateSubGizmos(TargetBounds.Bounds);
        }
    }
}

void UBoxGizmo::ClearActiveTarget()
{
    DestroySubGizmos();

    Bounds = FBoxSphereBounds(ForceInit);

    ActiveTarget = nullptr;
}

void UBoxGizmo::CreateSubGizmos(const FBoxSphereBounds& InputBounds)
{
    CreateElevationGizmo(InputBounds);
    // Top Left
    CreateCornerGizmo(InputBounds, false, false);
    // Top Right
    CreateCornerGizmo(InputBounds, true, false);
    // Bottom Right
    CreateCornerGizmo(InputBounds, true, true);
    // Bottom Left
    CreateCornerGizmo(InputBounds, false, true);
}

void UBoxGizmo::DestroySubGizmos()
{
    for (UInteractiveGizmo* Gizmo : ActiveGizmos)
    {
        GetGizmoManager()->DestroyGizmo(Gizmo);
    }
    ActiveGizmos.Empty();
}

void UBoxGizmo::CreateElevationGizmo(const FBoxSphereBounds& InputBounds)
{
    UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
    if (ElevationComponent)
    {
        //
        // Move gizmo to target location
        //
        FVector ElevationLocation = InputBounds.Origin + FVector(0.0f, 0.0f, InputBounds.BoxExtent.Z);
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
        // Bind delegates
        //
        ComponentTransformSource->OnTransformChanged.AddLambda(
            [this](IGizmoTransformSource* TransformSource)
            {
                RecreateBounds();
            }
        );

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
        // Reference the created gizmo
        //
        ActiveGizmos.Add(ElevationGizmo);
    }
}

void UBoxGizmo::CreateCornerGizmo(const FBoxSphereBounds& InputBounds, bool bPositiveX, bool bPositiveY)
{
    UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(bPositiveX, bPositiveY);
    if (CornerComponent)
    {
        //
        // Move gizmo to target location
        //
        FVector CornerLocation = GetPlanCornerLocation(InputBounds, bPositiveX, bPositiveY);
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
        // Bind delegates
        //
        ComponentTransformSource->OnTransformChanged.AddLambda(
            [this, bPositiveX, bPositiveY](IGizmoTransformSource* TransformSource)
            {
                RecreateBoundsFromCorner(bPositiveX, bPositiveY);
                SyncComponentsFromCorner(bPositiveX, bPositiveY);
            }
        );

        ParamSource->PositionConstraintFunction = [this, bPositiveX, bPositiveY](const FVector& RawPosition, FVector& ConstrainedPosition)
        {
            return ConstrainCornerPosition(RawPosition, ConstrainedPosition, bPositiveX, bPositiveY);
        };

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
        // Reference the created gizmo
        //
        ActiveGizmos.Add(CornerGizmo);
    }
}

void UBoxGizmo::RecreateBounds()
{
    if (GizmoActor)
    {
        TArray<UPrimitiveComponent*> GizmoComponents = GizmoActor->GetGizmoComponents();
        TArray<FVector> Locations;
        for (const auto& Component : GizmoComponents)
        {
            if (Component)
            {
                Locations.Add(Component->GetComponentLocation());
            }
        }
        Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());
    }
}

void UBoxGizmo::RecreateBoundsFromCorner(bool bPositiveX, bool bPositiveY)
{
    if (GizmoActor)
    {
        UPrimitiveComponent* SourceComponent = GizmoActor->GetPlanCornerComponent(bPositiveX, bPositiveY);
        UPrimitiveComponent* DiagonalComponent = GizmoActor->GetPlanCornerComponent(!bPositiveX, !bPositiveY);
        UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        if (SourceComponent && DiagonalComponent && ElevationComponent)
        {
            TArray<FVector> Locations;
            Locations.Add(SourceComponent->GetComponentLocation());
            Locations.Add(DiagonalComponent->GetComponentLocation());
            Locations.Add(ElevationComponent->GetComponentLocation());

            Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());
        }
    }
}

void UBoxGizmo::SyncComponentsFromCorner(bool bPositiveX, bool bPositiveY)
{
    if (GizmoActor)
    {
        // Sync corner neighbors on plan
        TArray<bool> NeighborPositiveX;
        TArray<bool> NeighborPositiveY;
        TArray<UPrimitiveComponent*> NeighborComponents;

        NeighborPositiveX.Add(bPositiveX);
        NeighborPositiveY.Add(!bPositiveY);
        NeighborComponents.Add(GizmoActor->GetPlanCornerComponent(NeighborPositiveX[0], NeighborPositiveY[0]));
        NeighborPositiveX.Add(!bPositiveX);
        NeighborPositiveY.Add(bPositiveY);
        NeighborComponents.Add(GizmoActor->GetPlanCornerComponent(NeighborPositiveX[1], NeighborPositiveY[1]));

        for (int32 Index = 0; Index < NeighborComponents.Num(); Index++)
        {
            UPrimitiveComponent* Neighbor = NeighborComponents[Index];
            if (Neighbor)
            {
                FVector Location = GetPlanCornerLocation(Bounds, NeighborPositiveX[Index], NeighborPositiveY[Index]);
                Neighbor->SetWorldLocation(Location);
            }
        }

        // Sync elevation to make its projection locate at the center of plan
        UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        if (ElevationComponent)
        {
            FVector ElevationLocation(Bounds.Origin.X, Bounds.Origin.Y, ElevationComponent->GetComponentLocation().Z);
            ElevationComponent->SetWorldLocation(ElevationLocation);
        }
    }
}

bool UBoxGizmo::ConstrainCornerPosition(const FVector& RawPosition, FVector& ConstrainedPosition, bool bPositiveX, bool bPositiveY) const
{
    bool Result = false;

    if (GizmoActor)
    {
        UPrimitiveComponent* DiagonalComponent = GizmoActor->GetPlanCornerComponent(!bPositiveX, !bPositiveY);
        if (DiagonalComponent)
        {
            FVector DiagonalLocation = DiagonalComponent->GetComponentLocation();

            ConstrainedPosition = RawPosition;
            if (bPositiveX)
            {
                if (RawPosition.X < DiagonalLocation.X + PlanSizeMin)
                {
                    Result = true;
                    ConstrainedPosition.X = DiagonalLocation.X + PlanSizeMin;
                }
            }
            else
            {
                if (RawPosition.X > DiagonalLocation.X - PlanSizeMin)
                {
                    Result = true;
                    ConstrainedPosition.X = DiagonalLocation.X - PlanSizeMin;
                }
            }

            if (bPositiveY)
            {
                if (RawPosition.Y < DiagonalLocation.Y + PlanSizeMin)
                {
                    Result = true;
                    ConstrainedPosition.Y = DiagonalLocation.Y + PlanSizeMin;
                }
            }
            else
            {
                if (RawPosition.Y > DiagonalLocation.Y - PlanSizeMin)
                {
                    Result = true;
                    ConstrainedPosition.Y = DiagonalLocation.Y - PlanSizeMin;
                }
            }
        }
    }

    return Result;
}

FVector UBoxGizmo::GetPlanCornerLocation(const FBoxSphereBounds& InBounds, bool bPositiveX, bool bPositiveY) const
{
    const float SignX = bPositiveX ? 1.0f : -1.0f;
    const float SignY = bPositiveY ? 1.0f : -1.0f;
    FVector CornerLocation = InBounds.Origin + FVector(InBounds.BoxExtent.X * SignX, InBounds.BoxExtent.Y * SignY, -InBounds.BoxExtent.Z);

    return CornerLocation;
}
