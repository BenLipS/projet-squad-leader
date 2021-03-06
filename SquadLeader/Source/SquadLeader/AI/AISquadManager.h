#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../Soldiers/Players/SoldierPlayer.h"
#include "AISquadController.h"
#include "../Soldiers/AIs/SoldierAI.h"
#include "Mission.h"
#include "AISquadManager.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSquadInfoArrayChanged, const TArray<FSoldierAIData>&, SquadArray);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSquadMemberFloatDataChanged, int, Index, float, newValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSquadMemberStateDataChanged, int, Index, AIBasicState, newValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSquadMemberClassDataChanged, int, Index, SoldierClass, newValue);

UENUM()
enum class FormationType { Circle, Arrow };

USTRUCT()
struct FAISquadManagerData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FSoldierAIData> SquadData;

	FAISquadManagerData() = default;

	void OnSquadDataChanged(const TArray<FSoldierAIData>& newValue);
};

UCLASS(Blueprintable)
class SQUADLEADER_API AAISquadManager : public AInfo
{
	GENERATED_BODY()

// TODO: Check if all this should be public
public:
	//-----DELEGATE-----
	FSquadInfoArrayChanged OnSquadChanged;

	FSquadMemberFloatDataChanged OnMemberHealthChanged;
	FSquadMemberFloatDataChanged OnMemberMaxHealthChanged;
	FSquadMemberFloatDataChanged OnMemberShieldChanged;
	FSquadMemberFloatDataChanged OnMemberMaxShieldChanged;
	FSquadMemberStateDataChanged OnMemberStateChanged;
	FSquadMemberClassDataChanged OnMemberClassChanged;

public:
	AAISquadManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:
	UPROPERTY(BlueprintReadOnly)
	TArray<AAISquadController*> AISquadControllerList;

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AISoldierList)
	TArray<ASoldierAI*> AISoldierList;

	// Utilsier les fonctions ici quand on ajoute une ia pur forcer le squadManage Onrep
	// Faudra verifier qu on n a pas de duplicat
	UFUNCTION()
	void OnRep_AISoldierList();

public:
	TArray<ASoldierAI*> GetAISoldierList() const;

protected:
	UPROPERTY(EditDefaultsOnly)
	int OverrideStartNumberOfSoldiers = -1;

public:
	UFUNCTION()
	void Init(ASoldierTeam* _Team, ASoldierPlayer* _Player);
	
	UPROPERTY()
	ASoldierTeam* Team;

	UPROPERTY(Replicated)
	ASoldierPlayer* Leader;

	UFUNCTION(Server, Reliable)
	void AddAnAIToSquad();
	void AddAnAIToSquad_Implementation();

	// Check whether this manager controls the given soldier
	bool HasSoldier(ASoldier* _Soldier) const;

	auto GetMission() { return Mission; };

	//For Formation
	TArray<FVector> FormationPos;

	FormationType TypeOfFormation;

	UFUNCTION()
	void UpdateFormation();

	UFUNCTION()
	void UpdateCircleFormation();

	UFUNCTION()
		void UpdateArrowFormation();

	UPROPERTY()
	UMission* Mission;

	UFUNCTION(BlueprintCallable)
	void UpdateMission(const MissionType _MissionType, const FVector& _Location);

	UFUNCTION()
	FVector GetValidMissionLocation(const FVector& _Location);

	// temp function, need to be replace by more robust code
	void UpdateSquadTeam(ASoldierTeam* _NewTeam);

	UFUNCTION()
	void BroadCastSquadData();

	UFUNCTION()
	void OnSquadMemberHealthChange(float newValue, AAISquadController* SoldierController);

	UFUNCTION()
	void OnSquadMemberMaxHealthChange(float newValue, AAISquadController* SoldierController);

	UFUNCTION()
	void OnSquadMemberShieldChange(float newValue, AAISquadController* SoldierController);

	UFUNCTION()
	void OnSquadMemberMaxShieldChange(float newValue, AAISquadController* SoldierController);

	UFUNCTION()
	void OnSquadMemberMissionChange(AIBasicState newValue, AAISquadController* SoldierController);

	UFUNCTION()
	void OnSquadMemberClassChange(SoldierClass newValue, AAISquadController* SoldierController);

	//for healing coordination
	UPROPERTY()
	bool IsASquadMemberHealing = false;

	//for shielding coordination
	UPROPERTY()
	bool IsASquadMemberShielding = false;

protected:
	bool m_inFormation = false;
};
