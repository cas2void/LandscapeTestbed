#include "TerrainMassShapeShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

//
// Vertex Layout
//
struct FTerrainMassShapeVertex
{
public:
    FVector4 Position;
    // Extra data
    FVector4 ExtraData;
};

class FTerrainMassShapeVertexDeclaration : public FRenderResource
{
public:
    FVertexDeclarationRHIRef VertexDeclarationRHI;

    virtual ~FTerrainMassShapeVertexDeclaration() {}

    virtual void InitRHI()
    {
        FVertexDeclarationElementList Elements;
        uint32 Stride = sizeof(FTerrainMassShapeVertex);
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassShapeVertex, Position), VET_Float4, 0, Stride));
        //Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassPolygonVertex, UV), VET_Float2, 1, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassShapeVertex, ExtraData), VET_Float4, 1, Stride));
        VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
    }

    virtual void ReleaseRHI()
    {
        VertexDeclarationRHI.SafeRelease();
    }
};

TGlobalResource<FTerrainMassShapeVertexDeclaration> GTerrainMassShapeVertexDeclaration;

#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
class FTerrainMassShapeShaderVS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassShapeShaderVS);

public:
    FTerrainMassShapeShaderVS()
        : FGlobalShader()
    {}

    FTerrainMassShapeShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassShapeShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), InvTextureSizeParam, Params.InvTextureSize);
    }

private:
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassShapeShaderVS, "/TerrainMassShaders/TerrainMassShape.usf", "MainVS", SF_Vertex);
#endif

class FTerrainMassShapeShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassShapeShaderPS);

public:
	FTerrainMassShapeShaderPS()
		: FGlobalShader()
	{}

	FTerrainMassShapeShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        SideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTexture"));
        SideFalloffTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassShapeShaderParameter& Params)
    {
        if (Params.SideFalloffTexture.IsValid() &&
            Params.SideFalloffTexture->Resource &&
            Params.SideFalloffTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SideFalloffTextureParam, SideFalloffTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.SideFalloffTexture->Resource->TextureRHI);
        }

        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureSamplerParam);
	LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassShapeShaderPS, "/TerrainMassShaders/TerrainMassShape.usf", "MainPS", SF_Pixel);

static void Render_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, const FIntPoint& Size, const TArray<FVector>& ShapePoints, const FTerrainMassShapeShaderParameter& ShaderParams)
{
    //
    // Vertex Buffer
    //
    TResourceArray<FTerrainMassShapeVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    Vertices.SetNumUninitialized(ShapePoints.Num());
    for (int32 Index = 0; Index < ShapePoints.Num(); Index++)
    {
        Vertices[Index].Position = FVector4(ShapePoints[Index], 1.0f);
    }

    // Create vertex buffer. Fill buffer with initial data upon creation
    FRHIResourceCreateInfo CreateInfoVB(&Vertices);
    FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfoVB);

    // Setup index buffer
    TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
    const uint32 NumIndices = (ShapePoints.Num() - 2) * 3;
    IndexBuffer.SetNumUninitialized(NumIndices);

    for (int32 Index = 0; Index < ShapePoints.Num() - 2; Index++)
    {
        IndexBuffer[Index * 3 + 0] = 0;
        IndexBuffer[Index * 3 + 1] = Index + 1;
        IndexBuffer[Index * 3 + 2] = Index + 2;
    }

    FRHIResourceCreateInfo CreateInfoIB(&IndexBuffer);
    FIndexBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfoIB);

    //
    // Render
    //
    FRHIRenderPassInfo RPInfoCanvas(OutputTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfoCanvas, TEXT("TerrainMassShape"));
    {
        RHICmdList.SetViewport(0, 0, 0, Size.X, Size.Y, 1);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<FTerrainMassShapeShaderVS> VertexShader(ShaderMap);
        VertexShader->SetParameters(RHICmdList, ShaderParams);
        TShaderMapRef<FTerrainMassShapeShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);

        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTerrainMassShapeVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
        RHICmdList.DrawIndexedPrimitive(
            IndexBufferRHI,
            0,                      //BaseVertexIndex
            0,                      //MinIndex
            ShapePoints.Num(),      //NumVertices
            0,                      //StartIndex
            ShapePoints.Num() - 2,  //NumPrimitives
            1                       //NumInstances
        );
    }
    RHICmdList.EndRenderPass();

    IndexBufferRHI.SafeRelease();
    VertexBufferRHI.SafeRelease();
}

void FTerrainMassShapeShader::Render(UTextureRenderTarget2D* OutputRT, const TArray<FVector>& ShapePoints, const FTerrainMassShapeShaderParameter& ShaderParams)
{
    ENQUEUE_RENDER_COMMAND(TerranMassDummyBrush)(
        [OutputRT, ShapePoints, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Render_RenderingThread(RHICmdList, OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(), 
                    FIntPoint(OutputRT->SizeX, OutputRT->SizeY), ShapePoints, ShaderParams);
            }
        }
    );
}
