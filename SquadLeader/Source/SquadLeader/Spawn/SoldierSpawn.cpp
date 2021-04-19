// Fill out your copyright notice in the Description page of Project Settings.


#include "SoldierSpawn.h"


ASoldierSpawn::ASoldierSpawn() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}


void ASoldierSpawn::BeginPlay() {
	Super::BeginPlay();
}


void ASoldierSpawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASoldierSpawn, canBeUsed);
}

FVector ASoldierSpawn::GetSpawnLocation()
{
	RespawnLoop = (RespawnLoop + 1) % NbSpawnPosition;
	FVector DirSpawn = GetActorForwardVector();
	float angle = FMath::RadiansToDegrees(FGenericPlatformMath::Acos(FVector::DotProduct({ 1,0,0 }, DirSpawn) / (1 * DirSpawn.Size())));
	if (DirSpawn.Y < 0) angle = -angle;  // regulate the angle signe
	float AnglePerSpawn = 360.f / NbSpawnPosition;

	FVector Offset = FVector{ static_cast<float>(SpawnRadius), 0.f, 0.f };  // take the diameter
	Offset = Offset.RotateAngleAxis(angle, { 0, 0, 1 });
	Offset = Offset.RotateAngleAxis(AnglePerSpawn * RespawnLoop, { 0, 0, 1 });
	return GetActorLocation() + Offset;
}


// Called every frame
void ASoldierSpawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}