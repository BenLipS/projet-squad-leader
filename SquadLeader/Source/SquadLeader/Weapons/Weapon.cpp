#include "Weapon.h"
#include "../Characters/Soldiers/Soldier.h"

AWeapon::AWeapon() : isNextFireReady{ true }, timeToReloadNextShoot{ 0.2f }, isAutomatic{ true }, penetration{ 1 }
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AWeapon::tryFiring()
{
	ASoldier* soldier = Cast<ASoldier>(GetOwner());
	if (soldier && soldier->GetWantsToFire() && isNextFireReady)
		fire();
}

void AWeapon::tryFiring(const FGameplayEffectSpecHandle _damageEffectSpecHandle)
{
	DamageEffectSpecHandle = _damageEffectSpecHandle;
	DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
	tryFiring();
}

void AWeapon::fire()
{
	isNextFireReady = false;
	GetWorldTimerManager().SetTimer(timerReloadNextShoot, this, &AWeapon::onReadyToShoot, timeToReloadNextShoot, false);
}

void AWeapon::onReadyToShoot()
{
	isNextFireReady = true;
	if (isAutomatic)
		tryFiring();
}