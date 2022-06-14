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
	static UTexture2DDynamic* CreateTexture(int32 Size);

	void WriteTexture(UTexture2DDynamic* OutTexture);

	UPROPERTY(EditAnywhere, AdvancedDisplay)
	FRuntimeFloatCurve Curve;
};
