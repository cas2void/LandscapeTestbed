// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmoActor.h"

#include "Components/SphereComponent.h"
#include "BaseGizmos/GizmoRectangleComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"

#include "RectGizmoComponent.h"

ABoxGizmoActor::ABoxGizmoActor()
{
	// Center
	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("GizmoCenter"));
	SphereComponent->InitSphereRadius(1.0f);
	SphereComponent->SetVisibility(false);
	SphereComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	SetRootComponent(SphereComponent);

	const FLinearColor GizmoColor(0.0f, 0.2f, 0.2f);
	float GizmoThickness = 10.0f;

	// Elevation
	UGizmoCircleComponent* TempElevationComponent = CreateDefaultSubobject<UGizmoCircleComponent>(TEXT("GizmoElevation"));
	TempElevationComponent->Color = GizmoColor;
	TempElevationComponent->Radius = 10.0f;
	TempElevationComponent->Thickness = GizmoThickness;
	TempElevationComponent->NumSides = 16;
	TempElevationComponent->bViewAligned = true;
	TempElevationComponent->bOnlyAllowFrontFacingHits = false;
	TempElevationComponent->SetupAttachment(GetRootComponent());

	ElevationComponent = TempElevationComponent;

	// Plane top left
	URectGizmoComponent* TempPlanTopLeftComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanTopLeft"));
	TempPlanTopLeftComponent->Color = GizmoColor;
	TempPlanTopLeftComponent->SetOffsetX(TempPlanTopLeftComponent->GetLengthX() * -0.5f);
	TempPlanTopLeftComponent->SetOffsetY(TempPlanTopLeftComponent->GetLengthY() * -0.5f);
	TempPlanTopLeftComponent->SetThickness(GizmoThickness);
	TempPlanTopLeftComponent->SetSegmentFlags(0x1 | 0x8);
	
	TempPlanTopLeftComponent->SetupAttachment(GetRootComponent());

	PlanTopLeftComponent = TempPlanTopLeftComponent;

	// Plane top right
	URectGizmoComponent* TempPlanTopRightComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanTopRight"));
	TempPlanTopRightComponent->Color = GizmoColor;
	TempPlanTopRightComponent->SetOffsetX(TempPlanTopRightComponent->GetLengthX() * -0.5f);
	TempPlanTopRightComponent->SetOffsetY(TempPlanTopRightComponent->GetLengthY() * -0.5f);
	TempPlanTopRightComponent->SetThickness(GizmoThickness);
	TempPlanTopRightComponent->SetSegmentFlags(0x1 | 0x2);
	TempPlanTopRightComponent->SetupAttachment(GetRootComponent());

	PlanTopRightComponent = TempPlanTopRightComponent;

	// Plane bottom right
	URectGizmoComponent* TempPlanBottomRightComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanBottomRight"));
	TempPlanBottomRightComponent->Color = GizmoColor;
	TempPlanBottomRightComponent->SetOffsetX(TempPlanBottomRightComponent->GetLengthX() * -0.5f);
	TempPlanBottomRightComponent->SetOffsetY(TempPlanBottomRightComponent->GetLengthY() * -0.5f);
	TempPlanBottomRightComponent->SetThickness(GizmoThickness);
	TempPlanBottomRightComponent->SetSegmentFlags(0x2 | 0x4);
	TempPlanBottomRightComponent->SetupAttachment(GetRootComponent());

	PlanBottomRightComponent = TempPlanBottomRightComponent;

	// Plane bottom left
	URectGizmoComponent* TempPlanBottomLeftComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanBottomLeft"));
	TempPlanBottomLeftComponent->Color = GizmoColor;
	TempPlanBottomLeftComponent->SetOffsetX(TempPlanBottomLeftComponent->GetLengthX() * -0.5f);
	TempPlanBottomLeftComponent->SetOffsetY(TempPlanBottomLeftComponent->GetLengthY() * -0.5f);
	TempPlanBottomLeftComponent->SetThickness(GizmoThickness);
	TempPlanBottomLeftComponent->SetSegmentFlags(0x4 | 0x8);
	TempPlanBottomLeftComponent->SetupAttachment(GetRootComponent());

	PlanBottomLeftComponent = TempPlanBottomLeftComponent;

	// Axis X Rotate
	//UGizmoCircleComponent* TempRotateXComponent = CreateDefaultSubobject<UGizmoCircleComponent>(TEXT("GizmoRotateX"));
	//TempRotateXComponent->Color = FLinearColor::Red;
	//TempRotateXComponent->Normal = FVector(1.0f, 0.0f, 0.0f);
	//TempRotateXComponent->Radius = 120.0f;
	//TempRotateXComponent->SetupAttachment(GetRootComponent());

	//RotateXComponent = TempRotateXComponent;
}

TArray<UPrimitiveComponent*> ABoxGizmoActor::GetGizmoComponents()
{
	TArray<UPrimitiveComponent*> Result;
	Result.Add(ElevationComponent);
	Result.Add(PlanTopLeftComponent);
	Result.Add(PlanTopRightComponent);
	Result.Add(PlanBottomRightComponent);
	Result.Add(PlanBottomLeftComponent);

	return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetPlanCornerComponent(bool bPositiveX, bool bPositiveY)
{
	UPrimitiveComponent* CornerComponent = nullptr;
	if (bPositiveX)
	{
		if (bPositiveY)
		{
			CornerComponent = PlanBottomRightComponent;
		}
		else
		{
			CornerComponent = PlanTopRightComponent;
		}
	}
	else
	{
		if (bPositiveY)
		{
			CornerComponent = PlanBottomLeftComponent;
		}
		else
		{
			CornerComponent = PlanTopLeftComponent;
		}
	}

	return CornerComponent;
}
