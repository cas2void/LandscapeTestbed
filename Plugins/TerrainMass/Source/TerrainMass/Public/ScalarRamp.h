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
	void SetSize(int32 TextureSize);

	void CreateTexture();

	void WriteTexture();

	UTexture2DDynamic* GetTexture();

	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve Curve;

protected:
	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 1024, ClampMin = 2, ClampMax = 1024))
	int32 Size = 512;

	UPROPERTY(VisibleInstanceOnly, Transient, NonTransactional, AdvancedDisplay)
	UTexture2DDynamic* Texture;
};
