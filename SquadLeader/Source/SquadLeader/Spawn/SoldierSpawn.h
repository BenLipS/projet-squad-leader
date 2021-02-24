// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "Engine/TargetPoint.h"
#include "Net/UnrealNetwork.h"
#include "SoldierSpawn.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API ASoldierSpawn : public ATargetPoint
{
	GENERATED_BODY()

public:
	ASoldierSpawn();

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// for replication purpose
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditInstanceOnly, Replicated, Category = "SpawnCondition")
	bool canBeUsed;
};
