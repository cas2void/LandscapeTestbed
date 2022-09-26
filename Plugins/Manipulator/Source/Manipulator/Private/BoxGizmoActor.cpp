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
	URectGizmoComponent* TempPlaneTopLeftComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlaneTopLeft"));
	TempPlaneTopLeftComponent->Color = GizmoColor;
	//PlaneTopLeftComponent->OffsetX = 140.0f;
	//PlaneTopLeftComponent->OffsetY = -10.0f;
	//PlaneTopLeftComponent->LengthX = 7.0f;
	//PlaneTopLeftComponent->LengthY = 20.0f;
	TempPlaneTopLeftComponent->SetThickness(10.0f);
	TempPlaneTopLeftComponent->SetSegmentFlags(0x1 | 0x8);
	TempPlaneTopLeftComponent->SetupAttachment(GetRootComponent());

	PlaneTopLeftComponent = TempPlaneTopLeftComponent;

	// Plane top right
	URectGizmoComponent* TempPlaneTopRightComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlaneTopRight"));
	TempPlaneTopRightComponent->Color = GizmoColor;
	TempPlaneTopRightComponent->SetThickness(10.0f);
	TempPlaneTopRightComponent->SetSegmentFlags(0x1 | 0x2);
	TempPlaneTopRightComponent->SetupAttachment(GetRootComponent());

	PlaneTopRightComponent = TempPlaneTopRightComponent;

	// Plane bottom right
	URectGizmoComponent* TempPlaneBottomRightComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlaneBottomRight"));
	TempPlaneBottomRightComponent->Color = GizmoColor;
	TempPlaneBottomRightComponent->SetThickness(10.0f);
	TempPlaneBottomRightComponent->SetSegmentFlags(0x2 | 0x4);
	TempPlaneBottomRightComponent->SetupAttachment(GetRootComponent());

	PlaneBottomRightComponent = TempPlaneBottomRightComponent;

	// Plane bottom left
	URectGizmoComponent* TempPlaneBottomLeftComponent = CreateDefaultSubobject<URectGizmoComponent>(TEXT("GizmoPlaneBottomLeft"));
	TempPlaneBottomLeftComponent->Color = GizmoColor;
	TempPlaneBottomLeftComponent->SetThickness(10.0f);
	TempPlaneBottomLeftComponent->SetSegmentFlags(0x4 | 0x8);
	TempPlaneBottomLeftComponent->SetupAttachment(GetRootComponent());

	PlaneBottomLeftComponent = TempPlaneBottomLeftComponent;
}
