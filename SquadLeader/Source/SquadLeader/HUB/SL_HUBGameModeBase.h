// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SL_HUBGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API ASL_HUBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;

	UPROPERTY(BlueprintReadOnly, Category = "GameData")
		int NbMaxPlayer = 6;

public:
	void TeleportAllPlayersToGame();  // non-seamless travel
};