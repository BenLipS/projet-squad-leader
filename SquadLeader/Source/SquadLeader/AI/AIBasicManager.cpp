// Fill out your copyright notice in the Description page of Project Settings.


#include "AIBasicManager.h"
#include "../SquadLeaderGameModeBase.h"

void UAIBasicManager::Init(TSubclassOf<ASoldierTeam> _Team, UObject* WorldContextObject)
{
	Team = _Team;

	/*For now Each AIBasicManager Spawn with 4 AIs*/
	/*TEMPORARY*/
	auto gameMode = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
	FVector Start{};
	if (Team == gameMode->SoldierTeamCollection[0]) {
		Start = { 8760.f, 15590.f, 87.999985f };
	}
	else if (Team == gameMode->SoldierTeamCollection[1]){
		Start = { 16350.f, 15590.f, 87.999985f };
	}
	
	FVector OffSet{ 0.f, 500.f, 0.f };

	FTransform LocationAI{  };
	LocationAI.SetLocation(Start);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // La maniere de faire le respawn
	ASoldierAI* BasicAI = GEngine->GetWorldFromContextObject(WorldContextObject)->SpawnActorDeferred<ASoldierAI>(ClassAI, LocationAI, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (BasicAI) {
		BasicAI->PlayerTeam = Team;
		BasicAI->SpawnDefaultController();
		BasicAI->FinishSpawning(LocationAI);
		AIBasicList.Add(Cast<AAIBasicController>(BasicAI->GetController()));
	}

	FTransform LocationAI1{};
	LocationAI1.SetLocation(LocationAI.GetLocation() + OffSet);
	ASoldierAI* BasicAI1 = GEngine->GetWorldFromContextObject(WorldContextObject)->SpawnActorDeferred<ASoldierAI>(ClassAI, LocationAI, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (BasicAI1) {
		BasicAI1->PlayerTeam = Team;
		BasicAI1->SpawnDefaultController();
		BasicAI1->FinishSpawning(LocationAI1);
		AIBasicList.Add(Cast<AAIBasicController>(BasicAI1->GetController()));
	}

	FTransform LocationAI2{};
	LocationAI2.SetLocation(LocationAI1.GetLocation() + OffSet);
	ASoldierAI* BasicAI2 = GEngine->GetWorldFromContextObject(WorldContextObject)->SpawnActorDeferred<ASoldierAI>(ClassAI, LocationAI, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (BasicAI2) {
		BasicAI2->PlayerTeam = Team;
		BasicAI2->SpawnDefaultController();
		BasicAI2->FinishSpawning(LocationAI2);
		AIBasicList.Add(Cast<AAIBasicController>(BasicAI2->GetController()));
	}

	FTransform LocationAI3{};
	LocationAI3.SetLocation(LocationAI2.GetLocation() + OffSet);
	ASoldierAI* BasicAI3 = GEngine->GetWorldFromContextObject(WorldContextObject)->SpawnActorDeferred<ASoldierAI>(ClassAI, LocationAI, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (BasicAI3) {
		BasicAI3->PlayerTeam = Team;
		BasicAI3->SpawnDefaultController();
		BasicAI3->FinishSpawning(LocationAI3);
		AIBasicList.Add(Cast<AAIBasicController>(BasicAI3->GetController()));
	}
}
