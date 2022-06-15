#include "TerrainMassShapeShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

class FTerrainMassShapeVertexDeclaration : public FRenderResource
{
public:
    FVertexDeclarationRHIRef VertexDeclarationRHI;

    virtual ~FTerrainMassShapeVertexDeclaration() {}

    virtual void InitRHI()
    {
        FVertexDeclarationElementList Elements;
        uint32 Stride = sizeof(FTerrainMassShapeVertex);
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassShapeVertex, Position), VET_Float3, 0, Stride));
        //Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTerrainMassShapeVertex, ExtraData), VET_Float4, 1, Stride));
        VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
    }

    virtual void ReleaseRHI()
    {
        VertexDeclarationRHI.SafeRelease();
    }
};

TGlobalResource<FTerrainMassShapeVertexDeclaration> GTerrainMassShapeVertexDeclaration;

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
        World2UVParam.Bind(Initializer.ParameterMap, TEXT("World2UV"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassShapeShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), InvTextureSizeParam, Params.InvTextureSize);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), World2UVParam, Params.World2UV);
    }

private:
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
    LAYOUT_FIELD(FShaderParameter, World2UVParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassShapeShaderVS, "/TerrainMassShaders/TerrainMassShape.usf", "MainVS", SF_Vertex);

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
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassShapeShaderParameter& Params)
    {
    }

private:
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassShapeShaderPS, "/TerrainMassShaders/TerrainMassShape.usf", "MainPS", SF_Pixel);

static void Render_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, const FIntPoint& Size, const TArray<FTerrainMassShapeVertex>& ShapePoints, const FTerrainMassShapeShaderParameter& ShaderParams)
{
    if (ShapePoints.Num() < 3)
    {
        return;
    }

    //
    // Vertex Buffer
    //
    TResourceArray<FTerrainMassShapeVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    Vertices.AddUninitialized(ShapePoints.Num());
    FMemory::Memcpy(Vertices.GetData(), ShapePoints.GetData(), ShapePoints.Num() * sizeof(FTerrainMassShapeVertex));

    // Create vertex buffer. Fill buffer with initial data upon creation
    FRHIResourceCreateInfo CreateInfoVB(&Vertices);
    FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfoVB);

    // Setup index buffer
    TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
    const int32 NumPrimitives = ShapePoints.Num() - 2;
    const int32 NumIndices = NumPrimitives * 3;
    IndexBuffer.AddUninitialized(NumIndices);

    for (int32 Index = 0; Index < NumPrimitives; Index++)
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
            0,                  //BaseVertexIndex
            0,                  //FirstInstance
            ShapePoints.Num(),  //NumVertices
            0,                  //StartIndex
            NumPrimitives,      //NumPrimitives
            1                   //NumInstances
        );
    }
    RHICmdList.EndRenderPass();

    IndexBufferRHI.SafeRelease();
    VertexBufferRHI.SafeRelease();
}

void FTerrainMassShapeShader::Render(UTextureRenderTarget2D* OutputRT, const TArray<FTerrainMassShapeVertex>& ShapePoints, const FTerrainMassShapeShaderParameter& ShaderParams)
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
