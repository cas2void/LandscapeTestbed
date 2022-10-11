// Fill out your copyright notice in the Description page of Project Settings.

#include "PrimitiveGizmoRotateComponent.h"

#include "PrimitiveSceneProxy.h"
#include "BaseGizmos/GizmoRenderingUtil.h"
#include "BaseGizmos/GizmoMath.h"

class FPrimitiveGizmoRotateComponentSceneProxy final : public FPrimitiveSceneProxy
{
public:
    SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

    FPrimitiveGizmoRotateComponentSceneProxy(const UPrimitiveGizmoRotateComponent* InComponent)
        : FPrimitiveSceneProxy(InComponent),
        Color(InComponent->Color),
        HoverThicknessMultiplier(InComponent->HoverSizeMultiplier),
        AxisIndex(InComponent->GetAxisIndex()),
        Normal(InComponent->GetNormal()),
        Gap(InComponent->GetGap()),
        Radius(InComponent->GetRadius()),
        NumSides(InComponent->GetNumSides()),
        Thickness(InComponent->GetThickness())
    {
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
    {
        // try to find focused scene view. May return nullptr.
        const FSceneView* FocusedView = GizmoRenderingUtil::FindFocusedEditorSceneView(Views, ViewFamily, VisibilityMap);

        const FMatrix& LocalToWorldMatrix = GetLocalToWorld();
        FVector Origin = LocalToWorldMatrix.TransformPosition(FVector::ZeroVector);
        FVector PlaneX, PlaneY;
        GizmoMath::MakeNormalPlaneBasis(Normal, PlaneX, PlaneY);

        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
        {
            if (VisibilityMap & (1 << ViewIndex))
            {
                const FSceneView* View = Views[ViewIndex];
                FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
                bool bIsFocusedView = (FocusedView != nullptr && View == FocusedView);
                bool bIsOrtho = !View->IsPerspectiveProjection();
                FVector ViewVector = View->GetViewDirection();

                // direction to origin of gizmo
                FVector GizmoViewDirection = (bIsOrtho) ? (View->GetViewDirection()) : (Origin - View->ViewLocation);
                GizmoViewDirection.Normalize();

                float PixelToWorldScale = GizmoRenderingUtil::CalculateLocalPixelToWorldScale(View, Origin);
                float LengthScale = PixelToWorldScale;
                if (bIsFocusedView && ExternalDynamicPixelToWorldScale != nullptr)
                {
                    *ExternalDynamicPixelToWorldScale = PixelToWorldScale;
                }

                double UseRadius = LengthScale * Radius;

                FLinearColor BackColor = FLinearColor(0.5f, 0.5f, 0.5f);
                float BackThickness = 0.5f;
                float UseThickness = (bExternalHoverState != nullptr && *bExternalHoverState == true) ? (HoverThicknessMultiplier * Thickness) : (Thickness);
                if (!bIsOrtho)
                {
                    UseThickness *= (View->FOV / 90.0);		// compensate for FOV scaling in Gizmos...
                    BackThickness *= (View->FOV / 90.0);		// compensate for FOV scaling in Gizmos...
                }

                const float	AngleDelta = 2.0f * PI / NumSides;

                FVector Offset = FVector::ZeroVector;
                if (ExternalExtent)
                {
                    switch (AxisIndex)
                    {
                    case 0:
                        Offset.X = ExternalExtent->X;
                        Offset.Z = ExternalExtent->Z;
                        break;
                    case 1:
                        Offset.X = ExternalExtent->X;
                        Offset.Z = ExternalExtent->Z;
                    default:
                        break;
                    }
                }

                FVector WorldOrigin = LocalToWorldMatrix.TransformPosition(FVector::ZeroVector + Offset);
                bool bWorldAxis = (bExternalWorldLocalState) ? (*bExternalWorldLocalState) : false;
                FVector WorldPlaneX = (bWorldAxis) ? PlaneX : FVector{ LocalToWorldMatrix.TransformVector(PlaneX) };
                FVector WorldPlaneY = (bWorldAxis) ? PlaneY : FVector{ LocalToWorldMatrix.TransformVector(PlaneY) };

                FVector PlaneWorldNormal = (bWorldAxis) ? Normal : FVector{ LocalToWorldMatrix.TransformVector(Normal) };
                double ViewDot = FVector::DotProduct(GizmoViewDirection, PlaneWorldNormal);
                bool bOnEdge = FMath::Abs(ViewDot) < 0.05;
                bool bIsViewPlaneParallel = FMath::Abs(ViewDot) > 0.95;

                bool bRenderVisibility = !bOnEdge;
                if (bIsFocusedView && bExternalRenderVisibility != nullptr)
                {
                    *bExternalRenderVisibility = bRenderVisibility;
                }
                if (bRenderVisibility)
                {
                    int32 StartIndex = NumSides / 8 * 3;
                    int32 EndIndex = NumSides / 8 * 5;

                    FVector LastVertex;
                    {
                        float DeltaX = FMath::Cos(AngleDelta * (StartIndex));
                        float DeltaY = FMath::Sin(AngleDelta * (StartIndex));
                        const FVector DeltaVector = WorldPlaneX * DeltaX + WorldPlaneY * DeltaY;
                        LastVertex = WorldOrigin + UseRadius * DeltaVector;
                    }

                    for (int32 SideIndex = StartIndex; SideIndex < EndIndex; SideIndex++)
                    {
                        float DeltaX = FMath::Cos(AngleDelta * (SideIndex + 1));
                        float DeltaY = FMath::Sin(AngleDelta * (SideIndex + 1));
                        const FVector DeltaVector = WorldPlaneX * DeltaX + WorldPlaneY * DeltaY;
                        const FVector Vertex = WorldOrigin + UseRadius * DeltaVector;
                        PDI->DrawLine(LastVertex, Vertex, Color, SDPG_Foreground, UseThickness, 0.0f, true);
                        LastVertex = Vertex;
                    }
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

    void SetExternalExtent(FVector* Extent)
    {
        ExternalExtent = Extent;
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
    int32 AxisIndex;
    FVector Normal;
    float Gap;
    float Radius;
    int32 NumSides;
    float Thickness;

    // set on Component for use in ::GetDynamicMeshElements()
    bool* bExternalHoverState = nullptr;
    bool* bExternalWorldLocalState = nullptr;
    bool* bExternalInteractionState = nullptr;
    FVector* ExternalExtent = nullptr;

    // set in ::GetDynamicMeshElements() for use by Component hit testing
    float* ExternalDynamicPixelToWorldScale = nullptr;
    bool* bExternalRenderVisibility = nullptr;
};


FPrimitiveSceneProxy* UPrimitiveGizmoRotateComponent::CreateSceneProxy()
{
    FPrimitiveGizmoRotateComponentSceneProxy* NewProxy = new FPrimitiveGizmoRotateComponentSceneProxy(this);
    NewProxy->SetExternalHoverState(&bHovering);
    NewProxy->SetExternalWorldLocalState(&bWorld);
    NewProxy->SetExternalInteractionState(&bInteracting);
    NewProxy->SetExternalExtent(&Extent);
    NewProxy->SetExternalDynamicPixelToWorldScale(&DynamicPixelToWorldScale);
    NewProxy->SetExternalRenderVisibility(&bRenderVisibility);
    return NewProxy;
}

bool UPrimitiveGizmoRotateComponent::LineTraceComponent(FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionQueryParams& Params)
{
    if (bRenderVisibility == false)
    {
        return false;
    }

    float LengthScale = DynamicPixelToWorldScale;
    double UseRadius = LengthScale * Radius;

    const FTransform& Transform = this->GetComponentToWorld();
    FVector WorldNormal = (bWorld) ? GetNormal() : Transform.TransformVector(GetNormal());

    FVector Offset = FVector::ZeroVector;
    switch (AxisIndex)
    {
    case 0:
        Offset.X = Extent.X;
        Offset.Z = Extent.Z;
        break;
    case 1:
        Offset.X = Extent.X;
        Offset.Z = Extent.Z;
    default:
        break;
    }

    FVector WorldOrigin = Transform.TransformPosition(FVector::ZeroVector + Offset);

    FRay Ray(Start, End - Start, false);

    // Find the intresection with the circle plane. Note that unlike the FMath version, GizmoMath::RayPlaneIntersectionPoint() 
    // checks that the ray isn't parallel to the plane.
    bool bIntersects;
    FVector HitPos;
    GizmoMath::RayPlaneIntersectionPoint(WorldOrigin, WorldNormal, Ray.Origin, Ray.Direction, bIntersects, HitPos);
    if (!bIntersects || Ray.GetParameter(HitPos) > Ray.GetParameter(End))
    {
        return false;
    }

    FVector NearestCircle;
    GizmoMath::ClosetPointOnCircle(HitPos, WorldOrigin, WorldNormal, UseRadius, NearestCircle);

    FVector NearestRay = Ray.ClosestPoint(NearestCircle);

    double Distance = FVector::Distance(NearestCircle, NearestRay);
    if (Distance > PixelHitDistanceThreshold * DynamicPixelToWorldScale)
    {
        return false;
    }

    OutHit.Component = this;
    OutHit.Distance = FVector::Distance(Start, NearestRay);
    OutHit.ImpactPoint = NearestRay;
    return true;
}

FBoxSphereBounds UPrimitiveGizmoRotateComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    return FBoxSphereBounds(FSphere(FVector::ZeroVector, Radius).TransformBy(LocalToWorld));
}

FVector UPrimitiveGizmoRotateComponent::GetNormal() const
{
    check(AxisIndex >= 0 && AxisIndex < 3);

    FVector Result = FVector::ZAxisVector;
    switch (AxisIndex)
    {
    case 0:
        Result = FVector::XAxisVector;
        break;
    case 1:
        Result = FVector::YAxisVector;
        break;
    case 2:
        Result = FVector::ZAxisVector;
        break;
    default:
        break;
    }
    return Result;
}
