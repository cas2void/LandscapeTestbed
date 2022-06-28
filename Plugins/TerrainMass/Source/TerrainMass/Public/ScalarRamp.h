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

	//
	// Texture
	//
	void SetSize(int32 TextureSize);

	UTexture2DDynamic* GetTexture();

	FRuntimeFloatCurve& GetCurve() { return Curve; }
	const FRuntimeFloatCurve& GetCurve() const { return Curve; }

	void CreateTexture();
	void WriteTexture();

protected:
	UPROPERTY(EditAnywhere, meta = (UIMin = 2, UIMax = 1024, ClampMin = 2, ClampMax = 1024))
	int32 Size = 128;

	UPROPERTY(VisibleInstanceOnly, Transient, NonTransactional, AdvancedDisplay)
	UTexture2DDynamic* Texture;

	//
	// Curve
	//
public:
	void SetDefaultCurve();

protected:
	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve Curve;

	//
	// Curve Delegates
	//
public:
#if WITH_EDITORONLY_DATA
	// Delegate called whenever the curve data is modified
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnRampModified, const FRuntimeFloatCurve& /*Curve*/, bool /*bFinished*/);
	FOnRampModified OnRampModified;
#endif
};
