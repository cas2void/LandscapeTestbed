// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmoActor.h"

#include "Components/SphereComponent.h"

#include "PrimitiveGizmoRectComponent.h"
#include "PrimitiveGizmoCircleComponent.h"
#include "PrimitiveGizmoArrowComponent.h"
#include "PrimitiveGizmoRotateComponent.h"

ABoxGizmoActor::ABoxGizmoActor()
{
	// Root
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("GizmoCenter"));
	SphereComponent->InitSphereRadius(1.0f);
	SphereComponent->SetVisibility(false);
	SphereComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SetRootComponent(SphereComponent);

	//
	// Bounds Group
	//
	BoundsGroupComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoBoundsGroup"));
	BoundsGroupComponent->SetupAttachment(GetRootComponent());

	const FLinearColor GizmoBoundsColor(0.0f, 0.4f, 0.4f);
	float GizmoBoundsThickness = 10.0f;

	// Elevation
	UPrimitiveGizmoCircleComponent* TempElevationComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoElevation"));
	TempElevationComponent->Color = GizmoBoundsColor;
	TempElevationComponent->SetRadius(10.0f);
	TempElevationComponent->SetCenterOffset(FVector(0.0f, 0.0f, 30.0f));
	TempElevationComponent->SetThickness(GizmoBoundsThickness);
	TempElevationComponent->SetNumSides(16);
	TempElevationComponent->SetViewAligned(true);
	TempElevationComponent->SetupAttachment(BoundsGroupComponent);
	ElevationComponent = TempElevationComponent;

	// Plane top left
	UPrimitiveGizmoRectComponent* TempPlanTopLeftComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanTopLeft"));
	TempPlanTopLeftComponent->Color = GizmoBoundsColor;
	TempPlanTopLeftComponent->SetOffsetX(TempPlanTopLeftComponent->GetLengthX() * -0.5f);
	TempPlanTopLeftComponent->SetOffsetY(TempPlanTopLeftComponent->GetLengthY() * -0.5f);
	TempPlanTopLeftComponent->SetThickness(GizmoBoundsThickness);
	TempPlanTopLeftComponent->SetSegmentFlags(0x1 | 0x8);
	TempPlanTopLeftComponent->SetupAttachment(BoundsGroupComponent);
	PlanTopLeftComponent = TempPlanTopLeftComponent;

	// Plane top right
	UPrimitiveGizmoRectComponent* TempPlanTopRightComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanTopRight"));
	TempPlanTopRightComponent->Color = GizmoBoundsColor;
	TempPlanTopRightComponent->SetOffsetX(TempPlanTopRightComponent->GetLengthX() * -0.5f);
	TempPlanTopRightComponent->SetOffsetY(TempPlanTopRightComponent->GetLengthY() * -0.5f);
	TempPlanTopRightComponent->SetThickness(GizmoBoundsThickness);
	TempPlanTopRightComponent->SetSegmentFlags(0x1 | 0x2);
	TempPlanTopRightComponent->SetupAttachment(BoundsGroupComponent);
	PlanTopRightComponent = TempPlanTopRightComponent;

	// Plane bottom right
	UPrimitiveGizmoRectComponent* TempPlanBottomRightComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanBottomRight"));
	TempPlanBottomRightComponent->Color = GizmoBoundsColor;
	TempPlanBottomRightComponent->SetOffsetX(TempPlanBottomRightComponent->GetLengthX() * -0.5f);
	TempPlanBottomRightComponent->SetOffsetY(TempPlanBottomRightComponent->GetLengthY() * -0.5f);
	TempPlanBottomRightComponent->SetThickness(GizmoBoundsThickness);
	TempPlanBottomRightComponent->SetSegmentFlags(0x2 | 0x4);
	TempPlanBottomRightComponent->SetupAttachment(BoundsGroupComponent);
	PlanBottomRightComponent = TempPlanBottomRightComponent;

	// Plane bottom left
	UPrimitiveGizmoRectComponent* TempPlanBottomLeftComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanBottomLeft"));
	TempPlanBottomLeftComponent->Color = GizmoBoundsColor;
	TempPlanBottomLeftComponent->SetOffsetX(TempPlanBottomLeftComponent->GetLengthX() * -0.5f);
	TempPlanBottomLeftComponent->SetOffsetY(TempPlanBottomLeftComponent->GetLengthY() * -0.5f);
	TempPlanBottomLeftComponent->SetThickness(GizmoBoundsThickness);
	TempPlanBottomLeftComponent->SetSegmentFlags(0x4 | 0x8);
	TempPlanBottomLeftComponent->SetupAttachment(BoundsGroupComponent);
	PlanBottomLeftComponent = TempPlanBottomLeftComponent;

	//
	// Rotation Group
	//
	RotationGroupComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotationGroup"));
	RotationGroupComponent->SetupAttachment(GetRootComponent());

	const float GizmoRotationThickness = GizmoBoundsThickness * 0.6f;
	const float GizmoRotationRadius = 60.0f;

	UPrimitiveGizmoRotateComponent* TempRotateXComponent = CreateDefaultSubobject<UPrimitiveGizmoRotateComponent>(TEXT("GizmoRotateX"));
	TempRotateXComponent->Color = FLinearColor(1.0f, 0.1, 0.1f);
	TempRotateXComponent->SetAxisIndex(0);
	TempRotateXComponent->SetRadius(GizmoRotationRadius);
	TempRotateXComponent->SetThickness(GizmoRotationThickness);
	TempRotateXComponent->SetupAttachment(RotationGroupComponent);
	RotateXComponent = TempRotateXComponent;

	//// Axis X Rotate
	//UPrimitiveGizmoCircleComponent* TempRotateXComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateX"));
	//TempRotateXComponent->Color = FLinearColor::Red;
	//TempRotateXComponent->Normal = FVector(1.0f, 0.0f, 0.0f);
	//TempRotateXComponent->Radius = GizmoRotationRadius;
	//TempRotateXComponent->Thickness = GizmoRotationThickness;
	//TempRotateXComponent->SetupAttachment(RotationGroupComponent);
	//RotateXComponent = TempRotateXComponent;

	//// Axis Y Rotate
	//UPrimitiveGizmoCircleComponent* TempRotateYComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateY"));
	//TempRotateYComponent->Color = FLinearColor::Green;
	//TempRotateYComponent->Normal = FVector(0.0f, 1.0f, 0.0f);
	//TempRotateYComponent->Radius = GizmoRotationRadius;
	//TempRotateYComponent->Thickness = GizmoRotationThickness;
	//TempRotateYComponent->SetupAttachment(RotationGroupComponent);
	//RotateYComponent = TempRotateYComponent;

	//// Axis Z Rotate
	//UPrimitiveGizmoCircleComponent* TempRotateZComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateZ"));
	//TempRotateZComponent->Color = FLinearColor::Blue;
	//TempRotateZComponent->Normal = FVector(0.0f, 0.0f, 1.0f);
	//TempRotateZComponent->Radius = GizmoRotationRadius;
	//TempRotateZComponent->Thickness = GizmoRotationThickness;
	//TempRotateZComponent->SetupAttachment(RotationGroupComponent);
	//RotateZComponent = TempRotateZComponent;

	// Rotation Proxy
	RotationProxyComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotationProxy"));
	RotationProxyComponent->SetupAttachment(RotationGroupComponent);

	////
	//// Translation Group
	////
	//TranslationGroupComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoTranslationGroup"));
	//TranslationGroupComponent->SetupAttachment(GetRootComponent());

	//const FLinearColor GizmoTranslationColor = FLinearColor(0.0f, 0.2f, 0.2f);
	//const float GizmoTranslationThickness = 8.0f;

	//// Translate Z
	//UPrimitiveGizmoArrowComponent* TempTranslateZComponent = CreateDefaultSubobject<UPrimitiveGizmoArrowComponent>(TEXT("GizmoTranslateZ"));
	//TempTranslateZComponent->Color = GizmoTranslationColor;
	//TempTranslateZComponent->SetDirection(FVector(0.0f, 0.0f, 1.0f));
	//TempTranslateZComponent->SetGap(40.0f);
	//TempTranslateZComponent->SetLength(40.0f);
	//TempTranslateZComponent->SetThickness(GizmoTranslationThickness);
	//TempTranslateZComponent->SetupAttachment(TranslationGroupComponent);
	//TranslateZComponent = TempTranslateZComponent;

	//// Translation Proxy
	//TranslationProxyComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoTranslationProxy"));
	//TranslationProxyComponent->SetupAttachment(TranslationGroupComponent);
}

TArray<UPrimitiveComponent*> ABoxGizmoActor::GetBoundsSubComponents()
{
	TArray<UPrimitiveComponent*> Result;
	Result.Add(ElevationComponent);
	Result.Add(PlanTopLeftComponent);
	Result.Add(PlanTopRightComponent);
	Result.Add(PlanBottomRightComponent);
	Result.Add(PlanBottomLeftComponent);

	return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetPlanCornerComponent(int32 CornerIndex)
{
	check(CornerIndex >= 0 && CornerIndex < 4);

	UPrimitiveComponent* CornerComponent = nullptr;
	switch (CornerIndex)
	{
	case 0:
		CornerComponent = PlanTopLeftComponent;
		break;
	case 1:
		CornerComponent = PlanTopRightComponent;
		break;
	case 2:
		CornerComponent = PlanBottomRightComponent;
		break;
	case 3:
		CornerComponent = PlanBottomLeftComponent;
		break;
	default:
		break;
	}

	return CornerComponent;
}

int32 ABoxGizmoActor::GetPlanCornerDiagonalIndex(int32 CornerIndex) const
{
	check(CornerIndex >= 0 && CornerIndex < 4);

	return ((CornerIndex + 2) % 4);
}

TArray<int32> ABoxGizmoActor::GetPlanCornerNeighborIndices(int32 CornerIndex) const
{
	check(CornerIndex >= 0 && CornerIndex < 4);

	TArray<int32> Result;
	Result.Add((CornerIndex + 1) % 4);
	Result.Add((CornerIndex + 3) % 4);

	return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetRotateAxisComponent(int32 AxisIndex)
{
	UPrimitiveComponent* RotationComponent = nullptr;
	switch (AxisIndex)
	{
	case 0:
		RotationComponent = RotateXComponent;
		break;
	case 1:
		RotationComponent = RotateYComponent;
		break;
	case 2:
		RotationComponent = RotateZComponent;
		break;
	default:
		break;
	}

	return RotationComponent;
}

UPrimitiveComponent* ABoxGizmoActor::GetTranslateXYComponent()
{
	UPrimitiveComponent* Result = nullptr;

	if (TranslateXYComponent.IsValid())
	{
		Result = TranslateXYComponent.Get();
	}

	return Result;
}

void ABoxGizmoActor::SetTranslateXYComponent(UPrimitiveComponent* InComponent)
{
	if (InComponent)
	{
		TranslateXYComponent = InComponent;
	}
	else
	{
		TranslateXYComponent.Reset();
	}
}
