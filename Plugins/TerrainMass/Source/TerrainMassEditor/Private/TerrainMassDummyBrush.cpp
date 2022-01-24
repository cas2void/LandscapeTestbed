#include "TerrainMassDummyBrush.h"

#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "Curves/CurveFloat.h"
#include "Engine/Texture2D.h"

#include "TerrainMassLibrary.h"
#include "TerrainMassDummyShader.h"

ATerrainMassDummyBrush::ATerrainMassDummyBrush()
{
    // At least one of these attributes has to be set true, otherwise the brush won't be created
    SetAffectsHeightmap(true);
    SetAffectsWeightmap(true);
}

UTextureRenderTarget2D* ATerrainMassDummyBrush::Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName)
{
    check(InCombinedResult);

    FIntPoint RenderTargetSize(InCombinedResult->SizeX, InCombinedResult->SizeY);
    if (!UTerrainMassLibrary::CreateOrUpdateRenderTarget2D(GetTransientPackage(), CanvasRT, RenderTargetSize, RTF_RGBA8, true))
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

    FVector Center = GetActorLocation();
    float RadiusScale = 1.0f;
    if (GetOwningLandscape())
    {
        FTransform BrushRelativeTransform = GetActorTransform().GetRelativeTransform(GetOwningLandscape()->GetActorTransform());
        Center = BrushRelativeTransform.GetLocation();
        RadiusScale = BrushRelativeTransform.GetScale3D().X;
    }
    Center.Z *= LANDSCAPE_INV_ZSCALE;

    FTerrainMassDummyShaderParameter ShaderParams;
    ShaderParams.SourceTexture = InCombinedResult;
    ShaderParams.SideFalloffTexture = SideFalloffTexture;
    ShaderParams.InvTextureSize = FVector2D(1.0f) / FVector2D(RenderTargetSize);
    ShaderParams.Center = Center;
    ShaderParams.Radius = Radius * RadiusScale / RenderTargetSize.X;
#if TERRAIN_MASS_DUMMY_CUSTOM_VERTEX_SHADER
    ShaderParams.PosScaleBias = FVector4(RenderTargetSize.X, RenderTargetSize.Y, 0.0f, 0.0f);
    ShaderParams.UVScaleBias = FVector4(1.0f, 1.0f, 0.0f, 0.0f);
    ShaderParams.InvTargetSizeAndTextureSize = FVector4(1.0f / RenderTargetSize.X, 1.0f / RenderTargetSize.Y, 1.0f, 1.0f);
#endif

    ENQUEUE_RENDER_COMMAND(TerranMassDummyBrush)(
        [this, InCombinedResult, RenderTargetSize, ShaderParams](FRHICommandListImmediate& RHICmdList)
        {
            if (InCombinedResult->GetRenderTargetResource() && InCombinedResult->GetRenderTargetResource()->GetRenderTargetTexture() &&
                CanvasRT->GetRenderTargetResource() && CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture())
            {
                FTerrainMassDummyShader::Render(RHICmdList,
                    InCombinedResult->GetRenderTargetResource()->GetRenderTargetTexture(),
                    CanvasRT->GetRenderTargetResource()->GetRenderTargetTexture(),
                    RenderTargetSize, ShaderParams);
            }
        }
    );

    return CanvasRT;
}

void ATerrainMassDummyBrush::Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize)
{
    InitSideFalloffCurve();
    UpdateSideFalloffTexture();
}

#if WITH_EDITOR
void ATerrainMassDummyBrush::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ATerrainMassDummyBrush, SideFalloffCurve))
    {
        UpdateSideFalloffTexture();
    }
}
#endif

void ATerrainMassDummyBrush::InitSideFalloffCurve()
{
    FRichCurve* Curve = SideFalloffCurve.GetRichCurve();
    if (Curve && Curve->GetNumKeys() < 1)
    {
        FKeyHandle FirstKeyHandle = Curve->AddKey(0, 0);
        FKeyHandle LastKeyHandle = Curve->AddKey(1, 1);
        Curve->SetKeyInterpMode(FirstKeyHandle, RCIM_Cubic, true);
        Curve->SetKeyInterpMode(LastKeyHandle, RCIM_Cubic, true);
        Curve->SetKeyTangentMode(FirstKeyHandle, RCTM_Auto);
        Curve->SetKeyTangentMode(LastKeyHandle, RCTM_Auto);
    }
}

void ATerrainMassDummyBrush::UpdateSideFalloffTexture()
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