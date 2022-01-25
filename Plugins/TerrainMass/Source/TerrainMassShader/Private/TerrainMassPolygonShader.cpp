#include "TerrainMassPolygonShader.h"

#include "GlobalShader.h"
#include "PipelineStateCache.h"

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
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), InvTextureSizeParam, Params.InvTextureSize);
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
        EndSideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("EndStartSideFalloffTexture"));
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
    FVector4 UV0;
    FVector4 UV1;
    FVector4 UV2;
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
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, UV0), VET_Float4, 1, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, UV1), VET_Float4, 2, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, UV2), VET_Float4, 3, Stride));
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
static void CookVertexData(TResourceArray<FTerrainMassPolygonVertex, VERTEXBUFFER_ALIGNMENT>& Vertices, const FIntPoint& Size, const FTerrainMassPolygonShaderParameter& ShaderParams)
{
    check(ShaderParams.NumSegments > 0);

    FVector Direction = ShaderParams.EndPosition - ShaderParams.StartPosition;
    FVector Normal = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

    TArray<FVector> Lefts;
    TArray<FVector> Rights;
    TArray<FVector> LeftFalloffs;
    TArray<FVector> RightFalloffs;

    for (int32 Index = 0; Index <= ShaderParams.NumSegments; Index++)
    {
        float Ratio = float(Index) / ShaderParams.NumSegments;
        FVector Center = FMath::Lerp(ShaderParams.StartPosition, ShaderParams.EndPosition, Ratio);
        FVector Left = Center + Normal * ShaderParams.Width * 0.5f;
        FVector Right = Center - Normal * ShaderParams.Width * 0.5f;
        FVector LeftFalloff = Left + Normal * ShaderParams.SideFalloff * 0.5f;
        FVector RightFalloff = Right - Normal * ShaderParams.SideFalloff * 0.5f;

        FVector ScaleVector(Size.X - 1.0f, Size.Y - 1.0f, 1.0f);
        Lefts.Add(Left / ScaleVector);
        Rights.Add(Right / ScaleVector);
        LeftFalloffs.Add(LeftFalloff / ScaleVector);
        RightFalloffs.Add(RightFalloff / ScaleVector);
    }

    int32 NumPoints = ShaderParams.NumSegments * 18;
    Vertices.SetNumUninitialized(NumPoints);

    for (int32 Index = 0; Index < ShaderParams.NumSegments; Index++)
    {
        // Center
        Vertices[Index * 18 + 0].Position = Lefts[Index];
        Vertices[Index * 18 + 1].Position = Rights[Index];
        Vertices[Index * 18 + 2].Position = Lefts[Index + 1];
        Vertices[Index * 18 + 3].Position = Lefts[Index + 1];
        Vertices[Index * 18 + 4].Position = Rights[Index];
        Vertices[Index * 18 + 5].Position = Rights[Index + 1];

        // Left Falloff
        Vertices[Index * 18 +  6].Position = LeftFalloffs[Index];
        Vertices[Index * 18 +  7].Position = Lefts[Index];
        Vertices[Index * 18 +  8].Position = LeftFalloffs[Index + 1];
        Vertices[Index * 18 +  9].Position = LeftFalloffs[Index + 1];
        Vertices[Index * 18 + 10].Position = Lefts[Index];
        Vertices[Index * 18 + 11].Position = Lefts[Index + 1];

        // Right Falloff
        Vertices[Index * 18 + 12].Position = Rights[Index];
        Vertices[Index * 18 + 13].Position = RightFalloffs[Index];
        Vertices[Index * 18 + 14].Position = Rights[Index + 1];
        Vertices[Index * 18 + 15].Position = Rights[Index + 1];
        Vertices[Index * 18 + 16].Position = RightFalloffs[Index];
        Vertices[Index * 18 + 17].Position = RightFalloffs[Index + 1];
    }

    //Vertices[0].Position = FVector4(1, 1, 0, 1);
    //Vertices[0].UV = FVector2D(1, 1);

    //Vertices[1].Position = FVector4(0, 1, 0, 1);
    //Vertices[1].UV = FVector2D(0, 1);

    //Vertices[2].Position = FVector4(1, 0, 0, 1);
    //Vertices[2].UV = FVector2D(1, 0);

    //Vertices[3].Position = FVector4(0, 0, 0, 1);
    //Vertices[3].UV = FVector2D(0, 0);

    //Vertices[4].Position = FVector4(0, 1, 0, 1);
    //Vertices[4].UV = FVector2D(0, 1);

    //Vertices[5].Position = FVector4(1, 0, 0, 1);
    //Vertices[5].UV = FVector2D(1, 0);
}

void FTerrainMassPolygonShader::Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassPolygonShaderParameter& ShaderParams)
{
    //
    // Vertex Buffer
    //
    FVertexBufferRHIRef VertexBufferRHI;

    TResourceArray<FTerrainMassPolygonVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    CookVertexData(Vertices, Size, ShaderParams);

    // Create vertex buffer. Fill buffer with initial data upon creation
    FRHIResourceCreateInfo CreateInfo(&Vertices);
    VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);

    //
    // Render
    //

    // Canvas
    FRHIRenderPassInfo RPInfoCanvas(DestTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfoCanvas, TEXT("TerrainMassPolygon"));
    {
        RHICmdList.SetViewport(0, 0, 0, Size.X, Size.Y, 1);

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