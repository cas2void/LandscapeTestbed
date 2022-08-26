// Fill out your copyright notice in the Description page of Project Settings.

#include "NaiveSplineSceneProxy.h"

FNaiveSplineSceneProxy::FNaiveSplineSceneProxy(UNaiveSplineComponent* Component)
	: FPrimitiveSceneProxy(Component)
{
}

FNaiveSplineSceneProxy::~FNaiveSplineSceneProxy()
{
}

SIZE_T FNaiveSplineSceneProxy::GetTypeHash() const
{
	return SIZE_T();
}

uint32 FNaiveSplineSceneProxy::GetMemoryFootprint(void) const
{
	return uint32();
}
