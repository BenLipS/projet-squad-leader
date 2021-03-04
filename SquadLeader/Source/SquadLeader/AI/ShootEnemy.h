// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ShootEnemy.generated.h"

/**
 * 
 */
UCLASS()
class SQUADLEADER_API UShootEnemy : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UShootEnemy();
	/* Fonction d'ex�cution de la t�che, cette t�che devra retourner Succeeded, Failed ou InProgress */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
		override;

	virtual void TickTask(class UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	/** Permet de d�finir une description pour la t�che. C'est ce texte qui
	 appara�tra dans le noeud que nous ajouterons au Behavior Tree */
	virtual FString GetStaticDescription() const override;
	
};
