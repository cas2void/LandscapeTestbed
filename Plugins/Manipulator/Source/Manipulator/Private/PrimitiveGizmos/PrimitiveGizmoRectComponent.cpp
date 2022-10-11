// Fill out your copyright notice in the Description page of Project Settings.

#include "PrimitiveGizmoRectComponent.h"

#include "PrimitiveSceneProxy.h"
#include "BaseGizmos/GizmoRenderingUtil.h"

class FPrimitiveGizmoRectComponentSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FPrimitiveGizmoRectComponentSceneProxy(const UPrimitiveGizmoRectComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent),
		Color(InComponent->Color),
		HoverThicknessMultiplier(InComponent->HoverSizeMultiplier),
		DirectionX(InComponent->GetDirectionX()),
		DirectionY(InComponent->GetDirectionY()),
		OffsetX(InComponent->GetOffsetX()),
		OffsetY(InComponent->GetOffsetY()),
		LengthX(InComponent->GetLengthX()),
		LengthY(InComponent->GetLengthY()),
		Thickness(InComponent->GetThickness()),
		SegmentFlags(InComponent->GetSegmentFlags())
	{
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		// try to find focused scene view. May return nullptr.
		const FSceneView* FocusedView = GizmoRenderingUtil::FindFocusedEditorSceneView(Views, ViewFamily, VisibilityMap);

		const FMatrix& LocalToWorldMatrix = GetLocalToWorld();
		FVector Origin = LocalToWorldMatrix.TransformPosition(FVector::ZeroVector);

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				bool bIsFocusedView = (FocusedView != nullptr && View == FocusedView);
				bool bIsOrtho = !View->IsPerspectiveProjection();

				// direction to origin of gizmo
				FVector ViewDirection =
					(bIsOrtho) ? (View->GetViewDirection()) : (Origin - View->ViewLocation);
				ViewDirection.Normalize();

				bool bWorldAxis = (bExternalWorldLocalState) ? (*bExternalWorldLocalState) : false;
				FVector UseDirectionX = (bWorldAxis) ? DirectionX : FVector{ LocalToWorldMatrix.TransformVector(DirectionX) };
				FVector UseDirectionY = (bWorldAxis) ? DirectionY : FVector{ LocalToWorldMatrix.TransformVector(DirectionY) };

				FVector PlaneNormal = FVector::CrossProduct(UseDirectionX, UseDirectionY);
				bool bRenderVisibility = FMath::Abs(FVector::DotProduct(PlaneNormal, ViewDirection)) > 0.25f;
				if (bIsFocusedView && bExternalRenderVisibility != nullptr)
				{
					*bExternalRenderVisibility = bRenderVisibility;
				}
				if (bRenderVisibility == false)
				{
					continue;
				}

				float PixelToWorldScale = GizmoRenderingUtil::CalculateLocalPixelToWorldScale(View, Origin);
				float LengthScale = PixelToWorldScale;
				if (bIsFocusedView && ExternalDynamicPixelToWorldScale != nullptr)
				{
					*ExternalDynamicPixelToWorldScale = PixelToWorldScale;
				}


				float UseThickness = (bExternalHoverState != nullptr && *bExternalHoverState == true) ?
					(HoverThicknessMultiplier * Thickness) : (Thickness);
				if (!bIsOrtho)
				{
					UseThickness *= (View->FOV / 90.0);		// compensate for FOV scaling in Gizmos...
				}

				double UseOffsetX = LengthScale * OffsetX;
				double UseOffsetLengthX = LengthScale * (OffsetX + LengthX);
				double UseOffsetY = LengthScale * OffsetY;
				double UseOffsetLengthY = LengthScale * (OffsetY + LengthY);

				FVector Point00 = Origin + UseOffsetX * UseDirectionX + UseOffsetY * UseDirectionY;
				FVector Point10 = Origin + UseOffsetLengthX * UseDirectionX + UseOffsetY * UseDirectionY;
				FVector Point11 = Origin + UseOffsetLengthX * UseDirectionX + UseOffsetLengthY * UseDirectionY;
				FVector Point01 = Origin + UseOffsetX * UseDirectionX + UseOffsetLengthY * UseDirectionY;

				if (SegmentFlags & 0x1)
				{
					PDI->DrawLine(Point00, Point10, Color, SDPG_Foreground, UseThickness, 0.0f, true);
				}
				if (SegmentFlags & 0x2)
				{
					PDI->DrawLine(Point10, Point11, Color, SDPG_Foreground, UseThickness, 0.0f, true);
				}
				if (SegmentFlags & 0x4)
				{
					PDI->DrawLine(Point11, Point01, Color, SDPG_Foreground, UseThickness, 0.0f, true);
				}
				if (SegmentFlags & 0x8)
				{
					PDI->DrawLine(Point01, Point00, Color, SDPG_Foreground, UseThickness, 0.0f, true);
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = false;
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();

		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return false;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof(*this) + GetAllocatedSize(); }

	void SetExternalHoverState(bool* HoverState)
	{
		bExternalHoverState = HoverState;
	}

	void SetExternalWorldLocalState(bool* bWorldLocalState)
	{
		bExternalWorldLocalState = bWorldLocalState;
	}

	void SetExternalInteractionState(bool* InteractionState)
	{
		bExternalInteractionState = InteractionState;
	}

	void SetExternalDynamicPixelToWorldScale(float* DynamicPixelToWorldScale)
	{
		ExternalDynamicPixelToWorldScale = DynamicPixelToWorldScale;
	}

	void SetExternalRenderVisibility(bool* bRenderVisibility)
	{
		bExternalRenderVisibility = bRenderVisibility;
	}

private:
	FLinearColor Color;
	float HoverThicknessMultiplier;
	FVector DirectionX, DirectionY;
	float OffsetX, OffsetY;
	float LengthX, LengthY;
	float Thickness;
	uint8 SegmentFlags;

	// set on Component for use in ::GetDynamicMeshElements()
	bool* bExternalHoverState = nullptr;
	bool* bExternalWorldLocalState = nullptr;
	bool* bExternalInteractionState = nullptr;

	// set in ::GetDynamicMeshElements() for use by Component hit testing
	float* ExternalDynamicPixelToWorldScale = nullptr;
	bool* bExternalRenderVisibility = nullptr;
};


FPrimitiveSceneProxy* UPrimitiveGizmoRectComponent::CreateSceneProxy()
{
	FPrimitiveGizmoRectComponentSceneProxy* NewProxy = new FPrimitiveGizmoRectComponentSceneProxy(this);
	NewProxy->SetExternalHoverState(&bHovering);
	NewProxy->SetExternalWorldLocalState(&bWorld);
	NewProxy->SetExternalInteractionState(&bInteracting);
	NewProxy->SetExternalDynamicPixelToWorldScale(&DynamicPixelToWorldScale);
	NewProxy->SetExternalRenderVisibility(&bRenderVisibility);
	return NewProxy;
}

bool UPrimitiveGizmoRectComponent::LineTraceComponent(FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionQueryParams& Params)
{
	if (bRenderVisibility == false)
	{
		return false;
	}

	const FTransform& Transform = this->GetComponentToWorld();

	FVector UseDirectionX = DirectionX;
	UseDirectionX = (bWorld) ? UseDirectionX : Transform.TransformVector(UseDirectionX);
	FVector UseDirectionY = DirectionY;
	UseDirectionY = (bWorld) ? UseDirectionY : Transform.TransformVector(UseDirectionY);
	FVector UseOrigin = Transform.TransformPosition(FVector::ZeroVector);

	float LengthScale = DynamicPixelToWorldScale;
	double UseOffsetX = LengthScale * OffsetX;
	double UseOffsetLengthX = LengthScale * (OffsetX + LengthX);
	double UseOffsetY = LengthScale * OffsetY;
	double UseOffsetLengthY = LengthScale * (OffsetY + LengthY);

	FVector Points[4] = {
		UseOrigin + UseOffsetX * UseDirectionX + UseOffsetY * UseDirectionY,
		UseOrigin + UseOffsetLengthX * UseDirectionX + UseOffsetY * UseDirectionY,
		UseOrigin + UseOffsetLengthX * UseDirectionX + UseOffsetLengthY * UseDirectionY,
		UseOrigin + UseOffsetX * UseDirectionX + UseOffsetLengthY * UseDirectionY
	};
	static const int Triangles[2][3] = { {0,1,2}, {0,2,3} };

	for (int j = 0; j < 2; ++j)
	{
		const int* Triangle = Triangles[j];
		FVector HitPoint, HitNormal;
		if (FMath::SegmentTriangleIntersection(Start, End,
			Points[Triangle[0]], Points[Triangle[1]], Points[Triangle[2]],
			HitPoint, HitNormal))
		{
			OutHit.Component = this;
			OutHit.Distance = FVector::Distance(Start, HitPoint);
			OutHit.ImpactPoint = HitPoint;
			OutHit.ImpactNormal = HitNormal;
			return true;
		}
	}

	return false;
}

FBoxSphereBounds UPrimitiveGizmoRectComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	float MaxOffset = FMath::Max(FMath::Abs(OffsetX), FMath::Abs(OffsetY));
	float MaxLength = FMath::Max(FMath::Abs(LengthX), FMath::Abs(LengthY));
	return FBoxSphereBounds(FSphere(FVector::ZeroVector, MaxOffset + MaxLength).TransformBy(LocalToWorld));
}
