// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PrimitiveGizmoBaseComponent.h"

#include "PrimitiveGizmoArrowComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API UPrimitiveGizmoArrowComponent : public UPrimitiveGizmoBaseComponent
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
	FVector GetDirection() const { return Direction; }
	void SetDirection(const FVector& InDirection) { Direction = InDirection; }

	float GetGap() const { return Gap; }
	void SetGap(float InGap) { Gap = InGap; }

	float GetLength() const { return Length; }
	void SetLength(float InLength) { Length = InLength; }
	
	float GetThickness() const { return Thickness; }
	void SetThickness(float InThickness) { Thickness = InThickness; }

protected:
	UPROPERTY(EditAnywhere, Category = Options)
	FVector Direction = FVector(1, 0, 0);

	UPROPERTY(EditAnywhere, Category = Options)
	float Gap = 5.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float Length = 60.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float Thickness = 2.0f;

	UPROPERTY(Transient, NonTransactional)
	bool bRenderVisibility = true;
};
