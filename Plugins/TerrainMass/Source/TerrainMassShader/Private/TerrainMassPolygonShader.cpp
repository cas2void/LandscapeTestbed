#include "TerrainMassPolygonShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"

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

void FTerrainMassPolygonShader::Render(FRHICommandListImmediate& RHICmdList, FRHITexture* SourceTexture, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassPolygonShaderParameter& ShaderParams)
{
    IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
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
        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
        TShaderMapRef<FTerrainMassPolygonShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);

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