#include "TerrainMassJumpFloodingShader.h"

#include "GlobalShader.h"
#include "Modules/ModuleManager.h"
#include "RendererInterface.h"
#include "CommonRenderResources.h"
#include "PipelineStateCache.h"
#include "ScreenRendering.h"
#include "RHICommandList.h"
#include "RHIResources.h"

class FTerrainMassJumpFloodingEncodeShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTerrainMassJumpFloodingEncodeShaderPS);

public:
	FTerrainMassJumpFloodingEncodeShaderPS()
		: FGlobalShader()
	{}

	FTerrainMassJumpFloodingEncodeShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
    }

    void SetInput(FRHICommandList& RHICmdList, FRHITexture* InputTexture)
    {
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), InputTexture);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassJumpFloodingEncodeShaderPS, "/TerrainMassShaders/TerrainMassJumpFlooding.usf", "EncodePS", SF_Pixel);

static void Encode_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, FRHITexture* InputTexture, const FIntPoint& Size)
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
        TShaderMapRef<FTerrainMassJumpFloodingEncodeShaderPS> PixelShader(ShaderMap);
        PixelShader->SetInput(RHICmdList, InputTexture);

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

void FTerrainMassJumpFloodingShader::Encode(UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* InputRT)
{
    ENQUEUE_RENDER_COMMAND(TerranMassJumpFloodingEncode)(
        [OutputRT, InputRT](FRHICommandListImmediate& RHICmdList)
        {
            if (OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture() ||
                InputRT->GetRenderTargetResource() && InputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                Encode_RenderingThread(RHICmdList, OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(), InputRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    FIntPoint(InputRT->SizeX, InputRT->SizeY));
            }
        }
    );
}

class FTerrainMassJumpFloodingStepShaderPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassJumpFloodingStepShaderPS);

public:
    FTerrainMassJumpFloodingStepShaderPS()
        : FGlobalShader()
    {}

    FTerrainMassJumpFloodingStepShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
        KernelSizeParam.Bind(Initializer.ParameterMap, TEXT("KernelSize"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassJumpFloodingShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
    }

    void SetInput(FRHICommandList& RHICmdList, FRHITexture* InputTexture)
    {
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), InputTexture);
    }

    void SetKernelSize(FRHICommandList& RHICmdList, float KernelSize)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), KernelSizeParam, KernelSize);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
    LAYOUT_FIELD(FShaderParameter, KernelSizeParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassJumpFloodingStepShaderPS, "/TerrainMassShaders/TerrainMassJumpFlooding.usf", "MainPS", SF_Pixel);

static void FloodSingleStep_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, FRHITexture* InputTexture, const FIntPoint& Size,
    int32 KernelSize, const FTerrainMassJumpFloodingShaderParameter& ShaderParams)
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
        TShaderMapRef<FTerrainMassJumpFloodingStepShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);
        PixelShader->SetInput(RHICmdList, InputTexture);
        PixelShader->SetKernelSize(RHICmdList, KernelSize);

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

static int32 GetJumpFloodingMaxIteration(UTextureRenderTarget2D* OutputRT)
{
    int32 MaxSize = FMath::Max<int32>(OutputRT->SizeX, OutputRT->SizeY);
    int32 MaxIteration = FMath::CeilToInt(FMath::Log2(MaxSize));
    return MaxIteration;
}

static void RenderJumpFlooding(UTextureRenderTarget2D* OutputRTs[], int32& OutputIndex, int32 InputIndex, int32 MaxIteration, int32 NumIteration, const FTerrainMassJumpFloodingShaderParameter& ShaderParams)
{
    for (int32 Index = 0; Index < NumIteration; Index++)
    {
        int32 KernelSize = FMath::Pow(2, MaxIteration - Index - 1);
        OutputIndex = (InputIndex + 1) % 2;

        UTextureRenderTarget2D* SourceRT = OutputRTs[InputIndex];
        UTextureRenderTarget2D* DestRT = OutputRTs[OutputIndex];

        ENQUEUE_RENDER_COMMAND(TerranMassJumpFlooding)(
            [DestRT, SourceRT, KernelSize, ShaderParams](FRHICommandListImmediate& RHICmdList)
            {
                if (DestRT->GetRenderTargetResource() && DestRT->GetRenderTargetResource()->GetRenderTargetTexture() ||
                    SourceRT->GetRenderTargetResource() && SourceRT->GetRenderTargetResource()->GetRenderTargetTexture())
                {
                    FloodSingleStep_RenderingThread(RHICmdList, DestRT->GetRenderTargetResource()->GetRenderTargetTexture(), SourceRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                        FIntPoint(SourceRT->SizeX, SourceRT->SizeY), KernelSize, ShaderParams);
                }
            }
        );

        InputIndex = OutputIndex;
    }
}

void FTerrainMassJumpFloodingShader::Flood(UTextureRenderTarget2D* OutputRTs[], int32& OutputIndex, int32 InputIndex, int32 NumIteration, const FTerrainMassJumpFloodingShaderParameter& ShaderParams)
{
    int32 MaxIteration = GetJumpFloodingMaxIteration(OutputRTs[InputIndex]);

    RenderJumpFlooding(OutputRTs, OutputIndex, InputIndex, MaxIteration, NumIteration, ShaderParams);
}

void FTerrainMassJumpFloodingShader::Flood(UTextureRenderTarget2D* OutputRTs[], int32& OutputIndex, int32 InputIndex, const FTerrainMassJumpFloodingShaderParameter& ShaderParams)
{
    int32 MaxIteration = GetJumpFloodingMaxIteration(OutputRTs[InputIndex]);

    RenderJumpFlooding(OutputRTs, OutputIndex, InputIndex, MaxIteration, MaxIteration, ShaderParams);
}

class FTerrainMassJumpFloodingDistanceFieldShaderPS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTerrainMassJumpFloodingDistanceFieldShaderPS);

public:
    FTerrainMassJumpFloodingDistanceFieldShaderPS()
        : FGlobalShader()
    {}

    FTerrainMassJumpFloodingDistanceFieldShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
        : FGlobalShader(Initializer)
    {
        InputTextureParam.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
        InputTextureSamplerParam.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
        InvTextureSizeParam.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"));
        WidthParam.Bind(Initializer.ParameterMap, TEXT("Width"));
    }

    void SetParameters(FRHICommandList& RHICmdList, const FTerrainMassDistanceFieldShaderParameter& Params)
    {
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvTextureSizeParam, Params.InvTextureSize);
        SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), WidthParam, Params.Width);
    }

    void SetInput(FRHICommandList& RHICmdList, FRHITexture* InputTexture)
    {
        SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InputTextureParam, InputTextureSamplerParam,
            TStaticSamplerState<>::GetRHI(), InputTexture);
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureParam);
    LAYOUT_FIELD(FShaderResourceParameter, InputTextureSamplerParam);
    LAYOUT_FIELD(FShaderParameter, InvTextureSizeParam);
    LAYOUT_FIELD(FShaderParameter, WidthParam);
};

IMPLEMENT_GLOBAL_SHADER(FTerrainMassJumpFloodingDistanceFieldShaderPS, "/TerrainMassShaders/TerrainMassJumpFlooding.usf", "DistanceFieldPS", SF_Pixel);

static void DistanceField_RenderingThread(FRHICommandListImmediate& RHICmdList, FRHITexture* OutputTexture, FRHITexture* InputTexture, const FIntPoint& Size,
    const FTerrainMassDistanceFieldShaderParameter& ShaderParams)
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
        TShaderMapRef<FTerrainMassJumpFloodingDistanceFieldShaderPS> PixelShader(ShaderMap);
        PixelShader->SetParameters(RHICmdList, ShaderParams);
        PixelShader->SetInput(RHICmdList, InputTexture);

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

void FTerrainMassJumpFloodingShader::DistanceField(UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* InputRT, const FTerrainMassDistanceFieldShaderParameter& ShaderParams)
{
    ENQUEUE_RENDER_COMMAND(TerranMassJumpFlooding)(
        [OutputRT, InputRT, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture() ||
                InputRT->GetRenderTargetResource() && InputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                DistanceField_RenderingThread(RHICmdList, OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(), InputRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    FIntPoint(InputRT->SizeX, InputRT->SizeY), ShaderParams);
            }
        }
    );
}
