// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "../SL_UserWidget.h"
#include "../Interface/WeaponInterface.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "AmmoWidget.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API UAmmoWidget : public USL_UserWidget, public IWeaponInterface
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* ProgressBarAmmo;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* TextAmmo;

	int Ammo = 0;
	int MaxAmmo = 1;

public:
	UAmmoWidget(const FObjectInitializer& ObjectInitializer);

	void SetupDelegateToObject_Implementation(UObject* ObjectIn) override;

	//-----IWeaponInterface-----
	void OnAmmoChanged(uint8 newValue) override;
	void OnMaxAmmoChanged(uint8 newValue) override;
};
