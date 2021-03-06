#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SquadLeader/Soldiers/Soldier.h"
#include "SquadLeader/ControlArea/ControlArea.h"
#include "MinimapInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMinimapInterface : public UInterface
{
	GENERATED_BODY()
};

class SQUADLEADER_API IMinimapInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void OnSoldierAddedToTeam(ASoldier* _Soldier) = 0;

	UFUNCTION()
	virtual void OnSoldierRemovedFromTeam(ASoldier* _Soldier) = 0;

	UFUNCTION()
	virtual void OnControlAreaAdded(AControlArea* _ControlArea) = 0;

	UFUNCTION()
	virtual void OnPingAdded(FVector2D PosPingActor) = 0;

	UFUNCTION()
	virtual void OnPingDestroyed() = 0;

	UFUNCTION()
	virtual void OnUpdatePOIs() = 0;

	UFUNCTION()
	virtual void OnFullMapDisplayBegin() = 0;

	UFUNCTION()
	virtual void OnFullMapDisplayEnd() = 0;
};
