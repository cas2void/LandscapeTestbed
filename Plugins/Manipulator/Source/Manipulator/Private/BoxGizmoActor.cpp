// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmoActor.h"

#include "Components/SphereComponent.h"
#include "BaseGizmos/GizmoRectangleComponent.h"
#include "BaseGizmos/GizmoCircleComponent.h"

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
	UGizmoRectangleComponent* TempPlaneTopLeftComponent = CreateDefaultSubobject<UGizmoRectangleComponent>(TEXT("GizmoPlaneTopLeft"));
	TempPlaneTopLeftComponent->DirectionX = FVector(1, 0, 0);
	TempPlaneTopLeftComponent->DirectionY = FVector(0, 1, 0);
	TempPlaneTopLeftComponent->Color = GizmoColor;
	//PlaneTopLeftComponent->OffsetX = 140.0f;
	//PlaneTopLeftComponent->OffsetY = -10.0f;
	//PlaneTopLeftComponent->LengthX = 7.0f;
	//PlaneTopLeftComponent->LengthY = 20.0f;
	TempPlaneTopLeftComponent->Thickness = 10.0f;
	TempPlaneTopLeftComponent->SegmentFlags = 0x1 | 0x8;
	TempPlaneTopLeftComponent->SetupAttachment(GetRootComponent());

	PlaneTopLeftComponent = TempPlaneTopLeftComponent;

	// Plane top right
	UGizmoRectangleComponent* TempPlaneTopRightComponent = CreateDefaultSubobject<UGizmoRectangleComponent>(TEXT("GizmoPlaneTopRight"));
	TempPlaneTopRightComponent->DirectionX = FVector(1, 0, 0);
	TempPlaneTopRightComponent->DirectionY = FVector(0, 1, 0);
	TempPlaneTopRightComponent->Color = GizmoColor;
	TempPlaneTopRightComponent->Thickness = 10.0f;
	TempPlaneTopRightComponent->SegmentFlags = 0x1 | 0x2;
	TempPlaneTopRightComponent->SetupAttachment(GetRootComponent());

	PlaneTopRightComponent = TempPlaneTopRightComponent;

	// Plane bottom right
	UGizmoRectangleComponent* TempPlaneBottomRightComponent = CreateDefaultSubobject<UGizmoRectangleComponent>(TEXT("GizmoPlaneBottomRight"));
	TempPlaneBottomRightComponent->DirectionX = FVector(1, 0, 0);
	TempPlaneBottomRightComponent->DirectionY = FVector(0, 1, 0);
	TempPlaneBottomRightComponent->Color = GizmoColor;
	TempPlaneBottomRightComponent->Thickness = 10.0f;
	TempPlaneBottomRightComponent->SegmentFlags = 0x2 | 0x4;
	TempPlaneBottomRightComponent->SetupAttachment(GetRootComponent());

	PlaneBottomRightComponent = TempPlaneBottomRightComponent;

	// Plane bottom left
	UGizmoRectangleComponent* TempPlaneBottomLeftComponent = CreateDefaultSubobject<UGizmoRectangleComponent>(TEXT("GizmoPlaneBottomLeft"));
	TempPlaneBottomLeftComponent->DirectionX = FVector(1, 0, 0);
	TempPlaneBottomLeftComponent->DirectionY = FVector(0, 1, 0);
	TempPlaneBottomLeftComponent->Color = GizmoColor;
	TempPlaneBottomLeftComponent->Thickness = 10.0f;
	TempPlaneBottomLeftComponent->SegmentFlags = 0x4 | 0x8;
	TempPlaneBottomLeftComponent->SetupAttachment(GetRootComponent());

	PlaneBottomLeftComponent = TempPlaneBottomLeftComponent;
}
