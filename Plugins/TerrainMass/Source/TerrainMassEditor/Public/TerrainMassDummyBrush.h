#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrush.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ScalarRamp.h"
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
#endif

	//
	// AActor Interfaces
	//
public:
	virtual void PostRegisterAllComponents() override;

	//
	// Brush Rendering
	//
protected:
	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* CanvasRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* BlendRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* OutputRT;

	//
	// Shape Falloff
	//
protected:
	UPROPERTY(EditAnywhere, Category="Landscape", meta=(UIMin=0))
	float Radius = 1000.0f;
	
	UPROPERTY(EditAnywhere, Category="Landscape", AdvancedDisplay)
	FScalarRamp SideFalloffRamp;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Landscape", AdvancedDisplay)
	UTexture2DDynamic* SideFalloffTexture;
};
