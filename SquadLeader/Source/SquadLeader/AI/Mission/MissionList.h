// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CaptureMission.h"
#include "DefendMission.h"
#include "PatrolMission.h"
#include "FormationMission.h"
#include "Misc/TVariant.h"
#include "MissionList.generated.h"

/**
 * This class represent the functionnality of a Mission Manager List
 * Each AI will possess one
 */


USTRUCT()
struct SQUADLEADER_API FMissionAction {
	GENERATED_USTRUCT_BODY()

	class AAIGeneralController* m_ai_controller;

	FMissionAction() = default;
	FMissionAction(AAIGeneralController* _ai_controller) { m_ai_controller = _ai_controller; }

	auto operator()(UCaptureMission* _mission)const;
	auto operator()(UDefendMission* _mission)const;
	auto operator()(UPatrolMission* _mission)const;
	auto operator()(UFormationMission* _mission)const;
};


USTRUCT()
struct SQUADLEADER_API FCompareMission {
	GENERATED_USTRUCT_BODY()


	FCompareMission() = default;

	auto operator()(UCaptureMission* _mission_left, UDefendMission* _mission_right)const{ return _mission_left->GetPriority() > _mission_right->GetPriority(); }
	auto operator()(UCaptureMission* _mission_left, UPatrolMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UCaptureMission* _mission_left, UCaptureMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UCaptureMission* _mission_left, UFormationMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };

	auto operator()(UDefendMission* _mission_left, UDefendMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UDefendMission* _mission_left, UPatrolMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UDefendMission* _mission_left, UCaptureMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UDefendMission* _mission_left, UFormationMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	
	auto operator()(UPatrolMission* _mission_left, UDefendMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UPatrolMission* _mission_left, UPatrolMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UPatrolMission* _mission_left, UCaptureMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UPatrolMission* _mission_left, UFormationMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };

	auto operator()(UFormationMission* _mission_left, UDefendMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); }
	auto operator()(UFormationMission* _mission_left, UPatrolMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UFormationMission* _mission_left, UCaptureMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };
	auto operator()(UFormationMission* _mission_left, UFormationMission* _mission_right)const { return _mission_left->GetPriority() > _mission_right->GetPriority(); };

};



UCLASS()
class SQUADLEADER_API UMissionList : public UObject
{
	GENERATED_BODY()

public:
	using type_mission = TVariant<UCaptureMission*, UDefendMission*, UPatrolMission*, UFormationMission*>;
	using array_mission_type = TArray<type_mission>;

	UMissionList();

protected:
	UPROPERTY()
	class AAIGeneralController* m_ai_controller;

	array_mission_type m_missions;

	FMissionAction m_action;
	FCompareMission m_compare;

	UPROPERTY()
	int m_index_current_mission = 0;


public:
	/*
	* Initialize the Object
	*/
	UFUNCTION()
		void Init(AAIGeneralController* _ai_controller);

	/*
	* Add the mission at the good place
	* After this the Array is still sorted
	*/
	void Add(type_mission _mission);

	/*
	* Return the current mission
	*/
	auto GetCurrentMission() const;

	void EndMission();

	void RunMission();
};