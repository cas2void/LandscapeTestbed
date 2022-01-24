#include "TerrainMassPolygonShader.h"

#include "GlobalShader.h"
#include "PipelineStateCache.h"

class FTerrainMassPolygonShaderVS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassPolygonShaderVS);

public:
    FTerrainMassPolygonShaderVS()
        : FGlobalShader()
    {}

    FTerrainMassPolygonShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {}
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassPolygonShaderVS, "/TerrainMassShaders/TerrainMassPolygon.usf", "MainVS", SF_Vertex)

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
        SourceTextureParam.Bind(Initializer.ParameterMap, TEXT("SourceTexture"));
        SourceTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SourceTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassPolygonShaderParameter& Params)
    {
        if (Params.SourceTexture &&
            Params.SourceTexture->Resource &&
            Params.SourceTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SourceTextureParam, SourceTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.SourceTexture->Resource->TextureRHI);
        }

        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, SourceTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, SourceTextureSamplerParam);
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassPolygonShaderPS, "/TerrainMassShaders/TerrainMassPolygon.usf", "MainPS", SF_Pixel);

struct FTerrainMassPolygonVertex
{
public:
    FVector4 Position;
    FVector2D UV;
};

class FTerrainMassPolygonVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FTerrainMassPolygonVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FTerrainMassPolygonVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FTerrainMassPolygonVertexDeclaration> GTerrainMassPolygonVertexDeclaration;

void FTerrainMassPolygonShader::Render(FRHICommandListImmediate& RHICmdList, FRHITexture* SourceTexture, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassPolygonShaderParameter& ShaderParams)
{
    //
    // Vertex Buffer
    //
    FVertexBufferRHIRef VertexBufferRHI;

    TResourceArray<FTerrainMassPolygonVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    Vertices.SetNumUninitialized(6);
    
    Vertices[0].Position = FVector4(1, 1, 0, 1);
    Vertices[0].UV = FVector2D(1, 1);

    Vertices[1].Position = FVector4(0, 1, 0, 1);
    Vertices[1].UV = FVector2D(0, 1);

    Vertices[2].Position = FVector4(1, 0, 0, 1);
    Vertices[2].UV = FVector2D(1, 0);

    Vertices[3].Position = FVector4(0, 0, 0, 1);
    Vertices[3].UV = FVector2D(0, 0);

    Vertices[4].Position = FVector4(0, 1, 0, 1);
    Vertices[4].UV = FVector2D(0, 1);

    Vertices[5].Position = FVector4(1, 0, 0, 1);
    Vertices[5].UV = FVector2D(1, 0);

    // Create vertex buffer. Fill buffer with initial data upon creation
    FRHIResourceCreateInfo CreateInfo(&Vertices);
    VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);

    //
    // Render
    //

    FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("TerrainMass"));
    {
        RHICmdList.SetViewport(0, 0, 0, Size.X, Size.Y, 1);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<FTerrainMassPolygonShaderVS> VertexShader(ShaderMap);
        TShaderMapRef<FTerrainMassPolygonShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);

        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTerrainMassPolygonVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
        RHICmdList.DrawPrimitive(0, 2, 1);
    }
    RHICmdList.EndRenderPass();

    VertexBufferRHI.SafeRelease();
}