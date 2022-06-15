#include "TerrainMassGaussianBlurShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

class FTerrainMassGaussianBlurShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassGaussianBlurShaderPS);

public:
	FTerrainMassGaussianBlurShaderPS()
		: FGlobalShader()
	{}

	FTerrainMassGaussianBlurShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
        KernelSizeParam.Bind(Initializer.ParameterMap, TEXT("KernelSize"));
        SigmaParam.Bind(Initializer.ParameterMap, TEXT("Sigma"));
        DirectionParam.Bind(Initializer.ParameterMap, TEXT("Direction"));
    }

	void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassGaussianBlurShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
        int32 ValidatedKernelSize = FMath::Clamp<int32>(Params.KernelSize, 0, 100);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), KernelSizeParam, ValidatedKernelSize);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SigmaParam, Params.Sigma);
    }

    void SetInput(FRHICommandList& RHICmdList, FRHITexture* InputTexture)
    {
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), InputTexture);
    }

    void SetDirection(FRHICommandList& RHICmdList, const FVector2D& Direction)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), DirectionParam, Direction);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
	LAYOUT_FIELD(FShaderParameter, KernelSizeParam);
    LAYOUT_FIELD(FShaderParameter, SigmaParam);
    LAYOUT_FIELD(FShaderParameter, DirectionParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassGaussianBlurShaderPS, "/TerrainMassShaders/TerrainMassGaussianBlur.usf", "MainPS", SF_Pixel);

static void Render_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* InputTexture, FRHITexture* OutputTexture, const FIntPoint& Size, 
    const FTerrainMassGaussianBlurShaderParameter& ShaderParams, bool bRowPass)
{
    IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
    FRHIRenderPassInfo RPInfo(OutputTexture, ERenderTargetActions::Load_Store);
    RHICmdList.BeginRenderPass(RPInfo, TEXT("TerrainMassGaussianBlur"));
    {
        RHICmdList.SetViewport(0, 0, 0, Size.X, Size.Y, 1);

        FGraphicsPipelineStateInitializer GraphicsPSOInit;
        RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
        GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
        GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
        GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

        FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
        TShaderMapRef<FTerrainMassGaussianBlurShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);
        PixelShader->SetInput(RHICmdList, InputTexture);
        FVector2D Direction = bRowPass ? FVector2D(1.0f, 0.0f) : FVector2D(0.0f, 1.0f);
        PixelShader->SetDirection(RHICmdList, Direction);

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

void FTerrainMassGaussianBlurShader::Render(UTextureRenderTarget2D* InputRT, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* IntermediateRT, 
    const FTerrainMassGaussianBlurShaderParameter& ShaderParams)
{
    ENQUEUE_RENDER_COMMAND(TerranMassGaussianBlurRow)(
        [InputRT, IntermediateRT, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (InputRT->GetRenderTargetResource() && InputRT->GetRenderTargetResource()->GetRenderTargetTexture() ||
                IntermediateRT->GetRenderTargetResource() && IntermediateRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Render_RenderingThread(RHICmdList, InputRT->GetRenderTargetResource()->GetRenderTargetTexture(), IntermediateRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    FIntPoint(InputRT->SizeX, InputRT->SizeY), ShaderParams, true);
            }
        }
    );

    ENQUEUE_RENDER_COMMAND(TerranMassGaussianBlurColumn)(
        [IntermediateRT, OutputRT, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (IntermediateRT->GetRenderTargetResource() && IntermediateRT->GetRenderTargetResource()->GetRenderTargetTexture() ||
                OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Render_RenderingThread(RHICmdList, IntermediateRT->GetRenderTargetResource()->GetRenderTargetTexture(), OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    FIntPoint(IntermediateRT->SizeX, IntermediateRT->SizeY), ShaderParams, false);
            }
        }
    );
}
