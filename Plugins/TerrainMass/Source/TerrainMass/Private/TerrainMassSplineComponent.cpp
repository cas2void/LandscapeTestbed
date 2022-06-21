// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainMassSplineComponent.h"

void UTerrainMassSplineComponent::UpdateSpline()
{
    Super::UpdateSpline();

    TerrainMassSplineUpdatedDelegate.Broadcast();
}
