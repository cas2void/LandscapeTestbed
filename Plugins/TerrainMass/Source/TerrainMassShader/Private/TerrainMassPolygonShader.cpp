#include "TerrainMassPolygonShader.h"

#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "LandscapeDataAccess.h"

//
// Vertex Shader
//
class FTerrainMassPolygonShaderVS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassPolygonShaderVS);

public:
    FTerrainMassPolygonShaderVS()
        : FGlobalShader()
    {}

    FTerrainMassPolygonShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassPolygonShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), InvTextureSizeParam, FVector2D(1.0f) / Params.RenderTargetSize);
    }

private:
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassPolygonShaderVS, "/TerrainMassShaders/TerrainMassPolygon.usf", "MainVS", SF_Vertex)

//
// Pixel Shader
//
class FTerrainMassPolygonShaderPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassPolygonShaderPS);

public:
    FTerrainMassPolygonShaderPS()
        : FGlobalShader()
    {}

    FTerrainMassPolygonShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        StartSideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("StartSideFalloffTexture"));
        StartSideFalloffTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("StartSideFalloffTextureSampler"));
        EndSideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("EndSideFalloffTexture"));
        EndSideFalloffTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("EndSideFalloffTextureSampler"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassPolygonShaderParameter& Params)
    {
        if (Params.StartSidefFalloffTexture &&
            Params.StartSidefFalloffTexture->Resource &&
            Params.StartSidefFalloffTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), StartSideFalloffTextureParam, StartSideFalloffTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.StartSidefFalloffTexture->Resource->TextureRHI);
        }

        if (Params.EndSideFalloffTexture &&
            Params.EndSideFalloffTexture->Resource &&
            Params.EndSideFalloffTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), EndSideFalloffTextureParam, EndSideFalloffTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.EndSideFalloffTexture->Resource->TextureRHI);
        }
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, StartSideFalloffTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, StartSideFalloffTextureSamplerParam);
    LAYOUT_FIELD(FShaderResourceParameter, EndSideFalloffTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, EndSideFalloffTextureSamplerParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassPolygonShaderPS, "/TerrainMassShaders/TerrainMassPolygon.usf", "MainPS", SF_Pixel);

//
// Vertex Layout
//
struct FTerrainMassPolygonVertex
{
public:
    FVector4 Position;
    // Point 0, xy: position, z: alpha
    FVector4 Point0;
    // Point 1
    FVector4 Point1;
    // Point 2
    FVector4 Point2;
    // Extra data, x: SegmentT
    FVector4 ExtraData;

    // for experiment with 4 points around
    FVector4 P0;
    FVector4 P1;
    FVector4 P2;
    FVector4 P3;
};

class FTerrainMassPolygonVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual ~FTerrainMassPolygonVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FTerrainMassPolygonVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, Position), VET_Float4, 0, Stride));
		//Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, UV), VET_Float2, 1, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, Point0), VET_Float4, 1, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, Point1), VET_Float4, 2, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, Point2), VET_Float4, 3, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, ExtraData), VET_Float4, 4, Stride));

        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, P0), VET_Float4, 5, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, P1), VET_Float4, 6, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, P2), VET_Float4, 7, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, P3), VET_Float4, 8, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FTerrainMassPolygonVertexDeclaration> GTerrainMassPolygonVertexDeclaration;

//
// Renderer
//
static void CookVertexData(TResourceArray<FTerrainMassPolygonVertex, VERTEXBUFFER_ALIGNMENT>& Vertices, const FTerrainMassPolygonShaderParameter& ShaderParams)
{
    check(ShaderParams.NumSegments > 0);

    FVector Direction = ShaderParams.EndPosition - ShaderParams.StartPosition;
    FVector Normal = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
    float Distance = FVector::Distance(ShaderParams.StartPosition, ShaderParams.EndPosition);

    TArray<FVector> Lefts;
    TArray<FVector> Rights;
    TArray<FVector> LeftFalloffs;
    TArray<FVector> RightFalloffs;
    TArray<float> Ratios;
    TArray<float> EndFalloffs;

    FVector ScaleVector = FVector(1.0f / (ShaderParams.RenderTargetSize.X - 1.0f), 1.0f / (ShaderParams.RenderTargetSize.Y - 1.0f), LANDSCAPE_INV_ZSCALE);

    for (int32 Index = 0; Index <= ShaderParams.NumSegments; Index++)
    {
        float Ratio = float(Index) / ShaderParams.NumSegments;
        Ratios.Add(Ratio);

        FVector Center = FMath::Lerp(ShaderParams.StartPosition, ShaderParams.EndPosition, Ratio);
        FVector Left = Center + Normal * ShaderParams.Width * 0.5f;
        FVector Right = Center - Normal * ShaderParams.Width * 0.5f;
        FVector LeftFalloff = Left + Normal * ShaderParams.SideFalloff * 0.5f;
        FVector RightFalloff = Right - Normal * ShaderParams.SideFalloff * 0.5f;

        Lefts.Add(ShaderParams.WorldToCanvasTransform.TransformPosition(Left) * ScaleVector);
        Rights.Add(ShaderParams.WorldToCanvasTransform.TransformPosition(Right) * ScaleVector);
        LeftFalloffs.Add(ShaderParams.WorldToCanvasTransform.TransformPosition(LeftFalloff) * ScaleVector);
        RightFalloffs.Add(ShaderParams.WorldToCanvasTransform.TransformPosition(RightFalloff) * ScaleVector);

        float DistanceToStart = FVector::Distance(ShaderParams.StartPosition, Center);
        float HeadEndFalloff = FMath::Min(FMath::Min(DistanceToStart, ShaderParams.EndFalloff), Distance) / ShaderParams.EndFalloff;

        float DistanceToEnd = FVector::Distance(ShaderParams.EndPosition, Center);
        float TailEndFalloff = FMath::Min(FMath::Min(DistanceToEnd, ShaderParams.EndFalloff), Distance) / ShaderParams.EndFalloff;

        float EndFalloff = HeadEndFalloff * TailEndFalloff;
        EndFalloffs.Add(EndFalloff);
    }

    int32 NumPoints = ShaderParams.NumSegments * 18;
    Vertices.SetNumUninitialized(NumPoints);

    for (int32 Index = 0; Index < ShaderParams.NumSegments; Index++)
    {
        // Center
        Vertices[Index * 18 +  0].Position = Lefts[Index];
        Vertices[Index * 18 +  0].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 +  0].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 +  1].Position = Rights[Index];
        Vertices[Index * 18 +  1].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 +  1].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 +  2].Position = Lefts[Index + 1];
        Vertices[Index * 18 +  2].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 +  2].ExtraData.X = Ratios[Index + 1];

        for (int32 InnerIndex = 0; InnerIndex < 3; InnerIndex++)
        {
            Vertices[Index * 18 + InnerIndex].Point0 = FVector(Lefts[Index].X, Lefts[Index].Y, 1.0f);
            Vertices[Index * 18 + InnerIndex].Point1 = FVector(Rights[Index].X, Rights[Index].Y, 1.0f);
            Vertices[Index * 18 + InnerIndex].Point2 = FVector(Lefts[Index + 1].X, Lefts[Index + 1].Y, 1.0f);
        }

        Vertices[Index * 18 +  3].Position = Lefts[Index + 1];
        Vertices[Index * 18 +  3].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 +  3].ExtraData.X = Ratios[Index + 1];

        Vertices[Index * 18 +  4].Position = Rights[Index];
        Vertices[Index * 18 +  4].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 +  4].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 +  5].Position = Rights[Index + 1];
        Vertices[Index * 18 +  5].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 +  5].ExtraData.X = Ratios[Index + 1];

        for (int32 InnerIndex = 0; InnerIndex < 3; InnerIndex++)
        {
            Vertices[Index * 18 + 3 + InnerIndex].Point0 = FVector(Lefts[Index + 1].X, Lefts[Index + 1].Y, 1.0f);
            Vertices[Index * 18 + 3 + InnerIndex].Point1 = FVector(Rights[Index].X, Rights[Index].Y, 1.0f);
            Vertices[Index * 18 + 3 + InnerIndex].Point2 = FVector(Rights[Index + 1].X, Rights[Index + 1].Y, 1.0f);
        }

        {
            // 4 points for each vertex, for experiment
            for (int32 InnerIndex = 0; InnerIndex < 6; InnerIndex++)
            {
                Vertices[Index * 18 + 0 + InnerIndex].P0 = FVector(Lefts[Index].X, Lefts[Index].Y, 1.0f);
                Vertices[Index * 18 + 0 + InnerIndex].P1 = FVector(Lefts[Index + 1].X, Lefts[Index + 1].Y, 1.0f);
                Vertices[Index * 18 + 0 + InnerIndex].P2 = FVector(Rights[Index].X, Rights[Index].Y, 1.0f);
                Vertices[Index * 18 + 0 + InnerIndex].P3 = FVector(Rights[Index + 1].X, Rights[Index + 1].Y, 1.0f);
            }
        }

        // Left Falloff
        Vertices[Index * 18 +  6].Position = LeftFalloffs[Index];
        Vertices[Index * 18 +  6].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 +  6].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 +  7].Position = Lefts[Index];
        Vertices[Index * 18 +  7].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 +  7].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 +  8].Position = LeftFalloffs[Index + 1];
        Vertices[Index * 18 +  8].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 +  8].ExtraData.X = Ratios[Index + 1];

        for (int32 InnerIndex = 0; InnerIndex < 3; InnerIndex++)
        {
            Vertices[Index * 18 + 6 + InnerIndex].Point0 = FVector(LeftFalloffs[Index].X, LeftFalloffs[Index].Y, 0.0f);
            Vertices[Index * 18 + 6 + InnerIndex].Point1 = FVector(Lefts[Index].X, Lefts[Index].Y, 1.0f);
            Vertices[Index * 18 + 6 + InnerIndex].Point2 = FVector(LeftFalloffs[Index + 1].X, LeftFalloffs[Index + 1].Y, 0.0f);
        }

        Vertices[Index * 18 +  9].Position = LeftFalloffs[Index + 1];
        Vertices[Index * 18 +  9].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 +  9].ExtraData.X = Ratios[Index + 1];

        Vertices[Index * 18 + 10].Position = Lefts[Index];
        Vertices[Index * 18 + 10].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 + 10].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 + 11].Position = Lefts[Index + 1];
        Vertices[Index * 18 + 11].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 + 11].ExtraData.X = Ratios[Index + 1];

        for (int32 InnerIndex = 0; InnerIndex < 3; InnerIndex++)
        {
            Vertices[Index * 18 + 9 + InnerIndex].Point0 = FVector(LeftFalloffs[Index + 1].X, LeftFalloffs[Index + 1].Y, 0.0f);
            Vertices[Index * 18 + 9 + InnerIndex].Point1 = FVector(Lefts[Index].X, Lefts[Index].Y, 1.0f);
            Vertices[Index * 18 + 9 + InnerIndex].Point2 = FVector(Lefts[Index + 1].X, Lefts[Index + 1].Y, 1.0f);
        }

        {
            // 4 points for each vertex, for experiment
            for (int32 InnerIndex = 0; InnerIndex < 6; InnerIndex++)
            {
                Vertices[Index * 18 + 6 + InnerIndex].P0 = FVector(Lefts[Index].X, Lefts[Index].Y, 1.0f);
                Vertices[Index * 18 + 6 + InnerIndex].P1 = FVector(Lefts[Index + 1].X, Lefts[Index + 1].Y, 1.0f);
                Vertices[Index * 18 + 6 + InnerIndex].P2 = FVector(LeftFalloffs[Index].X, LeftFalloffs[Index].Y, 0.0f);
                Vertices[Index * 18 + 6 + InnerIndex].P3 = FVector(LeftFalloffs[Index + 1].X, LeftFalloffs[Index + 1].Y, 0.0f);
            }
        }

        // Right Falloff
        Vertices[Index * 18 + 12].Position = Rights[Index];
        Vertices[Index * 18 + 12].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 + 12].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 + 13].Position = RightFalloffs[Index];
        Vertices[Index * 18 + 13].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 + 13].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 + 14].Position = Rights[Index + 1];
        Vertices[Index * 18 + 14].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 + 14].ExtraData.X = Ratios[Index + 1];

        for (int32 InnerIndex = 0; InnerIndex < 3; InnerIndex++)
        {
            Vertices[Index * 18 + 12 + InnerIndex].Point0 = FVector(Rights[Index].X, Rights[Index].Y, 1.0f);
            Vertices[Index * 18 + 12 + InnerIndex].Point1 = FVector(RightFalloffs[Index].X, RightFalloffs[Index].Y, 0.0f);
            Vertices[Index * 18 + 12 + InnerIndex].Point2 = FVector(Rights[Index + 1].X, Rights[Index + 1].Y, 1.0f);
        }

        Vertices[Index * 18 + 15].Position = Rights[Index + 1];
        Vertices[Index * 18 + 15].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 + 15].ExtraData.X = Ratios[Index + 1];

        Vertices[Index * 18 + 16].Position = RightFalloffs[Index];
        Vertices[Index * 18 + 16].Position.W = EndFalloffs[Index];
        Vertices[Index * 18 + 16].ExtraData.X = Ratios[Index];

        Vertices[Index * 18 + 17].Position = RightFalloffs[Index + 1];
        Vertices[Index * 18 + 17].Position.W = EndFalloffs[Index + 1];
        Vertices[Index * 18 + 17].ExtraData.X = Ratios[Index + 1];

        for (int32 InnerIndex = 0; InnerIndex < 3; InnerIndex++)
        {
            Vertices[Index * 18 + 15 + InnerIndex].Point0 = FVector(Rights[Index + 1].X, Rights[Index + 1].Y, 1.0f);
            Vertices[Index * 18 + 15 + InnerIndex].Point1 = FVector(RightFalloffs[Index].X, RightFalloffs[Index].Y, 0.0f);
            Vertices[Index * 18 + 15 + InnerIndex].Point2 = FVector(RightFalloffs[Index + 1].X, RightFalloffs[Index + 1].Y, 0.0f);
        }

        {
            // 4 points for each vertex, for experiment
            for (int32 InnerIndex = 0; InnerIndex < 6; InnerIndex++)
            {
                Vertices[Index * 18 + 12 + InnerIndex].P0 = FVector(Rights[Index].X, Rights[Index].Y, 1.0f);
                Vertices[Index * 18 + 12 + InnerIndex].P1 = FVector(Rights[Index + 1].X, Rights[Index + 1].Y, 1.0f);
                Vertices[Index * 18 + 12 + InnerIndex].P2 = FVector(RightFalloffs[Index].X, RightFalloffs[Index].Y, 0.0f);
                Vertices[Index * 18 + 12 + InnerIndex].P3 = FVector(RightFalloffs[Index + 1].X, RightFalloffs[Index + 1].Y, 0.0f);
            }
        }
    }
}

void FTerrainMassPolygonShader::Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FTerrainMassPolygonShaderParameter& ShaderParams)
{
    //
    // Vertex Buffer
    //
    FVertexBufferRHIRef VertexBufferRHI;

    TResourceArray<FTerrainMassPolygonVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    CookVertexData(Vertices, ShaderParams);

    // Create vertex buffer. Fill buffer with initial data upon creation
    FRHIResourceCreateInfo CreateInfo(&Vertices);
    VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);

    //
    // Render
    //
    FRHIRenderPassInfo RPInfoCanvas(DestTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfoCanvas, TEXT("TerrainMassPolygon"));
    {
        RHICmdList.SetViewport(0, 0, 0, ShaderParams.RenderTargetSize.X, ShaderParams.RenderTargetSize.Y, 1);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<FTerrainMassPolygonShaderVS> VertexShader(ShaderMap);
        VertexShader->SetParameters(RHICmdList, ShaderParams);
        TShaderMapRef<FTerrainMassPolygonShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);

        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTerrainMassPolygonVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
        RHICmdList.DrawPrimitive(0, ShaderParams.NumSegments * 6, 1);
    }
    RHICmdList.EndRenderPass();

    VertexBufferRHI.SafeRelease();
}