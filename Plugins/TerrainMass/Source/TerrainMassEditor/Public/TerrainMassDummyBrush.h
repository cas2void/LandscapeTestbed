#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrush.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TerrainMassDummyBrush.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMASSEDITOR_API ATerrainMassDummyBrush : public ALandscapeBlueprintBrush
{
	GENERATED_BODY()

public:
	ATerrainMassDummyBrush();

	//
	// ALandscapeBlueprintBrush Interfaces
	//
public:
	virtual UTextureRenderTarget2D* Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName) override;
	virtual void Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize) override;

	//
	//
	//
protected:
	UPROPERTY(VisibleAnywhere)
	UTextureRenderTarget2D* InputRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* CanvasRT;

	UPROPERTY(EditAnywhere, Category="Landscape", meta=(UIMin=0))
	float Radius = 1000.0f;
};
