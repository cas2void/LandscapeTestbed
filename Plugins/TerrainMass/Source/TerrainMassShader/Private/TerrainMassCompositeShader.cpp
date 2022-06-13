#include "TerrainMassCompositeShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
class FTerrainMassCompositeShaderVS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassCompositeShaderVS);

public:
    FTerrainMassCompositeShaderVS()
        : FGlobalShader()
    {}

    FTerrainMassCompositeShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        PosScaleBiasParam.Bind(Initializer.ParameterMap, TEXT("PosScaleBias"));
        UVScaleBiasParam.Bind(Initializer.ParameterMap, TEXT("UVScaleBias"));
        InvTargetSizeAndTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTargetSizeAndTextureSize"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassCompositeShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), PosScaleBiasParam, Params.PosScaleBias);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), UVScaleBiasParam, Params.UVScaleBias);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), InvTargetSizeAndTextureSizeParam, Params.InvTargetSizeAndTextureSize);
    }

private:
    LAYOUT_FIELD(FShaderParameter, PosScaleBiasParam);
    LAYOUT_FIELD(FShaderParameter, UVScaleBiasParam);
    LAYOUT_FIELD(FShaderParameter, InvTargetSizeAndTextureSizeParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassCompositeShaderVS, "/TerrainMassShaders/TerrainMassComposite.usf", "MainVS", SF_Vertex);
#endif

class FTerrainMassCompositeShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassCompositeShaderPS);

public:
	FTerrainMassCompositeShaderPS()
		: FGlobalShader()
	{}

	FTerrainMassCompositeShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        CanvasTextureParam.Bind(Initializer.ParameterMap, TEXT("CanvasTexture"));
        CanvasTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("CanvasTextureSampler"));
        BlendTextureParam.Bind(Initializer.ParameterMap, TEXT("BlendTexture"));
        BlendTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("BlendTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassCompositeShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
    }

    void SetInputOutput(FRHICommandList& RHICmdList, FRHITexture* InputTexture, FRHITexture* CanvasTexture, FRHITexture* BlendTexture)
    {
        if (InputTexture && CanvasTexture && BlendTexture)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam, TStaticSamplerState<>::GetRHI(), InputTexture);
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), CanvasTextureParam, CanvasTextureSamplerParam, TStaticSamplerState<>::GetRHI(), CanvasTexture);
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), BlendTextureParam, BlendTextureSamplerParam, TStaticSamplerState<>::GetRHI(), BlendTexture);
        }
    }

private:
	LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
	LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
    LAYOUT_FIELD(FShaderResourceParameter, CanvasTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, CanvasTextureSamplerParam);
    LAYOUT_FIELD(FShaderResourceParameter, BlendTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, BlendTextureSamplerParam);
	LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
	LAYOUT_FIELD(FShaderParameter, CenterParam);
	LAYOUT_FIELD(FShaderParameter, RadiusParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassCompositeShaderPS, "/TerrainMassShaders/TerrainMassComposite.usf", "MainPS", SF_Pixel);

static void Render_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* InputTexture, FRHITexture* CanvasTexture, 
    FRHITexture* BlendTexture, FRHITexture* OutputTexture, const FIntPoint& Size, const FTerrainMassCompositeShaderParameter& ShaderParams)
{
    IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
    FRHIRenderPassInfo RPInfo(OutputTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("TerrainMass"));
    {
        RHICmdList.SetViewport(0, 0, 0, Size.X, Size.Y, 1);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
        TShaderMapRef<FTerrainMassCompositeShaderVS> VertexShader(ShaderMap);
        VertexShader->SetParameters(RHICmdList, ShaderParams);
#else
        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
#endif
        TShaderMapRef<FTerrainMassCompositeShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);
        PixelShader->SetInputOutput(RHICmdList, InputTexture, CanvasTexture, BlendTexture);

        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
        RendererModule->DrawRectangle(
            RHICmdList,
            ShaderParams.PosScaleBias.Z, ShaderParams.PosScaleBias.W,             // Dest X, Y
            ShaderParams.PosScaleBias.X, ShaderParams.PosScaleBias.Y,             // Dest Width, Height
            ShaderParams.UVScaleBias.Z, ShaderParams.UVScaleBias.W,               // Source U, V
            ShaderParams.UVScaleBias.X, ShaderParams.UVScaleBias.Y,               // Source USize, VSize
            FIntPoint(ShaderParams.PosScaleBias.X, ShaderParams.PosScaleBias.Y),  // Target buffer size
            FIntPoint(ShaderParams.UVScaleBias.X, ShaderParams.UVScaleBias.Y),    // Source texture size
            VertexShader);
#else
        RendererModule->DrawRectangle(
            RHICmdList,
            0, 0,             // Dest X, Y
            Size.X, Size.Y,   // Dest Width, Height
            0, 0,             // Source U, V
            1, 1,             // Source USize, VSize
            Size,             // Target buffer size
            FIntPoint(1, 1),  // Source texture size
            VertexShader);
#endif
    }
    RHICmdList.EndRenderPass();
}

void FTerrainMassCompositeShader::Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* CanvasRT, UTextureRenderTarget2D* BlendRT, UTextureRenderTarget2D* OutputRT,
    const FIntPoint& Size, const FTerrainMassCompositeShaderParameter& ShaderParams)
{
    ENQUEUE_RENDER_COMMAND(TerranMassDummyBrush)(
        [InputRT, CanvasRT, BlendRT, OutputRT, Size, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (InputRT->GetRenderTargetResource() && InputRT->GetRenderTargetResource()->GetRenderTargetTexture() &&
                CanvasRT->GetRenderTargetResource() && CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture() &&
                BlendRT->GetRenderTargetResource() && BlendRT->GetRenderTargetResource()->GetRenderTargetTexture() &&
                OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Render_RenderingThread(
                    RHICmdList, 
                    InputRT->GetRenderTargetResource()->GetRenderTargetTexture(), 
                    CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    BlendRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    Size, ShaderParams);
            }
        }
    );
}
