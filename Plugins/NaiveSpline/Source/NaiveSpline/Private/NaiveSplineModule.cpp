// Fill out your copyright notice in the Description page of Project Settings.

#include "NaiveSplineModule.h"

#define LOCTEXT_NAMESPACE "FNaiveSplineModule"

void FNaiveSplineModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FNaiveSplineModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNaiveSplineModule, NaiveSpline)