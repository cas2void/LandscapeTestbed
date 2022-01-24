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
	void InitSideFalloffCurve(FRuntimeFloatCurve& SideFalloffCurve);
	void UpdateSideFalloffTexture(struct FRuntimeFloatCurve& SideFalloffCurve, class UTexture2D* SideFalloffTexture);

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* CanvasRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* OutputRT;

	UPROPERTY(EditAnywhere, Category="Landscape")
	float Width = 1000.0f;

	UPROPERTY(EditAnywhere, Category="Landscape")
	float SideFalloff = 600.0f;

	UPROPERTY(EditAnywhere, Category="Landscape")
	float EndFalloff = 800.0f;

	UPROPERTY(EditAnywhere, Category="Landscape")
	FVector StartPosition = FVector(-1000, -1000, 500);

	UPROPERTY(EditAnywhere, Category="Landscape")
	FVector EndPosition = FVector(1000, 1000, 500);

	UPROPERTY(EditAnywhere, Category="Landscape")
	int32 NumSegments = 1;
	
	UPROPERTY(EditAnywhere, Category="Landscape", AdvancedDisplay)
	struct FRuntimeFloatCurve StartSideFalloffCurve;
	
	UPROPERTY(EditAnywhere, Category="Landscape", AdvancedDisplay)
	struct FRuntimeFloatCurve EndSideFalloffCurve;

	UPROPERTY()
	class UTexture2D* StartSideFalloffTexture;

	UPROPERTY()
	class UTexture2D* EndSideFalloffTexture;
};
