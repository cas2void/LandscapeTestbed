// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainMassRingComponentVisualizer.h"

#include "SceneView.h"
#include "SceneManagement.h"

#include "TerrainMassRingComponent.h"

IMPLEMENT_HIT_PROXY(HTerrainMassDummyVisProxy, HComponentVisProxy);
IMPLEMENT_HIT_PROXY(HTerrainMassDummyHandleProxy, HTerrainMassDummyVisProxy);

void FTerrainMassRingComponentVisualizer::OnRegister()
{
	FSplineComponentVisualizer::OnRegister();
}

void FTerrainMassRingComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    ////UE_LOG(LogTemp, Warning, TEXT("FTerrainMassRingComponentVisualizer::DrawVisualization"));
    //const UTerrainMassRingComponent* DummyComponent = Cast<const UTerrainMassRingComponent>(Component);
    //if (DummyComponent)
    //{
    //    const FMatrix Local2World = DummyComponent->GetComponentTransform().ToMatrixNoScale();

    //    float Angle = FMath::DegreesToRadians(20.0f);

    //    TArray<FDynamicMeshVertex> MeshVerts;
    //    TArray<uint32> MeshIndices;
    //    BuildConeVerts(Angle, Angle, 100.f, 0.f, 32, MeshVerts, MeshIndices);

    //    FDynamicMeshBuilder MeshBuilder(PDI->View->GetFeatureLevel());
    //    MeshBuilder.AddVertices(MeshVerts);
    //    MeshBuilder.AddTriangles(MeshIndices);

    //    PDI->SetHitProxy(new HTerrainMassDummyHandleProxy(Component));
    //    MeshBuilder.Draw(PDI, Local2World, GEngine->ArrowMaterial->GetRenderProxy(), SDPG_World, 0.f);
    //    PDI->SetHitProxy(nullptr);
    //}

    FSplineComponentVisualizer::DrawVisualization(Component, View, PDI);
}

bool FTerrainMassRingComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
    return FSplineComponentVisualizer::VisProxyHandleClick(InViewportClient, VisProxy, Click);
}

bool FTerrainMassRingComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
    return FSplineComponentVisualizer::GetWidgetLocation(ViewportClient, OutLocation);
}

bool FTerrainMassRingComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltaRotate, FVector& DeltaScale)
{
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
		//else if (ViewportClient->IsAltPressed())
		//{
		//	if (ViewportClient->GetWidgetMode() == FWidget::WM_Translate && ViewportClient->GetCurrentWidgetAxis() != EAxisList::None && SelectionState->GetSelectedKeys().Num() == 1)
		//	{
		//		static const int MaxDuplicationDelay = 3;

		//		FVector Drag = DeltaTranslate;

		//		if (bAllowDuplication)
		//		{
		//			if (DuplicateDelay < MaxDuplicationDelay)
		//			{
		//				DuplicateDelay++;
		//				DuplicateDelayAccumulatedDrag += DeltaTranslate;
		//			}
		//			else
		//			{
		//				Drag += DuplicateDelayAccumulatedDrag;
		//				DuplicateDelayAccumulatedDrag = FVector::ZeroVector;

		//				bAllowDuplication = false;
		//				bDuplicatingSplineKey = true;

		//				DuplicateKeyForAltDrag(Drag);
		//			}
		//		}
		//		else
		//		{
		//			UpdateDuplicateKeyForAltDrag(Drag);
		//		}

		//		return true;
		//	}
		//}
		else
		{
			return TransformSelectedKeys(DeltaTranslate, DeltaRotate, DeltaScale);
		}
	}

	return false;
}
