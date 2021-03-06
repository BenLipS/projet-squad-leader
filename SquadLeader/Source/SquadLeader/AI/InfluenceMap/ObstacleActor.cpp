// Fill out your copyright notice in the Description page of Project Settings.


#include "ObstacleActor.h"
#include "../../GameState/SquadLeaderGameState.h"
#include "../../SquadLeaderGameModeBase.h"

// Sets default values
AObstacleActor::AObstacleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AObstacleActor::~AObstacleActor() {
	//die
}

// Called when the game starts or when spawned
void AObstacleActor::BeginPlay()
{
	Super::BeginPlay();
	InitObstacleActor();
}

// Called every frame
void AObstacleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TickNumber == 150)
		DestroyObstacle();
	else
		TickNumber++;
}

void AObstacleActor::InitObstacleActor() {
	auto GM = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM && GM->InfluenceMap) {
		FGridPackage Package;
		Package.m_location_on_map = GetActorLocation();
		Package.team_value = Team;
		Package.m_type = Type::Soldier;
		Package.ActorID = this->GetUniqueID();
		GM->InfluenceMap->ReceivedMessage(Package);
	}
}

void AObstacleActor::DestroyObstacle() {
	//GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Cyan, TEXT("big explosion or new update of the Obstacle"));
	auto GM = Cast<ASquadLeaderGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM && GM->InfluenceMap) {
		FGridPackage Package;
		Package.m_location_on_map = GetActorLocation();
		Package.team_value = Team;
		Package.m_type = Type::Soldier;
		Package.ActorID = this->GetUniqueID();
		GM->InfluenceMap->ReceivedMessage(Package);
	}
	Destroy();
}