// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineComponentVisualizer.h"

struct HTerrainMassDummyVisProxy : public HComponentVisProxy
{
    DECLARE_HIT_PROXY();

    HTerrainMassDummyVisProxy(const UActorComponent* InComponent)
        : HComponentVisProxy(InComponent)
    {}
};

struct HTerrainMassDummyHandleProxy : public HTerrainMassDummyVisProxy
{
    DECLARE_HIT_PROXY();

    HTerrainMassDummyHandleProxy(const UActorComponent* InComponent)
        : HTerrainMassDummyVisProxy(InComponent)
    {}
};

/**
 * 
 */
class TERRAINMASSEDITOR_API FTerrainMassRingComponentVisualizer : public FSplineComponentVisualizer
{
    //
    // FComponentVisualizer Interfaces
    //
public:
    virtual void OnRegister() override;
    
    virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

    virtual bool VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click) override;

    virtual bool GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;

    virtual bool HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale) override;
};
