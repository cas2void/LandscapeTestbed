// Fill out your copyright notice in the Description page of Project Settings.

#include "ManipulatorModule.h"

#define LOCTEXT_NAMESPACE "FManipulatorModule"

void FManipulatorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FManipulatorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FManipulatorModule, Manipulator)