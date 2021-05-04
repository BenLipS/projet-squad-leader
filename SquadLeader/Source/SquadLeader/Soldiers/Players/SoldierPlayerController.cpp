#include "SoldierPlayerController.h"
#include "SoldierPlayerState.h"
#include "SoldierPlayer.h"
#include "../../SquadLeaderGameInstance.h"
#include "AbilitySystemComponent.h"
#include "SquadLeader/Weapons/SL_Weapon.h"
#include "../SoldierTeam.h"
#include "../../AI/AISquadManager.h"
#include "../../UI/SL_HUD.h"
#include "SquadLeader/UI/Interface/AbilityCooldownDelegateInterface.h"
#include "SquadLeader/SquadLeader.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

//TODO: rmove next include -> only use for the team init -> only use on temporary debug
#include "../../GameState/SquadLeaderGameState.h"

ASoldierPlayerController::ASoldierPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SquadManagerData = FAISquadManagerData();
}

void ASoldierPlayerController::BeginPlay()
{
	Super::BeginPlay();
	CreateHUD();
}

void ASoldierPlayerController::CreateHUD_Implementation()
{
	if (!HUDClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing HUDWidgetClass. Please fill in on the Blueprint of the PlayerController."), *FString(__FUNCTION__));
		return;
	}
	ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>();
	if (CurrentHUD)
		return;
	ClientSetHUD(HUDClass);
	if (InputComponent)
	{
		if (auto HUD = GetHUD<ASL_HUD>(); HUD)
		{
			InputComponent->BindAction("GiveOrder", IE_Pressed, HUD, &ASL_HUD::OnOrderInputPressed);
			InputComponent->BindAction("GiveOrder", IE_Released, HUD, &ASL_HUD::OnOrderInputReleased);
		}

		if (auto HUD = GetHUD<ASL_HUD>(); HUD)
		{
			InputComponent->BindAction("DisplayMap", IE_Pressed, HUD, &ASL_HUD::OnFullMapDisplayBegin);
			InputComponent->BindAction("DisplayMap", IE_Released, HUD, &ASL_HUD::OnFullMapDisplayEnd);
		}
	}
}

void ASoldierPlayerController::BindMainAbilities()
{
	if (IAbilityCooldownDelegateInterface* HUD = GetHUD<IAbilityCooldownDelegateInterface>(); HUD)
	{
		HUD->AddAbilityID(ESoldierAbilityInputID::Ability1, "Q");
		HUD->AddAbilityID(ESoldierAbilityInputID::Ability2, "E");
		HUD->AddAbilityID(ESoldierAbilityInputID::Ability3, "F");
	}
}

void ASoldierPlayerController::NotifyMainAbilityCooldown(const float _Cooldown, const ESoldierAbilityInputID _ID)
{
	if (IAbilityCooldownDelegateInterface* HUD = GetHUD<IAbilityCooldownDelegateInterface>(); HUD)
		HUD->OnAbilityCooldownTriggered(_Cooldown, _ID);
}

// Server only
void ASoldierPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ASoldierPlayerState* PS = GetPlayerState<ASoldierPlayerState>(); PS)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);

	//TODO: remove the team init -> only use on temporary debug
	if (auto GS = GetWorld()->GetGameState<ASquadLeaderGameState>(); GS) {

		if (GS->GetSoldierTeamCollection().Num() == 0) {  // no team obtainable for now, we need to find one (used when playing as server or if no teams in the map)
			ASoldierTeam* LastTeamObtainable = nullptr;
			for (auto SceneActors : GetWorld()->PersistentLevel->Actors) {
				if (auto SoldierTeam = Cast<ASoldierTeam>(SceneActors); SoldierTeam) {
					LastTeamObtainable = SoldierTeam;
				}
			}
			ensure(LastTeamObtainable);  // if trigger, please place a team in the map
			SetTeam(LastTeamObtainable);
		}
		else SetTeam(GS->GetSoldierTeamCollection()[0]);
		
		if (auto soldier = Cast<ASoldierPlayer>(InPawn); soldier->GetSquadManager()) {
			soldier->GetSquadManager()->UpdateSquadTeam(GetTeam());
		}
	}
	Cast<ASoldierPlayer>(InPawn)->GetSquadManager()->OnSquadChanged.AddDynamic(this, &ASoldierPlayerController::OnSquadChanged);
	Cast<ASoldierPlayer>(InPawn)->GetSquadManager()->OnMemberHealthChanged.AddDynamic(this, &ASoldierPlayerController::OnSquadMemberHealthChanged);
	Cast<ASoldierPlayer>(InPawn)->GetSquadManager()->OnMemberMaxHealthChanged.AddDynamic(this, &ASoldierPlayerController::OnSquadMemberMaxHealthChanged);
	Cast<ASoldierPlayer>(InPawn)->GetSquadManager()->OnMemberShieldChanged.AddDynamic(this, &ASoldierPlayerController::OnSquadMemberShieldChanged);
	Cast<ASoldierPlayer>(InPawn)->GetSquadManager()->OnMemberMaxShieldChanged.AddDynamic(this, &ASoldierPlayerController::OnSquadMemberMaxShieldChanged);
	Cast<ASoldierPlayer>(InPawn)->GetSquadManager()->BroadCastSquadData();
}

void ASoldierPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// For edge cases where the PlayerState is repped before the Soldier is possessed.
	CreateHUD();
	if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>())
	{
		CurrentHUD->SetPlayerStateLink();
		CurrentHUD->SetAIStateLink();
	}
}

void ASoldierPlayerController::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD && GetTeam())
		CurrentHUD->OnUpdatePOIs();
}

void ASoldierPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);

	// Cameras
	InputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ASoldierPlayerController::OnSwitchCamera);

	// Movements
	InputComponent->BindAxis("MoveForward", this, &ASoldierPlayerController::OnMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASoldierPlayerController::OnMoveRight);

	InputComponent->BindAxis("Turn", this, &ASoldierPlayerController::OnTurn);
	InputComponent->BindAxis("LookUp", this, &ASoldierPlayerController::OnLookUp);

	//TODO : change debug bindAction when not need anymore
	InputComponent->BindAction("ChangeTeam", IE_Released, this, &ASoldierPlayerController::OnChangeTeam);
}

ASoldierTeam* ASoldierPlayerController::GetTeam()
{
	if (auto SoldierState = Cast<ASoldierPlayerState>(PlayerState); SoldierState) {
		return SoldierState->GetTeam();
	}
	return nullptr;
}

bool ASoldierPlayerController::SetTeam(ASoldierTeam* _Team)
{
	if (auto SoldierState = Cast<ASoldierPlayerState>(PlayerState); SoldierState) {
		return SoldierState->SetTeam(_Team);
	}
	return false;
}

void ASoldierPlayerController::OnSwitchCamera()
{
	if (ASoldier* Soldier = Cast<ASoldier>(K2_GetPawn()); Soldier)
		Soldier->onSwitchCamera();
}

void ASoldierPlayerController::OnMoveForward(const float _Val)
{
	if (ASoldier* Soldier = Cast<ASoldier>(K2_GetPawn()); Soldier)
		Soldier->MoveForward(_Val);
}

void ASoldierPlayerController::OnMoveRight(const float _Val) {

	if (ASoldier* Soldier = Cast<ASoldier>(K2_GetPawn()); Soldier)
		Soldier->MoveRight(_Val);
}

void ASoldierPlayerController::OnLookUp(const float _Val)
{
	if (ASoldier* Soldier = Cast<ASoldier>(K2_GetPawn()); Soldier)
		Soldier->LookUp(_Val);
}

void ASoldierPlayerController::OnTurn(const float _Val)
{
	if (ASoldier* Soldier = Cast<ASoldier>(K2_GetPawn()); Soldier)
		Soldier->Turn(_Val);
}

void ASoldierPlayerController::OnChangeTeam()
{
	if (ASoldier* Soldier = Cast<ASoldier>(K2_GetPawn()); Soldier)
		Soldier->cycleBetweenTeam();
}


void ASoldierPlayerController::ClientSendCommand_Implementation(const FString& Cmd, bool bWriteToLog)
{
	ConsoleCommand(Cmd, bWriteToLog);
}

void ASoldierPlayerController::OnSquadChanged_Implementation(const TArray<FSoldierAIData>& newValue)
{
	SquadManagerData.OnSquadDataChanged(newValue);
	if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
	{
		CurrentHUD->OnSquadChanged(newValue);
	}
}

void ASoldierPlayerController::OnSquadMemberHealthChanged_Implementation(int index, float newHealth)
{
	if (SquadManagerData.SquadData.IsValidIndex(index))
	{
		SquadManagerData.SquadData[index].Health = newHealth;
		//Appel au HUD
		if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
		{
			CurrentHUD->OnSquadHealthChanged(index, newHealth);
		}
	}
	// Erreur syncronisation client / serveur
}

void ASoldierPlayerController::OnSquadMemberMaxHealthChanged_Implementation(int index, float newMaxHealth)
{
	if (SquadManagerData.SquadData.IsValidIndex(index))
	{
		SquadManagerData.SquadData[index].MaxHealth = newMaxHealth;
		//Appel au HUD
		if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
		{
			CurrentHUD->OnSquadMaxHealthChanged(index, newMaxHealth);
		}
	}
	// Erreur syncronisation client / serveur
}

void ASoldierPlayerController::OnSquadMemberShieldChanged_Implementation(int index, float newShield)
{
	if (SquadManagerData.SquadData.IsValidIndex(index))
	{
		SquadManagerData.SquadData[index].Shield = newShield;
		//Appel au HUD
		if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
		{
			CurrentHUD->OnSquadShieldChanged(index, newShield);
		}
	}
	// Erreur syncronisation client / serveur
}

void ASoldierPlayerController::OnSquadMemberMaxShieldChanged_Implementation(int index, float newMaxShield)
{
	if (SquadManagerData.SquadData.IsValidIndex(index))
	{
		SquadManagerData.SquadData[index].MaxShield = newMaxShield;
		//Appel au HUD
		if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
		{
			CurrentHUD->OnSquadMaxShieldChanged(index, newMaxShield);
		}
	}
	// Erreur syncronisation client / serveur
}

void ASoldierPlayerController::OnTextNotification_Received_Implementation(const FString& notificationString)
{
	if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
	{
		CurrentHUD->OnTextNotification_Received(notificationString);
	}
}

void ASoldierPlayerController::OnAllyTicket_Received_Implementation(int newTicket)
{
	if (auto HUD = GetHUD<ITicketInterface>(); HUD)
	{
		HUD->OnAllyTicketChanged(newTicket);
	}
}

void ASoldierPlayerController::OnEnnemyTicket_Received_Implementation(int newTicket)
{
	if (auto HUD = GetHUD<ITicketInterface>(); HUD)
	{
		HUD->OnEnnemyTicketChanged(newTicket);
	}
}

void ASoldierPlayerController::OnGameEnd_Implementation(const int MatchResult, float GameDuration)
{
	if (auto HUD = GetHUD<IGameEndInterface>(); HUD)
	{
		if (MatchResult == 1) HUD->OnGameEnd("VICTORY !");
		else HUD->OnGameEnd("DEFEAT !");
	}

	auto XP = GetPawn<ASoldier>()->GetEXP();
	GetGameInstance<USquadLeaderGameInstance>()->UpdateNetworkStatus(MatchResult, GameDuration, XP, GetPlayerState<ASoldierPlayerState>()->PersonalRecord);  // notify the server
}

void ASoldierPlayerController::OnOrderGiven_Implementation(MissionType Order, FVector Pos)
{
	if (ASoldierPlayer* Soldier = GetPawn<ASoldierPlayer>(); Soldier)
	{
		if (AAISquadManager* SquadManager = Soldier->GetSquadManager(); SquadManager)
		{
			SquadManager->UpdateMission(Order, Pos);
		}
	}
}

void ASoldierPlayerController::AddAnAIToIndexSquad()
{
	if (GetLocalRole() < ROLE_Authority)
		ServerAddAnAIToIndexSquad_Implementation();

	if (Cast<ASoldierPlayer>(GetPawn())->GetSquadManager())
		Cast<ASoldierPlayer>(GetPawn())->GetSquadManager()->AddAnAIToSquad();
}

void ASoldierPlayerController::ServerAddAnAIToIndexSquad_Implementation()
{
	AddAnAIToIndexSquad();
}

void ASoldierPlayerController::BroadCastManagerData()
{
	if (ASL_HUD* CurrentHUD = GetHUD<ASL_HUD>(); CurrentHUD)
		CurrentHUD->OnSquadChanged(SquadManagerData.SquadData);
}

// TODO: Use the soldier list - Don't use all the actors from the world
void ASoldierPlayerController::OnWallVisionActivate_Implementation()
{
	bool Active = false;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASoldier::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		if (ASoldier* Soldier = Cast<ASoldier>(Actor); Soldier)
			if (Soldier->GetTeam() != GetTeam()) {
				Soldier->GetMesh()->SetRenderCustomDepth(true);
				Active = true;
			}
	}
	if (!Active) {
		FTimerHandle Timer;
		GetWorldTimerManager().SetTimer(Timer, this, &ASoldierPlayerController::OnWallVisionActivate_Implementation, 2.f, false);
	}
}

void ASoldierPlayerController::OnWallVisionDeactivate_Implementation()
{
	for (AActor* Actor : GetWorld()->PersistentLevel->Actors)
	{
		if (ASoldier* Soldier = Cast<ASoldier>(Actor); Soldier)
			Soldier->GetMesh()->SetRenderCustomDepth(false);
	}
}

//float ASoldierPlayerController::GetCooldown(const ESoldierAbilityInputID _AbilityID) const
//{
//
//}

void ASoldierPlayerController::Cheat_AddAISquad()
{
	AddAnAIToIndexSquad();
}

void ASoldierPlayerController::Cheat_SuperSoldier()
{
	if (GetLocalRole() < ROLE_Authority)
		ServerCheat_SuperSoldier();

	if (ASoldierPlayer* Soldier = GetPawn<ASoldierPlayer>(); Soldier)
	{
		const float badassValue = 9'999'999.f;

		Soldier->GetAttributeSet()->SetMaxHealth(badassValue);
		Soldier->GetAttributeSet()->SetHealth(badassValue);
		Soldier->GetAttributeSet()->SetMaxShield(badassValue);
		Soldier->GetAttributeSet()->SetShield(badassValue);
		Soldier->GetAttributeSet()->SetMoveSpeedWalk(1000.f);
		Soldier->GetAttributeSet()->SetMoveSpeedCrouch(1000.f);
		
		if (ASL_Weapon* Weapon = Soldier->GetCurrentWeapon(); Weapon)
		{
			Weapon->ReloadWeapon();
			Weapon->SetWeaponDamage(badassValue);
			Weapon->SetHasInfiniteAmmo(true);
		}
	}
}

void ASoldierPlayerController::ServerCheat_SuperSoldier_Implementation()
{
	Cheat_SuperSoldier();
}

void ASoldierPlayerController::Cheat_Die()
{
	if (GetLocalRole() < ROLE_Authority)
		ServerCheat_Die();

	if (ASoldierPlayer* Soldier = GetPawn<ASoldierPlayer>(); Soldier)
		Soldier->GetAttributeSet()->SetHealth(0.f);
}

void ASoldierPlayerController::ServerCheat_Die_Implementation()
{
	Cheat_Die();
}

void ASoldierPlayerController::Cheat_SuperDamage()
{
	if (GetLocalRole() < ROLE_Authority)
		ServerCheat_SuperDamage();
	
	if (ASoldierPlayer* Soldier = GetPawn<ASoldierPlayer>(); Soldier && Soldier->GetCurrentWeapon())
		Soldier->GetCurrentWeapon()->SetWeaponDamage(999999.f);
}

void ASoldierPlayerController::ServerCheat_SuperDamage_Implementation()
{
	Cheat_SuperDamage();
}

void ASoldierPlayerController::Cheat_LevelUp()
{
	if (GetLocalRole() < ROLE_Authority)
		ServerCheat_LevelUp();

	if (ASoldierPlayer* Soldier = GetPawn<ASoldierPlayer>(); Soldier)
		Soldier->GrantEXP(Soldier->GetRemainEXPForLevelUp());
}

void ASoldierPlayerController::ServerCheat_LevelUp_Implementation()
{
	Cheat_LevelUp();
}