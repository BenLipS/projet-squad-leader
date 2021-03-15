#include "SquadLeaderGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Soldiers/Players/SoldierPlayerController.h"
#include "Soldiers/Players/SoldierPlayerState.h"
#include "SquadLeaderGameInstance.h"
#include "Soldiers/Soldier.h"

#include "Interface/PreInitable.h"
#include "SquadLeaderGameState.h"
#include "SquadLeaderInitGameState.h"
#include "SquadLeaderCloseGameState.h"


ASquadLeaderGameModeBase::ASquadLeaderGameModeBase() : RespawnDelay{ 3.f }
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnObject(TEXT("/Game/BluePrints/Soldiers/Players/BP_SoldierPlayerSupport"));
	static ConstructorHelpers::FClassFinder<ASoldierPlayerController> PlayerControllerObject(TEXT("/Game/BluePrints/Soldiers/Players/BP_SoldierPlayerController"));
	static ConstructorHelpers::FClassFinder<ASoldierPlayerState> PlayerStateObject(TEXT("/Game/BluePrints/Soldiers/Players/BP_SoldierPlayerState"));

	if (PlayerPawnObject.Class != NULL)
		DefaultPawnClass = PlayerPawnObject.Class;

	if (PlayerControllerObject.Class != NULL)
		PlayerControllerClass = PlayerControllerObject.Class;

	if (PlayerStateObject.Class != NULL)
		PlayerStateClass = PlayerStateObject.Class;

	//HUDClass = maclasse::StaticClass();
}


void ASquadLeaderGameModeBase::InitGameWithGameState() {
	// set parameters for GameState's spawn
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	
	// change gameState
	GameStateClass = ASquadLeaderInitGameState::StaticClass();

	// spawn and set GameState
	UWorld* World = GetWorld();
	GameState = World->SpawnActor<ASquadLeaderInitGameState>(GameStateClass, SpawnInfo);
	World->SetGameState(GameState);
	if (GameState)
	{
		GameState->AuthorityGameMode = this;
	}

	InitGameState();
}


void ASquadLeaderGameModeBase::StartPlay() {
	// Set the GameState
	InitGameWithGameState();

	// launch the game initialization from the GameState 
	if (auto SLInitGameState = Cast<ASquadLeaderInitGameState>(GameState); SLInitGameState) {
		SLInitGameState->InitMapAndData();
	}
	
	//Init for AI
	//Cast<USquadLeaderGameInstance>(GetGameInstance())->InitInfluenceMap();
	Cast<USquadLeaderGameInstance>(GetGameInstance())->InitAIManagers();

	Super::StartPlay();
}

void ASquadLeaderGameModeBase::SoldierDied(AController* _Controller)
{
	FTimerHandle RespawnTimerHandle;
	FTimerDelegate RespawnDelegate;

	RespawnDelegate = FTimerDelegate::CreateUObject(this, &ASquadLeaderGameModeBase::RespawnSoldier, _Controller);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);

	/*if (ASoldierPlayerController* PC = Cast<ASoldierPlayerController>(_Controller); PC)
		PC->SetRespawnCountdown(RespawnDelay);*/
}

void ASquadLeaderGameModeBase::RespawnSoldier(AController* _Controller)
{
	if (ASoldier* soldier = Cast<ASoldier>(_Controller->GetPawn()); soldier)
	{
		soldier->SetActorLocation(soldier->GetRespawnPoint());
		soldier->Respawn();
	}
}

void ASquadLeaderGameModeBase::CheckControlAreaVictoryCondition()
{
	if (auto WinnerTeam = ControlAreaManager.GetDefaultObject()->GetTeamWithAllControl(); WinnerTeam) {
		GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, TEXT("END GAME: All control area taken\n") + WinnerTeam.GetDefaultObject()->TeamName + TEXT(" win !"), false, FVector2D(7, 7));
		FTimerHandle timerBeforeClosing;
		GetWorld()->GetTimerManager().SetTimer(timerBeforeClosing, this,
			&ASquadLeaderGameModeBase::EndGame, 5.f);  // request to the server to end the game
	}
}

void ASquadLeaderGameModeBase::EndGame()
{
	for (auto Teams : SoldierTeamCollection) {
		for (auto Soldiers : Teams.GetDefaultObject()->soldierList) {
			if (auto PlayerControler = Cast<ASoldierPlayerController>(Soldiers->GetController()); PlayerControler) {
				PlayerControler->ClientSendCommand("EXIT", true);
			}
		}
	}
	//FGenericPlatformMisc::RequestExit(false);
}
