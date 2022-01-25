#include "TerrainMassPolygonBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"

#include "TerrainMassLibrary.h"
#include "TerrainMassPolygonShader.h"
#include "TerrainMassPolygonCompositeShader.h"

ATerrainMassPolygonBrush::ATerrainMassPolygonBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);
}

UTextureRenderTarget2D* ATerrainMassPolygonBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);
    check(GetOwningLandscape());

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);
    if (!UTerrainMassLibrary::CreateOrUpdateRenderTarget2D(GetTransientPackage(), CanvasRT, RenderTargetSize, RTF_RG16f, true) ||
        !UTerrainMassLibrary::CreateOrUpdateRenderTarget2D(GetTransientPackage(), OutputRT, RenderTargetSize, RTF_RGBA8, true))
    {
        // LandscapeEditLayers.cpp - ALandscape::RegenerateLayersHeightmaps
        //
        // if Brush::Render_Native() return non-null value, returned RT will be copy to InCombinedResult by ExecuteCopyLayersTexture(),
        // otherwise, InCombinedResult won't be touched by the returned value.
        //
        // Side note: 
        // if returned RT's format is different to InCombinedResult, copy operation will be excecuted, but the content of destination does not change.
        return nullptr;
    }

    FVector TransformedStartPosition = GetOwningLandscape()->GetActorTransform().InverseTransformPosition(StartPosition);
    FVector TransformedEndPosition = GetOwningLandscape()->GetActorTransform().InverseTransformPosition(EndPosition);
    TransformedStartPosition.Z *= LANDSCAPE_INV_ZSCALE;
    TransformedEndPosition.Z *= LANDSCAPE_INV_ZSCALE;
    float Scale = 1.0f / GetOwningLandscape()->GetActorScale3D().X;
    float TransformedWidth = Width * Scale;
    float TransformdSideFalloff = SideFalloff * Scale;
    float TransformedEndFalloff = EndFalloff * Scale;

    FTerrainMassPolygonShaderParameter ShaderParams;
    ShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);

    ShaderParams.Width = Width;
    ShaderParams.SideFalloff = SideFalloff;
    ShaderParams.EndFalloff = EndFalloff;
    ShaderParams.StartPosition = StartPosition;
    ShaderParams.EndPosition = EndPosition;
    ShaderParams.NumSegments = NumSegments;
    ShaderParams.StartSidefFalloffTexture = StartSideFalloffTexture;
    ShaderParams.EndSideFalloffTexture = EndSideFalloffTexture;
    ShaderParams.WorldToCanvasTransform = GetOwningLandscape()->GetActorTransform().Inverse();

    ENQUEUE_RENDER_COMMAND(TerranMassPolygonBrush)(
        [this, RenderTargetSize, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (CanvasRT->GetRenderTargetResource() && CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                FTerrainMassPolygonShader::Render(RHICmdList,
                    CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    RenderTargetSize, ShaderParams);
            }
        }
    );

    FTerrainMassPolygonCompositeShaderParameter CompositeShaderParams;
    CompositeShaderParams.SourceTexture = InCombinedResult;
    CompositeShaderParams.CanvasTexture = CanvasRT;
    CompositeShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);

    ENQUEUE_RENDER_COMMAND(TerranMassPolygonBrushComposite)(
        [this, RenderTargetSize, CompositeShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (OutputRT->GetRenderTargetResource() && OutputRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                FTerrainMassPolygonCompositeShader::Render(RHICmdList,
                    OutputRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    RenderTargetSize, CompositeShaderParams);
            }
        }
    );

    return OutputRT;
}

void ATerrainMassPolygonBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
    InitSideFalloffCurve(StartSideFalloffCurve);
    InitSideFalloffCurve(EndSideFalloffCurve);
}

#if WITH_EDITOR
void ATerrainMassPolygonBrush::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassPolygonBrush, StartSideFalloffCurve))
    {
        UpdateSideFalloffTexture(StartSideFalloffCurve, StartSideFalloffTexture);
    }
    else if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassPolygonBrush, EndSideFalloffCurve))
    {
        UpdateSideFalloffTexture(EndSideFalloffCurve, EndSideFalloffTexture);
    }
}
#endif

void ATerrainMassPolygonBrush::InitSideFalloffCurve(FRuntimeFloatCurve& SideFalloffCurve)
{
    FRichCurve* Curve = SideFalloffCurve.GetRichCurve();
    if (Curve && Curve->GetNumKeys() < 1)
    {
        FKeyHandle FirstKeyHandle = Curve->AddKey(0, 0);
        FKeyHandle LastKeyHandle = Curve->AddKey(1, 1);
        Curve->SetKeyInterpMode(FirstKeyHandle, RCIM_Cubic, false);
        Curve->SetKeyInterpMode(LastKeyHandle, RCIM_Cubic, false);
        Curve->SetKeyTangentMode(FirstKeyHandle, RCTM_Auto, false);
        Curve->SetKeyTangentMode(LastKeyHandle, RCTM_Auto, false);
    }
}

void ATerrainMassPolygonBrush::UpdateSideFalloffTexture(FRuntimeFloatCurve& SideFalloffCurve, UTexture2D* SideFalloffTexture)
{
    if (!UTerrainMassLibrary::CreateOrUpdateTexture2D(this, SideFalloffTexture, FIntPoint(256, 1), PF_G8, TA_Clamp, TA_Clamp))
    {
        return;
    }

    FRichCurve* Curve = SideFalloffCurve.GetRichCurve();
    if (Curve)
    {
        FKeyHandle FirstKeyHandle = Curve->GetFirstKeyHandle();
        float ClampedValue = FMath::Min(Curve->GetKeyValue(FirstKeyHandle), 0.0f);
        Curve->SetKeyValue(FirstKeyHandle, ClampedValue);

        float ClampedTime = FMath::Max(Curve->GetKeyTime(FirstKeyHandle), 0.0f);
        Curve->SetKeyTime(FirstKeyHandle, ClampedTime);
    }

    const int32 TextureWidth = SideFalloffTexture->GetSizeX();
    const int32 TotalSize = TextureWidth * sizeof(uint8);
    uint8* Pixels = new uint8[TotalSize];

    for (int32 Index = 0; Index < TextureWidth; Index++)
    {
        float Time = (float)Index / (float)(TextureWidth - 1);
        float CurveValue = Time;
        if (Curve && Curve->GetNumKeys() > 0)
        {
            CurveValue = FMath::Clamp(Curve->Eval(Time), 0.0f, 1.0f);
        }
        Pixels[Index] = (uint8)(CurveValue * 255);
    }

    uint8* MipData = (uint8*)SideFalloffTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(MipData, Pixels, TotalSize);
    SideFalloffTexture->PlatformData->Mips[0].BulkData.Unlock();

    SideFalloffTexture->UpdateResource();

    delete[] Pixels;
}