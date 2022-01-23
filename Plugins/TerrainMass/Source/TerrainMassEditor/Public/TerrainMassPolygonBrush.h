#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrush.h"
#include "TerrainMassPolygonBrush.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMASSEDITOR_API ATerrainMassPolygonBrush : public ALandscapeBlueprintBrush
{
	GENERATED_BODY()

public:
	ATerrainMassPolygonBrush();

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
};
