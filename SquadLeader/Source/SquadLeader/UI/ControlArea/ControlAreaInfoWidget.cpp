// Fill out your copyright notice in the Description page of Project Settings.


#include "ControlAreaInfoWidget.h"

void UControlAreaInfoWidget::OnControlAreaCapturerChange(int newCapturer)
{
	if (ImageControlArea)
	{
		auto mat = ImageControlArea->GetDynamicMaterial();
		if (mat)
		{
			if (newCapturer != Capturer)
			{
				Capturer = newCapturer;
				mat->SetVectorParameterValue("BottomInnerColor", GetColorFromOwner(Capturer));
			}
			ImageControlArea->SetBrushFromMaterial(mat);
		}
	}
}

void UControlAreaInfoWidget::OnControlAreaOwnerChange(int newOwner)
{
	if (ImageControlArea)
	{
		auto mat = ImageControlArea->GetDynamicMaterial();
		if (mat)
		{
			if (newOwner != Owner)
			{
				Owner = newOwner;
				mat->SetVectorParameterValue("OutterColor", GetColorFromOwner(Owner) * 0.1f);
				if (Owner != 0)
				{
					PlayAnimation(CaptureAnimation);
				}
			}
			ImageControlArea->SetBrushFromMaterial(mat);
		}
	}
	
}

void UControlAreaInfoWidget::OnControlAreaPercentageChange(float Percentage)
{
	if (ImageControlArea)
	{
		auto mat = ImageControlArea->GetDynamicMaterial();
		if (mat)
		{
			mat->SetScalarParameterValue("Percentage_fill", Percentage);
			ImageControlArea->SetBrushFromMaterial(mat);
		}
	}
}

void UControlAreaInfoWidget::OnControlAreaNameChange(FString NameIn)
{
	if (IsValid(ControlAreaName))
	{
		ControlAreaName->SetText(FText::FromString(NameIn));
	}
}

FLinearColor UControlAreaInfoWidget::GetColorFromOwner(int ownerIn)
{
	return ownerIn < 0 ? EnnemyColor : ownerIn == 0 ? NeutralColor : AllyColor;
}