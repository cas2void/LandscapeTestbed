#include "TerrainMassLibrary.h"

#include "Engine/Engine.h"
#include "Engine/World.h"

bool UTerrainMassLibrary::CreateOrUpdateRenderTarget(UObject* WorldContextObject, UTextureRenderTarget2D*& RT, FIntPoint Size, ETextureRenderTargetFormat Format,
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