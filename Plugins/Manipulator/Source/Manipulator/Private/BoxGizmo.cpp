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
    // Reset gizmo root transform
    if (GizmoActor)
    {
        FVector BoundsWorldCenter = TransformConstructionFramePositionToWorld(Bounds.Origin);
        GizmoActor->SetActorLocation(BoundsWorldCenter);
        GizmoActor->SetActorRotation(ConstructionPlaneOrientation);
        GizmoActor->SetActorScale3D(FVector::OneVector);
    }
    UGizmoComponentAxisSource* GizmoAxisXSource = UGizmoComponentAxisSource::Construct(GizmoActor->GetRootComponent(), 0, true, this);
    UGizmoComponentAxisSource* GizmoAxisYSource = UGizmoComponentAxisSource::Construct(GizmoActor->GetRootComponent(), 1, true, this);
    UGizmoComponentAxisSource* GizmoAxisZSource = UGizmoComponentAxisSource::Construct(GizmoActor->GetRootComponent(), 2, true, this);

    UGizmoConstantAxisSource* ConstructionAxisSource = NewObject<UGizmoConstantAxisSource>(this);
    if (ConstructionAxisSource)
    {
        const FVector BottomCenterFramePosition = Bounds.Origin + FVector(0.0f, 0.0f, -Bounds.BoxExtent.Z);
        const FVector BottomCenterWorldPosition = TransformConstructionFramePositionToWorld(BottomCenterFramePosition);

        ConstructionAxisSource->Origin = BottomCenterWorldPosition;
        ConstructionAxisSource->Direction = ConstructionPlaneOrientation.GetUpVector();
    }

    //CreateElevationGizmo(GizmoAxisZSource);
    //for (int32 Index = 0; Index < 4; Index++)
    //{
    //    CreateCornerGizmo(ConstructionAxisSource, Index);
    //}

    //// Reset gizmo root location
    //if (GizmoActor)
    //{
    //    GizmoActor->SetActorLocation(ConstructionPlaneOrigin);
    //    GizmoActor->SetActorRotation(ConstructionPlaneOrientation);
    //}

    //UGizmoConstantAxisSource* ConstructionAxisSource = NewObject<UGizmoConstantAxisSource>(this);
    //if (ConstructionAxisSource)
    //{
    //    const FVector BottomCenterFramePosition = Bounds.Origin + FVector(0.0f, 0.0f, -Bounds.BoxExtent.Z);
    //    const FVector BottomCenterWorldPosition = TransformConstructionFramePositionToWorld(BottomCenterFramePosition);

    //    ConstructionAxisSource->Origin = BottomCenterWorldPosition;
    //    ConstructionAxisSource->Direction = ConstructionPlaneOrientation.GetUpVector();
    //}

    //CreateElevationGizmo(ConstructionAxisSource);
    //// Top Left
    //CreateCornerGizmo(ConstructionAxisSource, false, false);
    //// Top Right
    //CreateCornerGizmo(ConstructionAxisSource, true, false);
    //// Bottom Right
    //CreateCornerGizmo(ConstructionAxisSource, true, true);
    //// Bottom Left
    //CreateCornerGizmo(ConstructionAxisSource, false, true);
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
        // Move gizmo to target location, gizmo root is the bounds center
        //
        FVector ElevationLocation = FVector(0.0f, 0.0f, Bounds.BoxExtent.Z);
        ElevationComponent->SetRelativeLocation(ElevationLocation);

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

void UBoxGizmo::CreateCornerGizmo(UGizmoConstantAxisSource* AxisSource, int32 CornerIndex)
{
    UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(CornerIndex);
    if (CornerComponent)
    {
        //
        // Move gizmo to target location
        //
        const FVector CornerLocation = GetPlanCornerLocation(Bounds, CornerIndex);
        CornerComponent->SetRelativeLocation(CornerLocation);

        //
        // Create plane-position gizmo
        //
        UPlanePositionGizmo* CornerGizmo = Cast<UPlanePositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultPlanePositionBuilderIdentifier));
        check(CornerGizmo);

        //
        // Axis source provides the translation plane for corner
        //
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

void UBoxGizmo::CreateRotationGizmo(int32 AxisIndex)
{
    UPrimitiveComponent* RotationComponent = GizmoActor->GetRotationComponent(AxisIndex);
    if (RotationComponent)
    {
        //
        // Move gizmo to target location
        //
        const FVector RotationLocation = Bounds.Origin;
        RotationComponent->SetRelativeLocation(RotationLocation);

        //
        // Create axis-angle gizmo
        //
        UAxisAngleGizmo* RotationGizmo = Cast<UAxisAngleGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisAngleBuilderIdentifier));
        check(RotationGizmo);

        ////
        //// Axis source provides the rotation axis
        ////
        //UGizmoConstantAxisSource* CornerAxisSource = AxisSource;
        //CornerGizmo->AxisSource = CornerAxisSource;

        ////
        //// Plane-translation parameter will drive corner gizmo position along corner plane
        ////
        //UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(RotationComponent, this);
        //// Parameter source maps axis-parameter-change to translation of TransformSource's transform
        //UGizmoPlaneTranslationParameterSource* ParamSource = UGizmoPlaneTranslationParameterSource::Construct(CornerAxisSource, ComponentTransformSource, this);
        //CornerGizmo->ParameterSource = ParamSource;

        ////
        //// Bind delegates
        ////
        //ComponentTransformSource->OnTransformChanged.AddLambda(
        //    [this, bPositiveX, bPositiveY](IGizmoTransformSource* TransformSource)
        //    {
        //        RecreateBoundsByCorner(bPositiveX, bPositiveY);
        //        SyncComponentsByCorner(bPositiveX, bPositiveY);
        //    }
        //);

        //ParamSource->PositionConstraintFunction = [this, bPositiveX, bPositiveY](const FVector& RawPosition, FVector& ConstrainedPosition)
        //{
        //    return ConstrainCornerPosition(RawPosition, ConstrainedPosition, bPositiveX, bPositiveY);
        //};

        ////
        //// Sub-component provides hit target
        ////
        //UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(RotationComponent, this);
        //HitTarget->UpdateHoverFunction = [RotationComponent, this](bool bHovering)
        //{
        //    if (Cast<UGizmoBaseComponent>(RotationComponent) != nullptr)
        //    {
        //        Cast<UGizmoBaseComponent>(RotationComponent)->UpdateHoverState(bHovering);
        //    }
        //};
        //CornerGizmo->HitTarget = HitTarget;

        ////
        //// Reference the created gizmo
        ////
        //ActiveGizmos.Add(CornerGizmo);
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
        TArray<UPrimitiveComponent*> GizmoComponents = GizmoActor->GetBoundsSubComponents();
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
            Locations.Add(TransformWorldPositionToConstructionFrame(SourceComponent->GetComponentLocation()));
            Locations.Add(TransformWorldPositionToConstructionFrame(DiagonalComponent->GetComponentLocation()));
            Locations.Add(TransformWorldPositionToConstructionFrame(ElevationComponent->GetComponentLocation()));

            Bounds = FBoxSphereBounds(Locations.GetData(), Locations.Num());

            NotifyBoundsModified();
        }
    }
}

void UBoxGizmo::SyncComponentsByElevation()
{
    if (GizmoActor)
    {

        //UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
        //if (ElevationComponent)
        //{
        //    FVector ElevationWorldLocation = ElevationComponent->GetComponentLocation();
        //    FVector ElevationConstructionFrameLocation = TransformWorldPositionToConstructionFrame(ElevationWorldLocation);
        //    bool bFlipped = ElevationConstructionFrameLocation.Z < Bounds.Origin.Z + Bounds.BoxExtent.Z - 1.0f;
        //    UE_LOG(LogTemp, Warning, TEXT("bFlipped: %d"), bFlipped);
        //    //GizmoActor->SetActorScale3D(GizmoScale3D);
        //}

        //FVector BoundsWorldCenter = TransformConstructionFramePositionToWorld(Bounds.Origin);
        //GizmoActor->SetActorLocation(BoundsWorldCenter);

        //for (int32 Index = 0; Index < 4; Index++)
        //{
        //    UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(Index);
        //    if (CornerComponent)
        //    {
        //        const FVector CornerLocation = GetPlanCornerLocation(Bounds, Index);
        //        CornerComponent->SetRelativeLocation(CornerLocation);
        //    }
        //}
    }
}

void UBoxGizmo::SyncComponentsByCorner(int32 CornerIndex)
{
    if (GizmoActor)
    {
        // Sync corner neighbors on plan
        for (const auto& NeighborIndex : GizmoActor->GetPlanCornerNeighborIndices(CornerIndex))
        {
            UPrimitiveComponent* NeighborComponent = GizmoActor->GetPlanCornerComponent(NeighborIndex);
            if (NeighborComponent)
            {
                const FVector CornerLocation = GetPlanCornerLocation(Bounds, NeighborIndex);
                NeighborComponent->SetRelativeLocation(CornerLocation);
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
            const FVector RawFramePosition = TransformWorldPositionToConstructionFrame(RawPosition);
            const FVector DiagonalFrameLocation = TransformWorldPositionToConstructionFrame(DiagonalComponent->GetComponentLocation());

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
                ConstrainedPosition = TransformConstructionFramePositionToWorld(ConstrainedFramePosition);
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
    FVector CornerLocation = /*InBounds.Origin + */FVector(InBounds.BoxExtent.X * SignX, InBounds.BoxExtent.Y * SignY, -InBounds.BoxExtent.Z);

    return CornerLocation;
}
