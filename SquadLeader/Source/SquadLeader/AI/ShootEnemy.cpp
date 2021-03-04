// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootEnemy.h"
#include "AIGeneralController.h"

/** Constructeur de la classe */
UShootEnemy::UShootEnemy()
{
	// Nous d�finissons le nom que protera le noeud dans le BT
	NodeName = "ShootEnemy";
}
/* Fonction d'ex�cution de la t�che, cette t�che devra retourner Succeeded, Failed ou InProgress */
EBTNodeResult::Type UShootEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{

	AAIGeneralController* AIGeneralController = Cast<AAIGeneralController>(OwnerComp.GetOwner());

	ResultState _result = AIGeneralController->ShootEnemy();
	if (_result == ResultState::Success)
		return EBTNodeResult::Succeeded;
	if (_result == ResultState::Failed)
		return EBTNodeResult::Failed;
	return EBTNodeResult::InProgress;
}

void UShootEnemy::TickTask(class UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	AAIGeneralController* _controller = Cast<AAIGeneralController>(OwnerComp.GetOwner());

	if (_controller) {
		ResultState _result = _controller->ShootEnemy();
		if(_result == ResultState::Success)
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		if(_result == ResultState::InProgress)
			FinishLatentTask(OwnerComp, EBTNodeResult::InProgress);
		if(_result == ResultState::Failed)
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}
/** Permet de d�finir une description pour la t�che. C'est ce texte qui
 appara�tra dans le noeud que nous ajouterons au Behavior Tree */
FString UShootEnemy::GetStaticDescription() const
{
	return TEXT("Shoot on an enemy");
}

