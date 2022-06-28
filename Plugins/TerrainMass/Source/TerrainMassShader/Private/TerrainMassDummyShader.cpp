#include "TerrainMassDummyShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
class FTerrainMassDummyShaderVS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassDummyShaderVS);

public:
    FTerrainMassDummyShaderVS()
        : FGlobalShader()
    {}

    FTerrainMassDummyShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        PosScaleBiasParam.Bind(Initializer.ParameterMap, TEXT("PosScaleBias"));
        UVScaleBiasParam.Bind(Initializer.ParameterMap, TEXT("UVScaleBias"));
        InvTargetSizeAndTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTargetSizeAndTextureSize"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassDummyShaderParameter& Params)
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

IMPLEMENT_GLOBAL_SHADER(FTerrainMassDummyShaderVS, "/TerrainMassShaders/TerrainMassDummy.usf", "MainVS", SF_Vertex);
#endif

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
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        SideFalloffTextureParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTexture"));
        SideFalloffTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("SideFalloffTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
        CenterParam.Bind(Initializer.ParameterMap, TEXT("Center"));
        RadiusParam.Bind(Initializer.ParameterMap, TEXT("Radius"));
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassDummyShaderParameter& Params)
    {
        if (Params.SideFalloffTexture.IsValid() &&
            Params.SideFalloffTexture->Resource &&
            Params.SideFalloffTexture->Resource->TextureRHI)
        {
            SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SideFalloffTextureParam, SideFalloffTextureSamplerParam,
                TStaticSamplerState<SF_Bilinear>::GetRHI(), Params.SideFalloffTexture->Resource->TextureRHI);
        }

        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), CenterParam, Params.Center);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), RadiusParam, Params.Radius);
    }

    void SetInput(FRHICommandList& RHICmdList, FRHITexture* InputTexture)
    {
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), InputTexture);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureParam);
	LAYOUT_FIELD(FShaderResourceParameter, SideFalloffTextureSamplerParam);
	LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
	LAYOUT_FIELD(FShaderParameter, CenterParam);
	LAYOUT_FIELD(FShaderParameter, RadiusParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassDummyShaderPS, "/TerrainMassShaders/TerrainMassDummy.usf", "MainPS", SF_Pixel);

static void Render_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* InputTexture, FRHITexture* OutputTexture, const FIntPoint& Size, const FTerrainMassDummyShaderParameter& ShaderParams)
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
        TShaderMapRef<FTerrainMassDummyShaderVS> VertexShader(ShaderMap);
        VertexShader->SetParameters(RHICmdList, ShaderParams);
#else
        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
#endif
        TShaderMapRef<FTerrainMassDummyShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);
        PixelShader->SetInput(RHICmdList, InputTexture);

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

void FTerrainMassDummyShader::Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* OutputRT, const FTerrainMassDummyShaderParameter& ShaderParams)
{
    ENQUEUE_RENDER_COMMAND(TerranMassDummy)(
        [InputRT, OutputRT, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (InputRT->GetRenderTargetResource() && InputRT->GetRenderTargetResource()->GetRenderTargetTexture() ||
                OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Render_RenderingThread(RHICmdList, InputRT->GetRenderTargetResource()->GetRenderTargetTexture(), OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(), 
                    FIntPoint(InputRT->SizeX, InputRT->SizeY), ShaderParams);
            }
        }
    );
}
