// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2DDynamic.h"
#include "Curves/CurveFloat.h"
#include "ScalarRamp.generated.h"

/**
 * 
 */
USTRUCT()
struct TERRAINMASS_API FScalarRamp
{
	GENERATED_BODY()

public:
	FScalarRamp();

	void SetSize(int32 TextureSize);

	UTexture2DDynamic* GetTexture();

	FRuntimeFloatCurve& GetCurve() { return Curve; }

	const FRuntimeFloatCurve& GetCurve() const { return Curve; }

	void SetDefaultCurve();

	void CreateTexture();

	void WriteTexture();

protected:
	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve Curve;

	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 1024, ClampMin = 2, ClampMax = 1024))
	int32 Size = 128;

	UPROPERTY(VisibleInstanceOnly, Transient, NonTransactional, AdvancedDisplay)
	UTexture2DDynamic* Texture;
};
