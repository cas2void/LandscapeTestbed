// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PrimitiveGizmoBaseComponent.h"

#include "PrimitiveGizmoCircleComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UPrimitiveGizmoCircleComponent : public UPrimitiveGizmoBaseComponent
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

	bool IsAutoScaleRadius() const { return bAutoScaleRadius; }
	void SetAudoScaleRadius(bool bValue) { bAutoScaleRadius = bValue; }

	FVector GetCenterOffset() const { return CenterOffset; }
	void SetCenterOffset(const FVector& InCenterOffset) { CenterOffset = InCenterOffset; }

	float GetResolution() const { return Resolution; }
	void SetResolution(float InResolution) { Resolution = InResolution; }

	float GetStartAngle() const { return StartAngle; }
	void SetStartAngle(float InStartAngle) { StartAngle = InStartAngle; }

	float GetEndAngle() const { return EndAngle; }
	void SetEndAngle(float InEndAngle) { EndAngle = InEndAngle; }

	bool IsCullFace() const { return bCullFace; }
	void SetCullFace(bool bValue) { bCullFace = bValue; }

	FVector GetCullFaceNormal() const { return CullFaceNormal; }
	void SetCullFaceNormal(const FVector& InCullFaceNormal) { CullFaceNormal = InCullFaceNormal; }
	
	float GetThickness() const { return Thickness; }
	void SetThickness(float InThickness) { Thickness = InThickness; }

	bool IsViewAligned() const { return bViewAligned; }
	void SetViewAligned(bool InViewAligned) { bViewAligned = InViewAligned; }

protected:
	UPROPERTY(EditAnywhere, Category = Options)
	FVector Normal = FVector::ZAxisVector;

	UPROPERTY(EditAnywhere, Category = Options)
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	bool bAutoScaleRadius = false;

	UPROPERTY(EditAnywhere, Category = Options)
	FVector CenterOffset = FVector::ZeroVector;

	// Resolution in degrees
	UPROPERTY(EditAnywhere, Category = Options)
	float Resolution = 10.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float StartAngle = 0.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float EndAngle = 360.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	bool bCullFace = false;

	UPROPERTY(EditAnywhere, Category = Options)
	FVector CullFaceNormal = FVector::ZAxisVector;

	UPROPERTY(EditAnywhere, Category = Options)
	int32 NumSides = 64;

	UPROPERTY(EditAnywhere, Category = Options)
	float Thickness = 2.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	bool bViewAligned = false;

	UPROPERTY(Transient, NonTransactional)
	bool bRenderVisibility = true;
};
