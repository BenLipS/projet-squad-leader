// Fill out your copyright notice in the Description page of Project Settings.


#include "SquadLeaderGameInstance.h"
#include "SquadLeaderGameModeBase.h"

USquadLeaderGameInstance::USquadLeaderGameInstance() : 
		ListAISquadManagers{}
{
}

void USquadLeaderGameInstance::InitAIManagers()
{
	if(GEngine)GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Game Instance InitAIManagers()"));
	auto gameMode = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());

	/*Init AIBasic Manager*/

	FTransform LocationTemp{ {0.f, -1000.f, 0.f}, {0.f,0.f,0.f} };
	AAIBasicManager* AIBasicManager = GetWorld()->SpawnActorDeferred<AAIBasicManager>(AIBasicManagerClass, LocationTemp, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (AIBasicManager) {
		AIBasicManager->FinishSpawning(LocationTemp);
		AIBasicManagerTeam1 = AIBasicManager;
		AIBasicManagerTeam1->Init(gameMode->SoldierTeamCollection[0]);
	}

	//AIBasicManagerTeam1 = NewObject<AAIBasicManager>(this, AIBasicManagerClass);
	//AIBasicManagerTeam1->Init(gameMode->SoldierTeamCollection[0], GetWorld());

	AAIBasicManager* AIBasicManager2 = GetWorld()->SpawnActorDeferred<AAIBasicManager>(AIBasicManagerClass, LocationTemp, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (AIBasicManager2) {
		AIBasicManager2->FinishSpawning(LocationTemp);
		AIBasicManagerTeam2 = AIBasicManager2;
		AIBasicManagerTeam2->Init(gameMode->SoldierTeamCollection[1]);
	}


	/*Init AISquad Manager*/
	//Each Player init a SquadManager in SoldierPlayer.cpp PossessedBy
}

void USquadLeaderGameInstance::AddAIBasicToManager(AAIBasicController* AIBasic)
{
	auto gameMode = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
	if (!AIBasicManagerTeam1->AIBasicList.Contains(AIBasic) && !AIBasicManagerTeam2->AIBasicList.Contains(AIBasic)) {
		if (gameMode && Cast<ASoldier>(AIBasic->GetPawn())->PlayerTeam == gameMode->SoldierTeamCollection[0]) {
			AIBasicManagerTeam1->AIBasicList.Add(AIBasic);
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("AIBasic Team 1 added"));
		}
		else if (gameMode && Cast<ASoldier>(AIBasic->GetPawn())->PlayerTeam == gameMode->SoldierTeamCollection[1]) {
			AIBasicManagerTeam2->AIBasicList.Add(AIBasic);
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("AIBasic Team 2 added"));
		}
		else {
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Une AI n'a pas d'equipe"));
		}
	}
	else {
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AI Spawned thought the manager"));
	}
	
}

void USquadLeaderGameInstance::InitInfluenceMap() {
	FTransform LocationTemp{ {0.f, -1000.f, 0.f}, {0.f,0.f,0.f} };
	AGrdiSetUp* _influenceMap = GetWorld()->SpawnActorDeferred<AGrdiSetUp>(InfluenceMapClass, LocationTemp, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (_influenceMap) {
		_influenceMap->FinishSpawning(LocationTemp);
		InfluenceMap = _influenceMap;
	}
		


}
