#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "TerrainMassLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMASSEDITOR_API UTerrainMassLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:    
    UFUNCTION(BlueprintCallable, Category="TerrainMass")
    static bool CreateOrUpdateRenderTarget2D(UObject* WorldContextObject, UTextureRenderTarget2D*& RT, FIntPoint Size, ETextureRenderTargetFormat Format,
        bool NeedToClearColor = false, FLinearColor ClearColor = FLinearColor::Black, TextureFilter Filter = TextureFilter::TF_Nearest,
        bool CanCreateUAV = false, bool AutoGenerateMips = false, const FName& RTName = FName(TEXT("RenderTarget2D")));

    UFUNCTION(BlueprintCallable, Category="TerrainMass")
    static bool CreateOrUpdateTexture2D(UObject* WorldContextObject, UTexture2D*& Texture, FIntPoint Size, EPixelFormat Format,
        TextureAddress AddressX = TA_Wrap, TextureAddress AddressY = TA_Wrap, TextureMipGenSettings MipGenSettings = TMGS_NoMipmaps,
        const FName& TextureName = FName(TEXT("Texture2D")));
};
