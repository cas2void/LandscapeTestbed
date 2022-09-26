// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "RectGizmoComponent.generated.h"

/**
 * 
 */
UCLASS()
class MANIPULATOR_API URectGizmoComponent : public UGizmoBaseComponent
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
	FVector GetDirectionX() const { return DirectionX; }
	FVector GetDirectionY() const { return DirectionY; }
	float GetOffsetX() const { return OffsetX; }
	float GetOffsetY() const { return OffsetY; }

	float GetLengthX() const { return LengthX; }
	void SetLengthX(float InLengthX) { LengthX = InLengthX; }

	float GetLengthY() const { return LengthY; }
	void SetLengthY(float InLengthY) { LengthY = InLengthY; }
	
	float GetThickness() const { return Thickness; }
	void SetThickness(float InThickness) { Thickness = InThickness; }

	uint8 GetSegmentFlags() const { return SegmentFlags; }
	void SetSegmentFlags(uint8 InSegmentFlags) { SegmentFlags = InSegmentFlags; }

protected:
	UPROPERTY(EditAnywhere, Category = Options)
	FVector DirectionX = FVector(1, 0, 0);

	UPROPERTY(EditAnywhere, Category = Options)
	FVector DirectionY = FVector(0, 1, 0);

	UPROPERTY(EditAnywhere, Category = Options)
	float OffsetX = 0.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float OffsetY = 0.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float LengthX = 20.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float LengthY = 20.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	float Thickness = 2.0f;

	UPROPERTY(EditAnywhere, Category = Options)
	uint8 SegmentFlags = 0x1 | 0x2 | 0x4 | 0x8;

	UPROPERTY(Transient, NonTransactional)
	bool bRenderVisibility = true;
};
