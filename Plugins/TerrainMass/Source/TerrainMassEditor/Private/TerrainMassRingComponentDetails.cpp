// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainMassRingComponentDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "PropertyHandle.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"

#include "TerrainMassRingComponent.h"

#define LOCTEXT_NAMESPACE "TerrainMassRingComponentDetails"

TSharedRef<IDetailCustomization> FTerrainMassRingComponentDetails::MakeInstance()
{
	return MakeShareable(new FTerrainMassRingComponentDetails);
}

void FTerrainMassRingComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBegingCustomized;
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBegingCustomized);
	if (ObjectsBegingCustomized.Num() == 1)
	{
		UTerrainMassRingComponent* TerrainMassRingComponent = Cast<UTerrainMassRingComponent>(ObjectsBegingCustomized[0]);
		if (TerrainMassRingComponent)
		{
			
		}
	}
}

#undef LOCTEXT_NAMESPACE