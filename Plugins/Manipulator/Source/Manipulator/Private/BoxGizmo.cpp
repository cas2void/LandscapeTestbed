// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmo.h"

#include "Components/SceneComponent.h"
#include "BaseGizmos/AxisSources.h"
#include "BaseGizmos/AxisPositionGizmo.h"
#include "BaseGizmos/PlanePositionGizmo.h"
#include "BaseGizmos/AxisAngleGizmo.h"
#include "BaseGizmos/ParameterToTransformAdapters.h"
#include "BaseGizmos/TransformSources.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "InteractiveGizmoManager.h"
#include "ToolDataVisualizer.h"
#include "FrameTypes.h"
#include "Drawing/MeshDebugDrawing.h"

#include "BoxGizmoActor.h"

void UBoxGizmo::Setup()
{
    Super::Setup();

    if (World)
    {
        ABoxGizmoActor* NewActor = World->SpawnActor<ABoxGizmoActor>(FVector::ZeroVector, FRotator::ZeroRotator);
        if (NewActor)
        {
            //NewActor->SetActorHiddenInGame(true);
            GizmoActor = NewActor;
        }
    }

    SetConstructionPlane(FVector(0.0f, 0.0f, 20.0f), FQuat::MakeFromEuler(FVector(0.0f, 30.0f, 0.0f)));
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

void UBoxGizmo::Render(IToolsContextRenderAPI* RenderAPI)
{
    if (ActiveTarget)
    {
        FToolDataVisualizer Draw;
        Draw.BeginFrame(RenderAPI);

        Draw.LineThickness = 3.0f;

        const FTransform Frame2World = GetConstructionFrame();
        Draw.SetTransform(Frame2World);
        Draw.DrawWireBox(Bounds.GetBox());

        Draw.EndFrame();
    }

    RenderConstructionPlane(RenderAPI);
}

void UBoxGizmo::SetConstructionPlane(const FVector& PlaneOrigin, const FQuat& PlaneOrientation)
{
    ConstructionPlaneOrigin = PlaneOrigin;
    ConstructionPlaneOrientation = PlaneOrientation;

    if (ActiveTarget)
    {
        // Init bounds by current active target and construction plane
        InitBounds();

        // Sub gizmos have been created already if ActiveTarget is valid, recreate them from the new construction plane
        DestroySubGizmos();
        CreateSubGizmos();
    }
}

void UBoxGizmo::RenderConstructionPlane(IToolsContextRenderAPI* RenderAPI) const
{
    FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();

    FViewCameraState RenderCameraState = RenderAPI->GetCameraState();
    float PDIScale = RenderCameraState.GetPDIScalingFactor();
    int NumGridLines = 21;
    float GridThickness = 0.5f * PDIScale;
    FColor GridColor(200, 200, 200);

    FFrame3f DrawFrame(ConstructionPlaneOrigin, ConstructionPlaneOrientation);
    MeshDebugDraw::DrawSimpleFixedScreenAreaGrid(RenderCameraState, DrawFrame, NumGridLines, 45.0, GridThickness, GridColor, true, PDI, FTransform::Identity);
}

FTransform UBoxGizmo::GetConstructionFrame() const
{
    const FFrame3f ConstructionFrame(ConstructionPlaneOrigin, ConstructionPlaneOrientation);
    FTransform Frame2World = ConstructionFrame.ToFTransform();
    return Frame2World;
}

FVector UBoxGizmo::TransformPositionWorldToConstructionFrame(const FVector& WorldPosition) const
{
    return GetConstructionFrame().InverseTransformPosition(WorldPosition);
}

FVector UBoxGizmo::TransformPositionConstructionFrameToWorld(const FVector& FramePosition) const
{
    return GetConstructionFrame().TransformPosition(FramePosition);
}

void UBoxGizmo::SetActiveTarget(const TScriptInterface<IManipulable>& Target, IToolContextTransactionProvider* TransactionProvider)
{
    if (ActiveTarget)
    {
        ClearActiveTarget();
    }
    ActiveTarget = Target;

    // Init bounds by current active target and construction plane
    InitBounds();

    // Sub gizmos have been destroyed in ClearActiveTarget(), just recreate them
    CreateSubGizmos();

    // Init TargetProxy rotation
    if (GizmoActor)
    {
        USceneComponent* TargetProxyComponent = GizmoActor->GetTargetProxyComponent();
        if (TargetProxyComponent)
        {
            if (ActiveTarget)
            {
                FManipulableTransform ManipulableTransform = ActiveTarget->GetTransform();
                if (ManipulableTransform.bValid)
                {
                    FQuat Rotation = ManipulableTransform.Transform.GetRotation();
                    TargetProxyComponent->SetWorldRotation(Rotation);
                }
            }
        }
    }
}

void UBoxGizmo::ClearActiveTarget()
{
    DestroySubGizmos();

    Bounds = FBoxSphereBounds(ForceInit);

    ActiveTarget = nullptr;
}

void UBoxGizmo::CreateSubGizmos()
{
    RegulateGizmoRootTransform();

    //
    // Bounds Group
    //
    RegulateBoundsGroupTransform();
    if (GizmoActor)
    {
        USceneComponent* BoundsGroupComponent = GizmoActor->GetBoundsGroupComponent();
        if (BoundsGroupComponent)
        {
            // Elevation and plan corner gizmos share the same axis source, which is the Z axis of BoundsGroupComponent
            UGizmoComponentAxisSource* BoundsAxisZSource = UGizmoComponentAxisSource::Construct(BoundsGroupComponent, 2, true, this);

            CreateElevationGizmo(BoundsAxisZSource);
            for (int32 Index = 0; Index < 4; Index++)
            {
                CreatePlanCornerGizmo(BoundsAxisZSource, Index);
            }
        }
    }

    //
    // Rotation Group
    //
    RegulateRotationGroupTransform();
    for (int32 Index = 0; Index < 3; Index++)
    {
        CreateRotationAxisGizmo(Index);
    }
}

void UBoxGizmo::DestroySubGizmos()
{
    for (UInteractiveGizmo* Gizmo : ActiveGizmos)
    {
        GetGizmoManager()->DestroyGizmo(Gizmo);
    }
    ActiveGizmos.Empty();
}

void UBoxGizmo::CreateElevationGizmo(UGizmoComponentAxisSource* AxisSource)
{
    UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
    if (ElevationComponent)
    {
        //
        // Move gizmo to target location, parent(bounds group) is the bounds bottom center
        //
        RegulateElevationTransform();

        //
        // Create axis-position gizmo, axis-position parameter will drive elevation position along elevation axis
        //
        UAxisPositionGizmo* ElevationGizmo = Cast<UAxisPositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisPositionBuilderIdentifier));
        check(ElevationGizmo);

        //
        // Axis source provides the translation axis for elevation
        //
        ElevationGizmo->AxisSource = AxisSource;

        //
        // Axis-translation parameter will drive elevation position along elevation axis
        //
        UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(ElevationComponent, this);
        // Parameter source maps axis-parameter-change to translation of TransformSource's transform
        UGizmoAxisTranslationParameterSource* ParamSource = UGizmoAxisTranslationParameterSource::Construct(AxisSource, ComponentTransformSource, this);
        ElevationGizmo->ParameterSource = ParamSource;

        //
        // Bind delegates
        //
        ComponentTransformSource->OnTransformChanged.AddLambda(
            [this](IGizmoTransformSource* TransformSource)
            {
                RecreateBoundsByElevation();
                SyncComponentsByElevation();
            }
        );

        ParamSource->PositionConstraintFunction = [this](const FVector& RawPosition, FVector& ConstrainedPosition)
        {
            return ConstrainElevationPosition(RawPosition, ConstrainedPosition);
        };

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

void UBoxGizmo::CreatePlanCornerGizmo(UGizmoComponentAxisSource* AxisSource, int32 CornerIndex)
{
    UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(CornerIndex);
    if (CornerComponent)
    {
        //
        // Move gizmo to target location, parent(bounds group) is the bounds bottom center
        //
        RegulatePlanCornerTransform(CornerIndex);

        //
        // Create plane-position gizmo
        //
        UPlanePositionGizmo* CornerGizmo = Cast<UPlanePositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultPlanePositionBuilderIdentifier));
        check(CornerGizmo);

        //
        // Axis source provides the translation plane for corner
        //
        CornerGizmo->AxisSource = AxisSource;

        //
        // Plane-translation parameter will drive corner gizmo position along corner plane
        //
        UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(CornerComponent, this);
        // Parameter source maps axis-parameter-change to translation of TransformSource's transform
        UGizmoPlaneTranslationParameterSource* ParamSource = UGizmoPlaneTranslationParameterSource::Construct(AxisSource, ComponentTransformSource, this);
        CornerGizmo->ParameterSource = ParamSource;

        //
        // Bind delegates
        //
        ComponentTransformSource->OnTransformChanged.AddLambda(
            [this, CornerIndex](IGizmoTransformSource* TransformSource)
            {
                RecreateBoundsByCorner(CornerIndex);
                SyncComponentsByCorner(CornerIndex);
            }
        );

        ParamSource->PositionConstraintFunction = [this, CornerIndex](const FVector& RawPosition, FVector& ConstrainedPosition)
        {
            return ConstrainCornerPosition(RawPosition, ConstrainedPosition, CornerIndex);
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

void UBoxGizmo::RegulateGizmoRootTransform()
{
    if (GizmoActor)
    {
        FTransform GizmoActorTransform;
        GizmoActorTransform.SetScale3D(FVector::OneVector);
        GizmoActorTransform.SetRotation(ConstructionPlaneOrientation);
        FVector BoundsWorldCenter = TransformPositionConstructionFrameToWorld(Bounds.Origin);
        GizmoActorTransform.SetLocation(BoundsWorldCenter);

        GizmoActor->SetActorTransform(GizmoActorTransform);
    }
}

void UBoxGizmo::RegulateBoundsGroupTransform()
{
    if (GizmoActor)
    {
        USceneComponent* BoundsGroupComponent = GizmoActor->GetBoundsGroupComponent();
        if (BoundsGroupComponent)
        {
            FVector BoundsFrameBottomCenter = Bounds.Origin + FVector(0.0f, 0.0f, -Bounds.BoxExtent.Z);
            FVector BoundsWorldBottomCenter = TransformPositionConstructionFrameToWorld(BoundsFrameBottomCenter);
            BoundsGroupComponent->SetWorldLocation(BoundsWorldBottomCenter);
        }
    }
}

void UBoxGizmo::RegulateElevationTransform()
{
    if (GizmoActor)
    {
        UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        if (ElevationComponent)
        {
            const FVector ElevationLocation(0.0f, 0.0f, Bounds.BoxExtent.Z * 2.0f);
            ElevationComponent->SetRelativeLocation(ElevationLocation);
        }
    }
}

void UBoxGizmo::RegulatePlanCornerTransform(int32 CornerIndex)
{
    UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(CornerIndex);
    if (CornerComponent)
    {
        const FVector CornerLocation = GetPlanCornerLocation(Bounds, CornerIndex);
        CornerComponent->SetRelativeLocation(CornerLocation);
    }
}

void UBoxGizmo::CreateRotationAxisGizmo(int32 AxisIndex)
{
    USceneComponent* RotationGroupComponent = GizmoActor->GetRotationGroupComponent();
    UPrimitiveComponent* RotationAxisComponent = GizmoActor->GetRotationAxisComponent(AxisIndex);
    if (RotationGroupComponent && RotationAxisComponent)
    {
        //
        // No Need to move gizmo, it always locates at the origin of parent's frame
        //

        //
        // Create axis-angle gizmo
        //
        UAxisAngleGizmo* RotationGizmo = Cast<UAxisAngleGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisAngleBuilderIdentifier));
        check(RotationGizmo);

        //
        // Axis source provides the rotation axis
        //
        UGizmoComponentAxisSource* RotationAxisSource = UGizmoComponentAxisSource::Construct(RotationGroupComponent, AxisIndex, true, this);
        RotationGizmo->AxisSource = RotationAxisSource;

        //
        // Plane-translation parameter will drive corner gizmo position along corner plane
        //
        UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(RotationGroupComponent, this);
        // Parameter source maps angle-parameter-change to rotation of TransformSource's transform
        UGizmoAxisRotationParameterSource* ParamSource = UGizmoAxisRotationParameterSource::Construct(RotationAxisSource, ComponentTransformSource, this);
        RotationGizmo->AngleSource = ParamSource;

        //
        // Bind delegates
        //
        ComponentTransformSource->OnTransformChanged.AddLambda(
            [this](IGizmoTransformSource* TransformSource)
            {
                NotifyRotationModified();
            }
        );

        //
        // Sub-component provides hit target
        //
        UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(RotationAxisComponent, this);
        HitTarget->UpdateHoverFunction = [RotationAxisComponent, this](bool bHovering)
        {
            if (Cast<UGizmoBaseComponent>(RotationAxisComponent) != nullptr)
            {
                Cast<UGizmoBaseComponent>(RotationAxisComponent)->UpdateHoverState(bHovering);
            }
        };
        RotationGizmo->HitTarget = HitTarget;

        //
        // Reference the created gizmo
        //
        ActiveGizmos.Add(RotationGizmo);
    }
}

void UBoxGizmo::RegulateRotationGroupTransform()
{
    if (GizmoActor)
    {
        USceneComponent* RotationGroupComponent = GizmoActor->GetRotationGroupComponent();
        if (RotationGroupComponent)
        {
            FVector BoundsFrameCenter = Bounds.Origin;
            FVector BoundsWorldCenter = TransformPositionConstructionFrameToWorld(BoundsFrameCenter);
            RotationGroupComponent->SetWorldLocation(BoundsWorldCenter);
        }
    }
}

void UBoxGizmo::InitBounds()
{
    if (ActiveTarget)
    {
        FManipulableBounds TargetBounds = ActiveTarget->GetBounds();
        FManipulableTransform TargetTransform = ActiveTarget->GetTransform();
        if (TargetBounds.bValid && TargetTransform.bValid)
        {
            const FTransform Local2World = TargetTransform.Transform;
            const FTransform Frame2World = GetConstructionFrame();
            const FTransform Local2Frame = Local2World * Frame2World.Inverse();

            const FBox LocalBoundingBox = TargetBounds.Bounds.GetBox();
            const TArray<FVector> BoxCornerMapping{ 
                FVector(-1, -1, 1), FVector(1, -1, 1), FVector(1, 1, 1), FVector(-1, 1, 1), 
                FVector(-1, -1, -1), FVector(1, -1, -1), FVector(1, 1, -1), FVector(-1, 1, -1) 
            };

            const FVector BoxCenter = LocalBoundingBox.GetCenter();
            const FVector BoxExtent = LocalBoundingBox.GetExtent();
            TArray<FVector> CornerPoints;
            for (const auto& Corner : BoxCornerMapping)
            {
                const FVector CornerLocalLocation = BoxCenter + Corner * BoxExtent;
                const FVector CornerFrameLocation = Local2Frame.TransformPosition(CornerLocalLocation);

                CornerPoints.Add(CornerFrameLocation);
            }

            Bounds = FBoxSphereBounds(CornerPoints.GetData(), CornerPoints.Num());
        }
    }
}

void UBoxGizmo::RecreateBoundsByElevation()
{
    if (GizmoActor)
    {
        TArray<UPrimitiveComponent*> GizmoComponents = GizmoActor->GetBoundsSubComponents();
        TArray<FVector> Locations;
        for (const auto& Component : GizmoComponents)
        {
            if (Component)
            {
                const FVector ComponentFrameLocation = TransformPositionWorldToConstructionFrame(Component->GetComponentLocation());
                Locations.Add(ComponentFrameLocation);
            }
        }
        Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());

        NotifyBoundsModified();
    }
}

void UBoxGizmo::RecreateBoundsByCorner(int32 CornerIndex)
{
    if (GizmoActor)
    {
        UPrimitiveComponent* SourceComponent = GizmoActor->GetPlanCornerComponent(CornerIndex);
        UPrimitiveComponent* DiagonalComponent = GizmoActor->GetPlanCornerComponent(GizmoActor->GetPlanCornerDiagonalIndex(CornerIndex));
        UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        if (SourceComponent && DiagonalComponent && ElevationComponent)
        {
            TArray<FVector> Locations;
            Locations.Add(TransformPositionWorldToConstructionFrame(SourceComponent->GetComponentLocation()));
            Locations.Add(TransformPositionWorldToConstructionFrame(DiagonalComponent->GetComponentLocation()));
            Locations.Add(TransformPositionWorldToConstructionFrame(ElevationComponent->GetComponentLocation()));

            Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());

            NotifyBoundsModified();
        }
    }
}

void UBoxGizmo::SyncComponentsByElevation()
{
    RegulateRotationGroupTransform();
}

void UBoxGizmo::SyncComponentsByCorner(int32 CornerIndex)
{
    // Parent(bounds group) has been transformed, all corners need update
    (void)CornerIndex;

    RegulateBoundsGroupTransform();

    for (int32 PlanCornerIndex = 0; PlanCornerIndex < 4; PlanCornerIndex++)
    {
        RegulatePlanCornerTransform(PlanCornerIndex);
    }

    // Sync elevation to make its projection locate at the center of plan
    RegulateElevationTransform();

    RegulateRotationGroupTransform();
}

void UBoxGizmo::NotifyBoundsModified()
{
    //if (ActiveTarget)
    //{
    //    ActiveTarget->OnBoundsModified(Bounds);
    //}
}

void UBoxGizmo::NotifyRotationModified()
{
    if (GizmoActor)
    {
        USceneComponent* TargetProxyComponent = GizmoActor->GetTargetProxyComponent();
        if (TargetProxyComponent)
        {
            const FQuat Rotation = TargetProxyComponent->GetComponentQuat();
            if (ActiveTarget)
            {
                ActiveTarget->OnRotationModified(Rotation);
            }
        }
    }
}

bool UBoxGizmo::ConstrainElevationPosition(const FVector& RawPosition, FVector& ConstrainedPosition)
{
    bool Result = false;
    return Result;
}

bool UBoxGizmo::ConstrainCornerPosition(const FVector& RawPosition, FVector& ConstrainedPosition, int32 CornerIndex) const
{
    bool Result = false;

    if (GizmoActor)
    {
        UPrimitiveComponent* DiagonalComponent = GizmoActor->GetPlanCornerComponent(GizmoActor->GetPlanCornerDiagonalIndex(CornerIndex));
        if (DiagonalComponent)
        {
            const FVector RawFramePosition = TransformPositionWorldToConstructionFrame(RawPosition);
            const FVector DiagonalFrameLocation = TransformPositionWorldToConstructionFrame(DiagonalComponent->GetComponentLocation());

            check(CornerIndex >= 0 && CornerIndex < 4);
            const bool bPositiveX = CornerIndex == 1 || CornerIndex == 2;
            const bool bPositiveY = CornerIndex == 2 || CornerIndex == 3;

            FVector ConstrainedFramePosition = RawFramePosition;
            if (bPositiveX)
            {
                if (RawFramePosition.X < DiagonalFrameLocation.X + PlanSizeMin)
                {
                    Result = true;
                    ConstrainedFramePosition.X = DiagonalFrameLocation.X + PlanSizeMin;
                }
            }
            else
            {
                if (RawFramePosition.X > DiagonalFrameLocation.X - PlanSizeMin)
                {
                    Result = true;
                    ConstrainedFramePosition.X = DiagonalFrameLocation.X - PlanSizeMin;
                }
            }

            if (bPositiveY)
            {
                if (RawFramePosition.Y < DiagonalFrameLocation.Y + PlanSizeMin)
                {
                    Result = true;
                    ConstrainedFramePosition.Y = DiagonalFrameLocation.Y + PlanSizeMin;
                }
            }
            else
            {
                if (RawFramePosition.Y > DiagonalFrameLocation.Y - PlanSizeMin)
                {
                    Result = true;
                    ConstrainedFramePosition.Y = DiagonalFrameLocation.Y - PlanSizeMin;
                }
            }

            if (Result)
            {
                ConstrainedPosition = TransformPositionConstructionFrameToWorld(ConstrainedFramePosition);
            }
        }
    }

    return Result;
}

FVector UBoxGizmo::GetPlanCornerLocation(const FBoxSphereBounds& InBounds, int32 CornerIndex) const
{
    const bool bPositiveX = CornerIndex == 1 || CornerIndex == 2;
    const bool bPositiveY = CornerIndex == 2 || CornerIndex == 3;
    const float SignX = bPositiveX ? 1.0f : -1.0f;
    const float SignY = bPositiveY ? 1.0f : -1.0f;
    FVector CornerLocation = FVector(InBounds.BoxExtent.X * SignX, InBounds.BoxExtent.Y * SignY, 0.0f);

    return CornerLocation;
}
