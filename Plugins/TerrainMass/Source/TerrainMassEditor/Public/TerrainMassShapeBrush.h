#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrush.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ScalarRamp.h"
#include "TerrainMassShapeBrush.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMASSEDITOR_API ATerrainMassShapeBrush : public ALandscapeBlueprintBrush
{
	GENERATED_BODY()

public:
	ATerrainMassShapeBrush();

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
	UTextureRenderTarget2D* OutputRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* ShapeRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* BlurIntermediateRT;

	UPROPERTY(VisibleAnywhere, Transient)
	UTextureRenderTarget2D* BlurRT;

	//
	// Blur
	//
protected:
	UPROPERTY(EditAnywhere, Category = "Landscape", meta = (UIMin = 0, UIMax = 100))
	int32 KernelSize = 5;

	//
	// Shape Falloff
	//
protected:
	
	UPROPERTY(EditAnywhere, Category = "Landscape")
	FScalarRamp SideFalloffRamp;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Landscape", AdvancedDisplay)
	UTexture2DDynamic* SideFalloffTexture;

	//
	// Spline
	//
protected:
	UPROPERTY(VisibleAnywhere, Category = "Landscape")
	class USplineComponent* SplineComponent;
};
