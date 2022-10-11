// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PrimitiveGizmoBaseComponent.h"

#include "PrimitiveGizmoRotateComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UPrimitiveGizmoRotateComponent : public UPrimitiveGizmoBaseComponent
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
	int32 GetAxisIndex() const { return AxisIndex; }
	void SetAxisIndex(int32 InAxisIndex) { AxisIndex = InAxisIndex; }
	FVector GetNormal() const;

	FVector GetExtent() const { return Extent; }
	void SetExtent(const FVector& InExtent) { Extent = InExtent; }

	float GetGap() const { return Gap; }
	void SetGap(float InGap) { Gap = InGap; }

	float GetRadius() const { return Radius; }
	void SetRadius(float InRadius) { Radius = InRadius; }

	int32 GetNumSides() const { return NumSides; }
	void SetNumSides(int32 InNumSides) { NumSides = InNumSides; }
	
	float GetThickness() const { return Thickness; }
	void SetThickness(float InThickness) { Thickness = InThickness; }

protected:
	UPROPERTY(EditAnywhere, Category = Options)
	int32 AxisIndex = 0;

	UPROPERTY(EditAnywhere, Category = Options)
	FVector Extent = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = Options)
	float Gap = 0.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	int32 NumSides = 64;

	UPROPERTY(EditAnywhere, Category = Options)
	float Thickness = 2.0f;

	UPROPERTY(Transient, NonTransactional)
	bool bRenderVisibility = true;
};
