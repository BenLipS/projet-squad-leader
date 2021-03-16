// Fill out your copyright notice in the Description page of Project Settings.


#include "FlockingComponent.h"
#include "../AIGeneralController.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationPath.h"

// Sets default values for this component's properties
UFlockingComponent::UFlockingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFlockingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UFlockingComponent::ResetVectors()
{
	SeenBoids.Empty();
	AlignementVector = FVector::ZeroVector;
	CohesionVector = FVector::ZeroVector;
	SeparationVector = FVector::ZeroVector;
	ObjectifVector = FVector::ZeroVector;
}

void UFlockingComponent::UpdateNeighbourhood()
{
	for (ASoldier* soldier : Cast<AAIGeneralController>(GetOwner())->GetSeenSoldier())
	{
		if (AAIGeneralController* AI = Cast<AAIGeneralController>(soldier->Controller); AI && soldier->GetTeam() == Cast<ASoldier>(Cast<AAIGeneralController>(GetOwner())->GetPawn())->GetTeam() && Cast<AAIGeneralController>(GetOwner())->GetObjectifLocation() == AI->GetObjectifLocation())
			SeenBoids.Add(AI);
	};
}

void UFlockingComponent::UpdateCohesionVector()
{
	FVector SoldierLocation = Cast<ASoldier>(Cast<AAIGeneralController>(GetOwner())->GetPawn())->GetLocation();
	for (AAIGeneralController* Boid : SeenBoids)
	{
		/* Cohesion throught pathfinding and not absolute*/
		UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), SoldierLocation, Boid->GetPawn()->GetActorLocation(), NULL);

		FVector CohesionLocalDir;
		if (path->PathPoints.Num() > 2)
			CohesionLocalDir = path->PathPoints[1];
		else
			CohesionLocalDir = Boid->GetPawn()->GetActorLocation();

		//DrawDebugPoint(GetWorld(), CohesionLocalDir, 10, FColor::Red);

		CohesionVector += CohesionLocalDir.GetSafeNormal(DefaultNormalizeVectorTolerance) * Boid->GetPawn()->GetActorLocation().Size() - SoldierLocation;
	}

	CohesionVector = (CohesionVector / SeenBoids.Num()) / 100;
}

void UFlockingComponent::UpdateAlignementVector()
{
	for (AAIGeneralController* Boid : SeenBoids)
	{
		AlignementVector += Boid->FlockingComponent->GetMovementVector().GetSafeNormal(DefaultNormalizeVectorTolerance);
	}

	AlignementVector = (MovementVector + AlignementVector).GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UFlockingComponent::UpdateSeparationVector()
{
	FVector SoldierLocation = Cast<ASoldier>(Cast<AAIGeneralController>(GetOwner())->GetPawn())->GetLocation();

	for (AAIGeneralController* Boid : SeenBoids)
	{
		FVector Separation = SoldierLocation - Boid->GetPawn()->GetActorLocation();
		SeparationVector += Separation.GetSafeNormal(DefaultNormalizeVectorTolerance) / FMath::Abs(Separation.Size() - BoidPhysicalRadius);
	}

	const FVector SeparationForceComponent = SeparationVector * 100;
	SeparationVector += SeparationForceComponent + SeparationForceComponent * (5 / SeenBoids.Num());
}

void UFlockingComponent::UpdateObjectifVector()
{
	FVector SoldierLocation = Cast<ASoldier>(Cast<AAIGeneralController>(GetOwner())->GetPawn())->GetLocation();
	UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	UNavigationPath* path = navSys->FindPathToLocationSynchronously(GetWorld(), SoldierLocation, Cast<AAIGeneralController>(GetOwner())->GetObjectifLocation(), NULL);

	FVector ObjectifLocalDir;
	if (path) {
		if (path->PathPoints.Num() > 1)
			ObjectifLocalDir = path->PathPoints[1];
		else
			ObjectifLocalDir = Cast<AAIGeneralController>(GetOwner())->GetObjectifLocation();
	}


	ObjectifLocalDir.Z = SoldierLocation.Z;

	//DrawDebugPoint(GetWorld(), ObjectifLocalDir, 10, FColor::Red);

	ObjectifVector = ObjectifLocalDir - SoldierLocation;
	ObjectifVector = ObjectifVector.GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UFlockingComponent::UpdateMovementVector()
{
	MovementVector = AlignementVector * AlignementWeight
		+ CohesionVector * CohesionWeight
		+ SeparationVector * SeparationWeight
		+ ObjectifVector * ObjectifWeight;
	MovementVector = MovementVector.GetSafeNormal(DefaultNormalizeVectorTolerance);
}

void UFlockingComponent::DrawDebug()
{
	FVector SoldierLocation = Cast<ASoldier>(Cast<AAIGeneralController>(GetOwner())->GetPawn())->GetLocation();
	SoldierLocation.Z += 100;
	FVector TempFlockPos = Cast<AAIGeneralController>(GetOwner())->get_blackboard()->GetValueAsVector("FlockingLocation");
	DrawDebugLine(GetWorld(), SoldierLocation, SoldierLocation + AlignementVector * AlignementWeight * 100, FColor::Green); /*Alignement vector*/
	DrawDebugLine(GetWorld(), SoldierLocation, SoldierLocation + CohesionVector * CohesionWeight * 100, FColor::Blue); /*Cohesion vector*/
	DrawDebugLine(GetWorld(), SoldierLocation, SoldierLocation + SeparationVector * SeparationWeight * 100, FColor::Red); /*Separation vector*/
	DrawDebugLine(GetWorld(), SoldierLocation, SoldierLocation + ObjectifVector * ObjectifWeight * 100, FColor::Yellow); /*Objectif vector*/
	DrawDebugLine(GetWorld(), SoldierLocation, SoldierLocation + MovementVector, FColor::Black); /*Movement vector*/
	DrawDebugPoint(GetWorld(), Cast<AAIGeneralController>(GetOwner())->GetObjectifLocation(), 12, FColor::Purple);
	DrawDebugPoint(GetWorld(), TempFlockPos, 12, FColor::Black);
}

void UFlockingComponent::UpdateFlockingPosition(float DeltaSeconds)
{
	ResetVectors();

	UpdateNeighbourhood();

	UpdateAlignementVector();

	if (SeenBoids.Num() > 0) { //because this involve /0
		UpdateCohesionVector();

		UpdateSeparationVector();
	}

	UpdateObjectifVector();

	UpdateMovementVector();

	float MaxSpeed = Cast<AAIGeneralController>(GetOwner())->GetPawn()->GetMovementComponent()->GetMaxSpeed();
	MovementVector = MovementVector * MaxSpeed;
	if (IsFlockingPositionValid()) {
		Cast<AAIGeneralController>(GetOwner())->get_blackboard()->SetValueAsVector("FlockingLocation", Cast<AAIGeneralController>(GetOwner())->GetPawn()->GetActorLocation() + MovementVector);
	}
	else {
		FVector RealObjectifLocation = Cast<AAIGeneralController>(GetOwner())->GetObjectifLocation();
		RealObjectifLocation.Z += 100;
		Cast<AAIGeneralController>(GetOwner())->get_blackboard()->SetValueAsVector("FlockingLocation", RealObjectifLocation);
	}

	if(DoDrawDebug)DrawDebug();
}

bool UFlockingComponent::IsFlockingPositionValid()
{
	FHitResult outHit;

	FVector startLocation = Cast<AAIGeneralController>(GetOwner())->GetPawn()->GetActorLocation();
	FVector endLocation = startLocation + MovementVector;

	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(Cast<ASoldierAI>(Cast<AAIGeneralController>(GetOwner())->GetPawn()));

	GetWorld()->LineTraceSingleByChannel(outHit, startLocation, endLocation, ECollisionChannel::ECC_WorldStatic, collisionParams);
	if (outHit.bBlockingHit) return false;
	//DrawDebugLine(GetWorld(), startLocation, endLocation, FColor::Green);
	//DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(), GetPawn()->GetActorLocation() + MovementVector, FColor::Blue);
	return true;
}

// Called every frame
void UFlockingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

