// Fill out your copyright notice in the Description page of Project Settings.


#include "WheelWidget.h"

#include "WheelWidgetElement.h"

#include "../../Soldiers/Players/SoldierPlayerController.h"

#include "../Interface/OrderDelegateInterface.h"

#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"

UWheelWidget::UWheelWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UWheelWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	if (Background)
	{
		auto mat = Background->GetDynamicMaterial();
		if (mat)
		{
			mat->SetScalarParameterValue("InnerCircleScale", InnerCircleRadius / OutterCircleRadius);
			mat->SetVectorParameterValue("Color", BackgroundColor);
			Background->SetBrushFromMaterial(mat);
		}
	}
	if (ImageBox)
	{
		ImageBox->SetHeightOverride(OutterCircleRadius*2);
		ImageBox->SetWidthOverride(OutterCircleRadius*2);
	}

	ListItems = {};

	for (UWidget* Item : WheelElements->GetAllChildren())
	{
		if (UWheelWidgetElement* WheelItem = Cast<UWheelWidgetElement>(Item); Item)
		{
			ListItems.Add(WheelItem);
			if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(WheelItem->Slot); PanelSlot)
			{
				PanelSlot->SetAnchors(FAnchors(0.5f));
				PanelSlot->SetPosition(FVector2D(0.f));
				PanelSlot->SetAlignment(FVector2D(0.5f));
			}
		}
	}

	NbElement = ListItems.Num();

	float AngleSection = 360.f / float(NbElement);
	float Angle = AngleSection / 2.f;

	for (int i = 0; i < ListItems.Num(); ++i)
	{
		float NextAngle = Angle + AngleSection;
		ListItems[i]->SetRenderTranslation(FVector2D(UKismetMathLibrary::DegCos(Angle), UKismetMathLibrary::DegSin(Angle)) * (InnerCircleRadius + ((OutterCircleRadius - InnerCircleRadius) / 2.f)));
		Angle = NextAngle;
	}

	if(!IsDesignTime())
		SetVisibility(ESlateVisibility::Hidden);
}

const TArray<class UWheelWidgetElement*>& UWheelWidget::GetItems()
{
	return ListItems;
}

void UWheelWidget::AddToViewport()
{
	Super::AddToViewport();

	FVector2D CenterScreen = (UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / 2.0f) * (1.0f / UWidgetLayoutLibrary::GetViewportScale(GetWorld()));

	float AngleSection = 360.f / float(NbElement);
	float Angle = AngleSection / 2.f;

	for (int i = 0; i < ListItems.Num(); ++i)
	{
		float NextAngle = Angle + AngleSection;
		ListItems[i]->SetRenderTranslation(FVector2D(UKismetMathLibrary::DegCos(Angle), UKismetMathLibrary::DegSin(Angle)) * (InnerCircleRadius + ((OutterCircleRadius - InnerCircleRadius) / 2.f)));
		Angle = NextAngle;
	}
}

void UWheelWidget::RemoveFromViewport()
{
	Super::RemoveFromViewport();
}

void UWheelWidget::OnOrderInputPressed()
{
	if (ASoldierPlayerController* PC = GetOwningPlayer<ASoldierPlayerController>(); PC)
	{
		PC->SetShowMouseCursor(true);
		PC->ClientIgnoreMoveInput(true);
		PC->ClientIgnoreLookInput(true);

		FVector2D Center = UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / 2.0f;
		PC->SetMouseLocation(Center.X, Center.Y);
		SetVisibility(ESlateVisibility::Visible);
	}
}

void UWheelWidget::OnOrderInputReleased()
{
	if (!IsDesignTime())
		SetVisibility(ESlateVisibility::Hidden);
	if (ASoldierPlayerController* PC = GetOwningPlayer<ASoldierPlayerController>(); PC)
	{
		PC->SetShowMouseCursor(false);
		PC->ClientIgnoreMoveInput(false);
		PC->ClientIgnoreLookInput(false);
	}
		

	FVector2D CenterScreen = (UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / 2.0f) * (1.0f / UWidgetLayoutLibrary::GetViewportScale(GetWorld()));
	float AngleSection = 360.f / float(NbElement);

	//-----MOUSE_SELECTION-----
	int MouseIndex = -10;
	FVector2D RelativeMousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld()) - CenterScreen;

	if (RelativeMousePosition.Size() >= InnerCircleRadius)
	{
		MouseIndex = UKismetMathLibrary::GenericPercent_FloatFloat(UKismetMathLibrary::DegAtan2(RelativeMousePosition.Y, RelativeMousePosition.X) + 360.f, 360.f) / AngleSection;
		if (ListItems.IsValidIndex(MouseIndex) && ListItems[MouseIndex])
		{
			ListItems[MouseIndex]->OnitemSelected();
		}
	}
}

int32 UWheelWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	FVector2D CenterScreen = (UWidgetLayoutLibrary::GetViewportSize(GetWorld()) / 2.0f) * (1.0f / UWidgetLayoutLibrary::GetViewportScale(GetWorld()));
	float AngleSection = 360.f / float(NbElement);

	//-----MOUSE_SELECTION-----
	int MouseIndex = -10;
	FVector2D RelativeMousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld()) - CenterScreen;

	//Debug purpose
	/*UWidgetBlueprintLibrary::DrawLine(Context,
		CenterScreen,
		CenterScreen + RelativeMousePosition,
		FLinearColor::Red,
		true,
		2.f);*/

	if (RelativeMousePosition.Size() >= InnerCircleRadius)
	{
		MouseIndex = UKismetMathLibrary::GenericPercent_FloatFloat(UKismetMathLibrary::DegAtan2(RelativeMousePosition.Y, RelativeMousePosition.X) + 360.f, 360.f) / AngleSection;
	}

	//-----DRAW-----
	float Angle = 0.f;
	for (int i = 0; i < NbElement; i++)
	{
		if (i == MouseIndex)
		{
			UWidgetBlueprintLibrary::DrawLine(Context,
				CenterScreen + (FVector2D(UKismetMathLibrary::DegCos(Angle), UKismetMathLibrary::DegSin(Angle)) * (InnerCircleRadius + ((OutterCircleRadius - InnerCircleRadius) / 10.f))),
				CenterScreen + (FVector2D(UKismetMathLibrary::DegCos(Angle), UKismetMathLibrary::DegSin(Angle)) * (OutterCircleRadius - ((OutterCircleRadius - InnerCircleRadius) / 10.f))),
				SeparationColor,
				true,
				SelectionThickness);
			if (!ListItems[i]->IsItemHovered())
			{
				ListItems[i]->SetItemHovered(true);
				ListItems[i]->SetRenderScale(FVector2D(ItemHoverScale));
			}
		}
		else
		{
			if (ListItems[i]->IsItemHovered())
			{
				ListItems[i]->SetItemHovered(false);
				ListItems[i]->SetRenderScale(FVector2D(1.f));
			}

			if (MouseIndex >=0 && i == (MouseIndex + 1) % NbElement)
			{
				UWidgetBlueprintLibrary::DrawLine(Context,
					CenterScreen + (FVector2D(UKismetMathLibrary::DegCos(Angle), UKismetMathLibrary::DegSin(Angle)) * (InnerCircleRadius + ((OutterCircleRadius - InnerCircleRadius) / 10.f))),
					CenterScreen + (FVector2D(UKismetMathLibrary::DegCos(Angle), UKismetMathLibrary::DegSin(Angle)) * (OutterCircleRadius - ((OutterCircleRadius - InnerCircleRadius) / 10.f))),
					SeparationColor,
					true,
					SelectionThickness);
			}
			else
			{
				UWidgetBlueprintLibrary::DrawLine(Context,
					CenterScreen + (FVector2D(UKismetMathLibrary::DegCos(AngleSection * i), UKismetMathLibrary::DegSin(AngleSection * i)) * (InnerCircleRadius + ((OutterCircleRadius - InnerCircleRadius) / 10.f))),
					CenterScreen + (FVector2D(UKismetMathLibrary::DegCos(AngleSection * i), UKismetMathLibrary::DegSin(AngleSection * i)) * (OutterCircleRadius - ((OutterCircleRadius - InnerCircleRadius) / 10.f))),
					SeparationColor,
					true,
					DefaultThickness);
			}
		}
		Angle += AngleSection;
	}

	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void UWheelWidget::SetupDelegateToObject_Implementation(UObject* ObjectIn)
{
	if (IOrderDelegateInterface* OrderDelegateInterface = Cast<IOrderDelegateInterface>(ObjectIn); OrderDelegateInterface)
	{
		OrderDelegateInterface->AddOrderDelegate(this);
	}
}
