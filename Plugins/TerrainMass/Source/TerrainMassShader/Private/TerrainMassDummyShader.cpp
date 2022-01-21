#include "TerrainMassDummyShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"

class FTerrainMassDummyShaderVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassDummyShaderVS);

public:
	FTerrainMassDummyShaderVS()
		: FGlobalShader()
	{}

	FTerrainMassDummyShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassDummyShaderVS, "/TerrainMassShaders/TerrainMassDummy.usf", "MainVS", SF_Vertex)

class FTerrainMassDummyShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassDummyShaderPS);

public:
	FTerrainMassDummyShaderPS()
		: FGlobalShader()
	{}

	FTerrainMassDummyShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        SourceTextureParam.Bind(Initializer.ParameterMap, TEXT("SourceTexture"));
        SourceTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SourceTextureSampler"));
        SideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTexture"));
        SideFalloffTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
        CenterParam.Bind(Initializer.ParameterMap, TEXT("Center"));
        RadiusParam.Bind(Initializer.ParameterMap, TEXT("Radius"));
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassDummyShaderParameter& Params)
    {
        if (Params.SourceTexture &&
            Params.SourceTexture->Resource &&
            Params.SourceTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SourceTextureParam, SourceTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.SourceTexture->Resource->TextureRHI);
        }

        if (Params.SideFalloffTexture &&
            Params.SideFalloffTexture->Resource &&
            Params.SideFalloffTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SideFalloffTextureParam, SideFalloffTextureSamplerParam,
                TStaticSamplerState<>::GetRHI(), Params.SideFalloffTexture->Resource->TextureRHI);
        }

        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), CenterParam, Params.Center);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), RadiusParam, Params.Radius);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, SourceTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, SourceTextureSamplerParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureSamplerParam);
	LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
	LAYOUT_FIELD(FShaderParameter, CenterParam);
	LAYOUT_FIELD(FShaderParameter, RadiusParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassDummyShaderPS, "/TerrainMassShaders/TerrainMassDummy.usf", "MainPS", SF_Pixel);

void FTerrainMassDummyShader::Render(FRHICommandListImmediate& RHICmdList, FRHITexture* SourceTexture, FRHITexture* DestTexture, const FIntPoint& Size, const FTerrainMassDummyShaderParameter& ShaderParams)
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
        TShaderMapRef<FTerrainMassDummyShaderPS> PixelShader(ShaderMap);
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