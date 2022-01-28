#include "TerrainMassPolygonCompositeShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"

//
// Pixel Shader
//
class FTerrainMassPolygonCompositeShaderPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassPolygonCompositeShaderPS);

public:
    FTerrainMassPolygonCompositeShaderPS()
        : FGlobalShader()
    {}

    FTerrainMassPolygonCompositeShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        SourceTextureParam.Bind(Initializer.ParameterMap, TEXT("SourceTexture"));
        SourceTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SourceTextureSampler"));
        CanvasTextureParam.Bind(Initializer.ParameterMap, TEXT("CanvasTexture"));
        CanvasTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("CanvasTextureSampler"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassPolygonCompositeShaderParameter& Params)
    {
        if (Params.SourceTexture &&
            Params.SourceTexture->Resource &&
            Params.SourceTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SourceTextureParam, SourceTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.SourceTexture->Resource->TextureRHI);
        }

        if (Params.CanvasTexture &&
            Params.CanvasTexture->Resource &&
            Params.CanvasTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), CanvasTextureParam, CanvasTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.CanvasTexture->Resource->TextureRHI);
        }
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, SourceTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, SourceTextureSamplerParam);
    LAYOUT_FIELD(FShaderResourceParameter, CanvasTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, CanvasTextureSamplerParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassPolygonCompositeShaderPS, "/TerrainMassShaders/TerrainMassPolygonComposite.usf", "MainPS", SF_Pixel);

void FTerrainMassPolygonCompositeShader::Render(FRHICommandListImmediate& RHICmdList, FRHITexture* DestTexture, const FTerrainMassPolygonCompositeShaderParameter& ShaderParams)
{
    IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
    FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("TerrainMassComposite"));
    {
        RHICmdList.SetViewport(0, 0, 0, ShaderParams.RenderTargetSize.X, ShaderParams.RenderTargetSize.Y, 1);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
        TShaderMapRef<FTerrainMassPolygonCompositeShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);

        GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
        GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
        GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
        GraphicsPSOInit.PrimitiveType = PT_TriangleList;

        SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
        RendererModule->DrawRectangle(
            RHICmdList,
            0, 0,                                                                                       // Dest X, Y
            ShaderParams.RenderTargetSize.X, ShaderParams.RenderTargetSize.Y,                           // Dest Width, Height
            0, 0,                                                                                       // Source U, V
            1, 1,                                                                                       // Source USize, VSize
            FIntPoint((int32)ShaderParams.RenderTargetSize.X, (int32)ShaderParams.RenderTargetSize.Y),  // Target buffer size
            FIntPoint(1, 1),                                                                            // Source texture size
            VertexShader);
    }
    RHICmdList.EndRenderPass();
}