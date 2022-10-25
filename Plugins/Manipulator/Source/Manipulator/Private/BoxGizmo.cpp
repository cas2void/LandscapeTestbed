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
#include "BaseGizmos/StateTargets.h"
#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "InteractiveGizmoManager.h"
#include "InteractiveToolsContext.h"
#include "ToolDataVisualizer.h"
#include "InputRouter.h"
#include "FrameTypes.h"
#include "Drawing/MeshDebugDrawing.h"

#include "BoxGizmoActor.h"
#include "PrimitiveGizmoBaseComponent.h"
#include "PrimitiveGizmoCircleComponent.h"

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

	//SetConstructionPlane(FVector(0.0f, 0.0f, 20.0f), FQuat::MakeFromEuler(FVector(0.0f, 30.0f, 0.0f)));
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
	if (IsInteracting())
	{
		FToolDataVisualizer Draw;
		Draw.BeginFrame(RenderAPI);

		Draw.LineThickness = 3.0f;

		const FTransform FrameTransform = GetConstructionFrame();
		Draw.SetTransform(FrameTransform);
		Draw.DrawWireBox(Bounds.GetBox());

		if (bDebug)
		{
			Draw.LineThickness = 1.0f;
			Draw.SetTransform(FrameTransform);
			Draw.DrawWireBox(DebugAlignedBounds.GetBox());
		}

		Draw.EndFrame();
	}

	//RenderConstructionPlane(RenderAPI);
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

	SetActiveGizmoPrimitiveComponent();

	// Init bounds by current active target and construction plane
	InitBounds();

	// Sub gizmos have been destroyed in ClearActiveTarget(), just recreate them
	CreateSubGizmos();
}

void UBoxGizmo::ClearActiveTarget()
{
	DestroySubGizmos();

	Bounds = FBoxSphereBounds(ForceInit);

	ClearActiveGizmoPrimitiveComponent();
	ActiveTarget = nullptr;
}

void UBoxGizmo::SetActiveGizmoPrimitiveComponent()
{
	if (ActiveTarget)
	{
		UPrimitiveComponent* ManipulableComponent = ActiveTarget->GetPrimitiveComponent();
		if (GizmoActor)
		{
			GizmoActor->SetTranslateXYComponent(ManipulableComponent);
		}
	}
}

void UBoxGizmo::ClearActiveGizmoPrimitiveComponent()
{
	if (GizmoActor)
	{
		GizmoActor->SetTranslateXYComponent(nullptr);
	}
}

void UBoxGizmo::InitBounds()
{
	if (ActiveTarget)
	{
		FManipulableBounds ManipulableBounds = ActiveTarget->GetBounds();
		FManipulableTransform ManipulableTransform = ActiveTarget->GetTransform();
		if (ManipulableBounds.bValid && ManipulableTransform.bValid)
		{
			Bounds = ConvertOBBToAABB(ManipulableBounds.Bounds, ManipulableTransform.Transform, GetConstructionFrame());
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
		}
	}
}

void UBoxGizmo::CreateSubGizmos()
{
	//
	// BoundsGroupComponent:
	// - Origin: bounds bottom center
	// - SocketDirection: construction plane Z
	// 
	// BoundsAxisZSource will be shared by:
	// - Elevation
	// - Plan corners
	// - Translate Z
	//
	UGizmoComponentAxisSource* BoundsAxisZSource = nullptr;
	if (GizmoActor)
	{
		USceneComponent* BoundsGroupComponent = GizmoActor->GetBoundsGroupComponent();
		if (BoundsGroupComponent)
		{
			// Elevation and plan corner gizmos share the same axis source, which is the Z axis of BoundsGroupComponent
			BoundsAxisZSource = UGizmoComponentAxisSource::Construct(BoundsGroupComponent, 2, true, this);
		}
	}

	if (!BoundsAxisZSource)
	{
		return;
	}
	
	RegulateGizmoRootTransform();

	//
	// Bounds Group
	//
	RegulateBoundsGroupTransform();

	CreateElevationGizmo(BoundsAxisZSource);
	for (int32 Index = 0; Index < 4; Index++)
	{
		CreatePlanCornerGizmo(BoundsAxisZSource, Index);
	}

	//
	// Rotation Group
	//
	RegulateRotationGroupTransform();
	RegulateRotationProxyTransform();
	for (int32 AxisIndex = 0; AxisIndex < 3; AxisIndex++)
	{
		for (int32 FaceIndex = 0; FaceIndex < 2; FaceIndex++)
		{
			CreateRotateAxisGizmo(AxisIndex, FaceIndex);
		}
	}

	//
	// Translation Group
	//
	RegulateTranslationGroupTransform();
	RegulateTranslationProxyTransform();
	CreateTranslateZGizmo(BoundsAxisZSource);
	CreateTranslateXYGizmo(BoundsAxisZSource);
}

void UBoxGizmo::DestroySubGizmos()
{
	for (auto& Elem : ActiveBoundsElevationGizmos)
	{
		GetGizmoManager()->DestroyGizmo(Elem.Key);
	}
	ActiveBoundsElevationGizmos.Empty();

	for (auto& Elem : ActiveBoundsPlanCornerGizmos)
	{
		GetGizmoManager()->DestroyGizmo(Elem.Key);
	}
	ActiveBoundsPlanCornerGizmos.Empty();

	for (auto& Elem : ActiveRotateAxisGizmos)
	{
		GetGizmoManager()->DestroyGizmo(Elem.Key);
	}
	ActiveRotateAxisGizmos.Empty();

	for (auto& Elem : ActiveTranslateZGizmos)
	{
		GetGizmoManager()->DestroyGizmo(Elem.Key);
	}
	ActiveTranslateZGizmos.Empty();

	for (auto& Elem : ActiveTranslateXYGizmos)
	{
		GetGizmoManager()->DestroyGizmo(Elem.Key);
	}
	ActiveTranslateXYGizmos.Empty();
}

void UBoxGizmo::CreateElevationGizmo(UGizmoComponentAxisSource* AxisSource)
{
	if (GizmoActor)
	{
		UPrimitiveComponent* ElevationComponent = GizmoActor->GetElevationComponent();
		if (ElevationComponent)
		{
			//
			// Move gizmo to target location, parent(bounds group) is the bounds bottom center
			//
			RegulateElevationTransform();

			//
			// Create axis-position gizmo
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
					NotifyBoundsModified();
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

			UGizmoLambdaStateTarget* StateTarget = NewObject<UGizmoLambdaStateTarget>(this);
			StateTarget->BeginUpdateFunction = [ElevationComponent, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(ElevationComponent) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(ElevationComponent)->UpdateInteractionState(true);
				}

				SetRotationGizmoVisibility(false);
				SetTranslationGizmoVisibility(false);
			};
			StateTarget->EndUpdateFunction = [ElevationComponent, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(ElevationComponent) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(ElevationComponent)->UpdateInteractionState(false);
				}

				SetRotationGizmoVisibility(true);
				SetTranslationGizmoVisibility(true);
			};
			ElevationGizmo->StateTarget = StateTarget;

			//
			// Reference the created gizmo
			//
			ActiveBoundsElevationGizmos.Add(ElevationGizmo, ElevationComponent);
		}
	}
}

void UBoxGizmo::CreatePlanCornerGizmo(UGizmoComponentAxisSource* AxisSource, int32 CornerIndex)
{
	if (GizmoActor)
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
					NotifyBoundsModified();
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

			UGizmoLambdaStateTarget* StateTarget = NewObject<UGizmoLambdaStateTarget>(this);
			StateTarget->BeginUpdateFunction = [CornerGizmo, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(CornerGizmo) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(CornerGizmo)->UpdateInteractionState(true);
				}

				SetRotationGizmoVisibility(false);
				SetTranslationGizmoVisibility(false);
			};
			StateTarget->EndUpdateFunction = [CornerGizmo, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(CornerGizmo) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(CornerGizmo)->UpdateInteractionState(false);
				}

				SetRotationGizmoVisibility(true);
				SetTranslationGizmoVisibility(true);
			};
			CornerGizmo->StateTarget = StateTarget;

			//
			// Reference the created gizmo
			//
			ActiveBoundsPlanCornerGizmos.Add(CornerGizmo, CornerComponent);
		}
	}
}

void UBoxGizmo::CreateRotateAxisGizmo(int32 AxisIndex, int32 FaceIndex)
{
	if (GizmoActor)
	{
		USceneComponent* RotateAxisSocketComponent = GizmoActor->GetRotateAxisSocketComponent(AxisIndex, FaceIndex);
		UPrimitiveComponent* RotateAxisIndicatorComponent = GizmoActor->GetRotateAxisIndicatorComponent(AxisIndex, FaceIndex);
		USceneComponent* RotationProxyComponent = GizmoActor->GetRotationProxyComponent();
		if (RotateAxisSocketComponent && RotateAxisIndicatorComponent && RotationProxyComponent)
		{
			//
			// Move gizmo to target location, parent(rotation group) is the bounds center
			//
			RegulateRotateAxisTransform(AxisIndex, FaceIndex);

			//
			// Create axis-angle gizmo
			//
			UAxisAngleGizmo* RotationGizmo = Cast<UAxisAngleGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisAngleBuilderIdentifier));
			check(RotationGizmo);

			//
			// Axis source provides the rotation axis
			//
			UGizmoComponentAxisSource* RotationAxisSource = UGizmoComponentAxisSource::Construct(RotateAxisSocketComponent, AxisIndex, true, this);
			RotationGizmo->AxisSource = RotationAxisSource;

			//
			// Axis-rotation parameter will drive rotate gizmo rotation along axis
			//
			UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(RotationProxyComponent, this);
			// Parameter source maps angle-parameter-change to rotation of TransformSource's transform
			UGizmoAxisRotationParameterSource* ParamSource = UGizmoAxisRotationParameterSource::Construct(RotationAxisSource, ComponentTransformSource, this);
			RotationGizmo->AngleSource = ParamSource;

			//
			// Bind delegates
			//
			ComponentTransformSource->OnTransformChanged.AddLambda(
				[this, AxisIndex](IGizmoTransformSource* TransformSource)
				{
					NotifyRotationModified();
					// Bounds needs to be recreated by active target's transform after rotation
					InitBounds();
					SyncComponentsByRotation(AxisIndex);
				}
			);

			//
			// Sub-component provides hit target
			//
			UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(RotateAxisIndicatorComponent, this);
			HitTarget->UpdateHoverFunction = [RotateAxisIndicatorComponent, this](bool bHovering)
			{
				if (Cast<UGizmoBaseComponent>(RotateAxisIndicatorComponent) != nullptr)
				{
					Cast<UGizmoBaseComponent>(RotateAxisIndicatorComponent)->UpdateHoverState(bHovering);
				}
			};
			RotationGizmo->HitTarget = HitTarget;

			UGizmoLambdaStateTarget* StateTarget = NewObject<UGizmoLambdaStateTarget>(this);
			StateTarget->BeginUpdateFunction = [RotationGizmo, AxisIndex, FaceIndex, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(RotationGizmo) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(RotationGizmo)->UpdateInteractionState(true);
				}

				// Bounds may be modified by bounds gizmo, but not matched to current object, so we need to recreate the bounds,
				// to prevent sudden jump in ComponentTransformSource->OnTransformChanged
				NotifyRotationModified();
				InitBounds();
				RegulateRotationAndSubTransform();

				SetBoundsGizmoVisibility(false);
				SetTranslationGizmoVisibility(false);
				SetRotationGizmoVisibility(false);

				SetRotateDialGizmoEnabled(AxisIndex, FaceIndex, true);
			};
			StateTarget->EndUpdateFunction = [RotationGizmo, AxisIndex, FaceIndex, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(RotationGizmo) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(RotationGizmo)->UpdateInteractionState(false);
				}

				SetBoundsGizmoVisibility(true);
				SetTranslationGizmoVisibility(true);
				SetRotationGizmoVisibility(true);

				SetRotateDialGizmoEnabled(AxisIndex, FaceIndex, false);
			};
			RotationGizmo->StateTarget = StateTarget;

			//
			// Reference the created gizmo
			//
			ActiveRotateAxisGizmos.Add(RotationGizmo, RotateAxisIndicatorComponent);
		}
	}
}

void UBoxGizmo::CreateTranslateZGizmo(UGizmoComponentAxisSource* AxisSource)
{
	if (GizmoActor)
	{
		UPrimitiveComponent* TranslateZComponent = GizmoActor->GetTranslateZComponent();
		USceneComponent* TranslationProxyComponent = GizmoActor->GetTranslationProxyComponent();
		if (TranslateZComponent && TranslationProxyComponent)
		{
			//
			// No Need to move gizmo, it always locates at the origin of parent's frame
			//

			//
			// Create axis-position gizmo
			//
			UAxisPositionGizmo* TranslateZGizmo = Cast<UAxisPositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultAxisPositionBuilderIdentifier));
			check(TranslateZGizmo);

			//
			// Axis source provides the translation axis
			//
			TranslateZGizmo->AxisSource = AxisSource;

			//
			// Axis-translation parameter will drive position along translate axis
			//
			UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(TranslationProxyComponent, this);
			// Parameter source maps axis-parameter-change to translation of TransformSource's transform
			UGizmoAxisTranslationParameterSource* ParamSource = UGizmoAxisTranslationParameterSource::Construct(AxisSource, ComponentTransformSource, this);
			TranslateZGizmo->ParameterSource = ParamSource;

			//
			// Bind delegates
			//
			ComponentTransformSource->OnTransformChanged.AddLambda(
				[this](IGizmoTransformSource* TransformSource)
				{
					// Bounds needs to be recreated by active target's transform after translation
					NotifyTranslationModified();
					InitBounds();
					SyncComponentsByTranslation();
				}
			);

			//
			// Sub-component provides hit target
			//
			UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(TranslateZComponent, this);
			HitTarget->UpdateHoverFunction = [TranslateZComponent, this](bool bHovering)
			{
				if (Cast<UGizmoBaseComponent>(TranslateZComponent) != nullptr)
				{
					Cast<UGizmoBaseComponent>(TranslateZComponent)->UpdateHoverState(bHovering);
				}
			};
			TranslateZGizmo->HitTarget = HitTarget;

			UGizmoLambdaStateTarget* StateTarget = NewObject<UGizmoLambdaStateTarget>(this);
			StateTarget->BeginUpdateFunction = [TranslateZGizmo, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(TranslateZGizmo) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(TranslateZGizmo)->UpdateInteractionState(true);
				}

				// Bounds may be modified by bounds gizmo, but not matched to current object, so we need to recreate the bounds,
				// to prevent sudden jump in ComponentTransformSource->OnTransformChanged
				NotifyRotationModified();
				InitBounds();
				RegulateTranslationAndSubTransform();

				SetBoundsGizmoVisibility(false);
				SetRotationGizmoVisibility(false);
			};
			StateTarget->EndUpdateFunction = [TranslateZGizmo, this]()
			{
				if (Cast<UPrimitiveGizmoBaseComponent>(TranslateZGizmo) != nullptr)
				{
					Cast<UPrimitiveGizmoBaseComponent>(TranslateZGizmo)->UpdateInteractionState(false);
				}

				SetBoundsGizmoVisibility(true);
				SetRotationGizmoVisibility(true);
			};
			TranslateZGizmo->StateTarget = StateTarget;

			//
			// Reference the created gizmo
			//
			ActiveTranslateZGizmos.Add(TranslateZGizmo, TranslateZComponent);
		}
	}
}

void UBoxGizmo::CreateTranslateXYGizmo(UGizmoComponentAxisSource* AxisSource)
{
	if (GizmoActor)
	{
		UPrimitiveComponent* TranslateXYComponent = GizmoActor->GetTranslateXYComponent();
		USceneComponent* TranslationProxyComponent = GizmoActor->GetTranslationProxyComponent();
		if (TranslateXYComponent && TranslationProxyComponent)
		{
			//
			// No Need to move gizmo, it always locates at the origin of parent's frame
			//

			//
			// Create plane-position gizmo
			//
			UPlanePositionGizmo* TranslateXYGizmo = Cast<UPlanePositionGizmo>(GetGizmoManager()->CreateGizmo(UInteractiveGizmoManager::DefaultPlanePositionBuilderIdentifier));
			check(TranslateXYGizmo);

			// Set InputBehavior priority
			UInteractiveToolsContext* Context = Cast<UInteractiveToolsContext>(GetGizmoManager()->GetOuter());
			if (Context)
			{
				UInputRouter* InputRouter = Context->InputRouter;
				if (InputRouter)
				{
					InputRouter->DeregisterSource(TranslateXYGizmo);

					UInputBehaviorSet* TransaleXYInputBehaviors = const_cast<UInputBehaviorSet*>(TranslateXYGizmo->GetInputBehaviors());
					if (TransaleXYInputBehaviors)
					{
						TransaleXYInputBehaviors->RemoveAll();
					}

					UClickDragInputBehavior* MouseBehavior = NewObject<UClickDragInputBehavior>();
					MouseBehavior->Initialize(TranslateXYGizmo);
					MouseBehavior->SetDefaultPriority(FInputCapturePriority(FInputCapturePriority::DEFAULT_GIZMO_PRIORITY).MakeLower());
					TranslateXYGizmo->AddInputBehavior(MouseBehavior);
					UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>();
					HoverBehavior->Initialize(TranslateXYGizmo);
					HoverBehavior->SetDefaultPriority(FInputCapturePriority(FInputCapturePriority::DEFAULT_GIZMO_PRIORITY).MakeLower());
					TranslateXYGizmo->AddInputBehavior(HoverBehavior);

					InputRouter->RegisterSource(TranslateXYGizmo);
				}
			}

			//
			// Axis source provides the translation axis
			//
			TranslateXYGizmo->AxisSource = AxisSource;

			//
			// Plane-translation parameter will drive gizmo position along plane
			//
			UGizmoComponentWorldTransformSource* ComponentTransformSource = UGizmoComponentWorldTransformSource::Construct(TranslationProxyComponent, this);
			// Parameter source maps axis-parameter-change to translation of TransformSource's transform
			UGizmoPlaneTranslationParameterSource* ParamSource = UGizmoPlaneTranslationParameterSource::Construct(AxisSource, ComponentTransformSource, this);
			TranslateXYGizmo->ParameterSource = ParamSource;

			//
			// Bind delegates
			//
			ComponentTransformSource->OnTransformChanged.AddLambda(
				[this](IGizmoTransformSource* TransformSource)
				{
					// Bounds needs to be recreated by active target's transform after translation
					NotifyTranslationModified();
					InitBounds();
					SyncComponentsByTranslation();
				}
			);

			//
			// Sub-component provides hit target
			//
			UGizmoComponentHitTarget* HitTarget = UGizmoComponentHitTarget::Construct(TranslateXYComponent, this);
			HitTarget->UpdateHoverFunction = [TranslateXYComponent, this](bool bHovering)
			{
				if (Cast<UGizmoBaseComponent>(TranslateXYComponent) != nullptr)
				{
					Cast<UGizmoBaseComponent>(TranslateXYComponent)->UpdateHoverState(bHovering);
				}
			};
			TranslateXYGizmo->HitTarget = HitTarget;

			UGizmoLambdaStateTarget* StateTarget = NewObject<UGizmoLambdaStateTarget>(this);
			StateTarget->BeginUpdateFunction = [this]()
			{
				// Bounds may be modified by bounds gizmo, but not matched to current object, so we need to recreate the bounds,
				// to prevent sudden jump in ComponentTransformSource->OnTransformChanged
				NotifyRotationModified();
				InitBounds();
				RegulateTranslationAndSubTransform();

				SetBoundsGizmoVisibility(false);
				SetRotationGizmoVisibility(false);
				SetTranslationGizmoVisibility(false);
			};
			StateTarget->EndUpdateFunction = [this]()
			{
				SetBoundsGizmoVisibility(true);
				SetRotationGizmoVisibility(true);
				SetTranslationGizmoVisibility(true);
			};
			TranslateXYGizmo->StateTarget = StateTarget;

			//
			// Reference the created gizmo
			//
			ActiveTranslateXYGizmos.Add(TranslateXYGizmo, TranslateXYComponent);
		}
	}
}

void UBoxGizmo::SyncComponentsByElevation()
{
	RegulateRotationAndSubTransform();
	RegulateTranslationAndSubTransform();
}

void UBoxGizmo::SyncComponentsByCorner(int32 CornerIndex)
{
	// Parent(bounds group) will be transformed, all corners need update
	(void)CornerIndex;

	RegulateBoundsAndSubTransform();
	RegulateRotationAndSubTransform();
	RegulateTranslationAndSubTransform();
}

void UBoxGizmo::SyncComponentsByRotation(int32 AxisIndex)
{
	RegulateBoundsAndSubTransform();
	RegulateTranslationAndSubTransform();
}

void UBoxGizmo::SyncComponentsByTranslation()
{
	RegulateBoundsAndSubTransform();
	RegulateRotationAndSubTransform();
	RegulateTranslationAndSubTransform();
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
	if (GizmoActor)
	{
		UPrimitiveComponent* CornerComponent = GizmoActor->GetPlanCornerComponent(CornerIndex);
		if (CornerComponent)
		{
			const FVector CornerLocation = GetPlanCornerLocation(Bounds, CornerIndex);
			CornerComponent->SetRelativeLocation(CornerLocation);
		}
	}
}

void UBoxGizmo::RegulateBoundsAndSubTransform()
{
	RegulateBoundsGroupTransform();

	for (int32 PlanCornerIndex = 0; PlanCornerIndex < 4; PlanCornerIndex++)
	{
		RegulatePlanCornerTransform(PlanCornerIndex);
	}

	// Sync elevation to make its projection locate at the center of plan
	RegulateElevationTransform();
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

void UBoxGizmo::RegulateRotateAxisTransform(int32 AxisIndex, int32 FaceIndex)
{
	if (GizmoActor)
	{
		USceneComponent* AxisSocketComponent = GizmoActor->GetRotateAxisSocketComponent(AxisIndex, FaceIndex);
		UPrimitiveComponent* AxisIndicatorComponent = GizmoActor->GetRotateAxisIndicatorComponent(AxisIndex, FaceIndex);
		if (AxisSocketComponent && AxisIndicatorComponent)
		{
			FVector SocketDirection = FVector::ZeroVector;
			FVector IndicatorDirection = FVector::ZeroVector;
			switch (AxisIndex)
			{
			case 0:
				SocketDirection = FVector::XAxisVector;
				IndicatorDirection = FVector::ZAxisVector;
				break;
			case 1:
				SocketDirection = FVector::YAxisVector;
				IndicatorDirection = FVector::ZAxisVector;
				break;
			case 2:
				SocketDirection = FVector::ZAxisVector;
				break;
			default:
				break;
			}
			float Sign = (FaceIndex == 0) ? 1.0f : -1.0f;
			FVector SocketOffset = SocketDirection * Bounds.BoxExtent * Sign;
			FVector IndicatorOffset = IndicatorDirection * Bounds.BoxExtent;

			AxisSocketComponent->SetRelativeLocation(SocketOffset);
			AxisIndicatorComponent->SetRelativeLocation(IndicatorOffset);
		}
	}
}

void UBoxGizmo::RegulateRotationProxyTransform()
{
	if (ActiveTarget)
	{
		FManipulableTransform ManipulableTransform = ActiveTarget->GetTransform();
		if (ManipulableTransform.bValid)
		{
			if (GizmoActor)
			{
				// Rotation proxy
				USceneComponent* RotationProxyComponent = GizmoActor->GetRotationProxyComponent();
				if (RotationProxyComponent)
				{
					FQuat Rotation = ManipulableTransform.Transform.GetRotation();
					RotationProxyComponent->SetWorldRotation(Rotation);
				}
			}
		}
	}
}

void UBoxGizmo::RegulateRotationAndSubTransform()
{
	RegulateRotationGroupTransform();

	for (int32 AxisIndex = 0; AxisIndex < 3; AxisIndex++)
	{
		for (int32 FaceIndex = 0; FaceIndex < 2; FaceIndex++)
		{
			RegulateRotateAxisTransform(AxisIndex, FaceIndex);
		}
	}

	// Any gizmo other than rotation type will change proxy rotation, so it's unnecessary to regulate proxy's rotation here
}

void UBoxGizmo::RegulateTranslationGroupTransform()
{
	if (GizmoActor)
	{
		USceneComponent* TranslationGroupComponent = GizmoActor->GetTranslationGroupComponent();
		if (TranslationGroupComponent)
		{
			FVector BoundsFrameTopCenter = Bounds.Origin + FVector(0.0f, 0.0f, Bounds.BoxExtent.Z);
			FVector BoundsFrameWorldCenter = TransformPositionConstructionFrameToWorld(BoundsFrameTopCenter);
			TranslationGroupComponent->SetWorldLocation(BoundsFrameWorldCenter);
		}
	}
}

void UBoxGizmo::RegulateTranslationProxyTransform()
{
	if (ActiveTarget)
	{
		FManipulableTransform ManipulableTransform = ActiveTarget->GetTransform();
		if (ManipulableTransform.bValid)
		{
			if (GizmoActor)
			{
				// Translation proxy
				USceneComponent* TranslationProxyComponent = GizmoActor->GetTranslationProxyComponent();
				if (TranslationProxyComponent)
				{
					FVector Location = ManipulableTransform.Transform.GetLocation();
					TranslationProxyComponent->SetWorldLocation(Location);
				}
			}
		}
	}
}

void UBoxGizmo::RegulateTranslationAndSubTransform()
{
	RegulateTranslationGroupTransform();
	RegulateTranslationProxyTransform();
}

void UBoxGizmo::NotifyBoundsModified()
{
	if (ActiveTarget)
	{
		FManipulableBounds ManipulableBounds = ActiveTarget->GetBounds();
		FManipulableTransform ManipulableTransform = ActiveTarget->GetTransform();
		if (ManipulableBounds.bValid && ManipulableTransform.bValid)
		{
			//
			// Original OBB needs to be concentric with current Bounds, set its transform's location before the conversion
			//
			const FTransform FrameTransform = GetConstructionFrame();
			const FVector BoundsWorldCenter = FrameTransform.TransformPosition(Bounds.Origin);
			FTransform ModifiedTransform = ManipulableTransform.Transform;
			ModifiedTransform.SetLocation(BoundsWorldCenter);
			FBoxSphereBounds AlignedBounds = ConvertOBBToAABB(ManipulableBounds.Bounds, ModifiedTransform, FrameTransform);

			ActiveTarget->OnBoundsModified(Bounds, FrameTransform, AlignedBounds);

			if (bDebug)
			{
				DebugAlignedBounds = AlignedBounds;
			}
		}
	}
}

void UBoxGizmo::NotifyRotationModified()
{
	if (GizmoActor)
	{
		USceneComponent* RotationProxyComponent = GizmoActor->GetRotationProxyComponent();
		if (RotationProxyComponent)
		{
			const FQuat Rotation = RotationProxyComponent->GetComponentQuat();
			if (ActiveTarget)
			{
				ActiveTarget->OnRotationModified(Rotation);
			}
		}
	}
}

void UBoxGizmo::NotifyTranslationModified()
{
	if (GizmoActor)
	{
		USceneComponent* TranslationProxyComponent = GizmoActor->GetTranslationProxyComponent();
		if (TranslationProxyComponent)
		{
			const FVector Location = TranslationProxyComponent->GetComponentLocation();
			if (ActiveTarget)
			{
				ActiveTarget->OnLocationModified(Location);
			}
		}
	}
}

bool UBoxGizmo::IsInteracting() const
{
	for (const auto& Elem : ActiveBoundsElevationGizmos)
	{
		if (Elem.Value && !Elem.Value->IsVisible())
		{
			return true;
		}
	}

	for (const auto& Elem : ActiveBoundsPlanCornerGizmos)
	{
		if (Elem.Value && !Elem.Value->IsVisible())
		{
			return true;
		}
	}

	for (const auto& Elem : ActiveRotateAxisGizmos)
	{
		if (Elem.Value && !Elem.Value->IsVisible())
		{
			return true;
		}
	}

	for (const auto& Elem : ActiveTranslateZGizmos)
	{
		if (Elem.Value && !Elem.Value->IsVisible())
		{
			return true;
		}
	}

	return false;
}

void UBoxGizmo::SetBoundsGizmoVisibility(bool bVisible)
{
	for (const auto& Elem : ActiveBoundsElevationGizmos)
	{
		if (Elem.Value && Elem.Value->IsVisible() != bVisible)
		{
			Elem.Value->SetVisibility(bVisible);
		}
	}

	for (const auto& Elem : ActiveBoundsPlanCornerGizmos)
	{
		if (Elem.Value && Elem.Value->IsVisible() != bVisible)
		{
			Elem.Value->SetVisibility(bVisible);
		}
	}
}

void UBoxGizmo::SetRotationGizmoVisibility(bool bVisible)
{
	for (const auto& Elem : ActiveRotateAxisGizmos)
	{
		if (Elem.Value && Elem.Value->IsVisible() != bVisible)
		{
			Elem.Value->SetVisibility(bVisible);
		}
	}
}

void UBoxGizmo::SetTranslationGizmoVisibility(bool bVisible)
{
	for (const auto& Elem : ActiveTranslateZGizmos)
	{
		if (Elem.Value && Elem.Value->IsVisible() != bVisible)
		{
			Elem.Value->SetVisibility(bVisible);
		}
	}
}

void UBoxGizmo::SetRotateDialGizmoEnabled(int32 AxisIndex, int32 FaceIndex, bool bEnabled)
{
	UPrimitiveGizmoCircleComponent* RotateAxisDialComponent = Cast<UPrimitiveGizmoCircleComponent>(GizmoActor->GetRotateAxisDialComponent(AxisIndex, FaceIndex));
	if (RotateAxisDialComponent)
	{
		if (bEnabled)
		{
			float DialRadius = GetRotateDialRadius(Bounds, AxisIndex);
			RotateAxisDialComponent->SetRadius(DialRadius);
		}
		RotateAxisDialComponent->SetVisibility(bEnabled);
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

FBoxSphereBounds UBoxGizmo::ConvertOBBToAABB(const FBoxSphereBounds& InBounds, const FTransform& InBoundsTransform, const FTransform& FrameTransform)
{
	const FTransform Local2World = InBoundsTransform;
	const FTransform Frame2World = FrameTransform;
	const FTransform Local2Frame = Local2World * Frame2World.Inverse();

	const FBox LocalBoundingBox = InBounds.GetBox();
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

	FBoxSphereBounds Result = FBoxSphereBounds(CornerPoints.GetData(), CornerPoints.Num());

	return Result;
}

float UBoxGizmo::GetRotateDialRadius(const FBoxSphereBounds& InBounds, int32 AxisIndex)
{
	float Result = 0.0f;

	float Edge0 = 0.0f;
	float Edge1 = 0.0f;
	switch (AxisIndex)
	{
	case 0:
		Edge0 = InBounds.BoxExtent.Y;
		Edge1 = InBounds.BoxExtent.Z;
		break;
	case 1:
		Edge0 = InBounds.BoxExtent.Z;
		Edge1 = InBounds.BoxExtent.X;
		break;
	case 2:
		Edge0 = InBounds.BoxExtent.X;
		Edge1 = InBounds.BoxExtent.Y;
		break;
	default:
		break;
	}

	Result = FMath::Sqrt(Edge0 * Edge0 + Edge1 * Edge1);

	return Result;
}
