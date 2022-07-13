// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainMassRingComponent.h"
#include "PrimitiveSceneProxy.h"
#include "DynamicMeshBuilder.h"
#include "MaterialShared.h"

#define DEFAULT_SCREEN_SIZE	(0.0025f)
#define ARROW_SCALE			(80.0f)
#define ARROW_RADIUS_FACTOR	(0.03f)
#define ARROW_HEAD_FACTOR	(0.2f)
#define ARROW_HEAD_ANGLE	(20.f)

// Copy from PrimitiveDrawingUtils.cpp
static FVector TerrainMassCalcConeVert(float Angle1, float Angle2, float AzimuthAngle)
{
    float ang1 = FMath::Clamp<float>(Angle1, 0.01f, (float)PI - 0.01f);
    float ang2 = FMath::Clamp<float>(Angle2, 0.01f, (float)PI - 0.01f);

    float sinX_2 = FMath::Sin(0.5f * ang1);
    float sinY_2 = FMath::Sin(0.5f * ang2);

    float sinSqX_2 = sinX_2 * sinX_2;
    float sinSqY_2 = sinY_2 * sinY_2;

    float tanX_2 = FMath::Tan(0.5f * ang1);
    float tanY_2 = FMath::Tan(0.5f * ang2);

    float phi = FMath::Atan2(FMath::Sin(AzimuthAngle) * sinY_2, FMath::Cos(AzimuthAngle) * sinX_2);
    float sinPhi = FMath::Sin(phi);
    float cosPhi = FMath::Cos(phi);
    float sinSqPhi = sinPhi * sinPhi;
    float cosSqPhi = cosPhi * cosPhi;

    float rSq, r, Sqr, alpha, beta;

    rSq = sinSqX_2 * sinSqY_2 / (sinSqX_2 * sinSqPhi + sinSqY_2 * cosSqPhi);
    r = FMath::Sqrt(rSq);
    Sqr = FMath::Sqrt(1 - rSq);
    alpha = r * cosPhi;
    beta = r * sinPhi;

    FVector ConeVert;

    ConeVert.X = 2 * Sqr * beta;
    ConeVert.Y = 2 * Sqr * alpha;
    ConeVert.Z = (1 - 2 * rSq);

    return ConeVert;
}

// Copy from PrimitiveDrawingUtils.cpp
static void TerrainMassBuildConeVerts(float Angle1, float Angle2, float Scale, float ZOffset, uint32 NumSides, TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices)
{
    TArray<FVector> ConeVerts;
    ConeVerts.AddUninitialized(NumSides);

    for (uint32 i = 0; i < NumSides; i++)
    {
        float Fraction = (float)i / (float)(NumSides);
        float Azi = 2.f * PI * Fraction;
        ConeVerts[i] = (TerrainMassCalcConeVert(Angle1, Angle2, Azi) * Scale) + FVector(0, 0, ZOffset);
    }

    for (uint32 i = 0; i < NumSides; i++)
    {
        // Normal of the current face 
        FVector TriTangentZ = ConeVerts[(i + 1) % NumSides] ^ ConeVerts[i]; // aka triangle normal
        FVector TriTangentY = ConeVerts[i];
        FVector TriTangentX = TriTangentZ ^ TriTangentY;

        FDynamicMeshVertex V0, V1, V2;

        V0.Position = FVector(0, 0, ZOffset);
        V0.TextureCoordinate[0].X = 0.0f;
        V0.TextureCoordinate[0].Y = (float)i / NumSides;
        V0.SetTangents(TriTangentX, TriTangentY, FVector(0, 0, 1));
        int32 I0 = OutVerts.Add(V0);

        V1.Position = ConeVerts[i];
        V1.TextureCoordinate[0].X = 1.0f;
        V1.TextureCoordinate[0].Y = (float)i / NumSides;
        FVector TriTangentZPrev = ConeVerts[i] ^ ConeVerts[i == 0 ? NumSides - 1 : i - 1]; // Normal of the previous face connected to this face
        V1.SetTangents(TriTangentX, TriTangentY, -(TriTangentZPrev + TriTangentZ).GetSafeNormal());
        int32 I1 = OutVerts.Add(V1);

        V2.Position = ConeVerts[(i + 1) % NumSides];
        V2.TextureCoordinate[0].X = 1.0f;
        V2.TextureCoordinate[0].Y = (float)((i + 1) % NumSides) / NumSides;
        FVector TriTangentZNext = ConeVerts[(i + 2) % NumSides] ^ ConeVerts[(i + 1) % NumSides]; // Normal of the next face connected to this face
        V2.SetTangents(TriTangentX, TriTangentY, -(TriTangentZNext + TriTangentZ).GetSafeNormal());
        int32 I2 = OutVerts.Add(V2);

        // Flip winding for negative scale
        if (Scale >= 0.f)
        {
            OutIndices.Add(I0);
            OutIndices.Add(I1);
            OutIndices.Add(I2);
        }
        else
        {
            OutIndices.Add(I0);
            OutIndices.Add(I2);
            OutIndices.Add(I1);
        }
    }
}

// Copy from PrimitiveDrawingUtils.cpp
static void TerrainMassBuildCylinderVerts(const FVector& Base, const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis, float Radius, float HalfHeight, uint32 Sides, TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices)
{
    const float	AngleDelta = 2.0f * PI / Sides;
    FVector	LastVertex = Base + XAxis * Radius;

    FVector2D TC = FVector2D(0.0f, 0.0f);
    float TCStep = 1.0f / Sides;

    FVector TopOffset = HalfHeight * ZAxis;

    int32 BaseVertIndex = OutVerts.Num();

    //Compute vertices for base circle.
    for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
    {
        const FVector Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) + YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
        FVector Normal = Vertex - Base;
        Normal.Normalize();

        FDynamicMeshVertex MeshVertex;

        MeshVertex.Position = Vertex - TopOffset;
        MeshVertex.TextureCoordinate[0] = TC;

        MeshVertex.SetTangents(
            -ZAxis,
            (-ZAxis) ^ Normal,
            Normal
        );

        OutVerts.Add(MeshVertex); //Add bottom vertex

        LastVertex = Vertex;
        TC.X += TCStep;
    }

    LastVertex = Base + XAxis * Radius;
    TC = FVector2D(0.0f, 1.0f);

    //Compute vertices for the top circle
    for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
    {
        const FVector Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) + YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
        FVector Normal = Vertex - Base;
        Normal.Normalize();

        FDynamicMeshVertex MeshVertex;

        MeshVertex.Position = Vertex + TopOffset;
        MeshVertex.TextureCoordinate[0] = TC;

        MeshVertex.SetTangents(
            -ZAxis,
            (-ZAxis) ^ Normal,
            Normal
        );

        OutVerts.Add(MeshVertex); //Add top vertex

        LastVertex = Vertex;
        TC.X += TCStep;
    }

    //Add top/bottom triangles, in the style of a fan.
    //Note if we wanted nice rendering of the caps then we need to duplicate the vertices and modify
    //texture/tangent coordinates.
    for (uint32 SideIndex = 1; SideIndex < Sides; SideIndex++)
    {
        int32 V0 = BaseVertIndex;
        int32 V1 = BaseVertIndex + SideIndex;
        int32 V2 = BaseVertIndex + ((SideIndex + 1) % Sides);

        //bottom
        OutIndices.Add(V0);
        OutIndices.Add(V1);
        OutIndices.Add(V2);

        // top
        OutIndices.Add(Sides + V2);
        OutIndices.Add(Sides + V1);
        OutIndices.Add(Sides + V0);
    }

    //Add sides.

    for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
    {
        int32 V0 = BaseVertIndex + SideIndex;
        int32 V1 = BaseVertIndex + ((SideIndex + 1) % Sides);
        int32 V2 = V0 + Sides;
        int32 V3 = V1 + Sides;

        OutIndices.Add(V0);
        OutIndices.Add(V2);
        OutIndices.Add(V1);

        OutIndices.Add(V2);
        OutIndices.Add(V3);
        OutIndices.Add(V1);
    }
}

class FRingSceneProxy : public FPrimitiveSceneProxy
{
public:
    SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

    FRingSceneProxy(const UTerrainMassRingComponent* InComponent)
        : FPrimitiveSceneProxy(InComponent)
        , SplineInfo(InComponent->SplineCurves.Position)
#if WITH_EDITORONLY_DATA
        , LineColor(InComponent->EditorUnselectedSplineSegmentColor)
#else
        , LineColor(FLinearColor::White)
#endif
        , VertexFactory(GetScene().GetFeatureLevel(), "FRingSceneProxy")
        , HandleColor(InComponent->HandleColor)
    {
        TArray<FDynamicMeshVertex> OutVerts;
        InComponent->CreateHandleGeometry(OutVerts, IndexBuffer.Indices);

        VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);

        // Enqueue initialization of render resource
        BeginInitResource(&IndexBuffer);
    }

    virtual ~FRingSceneProxy()
    {
        VertexBuffers.PositionVertexBuffer.ReleaseResource();
        VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
        VertexBuffers.ColorVertexBuffer.ReleaseResource();
        IndexBuffer.ReleaseResource();
        VertexFactory.ReleaseResource();
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
    {
        QUICK_SCOPE_CYCLE_COUNTER(STAT_SplineSceneProxy_GetDynamicMeshElements);

        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
        {
            if (VisibilityMap & (1 << ViewIndex))
            {
                const FSceneView* View = Views[ViewIndex];
                FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

                const FMatrix& Local2World = GetLocalToWorld();

                // Taking into account the min and maximum drawing distance
                const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - Local2World.GetOrigin()).SizeSquared();
                if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
                {
                    continue;
                }

                USplineComponent::Draw(PDI, View, SplineInfo, Local2World, LineColor, SDPG_World);
            }
        }

        FMatrix EffectiveLocalToWorld = GetLocalToWorld();
        EffectiveLocalToWorld.RemoveScaling();

        auto HandleMaterialRenderProxy = new FColoredMaterialRenderProxy(
        	GEngine->ArrowMaterial->GetRenderProxy(),
        	HandleColor,
        	"GizmoColor"
        );

        Collector.RegisterOneFrameMaterialProxy(HandleMaterialRenderProxy);

        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
        {
        	if (VisibilityMap & (1 << ViewIndex))
        	{
        		const FSceneView* View = Views[ViewIndex];

        		// Draw the mesh.
        		FMeshBatch& Mesh = Collector.AllocateMesh();
        		FMeshBatchElement& BatchElement = Mesh.Elements[0];
        		BatchElement.IndexBuffer = &IndexBuffer;
        		Mesh.bWireframe = false;
        		Mesh.VertexFactory = &VertexFactory;
        		Mesh.MaterialRenderProxy = HandleMaterialRenderProxy;

        		FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
        		DynamicPrimitiveUniformBuffer.Set(EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(), GetLocalBounds(), true, false, DrawsVelocity(), false);
        		BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

        		BatchElement.FirstIndex = 0;
        		BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
        		BatchElement.MinVertexIndex = 0;
        		BatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
        		Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
        		Mesh.Type = PT_TriangleList;
        		Mesh.DepthPriorityGroup = SDPG_World;
        		Mesh.bCanApplyViewModeOverrides = false;
        		Collector.AddMesh(ViewIndex, Mesh);
        	}
        }
    }

    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance Result;
        // if selected, GetDynamicMeshElements() will not get called.
        Result.bDrawRelevance = !IsSelected() && IsShown(View) && View->Family->EngineShowFlags.Splines;
        Result.bDynamicRelevance = true;
        Result.bShadowRelevance = IsShadowCast(View);
        Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
        return Result;
    }

    virtual uint32 GetMemoryFootprint(void) const override { return sizeof * this + GetAllocatedSize(); }
    uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

public:
    FInterpCurveVector SplineInfo;
    FLinearColor LineColor;

    FStaticMeshVertexBuffers VertexBuffers;
    FDynamicMeshIndexBuffer32 IndexBuffer;
    FLocalVertexFactory VertexFactory;

    FColor HandleColor;
};

// Sets default values for this component's properties
UTerrainMassRingComponent::UTerrainMassRingComponent()
{
}

#if !UE_BUILD_SHIPPING
FPrimitiveSceneProxy* UTerrainMassRingComponent::CreateSceneProxy()
{
    return new FRingSceneProxy(this);
}

FBoxSphereBounds UTerrainMassRingComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    const int32 NumPoints = SplineCurves.Position.Points.Num();
    const int32 NumSegments = IsClosedLoop() ? NumPoints : NumPoints - 1;

    FVector Min(WORLD_MAX);
    FVector Max(-WORLD_MAX);
    if (NumSegments > 0)
    {
        for (int32 Index = 0; Index < NumSegments; Index++)
        {
            const bool bLoopSegment = (Index == NumPoints - 1);
            const int32 NextIndex = bLoopSegment ? 0 : Index + 1;
            const FInterpCurvePoint<FVector>& ThisInterpPoint = SplineCurves.Position.Points[Index];
            FInterpCurvePoint<FVector> NextInterpPoint = SplineCurves.Position.Points[NextIndex];
            if (bLoopSegment)
            {
                NextInterpPoint.InVal = ThisInterpPoint.InVal + SplineCurves.Position.LoopKeyOffset;
            }

            CurveVectorFindIntervalBounds(ThisInterpPoint, NextInterpPoint, Min, Max);
        }
    }
    else if (NumPoints == 1)
    {
        Min = Max = SplineCurves.Position.Points[0].OutVal;
    }

    return FBoxSphereBounds(FBox(Min, Max).TransformBy(LocalToWorld));
}
#endif

void UTerrainMassRingComponent::CreateHandleGeometry(TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices) const
{
    const float HeadAngle = FMath::DegreesToRadians(ARROW_HEAD_ANGLE);
    const float DefaultLength = HandleSize * ARROW_SCALE;
    const float TotalLength = HandleSize * HandleLength;
    const float HeadLength = DefaultLength * ARROW_HEAD_FACTOR;
    const float ShaftRadius = DefaultLength * ARROW_RADIUS_FACTOR;
    const float ShaftLength = (TotalLength - HeadLength * 0.5); // 10% overlap between shaft and head
    const FVector ShaftCenter = FVector(0, 0, 0.5f * ShaftLength);

    TerrainMassBuildConeVerts(HeadAngle, HeadAngle, -HeadLength, TotalLength, 32, OutVerts, OutIndices);
    TerrainMassBuildCylinderVerts(ShaftCenter, FVector::XAxisVector, FVector::YAxisVector, FVector::ZAxisVector, ShaftRadius, 0.5f * ShaftLength, 16, OutVerts, OutIndices);
}