// Fill out your copyright notice in the Description page of Project Settings.

#include "TerrainMassTypeCustomization.h"

#include "DetailLayoutBuilder.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "Framework/Application/SlateApplication.h"

#include "ScalarRamp.h"

#define LOCTEXT_NAMESPACE "ScalarRampTypeCustomization"

TSharedRef<IPropertyTypeCustomization> FScalarRampTypeCustomization::MakeInstance()
{
	return MakeShareable(new FScalarRampTypeCustomization);
}

void FScalarRampTypeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		];
}

void FScalarRampTypeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	FScalarRamp* Ramp = nullptr;
	TArray<void*> RawData;
	PropertyHandle->AccessRawData(RawData);
	if (RawData.Num() == 1)
	{
		Ramp = reinterpret_cast<FScalarRamp*>(RawData[0]);
	}

	if (!Ramp)
	{
		return;
	}

	TSharedPtr<IPropertyHandle> CurveProperty = PropertyHandle->GetChildHandle("Curve");
	if (CurveProperty.IsValid())
	{
		CurveProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(
			[Ramp]()
			{
				Ramp->WriteTexture();

				bool bFinished = FSlateApplication::Get().GetPressedMouseButtons().Num() < 1;
				Ramp->OnRampModified.Broadcast(Ramp->GetCurve(), bFinished);
			}));

		ChildBuilder.AddProperty(CurveProperty.ToSharedRef());
	}

	TSharedPtr<IPropertyHandle> SizeProperty = PropertyHandle->GetChildHandle(FName("Size"));
	if (SizeProperty.IsValid())
	{
		SizeProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda(
			[Ramp]()
			{
				Ramp->CreateTexture();
				Ramp->WriteTexture();
			}));

		ChildBuilder.AddProperty(SizeProperty.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE