#include "SL_HUD.h"
#include "../SL_UserWidget.h"


void ASL_HUD::InitHUD()
{
	WidgetList.Empty();

	for (TSubclassOf<USL_UserWidget> WidgetClass : WidgetClassList)
	{
		if (WidgetClass)
		{
			USL_UserWidget* newEntry = CreateWidget<USL_UserWidget>(GetWorld(), WidgetClass);
			if (newEntry)
			{
				newEntry->AddToViewport();
				WidgetList.Add(newEntry);
				newEntry->SetupDelegateToObject(this);
			}
		}
	}

	OnInitHUD();
}

void ASL_HUD::OnInitHUD()
{
}

void ASL_HUD::BeginPlay()
{
	Super::BeginPlay();
}