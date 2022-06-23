// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "TerrainMassHandleComponent.generated.h"

/**
 * 
 */
UCLASS()
class TERRAINMASS_API UTerrainMassHandleComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UTerrainMassHandleComponent();

	// Color to draw arrow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	FColor HandleColor;

	// Relative size to scale drawn arrow by
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	float HandleSize;

	// Total length of drawn arrow including head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	float HandleLength;

	// Ignore scene component scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=HandleComponent)
	bool bKeepHandleSize = true;

	/** Updates the arrow's colour, and tells it to refresh */
	UFUNCTION(BlueprintCallable, DisplayName="SetHandleColor", Category="Components|Handle")
	virtual void SetHandleColor(FLinearColor NewColor);

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
#if WITH_EDITOR
	virtual bool ComponentIsTouchingSelectionBox(const FBox& InSelBBox, const FEngineShowFlags& ShowFlags, const bool bConsiderOnlyBSP, const bool bMustEncompassEntireComponent) const override;
	virtual bool ComponentIsTouchingSelectionFrustum(const FConvexVolume& InFrustum, const FEngineShowFlags& ShowFlags, const bool bConsiderOnlyBSP, const bool bMustEncompassEntireComponent) const override;
#endif
	//~ End UPrimitiveComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.
};
