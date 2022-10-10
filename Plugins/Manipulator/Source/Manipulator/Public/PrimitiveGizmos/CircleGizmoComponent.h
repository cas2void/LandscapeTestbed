// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "CircleGizmoComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UCircleGizmoComponent : public UGizmoBaseComponent
{
	GENERATED_BODY()

	// UPrimitiveComponent Interfaces
protected:
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual bool LineTraceComponent(FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionQueryParams& Params) override;

	//
	// USceneComponent Interfaces
	//
protected:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	//
	// Gizmo
	//
public:
	FVector GetNormal() const { return Normal; }
	void SetNormal(const FVector& InNormal) { Normal = InNormal; }

	float GetRadius() const { return Radius; }
	void SetRadius(float InRadius) { Radius = InRadius; }

	FVector GetCenterOffset() const { return CenterOffset; }
	void SetCenterOffset(const FVector& InCenterOffset) { CenterOffset = InCenterOffset; }

	int32 GetNumSides() const { return NumSides; }
	void SetNumSides(int32 InNumSides) { NumSides = InNumSides; }
	
	float GetThickness() const { return Thickness; }
	void SetThickness(float InThickness) { Thickness = InThickness; }

	bool IsViewAligned() const { return bViewAligned; }
	void SetViewAligned(bool InViewAligned) { bViewAligned = InViewAligned; }

protected:
	UPROPERTY(EditAnywhere, Category = Options)
	FVector Normal = FVector(1, 0, 0);

	UPROPERTY(EditAnywhere, Category = Options)
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	FVector CenterOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = Options)
	int32 NumSides = 64;

	UPROPERTY(EditAnywhere, Category = Options)
	float Thickness = 2.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	bool bViewAligned = false;

	UPROPERTY(Transient, NonTransactional)
	bool bRenderVisibility = true;
};
