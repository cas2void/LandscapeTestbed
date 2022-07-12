#include "TerrainMassEditorModule.h"

#include "PropertyEditorModule.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"

#include "ScalarRamp.h"
#include "TerrainMassTypeCustomization.h"
#include "TerrainMassRingComponent.h"
#include "TerrainMassRingComponentDetails.h"
#include "TerrainMassRingComponentVisualizer.h"
#include "Components/SplineComponent.h"

#define LOCTEXT_NAMESPACE "FTerrainMassEditorModule"

void FTerrainMassEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout(FScalarRamp::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FScalarRampTypeCustomization::MakeInstance));

	//PropertyEditorModule.RegisterCustomClassLayout(UTerrainMassRingComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FTerrainMassRingComponentDetails::MakeInstance));

	//PropertyEditorModule.UnregisterCustomClassLayout(USplineComponent::StaticClass()->GetFName());

	PropertyEditorModule.NotifyCustomizationModuleChanged();
	//
	// Component Visualizer
	//
	if (GUnrealEd)
	{
		TSharedPtr<FTerrainMassRingComponentVisualizer> ComponentVisualizer = MakeShareable(new FTerrainMassRingComponentVisualizer);
		GUnrealEd->RegisterComponentVisualizer(UTerrainMassRingComponent::StaticClass()->GetFName(), ComponentVisualizer);
		ComponentVisualizer->OnRegister();
	}
}

void FTerrainMassEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout(FScalarRamp::StaticStruct()->GetFName());

		//PropertyEditorModule.UnregisterCustomClassLayout(UTerrainMassRingComponent::StaticClass()->GetFName());

		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}

	//
	// Component Visualizer
	//
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UTerrainMassRingComponent::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTerrainMassEditorModule, TerrainMassEditor)