// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class AControlAreaManager;
class ASoldierTeam;

#include "Core.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

//#include "../ControlArea/ControlAreaManager.h"
//#include "../Soldiers/SoldierTeam.h"

#include "SquadLeaderGameState.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API ASquadLeaderGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASquadLeaderGameState();

	// for replication purpose
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;


protected:
	// storage of global data for all the game
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "GameModeData")
		AControlAreaManager* ControlAreaManager;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "GameModeData")
		TArray<ASoldierTeam*> SoldierTeamCollection;

public:
	UFUNCTION()
	void AddSoldierTeam(ASoldierTeam* _SoldierTeam);
	UFUNCTION()
	TArray<ASoldierTeam*> GetSoldierTeamCollection();
	UFUNCTION()
	void SetControlAreaManager(AControlAreaManager* _ControlAreaManager);
	UFUNCTION()
	AControlAreaManager* GetControlAreaManager();
};
