#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "../../SquadLeader.h"
#include "GameplayAbilitySoldier.generated.h"

UCLASS()
class SQUADLEADER_API UGameplayAbilitySoldier : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbilitySoldier();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle _Handle, const FGameplayAbilityActorInfo* _OwnerInfo, const FGameplayAbilityActivationInfo _ActivationInfo, const FGameplayEventData* _TriggerEventData) override;

	// Abilities with input
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ESoldierAbilityInputID AbilityInputID = ESoldierAbilityInputID::None;

	// Abilities with no input - like passives
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ESoldierAbilityInputID AbilityID = ESoldierAbilityInputID::None;

	// Immediately activate ability when its granted. For passive abilities and abilities forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool ActivateAbilityOnGranted = false;

	// Attempts to activate the given ability handle and batch all RPCs into one. This will only batch all RPCs that happen
	// in one frame. Best case scenario we batch ActivateAbility, SendTargetData, and EndAbility into one RPC instead of three.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle _InAbilityHandle, const bool _EndAbilityImmediately);

	// Same as calling K2_EndAbility. Meant for use with batching system to end the ability externally.
	virtual void ExternalEndAbility();

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

//////////////// Cooldown
protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FScalableFloat CooldownDuration;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Cooldowns")
	FGameplayTagContainer CooldownTags;

	// Temp container that we will return the pointer in GetCooldownTags()
	UPROPERTY()
	FGameplayTagContainer TempCooldownTags;

public:
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;
};
