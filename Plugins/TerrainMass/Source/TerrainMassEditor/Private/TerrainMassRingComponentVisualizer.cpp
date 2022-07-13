// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainMassRingComponentVisualizer.h"

#include "SceneView.h"
#include "SceneManagement.h"

#include "TerrainMassRingComponent.h"

IMPLEMENT_HIT_PROXY(HTerrainMassRingVisProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HTerrainMassRingHandleProxy, HTerrainMassRingVisProxy);

void FTerrainMassRingComponentVisualizer::OnRegister()
{
    FSplineComponentVisualizer::OnRegister();
}

void FTerrainMassRingComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    ////UE_LOG(LogTemp, Warning, TEXT("FTerrainMassRingComponentVisualizer::DrawVisualization"));

    FSplineComponentVisualizer::DrawVisualization(Component, View, PDI);
    const UTerrainMassRingComponent* RingComponent = Cast<const UTerrainMassRingComponent>(Component);
    if (RingComponent)
    {
        TArray<FDynamicMeshVertex> MeshVerts;
        TArray<uint32> MeshIndices;
        RingComponent->CreateHandleGeometry(MeshVerts, MeshIndices);

        FDynamicMeshBuilder MeshBuilder(PDI->View->GetFeatureLevel());
        MeshBuilder.AddVertices(MeshVerts);
        MeshBuilder.AddTriangles(MeshIndices);

        FMatrix EffectiveLocalToWorld = RingComponent->GetComponentTransform().ToMatrixNoScale();
        PDI->SetHitProxy(new HTerrainMassRingHandleProxy(Component));
        MeshBuilder.Draw(PDI, EffectiveLocalToWorld, GEngine->ArrowMaterial->GetRenderProxy(), SDPG_World, 0.f);
        PDI->SetHitProxy(nullptr);
    }
}

bool FTerrainMassRingComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
    bool Result = FSplineComponentVisualizer::VisProxyHandleClick(InViewportClient, VisProxy, Click);

    if (VisProxy && VisProxy->Component.IsValid())
    {
        check(SelectionState);

        if (VisProxy->IsA(HTerrainMassRingHandleProxy::StaticGetType()))
        {
            const USplineComponent* SplineComp = CastChecked<const USplineComponent>(VisProxy->Component.Get());

            AActor* OldSplineOwningActor = SelectionState->GetSplinePropertyPath().GetParentOwningActor();
            FComponentPropertyPath NewSplinePropertyPath(SplineComp);
            SelectionState->SetSplinePropertyPath(NewSplinePropertyPath);
            AActor* NewSplineOwningActor = NewSplinePropertyPath.GetParentOwningActor();

            if (NewSplinePropertyPath.IsValid())
            {
                if (OldSplineOwningActor != NewSplineOwningActor)
                {
                    // Reset selection state if we are selecting a different actor to the one previously selected
                    ChangeSelectionState(INDEX_NONE, false);
                    SelectionState->ClearSelectedSegmentIndex();
                    SelectionState->ClearSelectedTangentHandle();
                    SelectionState->ModifySelectedKeys().Reset();

                    USplineComponent* EditedSplineComp = GetEditedSplineComponent();
                    if (EditedSplineComp != nullptr)
                    {
                        SelectionState->SetCachedRotation(EditedSplineComp->GetComponentRotation().Quaternion());
                    }
                }
            }

            bHandleSelected = true;

            Result = true;
        }
        else
        {
            bHandleSelected = false;
        }
    }

    return Result;
}

bool FTerrainMassRingComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
    bool Result = false;
    if (bHandleSelected)
    {
        USplineComponent* SplineComp = GetEditedSplineComponent();
        if (SplineComp != nullptr)
        {
            check(SelectionState);
            OutLocation = SplineComp->GetComponentLocation();
            Result = true;
        }
    }
    else
    {
        Result = FSplineComponentVisualizer::GetWidgetLocation(ViewportClient, OutLocation);
    }

    return Result;
}

bool FTerrainMassRingComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
    if (bHandleSelected)
    {
        return false;
    }

    ResetTempModes();

    USplineComponent* SplineComp = GetEditedSplineComponent();
    if (SplineComp != nullptr)
    {
        if (IsAnySelectedKeyIndexOutOfRange(SplineComp))
        {
            // Something external has changed the number of spline points, meaning that the cached selected keys are no longer valid
            EndEditing();
            return false;
        }

        check(SelectionState);
        if (SelectionState->GetSelectedTangentHandle() != INDEX_NONE)
        {
            return TransformSelectedTangent(DeltaTranslate);
        }
        else
        {
            return TransformSelectedKeys(DeltaTranslate, DeltaRotate, DeltaScale);
        }
    }

    return false;
}

void FTerrainMassRingComponentVisualizer::EndEditing()
{
    FSplineComponentVisualizer::EndEditing();

    bHandleSelected = false;
}
