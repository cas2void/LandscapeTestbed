// Fill out your copyright notice in the Description page of Project Settings.

#include "ScalarRamp.h"

#include "ScalarRampShader.h"

UTexture2DDynamic* FScalarRamp::CreateTexture(int32 Size)
{
	FTexture2DDynamicCreateInfo CreateInfo(PF_G8, false, false, TF_Bilinear, AM_Clamp);
	UTexture2DDynamic* RampTexture = UTexture2DDynamic::Create(Size, 1, CreateInfo);

	FScalarRampShader::WaitForGPU();

	return RampTexture;
}

void FScalarRamp::WriteTexture(UTexture2DDynamic* OutTexture)
{
	check(OutTexture);

	FScalarRampShader::RenderRampToTexture(Curve, OutTexture);
}
