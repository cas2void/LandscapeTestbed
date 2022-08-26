// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"

#include "NaiveSplineComponent.h"

/**
 * 
 */
class NAIVESPLINE_API FNaiveSplineSceneProxy : public FPrimitiveSceneProxy
{
public:
	FNaiveSplineSceneProxy(UNaiveSplineComponent* Component);
	~FNaiveSplineSceneProxy();

	//
	// FPrimitiveSceneProxy Interfaces
	//
public:
	virtual SIZE_T GetTypeHash() const override;
	virtual uint32 GetMemoryFootprint(void) const override;
};
