// Fill out your copyright notice in the Description page of Project Settings.

#include "NaiveSplineShaderModule.h"

#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FNaiveSplineShaderModule"

void FNaiveSplineShaderModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("NaiveSpline"), TEXT("Shaders"), TEXT("Private"));
	AddShaderSourceDirectoryMapping("/NaiveSplineShaders", ShaderDirectory);
}

void FNaiveSplineShaderModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNaiveSplineShaderModule, NaiveSplineShader)