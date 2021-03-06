#include "ControlArea.h"
#include "../GameState/SquadLeaderGameState.h"
#include "../SquadLeaderGameModeBase.h"
#include "../Soldiers/Soldier.h"
#include "../Soldiers/Players/SoldierPlayerController.h"
#include "../UI/HUD/SL_HUD.h"
#include "../AI/AIBasicManager.h"
#include "ControlAreaManager.h"
#include "Camera/CameraActor.h"

AControlArea::AControlArea()
{
	initCollideElement();

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	CameraActor = CreateDefaultSubobject<ACameraActor>(TEXT("Camera Actor"));
}

// used when initialising the control area
void AControlArea::initCollideElement() {
}

void AControlArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlArea, ControlAreaName, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlArea, IsTakenBy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlArea, IsCapturedBy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AControlArea, PercentageCapture, COND_None, REPNOTIFY_Always);
}

void AControlArea::PreInitialisation()
{
	if (auto GS = GetWorld()->GetGameState<ASquadLeaderGameState>(); GS) {
		// add this to the game mode collection
		auto test = GS->GetControlAreaManager();
		test->AddControlArea(this);

		UpdateTeamData();
	}
}

int AControlArea::GetPriority() const
{
	return 2;
}

void AControlArea::AddSoldierPresence(ASoldier* _Soldier)
{
	if (TeamData.Contains(_Soldier->GetTeam()))
	{
		TeamData[_Soldier->GetTeam()]->presenceTeam++;

		// Initiate the calculation of the control zone value if needed
		if (!timerCalculationControlValue.IsValid())
			GetWorldTimerManager().SetTimer(timerCalculationControlValue, this, &AControlArea::calculateControlValue, timeBetweenCalcuation, true, timeBetweenCalcuation);

		_Soldier->OnSoldierDeath.AddDynamic(this, &AControlArea::OnSoldierDeath);
	}
	// else GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("ControlArea : Player of an unknow team"));
}

void AControlArea::RemoveSoldierPresence(ASoldier* _Soldier)
{
	if (TeamData.Contains(_Soldier->GetTeam()))
	{
		if (TeamData[_Soldier->GetTeam()]->presenceTeam > 0)
			TeamData[_Soldier->GetTeam()]->presenceTeam--;

		// Begin the calculation if everybody of this team left and the calculation is not already working
		if (TeamData[_Soldier->GetTeam()]->presenceTeam == 0) {
			if (!timerCalculationControlValue.IsValid())
				GetWorldTimerManager().SetTimer(timerCalculationControlValue, this,
					&AControlArea::calculateControlValue, timeBetweenCalcuation, true, timeBetweenCalcuation);
		}

		_Soldier->OnSoldierDeath.RemoveDynamic(this, &AControlArea::OnSoldierDeath);
	}
}

void AControlArea::OnSoldierDeath(ASoldier* _Soldier)
{
	RemoveSoldierPresence(_Soldier);
}

ASoldierTeam* AControlArea::GetIsTakenBy()
{
	return IsTakenBy;
}

void AControlArea::SetIsTakenBy(ASoldierTeam* newTeam)
{
	if (newTeam != IsTakenBy)
	{
		IsTakenBy = newTeam;

		if (ASoldierPlayerController* playerController = GetWorld()->GetFirstPlayerController<ASoldierPlayerController>(); playerController)
		{
			int AreaOwner = 0;
			if (IsTakenBy) {
				if (IsTakenBy == playerController->GetTeam()) {
					AreaOwner = 1;
				}
				else AreaOwner = -1;
			}
			OnOwnerChanged.Broadcast(AreaOwner);
		}
	}
}

ASoldierTeam* AControlArea::GetIsCapturedBy()
{
	return IsCapturedBy;
}

void AControlArea::SetIsCapturedBy(ASoldierTeam* newTeam)
{
	if (newTeam != IsCapturedBy)
	{
		IsCapturedBy = newTeam;

		if (ASoldierPlayerController* playerController = GetWorld()->GetFirstPlayerController<ASoldierPlayerController>(); playerController)
		{
			int AreaCapturer = 0;
			if (IsCapturedBy) {
				if (IsCapturedBy == playerController->GetTeam()) {
					AreaCapturer = 1;
				}
				else AreaCapturer = -1;
			}
			OnCapturerChanged.Broadcast(AreaCapturer);
		}
	}
}

void AControlArea::OnRepOwner()
{
	if (ASoldierPlayerController* playerController = GetWorld()->GetFirstPlayerController<ASoldierPlayerController>(); playerController)
	{
		int AreaOwner = 0;
		if (IsTakenBy) {
			if (IsTakenBy == playerController->GetTeam()) {
				AreaOwner = 1;
			}
			else AreaOwner = -1;
		}
		OnOwnerChanged.Broadcast(AreaOwner);
	}
}

void AControlArea::OnRepCapturer()
{
	if (ASoldierPlayerController* playerController = GetWorld()->GetFirstPlayerController<ASoldierPlayerController>(); playerController)
	{
		int AreaCapturer = 0;
		if (IsCapturedBy) {
			if (IsCapturedBy == playerController->GetTeam()) {
				AreaCapturer = 1;
			}
			else AreaCapturer = -1;
		}
		OnCapturerChanged.Broadcast(AreaCapturer);
	}
}

void AControlArea::OnRepPercentage()
{
	OnPercentageChanged.Broadcast(PercentageCapture);
}

void AControlArea::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (GetLocalRole() == ROLE_Authority) 
	{
		if (ASoldier* Soldier = Cast<ASoldier>(OtherActor); Soldier && Soldier->IsAlive())
			AddSoldierPresence(Soldier);
	}
}

void AControlArea::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
	if (GetLocalRole() == ROLE_Authority)
	{
		if (ASoldier* Soldier = Cast<ASoldier>(OtherActor); Soldier && Soldier->IsAlive())
			RemoveSoldierPresence(Soldier);
	}
}

void AControlArea::calculateControlValue()
{
	/*GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea : None"));
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea : Max value reach"));*/
	if (GetLocalRole() == ROLE_Authority) {  // only for the server

		// check info about the differents teams on point
		int nbTeamOnPoint = 0;
		ASoldierTeam* presentTeam = nullptr;
		for (TPair<ASoldierTeam*, AControlAreaTeamStat*> team : TeamData) {
			if (team.Value->presenceTeam > 0) {
				nbTeamOnPoint++;
				presentTeam = team.Key;
			}
		}

		// handling teams information
		if (presentTeam && nbTeamOnPoint == 1) {
			if (TeamData[presentTeam]->controlValue < MaxControlValue) {
				bool needToDecreaseOtherPresenceFirst = false;
				for (auto& otherTeam : TeamData) {  // reduce the control value in each other team by the number of teamate
					if (otherTeam.Key != presentTeam) {
						if (otherTeam.Value->controlValue >= TeamData[presentTeam]->presenceTeam) {
							otherTeam.Value->controlValue -= TeamData[presentTeam]->presenceTeam;
							needToDecreaseOtherPresenceFirst = true;
							if (IsCapturedBy != otherTeam.Key)
							{
								SetIsCapturedBy(otherTeam.Key);
							}
							PercentageCapture = static_cast<float>(otherTeam.Value->controlValue) / MaxControlValue;
							OnPercentageChanged.Broadcast(PercentageCapture);

							auto GM = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
							for (auto& Manager : GM->AIBasicManagerCollection) {
								if (Manager.Value->GetTeamID() == otherTeam.Key->Id)
									Manager.Value->LostControlArea(IndexControlArea);
							}

						}
						else {
							otherTeam.Value->controlValue = 0;
						}
						if (IsTakenBy == otherTeam.Key && otherTeam.Value->controlValue <= MinControlValueToControl) {  // remove IsTakenBy if needed
							// notify here the changement if needed
							SetIsTakenBy(nullptr);
							otherTeam.Value->ChangeSpawnState(false);
							//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea : Team control = None"));
						}
						//ClientNotifyValueChange(otherTeam.Value->controlValue, IsTakenBy, otherTeam.Key);  // call client function to notify the modification
					}
				}
				if (!needToDecreaseOtherPresenceFirst) {  // if all other presence value is 0
					if (TeamData[presentTeam]->controlValue + TeamData[presentTeam]->presenceTeam < MaxControlValue)
						TeamData[presentTeam]->controlValue += TeamData[presentTeam]->presenceTeam;
					else
						TeamData[presentTeam]->controlValue = MaxControlValue;

					if (IsTakenBy != presentTeam && TeamData[presentTeam]->controlValue >= ControlValueToTake) {  // take control of the point
						SetIsTakenBy(presentTeam);
						TeamData[presentTeam]->ChangeSpawnState(true);
						// notify here the changement if needed
						//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea : Team control =" + presentTeam->TeamName));

						if (ASquadLeaderGameModeBase* GameMode = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode()); GameMode) {
							GameMode->NotifyControlAreaCaptured(this);
						}

						auto GM = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
						FGridPackage m_package;
						m_package.m_location_on_map = GetActorLocation();

						m_package.team_value = presentTeam->Id;
						m_package.m_type = Type::ControlArea;
						m_package.ActorID = this->GetUniqueID();
						GM->InfluenceMap->ReceivedMessage(m_package);

						for (auto& Manager : GM->AIBasicManagerCollection)
							Manager.Value->UpdateControlArea(presentTeam->Id, IndexControlArea);

						//double Value = GetInfluenceAverage();
						//GEngine->AddOnScreenDebugMessage(10,10.f, FColor::Black, FString::Printf(TEXT("L'influence de l'equipe est de : %f."), Value));
					}
					if (IsCapturedBy != presentTeam)
					{
						SetIsCapturedBy(presentTeam);
					}
					//ClientNotifyValueChange(TeamData[presentTeam]->controlValue, IsTakenBy, presentTeam);  // call client function to notify the modification
					PercentageCapture = static_cast<float>(TeamData[presentTeam]->controlValue) / MaxControlValue;
					OnPercentageChanged.Broadcast(PercentageCapture);
				}
			}
			else { // stop the timer
				GetWorld()->GetTimerManager().ClearTimer(timerCalculationControlValue);
				//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea : Max control for " + presentTeam->TeamName));
			}
		}
		else {  // too much teams on points or nobody
			if (nbTeamOnPoint == 0) {
			}
			// stop the timer
			GetWorld()->GetTimerManager().ClearTimer(timerCalculationControlValue);
		}
	}
}

void AControlArea::UpdateTeamData()
{
	if (GetLocalRole() == ROLE_Authority) {  // only for the server
		if (auto GS = GetWorld()->GetGameState<ASquadLeaderGameState>(); GS) {
			auto teamCollection = GS->GetSoldierTeamCollection();

			TArray<ASoldierTeam*> keyToRemove;  // remove element
			for (TPair<ASoldierTeam*, AControlAreaTeamStat*> team : TeamData) {
				if (!teamCollection.Contains(team.Key)) {
					keyToRemove.Add(team.Key);
					//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea Update : Unknown team removed : " + team.Key.GetDefaultObject()->TeamName));
				}
			}
			for (auto key : keyToRemove) {
				TeamData.Remove(key);
			}

			for (auto team : teamCollection) {  // add element
				if (!TeamData.Contains(team)) {
					TeamData.Add(team, NewObject<AControlAreaTeamStat>());
					//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("ControlArea Update : Unknown team added : " + team.GetDefaultObject()->TeamName));
				}
			}

			for (auto& team : TeamData) {  // update spawn state
				if (team.Value) {
					team.Value->ChangeSpawnState(IsTakenBy == team.Key);
					team.Value->ChangeSpawnTeam(team.Key);
				}
			}
		}
	}
}

double AControlArea::GetInfluenceAverage() {
	auto GM = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
	double Value = GM->InfluenceMap->GetInfluenceAverage(this->GetUniqueID(), GetIsCapturedBy()->Id);
	return Value;
}

double AControlArea::GetEnnemiInfluenceAverage() {
	auto GM = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
	double Value = 0.0;
	switch (GetIsCapturedBy()->Id) {
	case 1:
		Value = GM->InfluenceMap->GetInfluenceAverage(this->GetUniqueID(), 2);
		break;
	case 2:
		Value = GM->InfluenceMap->GetInfluenceAverage(this->GetUniqueID(), 1);
		break;
	default:
		break;
	}

	return Value;
}

void AControlArea::BroadcastDatas()
{
	if (ASoldierPlayerController* playerController = GetWorld()->GetFirstPlayerController<ASoldierPlayerController>(); playerController)
	{
		int AreaOwner = 0;
		if (IsTakenBy) {
			if (IsTakenBy == playerController->GetTeam()) {
				AreaOwner = 1;
			}
			else AreaOwner = -1;
		}
		OnOwnerChanged.Broadcast(AreaOwner);

		int AreaCapturer = 0;
		if (IsCapturedBy) {
			if (IsCapturedBy == playerController->GetTeam()) {
				AreaCapturer = 1;
			}
			else AreaCapturer = -1;
		}
		OnCapturerChanged.Broadcast(AreaCapturer);
	}

	OnPercentageChanged.Broadcast(PercentageCapture);
}
