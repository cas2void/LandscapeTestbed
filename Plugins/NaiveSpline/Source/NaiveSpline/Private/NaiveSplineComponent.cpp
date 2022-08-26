// Fill out your copyright notice in the Description page of Project Settings.

#include "NaiveSplineComponent.h"

// Sets default values for this component's properties
UNaiveSplineComponent::UNaiveSplineComponent()
{}

FPrimitiveSceneProxy* UNaiveSplineComponent::CreateSceneProxy()
{
	return nullptr;
}

FBoxSphereBounds UNaiveSplineComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return FBoxSphereBounds();
}

