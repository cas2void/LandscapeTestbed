// Fill out your copyright notice in the Description page of Project Settings.

#include "PrimitiveGizmoCircleComponent.h"

#include "PrimitiveSceneProxy.h"
#include "BaseGizmos/GizmoRenderingUtil.h"
#include "BaseGizmos/GizmoMath.h"

class FPrimitiveGizmoCircleComponentSceneProxy final : public FPrimitiveSceneProxy
{
public:
    SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

    FPrimitiveGizmoCircleComponentSceneProxy(const UPrimitiveGizmoCircleComponent* InComponent)
        : FPrimitiveSceneProxy(InComponent),
        Color(InComponent->Color),
        HoverThicknessMultiplier(InComponent->HoverSizeMultiplier),
        Normal(InComponent->GetNormal().GetSafeNormal()),
        Radius(InComponent->GetRadius()),
        bAutoScaleRadius(InComponent->IsAutoScaleRadius()),
        CenterOffset(InComponent->GetCenterOffset()),
        Resolution(InComponent->GetResolution()),
        StartAngle(InComponent->GetStartAngle()),
        EndAngle(InComponent->GetEndAngle()),
        bCullFace(InComponent->IsCullFace()),
        CullFaceNormal(InComponent->GetCullFaceNormal().GetSafeNormal()),
        Thickness(InComponent->GetThickness()),
        bViewAligned(InComponent->IsViewAligned())
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

                double UseRadius = bAutoScaleRadius ? LengthScale * Radius : Radius;

                FLinearColor BackColor = FLinearColor(0.5f, 0.5f, 0.5f);
                float BackThickness = 0.5f;
                float UseThickness = (bExternalHoverState != nullptr && *bExternalHoverState == true) ?
                    (HoverThicknessMultiplier * Thickness) : (Thickness);
                if (!bIsOrtho)
                {
                    UseThickness *= (View->FOV / 90.0);		// compensate for FOV scaling in Gizmos...
                    BackThickness *= (View->FOV / 90.0);		// compensate for FOV scaling in Gizmos...
                }

                const float	AngleDeltaRadian = Resolution / 180.0f * PI;
                const float StartAngleRadian = StartAngle / 180.0f * PI;
                const float EndAngleRadian = EndAngle / 180.0f * PI;
                const FVector UseCenterOffset = CenterOffset * LengthScale;

                if (bViewAligned)
                {
                    FVector WorldOrigin = LocalToWorldMatrix.TransformPosition(FVector::ZeroVector + UseCenterOffset);
                    WorldOrigin += 0.001 * ViewVector;
                    FVector WorldPlaneX, WorldPlaneY;
                    GizmoMath::MakeNormalPlaneBasis(ViewVector, WorldPlaneX, WorldPlaneY);

                    float CurrentAngle = StartAngleRadian;
                    float DeltaX = FMath::Cos(CurrentAngle);
                    float DeltaY = FMath::Sin(CurrentAngle);
                    FVector DeltaVector = WorldPlaneX * DeltaX + WorldPlaneY * DeltaY;
                    FVector LastVertex = WorldOrigin + UseRadius * DeltaVector;

                    while (CurrentAngle < EndAngleRadian)
                    {
                        CurrentAngle += AngleDeltaRadian;

                        DeltaX = FMath::Cos(CurrentAngle);
                        DeltaY = FMath::Sin(CurrentAngle);
                        DeltaVector = WorldPlaneX * DeltaX + WorldPlaneY * DeltaY;
                        FVector Vertex = WorldOrigin + UseRadius * DeltaVector;
                        PDI->DrawLine(LastVertex, Vertex, Color, SDPG_Foreground, UseThickness, 0.0f, true);
                        LastVertex = Vertex;
                    }
                }
                else
                {
                    FVector WorldOrigin = LocalToWorldMatrix.TransformPosition(FVector::ZeroVector + UseCenterOffset);
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
                        float CurrentAngle = StartAngleRadian;
                        float DeltaX = FMath::Cos(CurrentAngle);
                        float DeltaY = FMath::Sin(CurrentAngle);
                        FVector DeltaVector = WorldPlaneX * DeltaX + WorldPlaneY * DeltaY;
                        FVector LastVertex = WorldOrigin + UseRadius * DeltaVector;

                        while (CurrentAngle < EndAngleRadian)
                        {
                            CurrentAngle += AngleDeltaRadian;

                            DeltaX = FMath::Cos(CurrentAngle);
                            DeltaY = FMath::Sin(CurrentAngle);
                            DeltaVector = WorldPlaneX * DeltaX + WorldPlaneY * DeltaY;
                            FVector Vertex = WorldOrigin + UseRadius * DeltaVector;
                            PDI->DrawLine(LastVertex, Vertex, Color, SDPG_Foreground, UseThickness, 0.0f, true);
                            LastVertex = Vertex;
                        }
                    }
                }
            }
        }
    }

    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        bool bVisibleByFaceCulling = true;
        if (bCullFace)
        {
            const FMatrix& LocalToWorldMatrix = GetLocalToWorld();
            const FVector Origin = LocalToWorldMatrix.TransformPosition(FVector::ZeroVector);
            const bool bIsOrtho = !View->IsPerspectiveProjection();
            FVector GizmoViewDirection = (bIsOrtho) ? (View->GetViewDirection()) : (Origin - View->ViewLocation);
            GizmoViewDirection.Normalize();
            bVisibleByFaceCulling = FVector::DotProduct(-GizmoViewDirection, CullFaceNormal) > 0;
        }

        FPrimitiveViewRelevance Result;
        Result.bDrawRelevance = IsShown(View) && bVisibleByFaceCulling;
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
    FVector Normal;
    float Radius;
    bool bAutoScaleRadius;
    FVector CenterOffset;
    float Resolution;
    float StartAngle;
    float EndAngle;
    bool bCullFace;
    FVector CullFaceNormal;
    float Thickness;
    bool bViewAligned;

    // set on Component for use in ::GetDynamicMeshElements()
    bool* bExternalHoverState = nullptr;
    bool* bExternalWorldLocalState = nullptr;
    bool* bExternalInteractionState = nullptr;

    // set in ::GetDynamicMeshElements() for use by Component hit testing
    float* ExternalDynamicPixelToWorldScale = nullptr;
    bool* bExternalRenderVisibility = nullptr;
};


FPrimitiveSceneProxy* UPrimitiveGizmoCircleComponent::CreateSceneProxy()
{
    FPrimitiveGizmoCircleComponentSceneProxy* NewProxy = new FPrimitiveGizmoCircleComponentSceneProxy(this);
    NewProxy->SetExternalHoverState(&bHovering);
    NewProxy->SetExternalWorldLocalState(&bWorld);
    NewProxy->SetExternalInteractionState(&bInteracting);
    NewProxy->SetExternalDynamicPixelToWorldScale(&DynamicPixelToWorldScale);
    NewProxy->SetExternalRenderVisibility(&bRenderVisibility);
    return NewProxy;
}

bool UPrimitiveGizmoCircleComponent::LineTraceComponent(FHitResult& OutHit, const FVector Start, const FVector End, const FCollisionQueryParams& Params)
{
    if (bRenderVisibility == false)
    {
        return false;
    }

    float LengthScale = DynamicPixelToWorldScale;
    double UseRadius = bAutoScaleRadius ? LengthScale * Radius : Radius;
    const FVector UseCenterOffset = CenterOffset * LengthScale;

    const FTransform& Transform = this->GetComponentToWorld();
    FVector WorldNormal = (bWorld) ? Normal : Transform.TransformVector(Normal);
    FVector WorldOrigin = Transform.TransformPosition(FVector::ZeroVector + UseCenterOffset);

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

FBoxSphereBounds UPrimitiveGizmoCircleComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    return FBoxSphereBounds(FSphere(FVector::ZeroVector, Radius).TransformBy(LocalToWorld));
}
