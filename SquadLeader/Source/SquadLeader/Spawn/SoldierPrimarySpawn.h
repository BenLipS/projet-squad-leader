// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "SoldierSpawn.h"
#include "../Soldiers/SoldierTeam.h"
#include "Net/UnrealNetwork.h"
#include "SoldierPrimarySpawn.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API ASoldierPrimarySpawn : public ASoldierSpawn
{
	GENERATED_BODY()

public:
	ASoldierPrimarySpawn();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// for replication purpose
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:  // Owner part
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "SpawnCondition")
		TSubclassOf<ASoldierTeam> teamOwner = nullptr;
};
