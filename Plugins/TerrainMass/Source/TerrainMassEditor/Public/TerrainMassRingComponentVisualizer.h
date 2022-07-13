// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineComponentVisualizer.h"

struct HTerrainMassRingVisProxy : public HComponentVisProxy
{
    DECLARE_HIT_PROXY();

    HTerrainMassRingVisProxy(const UActorComponent* InComponent)
        : HComponentVisProxy(InComponent)
    {}
};

struct HTerrainMassRingHandleProxy : public HTerrainMassRingVisProxy
{
    DECLARE_HIT_PROXY();

    HTerrainMassRingHandleProxy(const UActorComponent* InComponent)
        : HTerrainMassRingVisProxy(InComponent)
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

    virtual void EndEditing() override;

    bool bHandleSelected;
};
