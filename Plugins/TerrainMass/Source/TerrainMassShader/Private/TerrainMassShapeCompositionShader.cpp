#include "TerrainMassShapeCompositionShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

class FTerrainMassShapeCompositionShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassShapeCompositionShaderPS);

public:
	FTerrainMassShapeCompositionShaderPS()
		: FGlobalShader()
	{}

	FTerrainMassShapeCompositionShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        BlendTextureParam.Bind(Initializer.ParameterMap, TEXT("BlendTexture"));
        BlendTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("BlendTextureSampler"));
        SideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTexture"));
        SideFalloffTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTextureSampler"));
        ElevationParam.Bind(Initializer.ParameterMap, TEXT("Elevation"));
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassShapeCompositionShaderParameter& Params)
    {
        if (Params.SideFalloffTexture.IsValid() &&
            Params.SideFalloffTexture->Resource &&
            Params.SideFalloffTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SideFalloffTextureParam, SideFalloffTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.SideFalloffTexture->Resource->TextureRHI);
        }

        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ElevationParam, Params.Elevation);
    }

    void SetInput(FRHICommandList& RHICmdList, FRHITexture* InputTexture, FRHITexture* BlendTexture)
    {
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), InputTexture);
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), BlendTextureParam, BlendTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), BlendTexture);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
    LAYOUT_FIELD(FShaderResourceParameter, BlendTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, BlendTextureSamplerParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureSamplerParam);
	LAYOUT_FIELD(FShaderParameter, ElevationParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassShapeCompositionShaderPS, "/TerrainMassShaders/TerrainMassShapeComposition.usf", "MainPS", SF_Pixel);

static void Render_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, FRHITexture* InputTexture, FRHITexture* BlendTexture,
    const FIntPoint& Size, const FTerrainMassShapeCompositionShaderParameter& ShaderParams)
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
        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
        TShaderMapRef<FTerrainMassShapeCompositionShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);
        PixelShader->SetInput(RHICmdList, InputTexture, BlendTexture);

        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

        RendererModule->DrawRectangle(
            RHICmdList,
            0, 0,             // Dest X, Y
            Size.X, Size.Y,   // Dest Width, Height
            0, 0,             // Source U, V
            1, 1,             // Source USize, VSize
            Size,             // Target buffer size
            FIntPoint(1, 1),  // Source texture size
            VertexShader);
    }
    RHICmdList.EndRenderPass();
}

void FTerrainMassShapeCompositionShader::Render(UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* BlendRT,
    const FTerrainMassShapeCompositionShaderParameter& ShaderParams)
{
    ENQUEUE_RENDER_COMMAND(TerranMassShapeComposition)(
        [InputRT, BlendRT, OutputRT, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture() &&
                InputRT->GetRenderTargetResource() && InputRT->GetRenderTargetResource()->GetRenderTargetTexture() &&
                BlendRT->GetRenderTargetResource() && BlendRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Render_RenderingThread(RHICmdList, OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(), 
                    InputRT->GetRenderTargetResource()->GetRenderTargetTexture(), BlendRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    FIntPoint(InputRT->SizeX, InputRT->SizeY), ShaderParams);
            }
        }
    );
}
