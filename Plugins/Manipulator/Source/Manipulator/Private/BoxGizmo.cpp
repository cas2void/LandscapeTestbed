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

FVector UBoxGizmo::TransformWorldPositionToConstructionFrame(const FVector& WorldPosition) const
{
    return GetConstructionFrame().InverseTransformPosition(WorldPosition);
}

FVector UBoxGizmo::TransformConstructionFramePositionToWorld(const FVector& FramePosition) const
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
}

void UBoxGizmo::ClearActiveTarget()
{
    DestroySubGizmos();

    Bounds = FBoxSphereBounds(ForceInit);

    ActiveTarget = nullptr;
}

void UBoxGizmo::CreateSubGizmos()
{
    // Reset gizmo root location
    if (GizmoActor)
    {
        GizmoActor->SetActorLocation(ConstructionPlaneOrigin);
        GizmoActor->SetActorRotation(ConstructionPlaneOrientation);
    }

    UGizmoConstantAxisSource* ConstructionAxisSource = NewObject<UGizmoConstantAxisSource>(this);
    if (ConstructionAxisSource)
    {
        ConstructionAxisSource->Origin = ConstructionPlaneOrigin;
        ConstructionAxisSource->Direction = ConstructionPlaneOrientation.GetUpVector();
    }

    CreateElevationGizmo(ConstructionAxisSource);
    // Top Left
    CreateCornerGizmo(ConstructionAxisSource, false, false);
    // Top Right
    CreateCornerGizmo(ConstructionAxisSource, true, false);
    // Bottom Right
    CreateCornerGizmo(ConstructionAxisSource, true, true);
    // Bottom Left
    CreateCornerGizmo(ConstructionAxisSource, false, true);
}

void UBoxGizmo::DestroySubGizmos()
{
    for (UInteractiveGizmo* Gizmo : ActiveGizmos)
    {
        GetGizmoManager()->DestroyGizmo(Gizmo);
    }
    ActiveGizmos.Empty();
}

void UBoxGizmo::CreateElevationGizmo(UGizmoConstantAxisSource* AxisSource)
{
    UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
    if (ElevationComponent)
    {
        //
        // Move gizmo to target location
        //
        FVector ElevationLocation = Bounds.Origin + FVector(0.0f, 0.0f, Bounds.BoxExtent.Z);
        //ElevationComponent->SetWorldLocation(ElevationLocation);
        ElevationComponent->SetRelativeLocation(ElevationLocation);

        //
        // Create axis-position gizmo, axis-position parameter will drive elevation position along elevation axis
        //
        UAxisPositionGizmo* ElevationGizmo = Cast<UAxisPositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisPositionBuilderIdentifier));
        check(ElevationGizmo);

        //
        // Axis source provides the translation axis for elevation
        //
        //UGizmoWorldAxisSource* ElevationAxisSource = UGizmoWorldAxisSource::Construct(ElevationLocation, 2, this);
        UGizmoConstantAxisSource* ElevationAxisSource = AxisSource;
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
                RecreateBoundsByElevation();
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

void UBoxGizmo::CreateCornerGizmo(UGizmoConstantAxisSource* AxisSource, bool bPositiveX, bool bPositiveY)
{
    UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(bPositiveX, bPositiveY);
    if (CornerComponent)
    {
        //
        // Move gizmo to target location
        //
        const FVector CornerLocation = GetPlanCornerLocation(Bounds, bPositiveX, bPositiveY);
        //CornerComponent->SetWorldLocation(CornerLocation);
        CornerComponent->SetRelativeLocation(CornerLocation);

        //
        // Create plane-position gizmo
        //
        UPlanePositionGizmo* CornerGizmo = Cast<UPlanePositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultPlanePositionBuilderIdentifier));
        check(CornerGizmo);

        //
        // Axis source provides the translation plane for corner
        //
        //UGizmoWorldAxisSource* CornerAxisSource = UGizmoWorldAxisSource::Construct(CornerLocation, 2, this);
        UGizmoConstantAxisSource* CornerAxisSource = AxisSource;
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
                RecreateBoundsByCorner(bPositiveX, bPositiveY);
                SyncComponentsByCorner(bPositiveX, bPositiveY);
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

void UBoxGizmo::InitBounds()
{
    if (ActiveTarget)
    {
        FManipulableBounds TargetBounds = ActiveTarget->GetBounds();
        FManipulableTransform TargetTransform = ActiveTarget->GetTransform();
        if (TargetBounds.bValid && TargetTransform.bValid)
        {
            //Bounds = TargetBounds.Bounds;

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
        TArray<UPrimitiveComponent*> GizmoComponents = GizmoActor->GetGizmoComponents();
        TArray<FVector> Locations;
        for (const auto& Component : GizmoComponents)
        {
            if (Component)
            {
                const FVector ComponentFrameLocation = TransformWorldPositionToConstructionFrame(Component->GetComponentLocation());
                Locations.Add(ComponentFrameLocation);
            }
        }
        Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());

        NotifyBoundsModified();
    }
}

void UBoxGizmo::RecreateBoundsByCorner(bool bPositiveX, bool bPositiveY)
{
    if (GizmoActor)
    {
        UPrimitiveComponent* SourceComponent = GizmoActor->GetPlanCornerComponent(bPositiveX, bPositiveY);
        UPrimitiveComponent* DiagonalComponent = GizmoActor->GetPlanCornerComponent(!bPositiveX, !bPositiveY);
        UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        if (SourceComponent && DiagonalComponent && ElevationComponent)
        {
            TArray<FVector> Locations;
            Locations.Add(TransformWorldPositionToConstructionFrame(SourceComponent->GetComponentLocation()));
            Locations.Add(TransformWorldPositionToConstructionFrame(DiagonalComponent->GetComponentLocation()));
            Locations.Add(TransformWorldPositionToConstructionFrame(ElevationComponent->GetComponentLocation()));

            Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());

            NotifyBoundsModified();
        }
    }
}

void UBoxGizmo::SyncComponentsByCorner(bool bPositiveX, bool bPositiveY)
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
                const FVector CornerLocation = GetPlanCornerLocation(Bounds, NeighborPositiveX[Index], NeighborPositiveY[Index]);
                Neighbor->SetRelativeLocation(CornerLocation);
            }
        }

        // Sync elevation to make its projection locate at the center of plan
        UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        if (ElevationComponent)
        {
            const FVector ElevationFrameLocation = TransformWorldPositionToConstructionFrame(ElevationComponent->GetComponentLocation());
            const FVector NewElevationLocation(Bounds.Origin.X, Bounds.Origin.Y, ElevationFrameLocation.Z);
            ElevationComponent->SetRelativeLocation(NewElevationLocation);
        }
    }
}

void UBoxGizmo::NotifyBoundsModified()
{
    //if (ActiveTarget)
    //{
    //    ActiveTarget->OnBoundsModified(Bounds);
    //}
}

bool UBoxGizmo::ConstrainCornerPosition(const FVector& RawPosition, FVector& ConstrainedPosition, bool bPositiveX, bool bPositiveY) const
{
    bool Result = false;

    if (GizmoActor)
    {
        UPrimitiveComponent* DiagonalComponent = GizmoActor->GetPlanCornerComponent(!bPositiveX, !bPositiveY);
        if (DiagonalComponent)
        {
            const FVector RawFramePosition = TransformWorldPositionToConstructionFrame(RawPosition);
            const FVector DiagonalFrameLocation = TransformWorldPositionToConstructionFrame(DiagonalComponent->GetComponentLocation());

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
                ConstrainedPosition = TransformConstructionFramePositionToWorld(ConstrainedFramePosition);
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
