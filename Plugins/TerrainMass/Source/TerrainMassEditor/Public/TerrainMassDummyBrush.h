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
	// UObject Interfaces
	//
protected:
#if WITH_EDITOR	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//virtual void PostLoad() override;
#endif

	//
	// Brush Rendering
	//
protected:
	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* CanvasRT;

	//
	// Shape Falloff
	//
protected:
	void InitSideFalloffTexture();
	void UpdateSideFalloffTexture();

	UPROPERTY(EditAnywhere, Category="Landscape", meta=(UIMin=0))
	float Radius = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category="Landscape", AdvancedDisplay)
	struct FRuntimeFloatCurve SideFalloffCurve;

	UPROPERTY(VisibleAnywhere, Category = "Landscape")
	class UTexture2D* SideFalloffTexture;
};
