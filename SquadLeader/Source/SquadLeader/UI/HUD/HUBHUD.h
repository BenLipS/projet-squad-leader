// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SL_HUD.h"



#include "HUBHUD.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API AHUBHUD : public ASL_HUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
};
