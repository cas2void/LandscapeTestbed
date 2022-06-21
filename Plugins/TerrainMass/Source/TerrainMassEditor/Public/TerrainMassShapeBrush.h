#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrush.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ScalarRamp.h"
#include "TerrainMassShapeVertex.h"
#include "TerrainMassShapeBrush.generated.h"

enum class EShapeBrushDirtyLevel : uint8
{
	ShapeData,
	ShapeRT,
	JumpFlooding,
	DistanceField,
	Blur,
	Max
};

/**
 * 
 */
UCLASS(NotBlueprintable, NotPlaceable, HideCategories = (Collision, Mobility))
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
	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Buffers")
	UTextureRenderTarget2D* OutputRT;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Buffers")
	UTextureRenderTarget2D* ShapeRT;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Buffers")
	UTextureRenderTarget2D* JumpFloodingRTs[2];

	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Buffers")
	UTextureRenderTarget2D* DistanceFieldRT;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Buffers")
	UTextureRenderTarget2D* BlurIntermediateRT;

	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Buffers")
	UTextureRenderTarget2D* BlurRT;

	//
	// Dirty Flags
	//
protected:
	UPROPERTY(VisibleAnywhere, Transient, Category = "Landscape|Dirty Flags")
	TArray<bool> DirtyFlags;

	void MarkDirty(EShapeBrushDirtyLevel DirtyLevel);
	bool IsDirty(EShapeBrushDirtyLevel DirtyLevel) const;
	void ResetDirty(EShapeBrushDirtyLevel DirtyLevel);

	//
	// Shape
	//
protected:
	UPROPERTY(EditAnywhere, Category = "Landscape|Shape")
	bool bUVOffset = true;

	UPROPERTY(Transient)
	TArray<FTerrainMassShapeVertex> ShapeVertices;

	UPROPERTY(Transient)
	TArray<uint16> ShapeIndices;

	//
	// Jump Flooding
	//
protected:
	UPROPERTY(EditAnywhere, Category = "Landscape|Distance Field", meta = (InlineEditConditionToggle))
	bool bSetIteration = false;

	UPROPERTY(EditAnywhere, Category = "Landscape|Distance Field", meta = (UIMin = 0, UIMax = 15, EditCondition = "bSetIteration"))
	int32 NumIteration = 1;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Landscape|Distance Field")
	int32 OutputIndex = -1;

	UPROPERTY(EditAnywhere, Category = "Landscape|Distance Field")
	float Width = 400.0f;

	//
	// Blur
	//
protected:
	UPROPERTY(EditAnywhere, Category = "Landscape|Blur")
	bool bBlur = true;

	UPROPERTY(EditAnywhere, Category = "Landscape|Blur", meta = (UIMin = 0, UIMax = 100, Tooltip = "Half of the kernel span"))
	int32 KernelSize = 8;

	UPROPERTY(EditAnywhere, Category = "Landscape|Blur", meta = (UIMin = 0.1, UIMax = 10))
	float Sigma = 3.0f;

	//
	// Compostion
	//
protected:
	
	UPROPERTY(EditAnywhere, Category = "Landscape|Falloff")
	FScalarRamp SideFalloffRamp;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Landscape|Falloff", AdvancedDisplay)
	UTexture2DDynamic* SideFalloffTexture;

	//
	// Components
	//
protected:
	UPROPERTY(VisibleAnywhere)
	class UTerrainMassSplineComponent* SplineComponent;

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* ArrowComponent;

	//
	// Transform Delegates
	//
protected:
	void OnTransformUpdated(USceneComponent* UpdatedComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

	UPROPERTY(Transient)
	FVector PreviousLocation;

	UPROPERTY(Transient)
	FQuat PreviousRotation;

	UPROPERTY(Transient)
	FVector PreviousScale;

	//
	// Spline Delegates
	//
protected:
	void OnSplineUpdated();
};
