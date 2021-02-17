// Fill out your copyright notice in the Description page of Project Settings.


#include "AIGeneralController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Soldiers/Soldier.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../Soldiers/AIs/SoldierAI.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

AAIGeneralController::AAIGeneralController(FObjectInitializer const& object_initializer)
{
	setup_BehaviorTree();
	setup_perception_system();
	m_destination = FVector(0.f, 0.f, 0.f);
}

void AAIGeneralController::BeginPlay() {
	Super::BeginPlay();
	RunBehaviorTree(m_behaviorTree);
}

void AAIGeneralController::on_update_sight2(AActor* actor, FAIStimulus const stimulus) {
	/*if(auto const ch = Cast<ASoldier>(actor)){
		if(GEngine)
			GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Red, FString::Printf(TEXT("I see: %s"), *actor->GetName()));
	}*/
	//todo clear Focus when Soldier out of range
	if (stimulus.IsValid()) this->SetFocus(actor);
	else this->ClearFocus(EAIFocusPriority::Gameplay);	
	UBlackboardComponent* BlackboardComponent = BrainComponent->GetBlackboardComponent();
	BlackboardComponent->SetValueAsObject("EnemyActor", actor);
};

void AAIGeneralController::on_update_sight(const TArray<AActor*>& AArray) {
	//if (GEngine)GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Red, TEXT("I see 2"));

	//todo clear Focus when Soldier out of range
	if (AArray.Num() > 0) this->SetFocus(AArray[0]);
	else this->ClearFocus(EAIFocusPriority::Gameplay);

	UBlackboardComponent* BlackboardComponent = BrainComponent->GetBlackboardComponent();
	BlackboardComponent->SetValueAsObject("EnemyActor", AArray[0]);
};

void AAIGeneralController::setup_perception_system() {
	// create and initialise sight configuration object
	sight_config = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (sight_config)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
		sight_config->SightRadius = m_distancePerception;
		sight_config->LoseSightRadius = sight_config->SightRadius + 50.0f;
		sight_config->PeripheralVisionAngleDegrees = 82.5f;
		sight_config->SetMaxAge(.2f);
		sight_config->AutoSuccessRangeFromLastSeenLocation = 100.0f;
		sight_config->DetectionByAffiliation.bDetectEnemies = true;
		sight_config->DetectionByAffiliation.bDetectFriendlies = true;
		sight_config->DetectionByAffiliation.bDetectNeutrals = true;

		// add sight configuration component to perception component
		GetPerceptionComponent()->SetDominantSense(*sight_config->GetSenseImplementation());
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &AAIGeneralController::on_update_sight2);
		GetPerceptionComponent()->OnPerceptionUpdated.AddDynamic(this, &AAIGeneralController::on_update_sight);
		GetPerceptionComponent()->ConfigureSense(*sight_config);
	}
};

void AAIGeneralController::setup_BehaviorTree() {
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> obj(TEXT("BehaviorTree'/Game/AI/BT_AIMainBehavior.BT_AIMainBehavior'"));
	if (obj.Succeeded())
		m_behaviorTree = obj.Object;
}

EPathFollowingRequestResult::Type AAIGeneralController::MoveToActorLocation() {
	UBlackboardComponent* BlackboardComponent = BrainComponent->GetBlackboardComponent();
	AActor* _actor = Cast<AActor>(BlackboardComponent->GetValueAsObject("ActorLocation"));

	EPathFollowingRequestResult::Type _movetoResult = MoveToActor(_actor);

	return _movetoResult;
}

EPathFollowingRequestResult::Type AAIGeneralController::MoveToVectorLocation() {
	UBlackboardComponent* BlackboardComponent = BrainComponent->GetBlackboardComponent();
	
	if (BlackboardComponent->GetValueAsBool("is_attacking"))
		return EPathFollowingRequestResult::Type::Failed;

	//TO-DO : if follow an enemy be at the distance to shoot 
	EPathFollowingRequestResult::Type _movetoResult = MoveToLocation(m_destination, 500.f);
	if(_movetoResult == EPathFollowingRequestResult::Type::AlreadyAtGoal)
		BlackboardComponent->ClearValue("VectorLocation");
	
	return _movetoResult;
}

EPathFollowingRequestResult::Type AAIGeneralController::MoveToEnemyLocation() {
	UBlackboardComponent* BlackboardComponent = BrainComponent->GetBlackboardComponent();
	ASoldier* _soldier_enemy = Cast<ASoldier>(BlackboardComponent->GetValueAsObject("EnemyActor"));

	ASoldierAI* _soldier = Cast<ASoldierAI>(GetPawn());
	if (_soldier) {
		UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), _soldier_enemy->GetActorLocation(), NULL);

		if (path->GetPathLength() >= m_distanceShootAndWalk)
			_soldier->ActivateAbilityRun();
		else
			_soldier->CancelAbilityRun();
	}

	//TO-DO : if follow an enemy be at the distance to shoot 
	EPathFollowingRequestResult::Type _movetoResult = MoveToLocation(_soldier_enemy->GetActorLocation(), m_distanceShootAndStop);
	if (_movetoResult == EPathFollowingRequestResult::Type::AlreadyAtGoal)
		BlackboardComponent->ClearValue("VectorLocation");

	return _movetoResult;
}

void AAIGeneralController::ShootEnemy() {
	if (GEngine) GEngine->AddOnScreenDebugMessage(30, 1.f, FColor::Red, TEXT("I shoot !"));
};

void AAIGeneralController::Tick(float DeltaSeconds) {
	
	Sens();
	Think(); // == if we need to change the BehaviorTree,
	//Act will be done in the behavior tree
	Super::Tick(DeltaSeconds);
}

void AAIGeneralController::Sens() {
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Yellow, TEXT("Sens !!"));
}

void AAIGeneralController::Think() {
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(20, 1.f, FColor::Purple, TEXT("Think !!"));

	UBlackboardComponent* BlackboardComponent = BrainComponent->GetBlackboardComponent();

	if (BlackboardComponent->GetValueAsObject("EnemyActor")) {
		//Attack Comportment
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(21, 1.f, FColor::Purple, TEXT("In Attack mode"));
		m_behavior = AIBehavior::Attack;
		BlackboardComponent->SetValueAsBool("is_attacking", true);
	}
	else {
		//Defens comportment
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(21, 1.f, FColor::Purple, TEXT("In Defensiv mode"));
		m_behavior = AIBehavior::Defense;
		BlackboardComponent->SetValueAsBool("is_attacking", false);
	}

}