#include "TerrainMassLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

bool UTerrainMassLibrary::CreateOrUpdateRenderTarget2D(UObject* WorldContextObject, UTextureRenderTarget2D*& RT, FIntPoint Size, ETextureRenderTargetFormat Format,
    bool NeedToClearColor, FLinearColor ClearColor, TextureFilter Filter, bool CanCreateUAV, bool AutoGenerateMips, const FName& RTName)
{
    bool Result = false;

    if (RT && 
        RT->SizeX == Size.X && RT->SizeY == Size.Y && 
        RT->RenderTargetFormat == Format && 
        RT->Filter == Filter &&
        RT->Resource)
    {
        if (NeedToClearColor)
        {
            RT->ClearColor = ClearColor;
            RT->UpdateResourceImmediate(true);
        }
        Result = true;
    }
    else
    {
        if (Size.X > 0 && Size.Y > 0)
        {
            UObject* Outer = GetTransientPackage();
            if (WorldContextObject != GetTransientPackage())
            {
                Outer = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
            }
            
            if (Outer)
            {
                if (RT)
                {
                    RT->ReleaseResource();
                }

                RT = NewObject<UTextureRenderTarget2D>(Outer, MakeUniqueObjectName(Outer, UTextureRenderTarget2D::StaticClass(), RTName));
                if (RT)
                {
                    RT->RenderTargetFormat = Format;
                    RT->ClearColor = ClearColor;
                    RT->Filter = Filter;
                    RT->bCanCreateUAV = CanCreateUAV;
                    RT->bAutoGenerateMips = AutoGenerateMips;
                    RT->InitAutoFormat(Size.X, Size.Y);
                    RT->UpdateResourceImmediate(true);

                    Result = true;
                }
            }
        }
    }

    return Result;
}

bool UTerrainMassLibrary::CreateOrUpdateTexture2D(UObject* WorldContextObject, UTexture2D*& Texture, FIntPoint Size, EPixelFormat Format, 
    TextureAddress AddressX, TextureAddress AddressY, TextureMipGenSettings MipGenSettings, const FName& TextureName)
{
    bool Result = false;

    if (Texture &&
        Texture->GetSizeX() == Size.X && Texture->GetSizeY() == Size.Y &&
        Texture->GetPixelFormat() == Format &&
        Texture->AddressX == AddressX &&
        Texture->AddressY == AddressY &&
#if WITH_EDITOR
        Texture->MipGenSettings == MipGenSettings &&
#endif
        Texture->GetNumMips() > 0 &&
        Texture->PlatformData)
    {
        Result = true;
    }
    else
    {
        if (Size.X > 0 && Size.Y > 0)
        {
            UObject* Outer = GetTransientPackage();
            if (WorldContextObject != GetTransientPackage())
            {
                Outer = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
            }

            if (Outer)
            {
                if (Texture)
                {
                    Texture->ReleaseResource();
                }

                Texture = NewObject<UTexture2D>(Outer, MakeUniqueObjectName(Outer, UTextureRenderTarget2D::StaticClass(), TextureName));
                if (Texture)
                {
                    Texture->PlatformData = new FTexturePlatformData();
                    Texture->PlatformData->SizeX = Size.X;
                    Texture->PlatformData->SizeY = Size.Y;
                    Texture->PlatformData->PixelFormat = Format;

                    // Allocate first mipmap.
                    int32 NumBlocksX = Size.X / GPixelFormats[Format].BlockSizeX;
                    int32 NumBlocksY = Size.Y / GPixelFormats[Format].BlockSizeY;
                    FTexture2DMipMap* Mip = new FTexture2DMipMap();
                    Texture->PlatformData->Mips.Add(Mip);
                    Mip->SizeX = Size.X;
                    Mip->SizeY = Size.Y;
                    Mip->BulkData.Lock(LOCK_READ_WRITE);
                    Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[Format].BlockBytes);
                    Mip->BulkData.Unlock();

                    Texture->AddressX = AddressX;
                    Texture->AddressY = AddressY;
#if WITH_EDITOR
                    Texture->MipGenSettings = MipGenSettings;
#endif

                    Result = true;
                }
            }
        }
    }

    return Result;
}
