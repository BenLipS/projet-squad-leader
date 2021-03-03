// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Teamable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTeamable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SQUADLEADER_API ITeamable
{
	GENERATED_BODY()

private:
	class ASoldierTeam* Team = nullptr;

public:
	UFUNCTION()
	virtual ASoldierTeam* GetTeam();

	UFUNCTION()
	virtual bool SetTeam(ASoldierTeam* _Team);
};
