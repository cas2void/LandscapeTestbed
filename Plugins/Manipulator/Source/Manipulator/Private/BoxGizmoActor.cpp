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

	// Elevation
	UGizmoCircleComponent* TempElevationComponent = CreateDefaultSubobject<UGizmoCircleComponent>(TEXT("GizmoElevation"));
	TempElevationComponent->Color = GizmoColor;
	TempElevationComponent->Radius = 10.0f;
	TempElevationComponent->Thickness = 10.0f;
	TempElevationComponent->NumSides = 16;
	TempElevationComponent->bViewAligned = true;
	TempElevationComponent->bOnlyAllowFrontFacingHits = false;
	TempElevationComponent->SetupAttachment(GetRootComponent());

	ElevationComponent = TempElevationComponent;

	// Plane top left
	URectGizmoComponent* TempPlanTopLeftComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanTopLeft"));
	TempPlanTopLeftComponent->Color = GizmoColor;
	//PlanTopLeftComponent->OffsetX = 140.0f;
	//PlanTopLeftComponent->OffsetY = -10.0f;
	//PlanTopLeftComponent->LengthX = 7.0f;
	//PlanTopLeftComponent->LengthY = 20.0f;
	TempPlanTopLeftComponent->SetThickness(10.0f);
	TempPlanTopLeftComponent->SetSegmentFlags(0x1 | 0x8);
	TempPlanTopLeftComponent->SetupAttachment(GetRootComponent());

	PlanTopLeftComponent = TempPlanTopLeftComponent;

	// Plane top right
	URectGizmoComponent* TempPlanTopRightComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanTopRight"));
	TempPlanTopRightComponent->Color = GizmoColor;
	TempPlanTopRightComponent->SetThickness(10.0f);
	TempPlanTopRightComponent->SetSegmentFlags(0x1 | 0x2);
	TempPlanTopRightComponent->SetupAttachment(GetRootComponent());

	PlanTopRightComponent = TempPlanTopRightComponent;

	// Plane bottom right
	URectGizmoComponent* TempPlanBottomRightComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanBottomRight"));
	TempPlanBottomRightComponent->Color = GizmoColor;
	TempPlanBottomRightComponent->SetThickness(10.0f);
	TempPlanBottomRightComponent->SetSegmentFlags(0x2 | 0x4);
	TempPlanBottomRightComponent->SetupAttachment(GetRootComponent());

	PlanBottomRightComponent = TempPlanBottomRightComponent;

	// Plane bottom left
	URectGizmoComponent* TempPlanBottomLeftComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlanBottomLeft"));
	TempPlanBottomLeftComponent->Color = GizmoColor;
	TempPlanBottomLeftComponent->SetThickness(10.0f);
	TempPlanBottomLeftComponent->SetSegmentFlags(0x4 | 0x8);
	TempPlanBottomLeftComponent->SetupAttachment(GetRootComponent());

	PlanBottomLeftComponent = TempPlanBottomLeftComponent;
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
