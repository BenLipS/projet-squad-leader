#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "AbilitySystemInterface.h"
#include "../AbilitySystem/Soldiers/AbilitySystemSoldier.h"
#include "Weapon.generated.h"

UCLASS(Abstract)
class SQUADLEADER_API AWeapon : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	AWeapon();

//////////////// Ability System
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System Component", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemSoldier* AbilitySystemComponent;

public:
	void InitializeAbilitySystemComponent(UAbilitySystemSoldier* _AbilitySystemComponent);
	UAbilitySystemSoldier* GetAbilitySystemComponent() const;

//////////////// Fire
protected:
	// TODO: Use cooldown from ability system ?
	FTimerHandle TimerReloadNextShoot;
	FTimerHandle TimerReloadAmmo;

	UPROPERTY(EditDefaultsOnly, BluePrintReadWrite, Category = "Stats")
	uint8 MaxAmmo;

	UPROPERTY(BluePrintReadOnly, Category = "Stats")
	uint8 CurrentAmmo;

	bool IsNextFireReady;

	UPROPERTY(EditDefaultsOnly, BluePrintReadWrite, Category = "Stats")
	float TimeToReloadAmmo;

	UPROPERTY(EditDefaultsOnly, BluePrintReadWrite, Category = "Stats")
	float TimeToReloadNextShoot;

	UPROPERTY(EditDefaultsOnly, BluePrintReadWrite, Category = "Stats")
	bool IsAutomatic;

	UPROPERTY(EditDefaultsOnly, BluePrintReadWrite, Category = "Stats")
	int Penetration;

	UPROPERTY(EditDefaultsOnly, BluePrintReadWrite, Category = "Stats")
	float FieldOfViewAim;

public:
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetFieldOfViewAim() const;

protected:
	UPROPERTY(BluePrintReadWrite, EditAnywhere, Category = "Stats")
	float Damage;

	// Damage impact
	UPROPERTY(BlueprintReadWrite, Category = "Fire | Impact")
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UFUNCTION(BlueprintCallable, Category = "Fire | Impact")
	virtual void ApplyImpactDamage(UAbilitySystemComponent* _TargetASC);

	// Additional impact effects
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Fire | Impact")
	TArray<TSubclassOf<class UGameplayEffect>> ImpactEffects;

	UFUNCTION(BlueprintCallable, Category = "Fire | Impact")
	virtual void ApplyImpactEffects(UAbilitySystemComponent* _TargetASC);

public:
	UFUNCTION(BlueprintCallable, Category = "Fire")
	virtual void TryFiring();
	void TryFiring(const FGameplayEffectSpecHandle _DamageEffectSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Fire")
	virtual void Fire();

	virtual void BeginPlay() override;

	virtual void Reload();

protected:
	virtual void OnReadyToShoot();
	virtual void OnReloaded();
};