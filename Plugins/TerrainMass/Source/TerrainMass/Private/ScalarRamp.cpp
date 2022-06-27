// Fill out your copyright notice in the Description page of Project Settings.

#include "ScalarRamp.h"

#include "ScalarRampShader.h"

void FScalarRamp::SetSize(int32 TextureSize)
{
	Size = TextureSize;

	CreateTexture();
}

void FScalarRamp::CreateTexture()
{
	FTexture2DDynamicCreateInfo CreateInfo(PF_G16, false, false, TF_Bilinear, AM_Clamp);
	Texture = UTexture2DDynamic::Create(Size, 1, CreateInfo);

	FScalarRampShader::WaitForGPU();
}

void FScalarRamp::WriteTexture()
{
	if (!Texture)
	{
		CreateTexture();
	}

	FScalarRampShader::RenderRampToTexture(Curve, Texture);
}

UTexture2DDynamic* FScalarRamp::GetTexture()
{
	if (!Texture)
	{
		CreateTexture();
		WriteTexture();
	}

	return Texture;
}