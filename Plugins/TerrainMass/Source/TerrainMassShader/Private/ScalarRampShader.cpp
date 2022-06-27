// Fill out your copyright notice in the Description page of Project Settings.

#include "ScalarRampShader.h"

#include "RenderCommandFence.h"
#include "RenderingThread.h"
#include "RHICommandList.h"

void FScalarRampShader::WaitForGPU()
{
    FRenderCommandFence Fence;
    Fence.BeginFence();
    Fence.Wait();
}

void FScalarRampShader::RenderRampToTexture(const FRuntimeFloatCurve& Curve, UTexture2DDynamic* ScalarRampTexture)
{
    check(ScalarRampTexture);

    ENQUEUE_RENDER_COMMAND(UpdateScalarRampTexture)(
        [Curve, ScalarRampTexture](FRHICommandListImmediate& RHICmdList)
        {
            FRHITexture2D* RHITexture2D = ScalarRampTexture->GetResource()->GetTexture2DRHI();
            if (RHITexture2D)
            {
                uint32 DestStride;
                uint16* Buffer = static_cast<uint16*>(RHILockTexture2D(RHITexture2D, 0, RLM_WriteOnly, DestStride, false));
                if (Buffer)
                {
                    uint32 TextureWidth = RHITexture2D->GetSizeX();
                    check(TextureWidth > 1);

                    const FRichCurve* RichCurve = Curve.GetRichCurveConst();
                    check(RichCurve);

                    if (RichCurve->GetNumKeys() < 1)
                    {
                    	FMemory::Memzero(Buffer, TextureWidth * sizeof(uint16));
                    }
                    else
                    {
                    	for (int32 Index = 0; Index < static_cast<int32>(TextureWidth); Index++)
                    	{
                            float Time = (float)Index / (float)(TextureWidth - 1);
                            float CurveValue = FMath::Clamp(RichCurve->Eval(Time), 0.0f, 1.0f);
                    		Buffer[Index] = (uint16)(CurveValue * 65535);
                    	}
                    }
                }
                RHIUnlockTexture2D(RHITexture2D, 0, false);
            }
        }
    );
}
