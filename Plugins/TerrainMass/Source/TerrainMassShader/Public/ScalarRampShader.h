// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Curves/CurveFloat.h"
#include "Engine/Texture2DDynamic.h"

/**
 * 
 */
class TERRAINMASSSHADER_API FScalarRampShader
{
public:
	static void WaitForGPU();

	static void RenderRampToTexture(const FRuntimeFloatCurve& Curve, UTexture2DDynamic* ScalarRampTexture);
};
