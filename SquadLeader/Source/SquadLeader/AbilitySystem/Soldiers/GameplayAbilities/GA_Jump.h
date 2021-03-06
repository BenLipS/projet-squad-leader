#pragma once

#include "CoreMinimal.h"
#include "../GameplayAbilitySoldier.h"
#include "../GameplayEffects/States/GE_StateJumping.h"
#include "GA_Jump.generated.h"

UCLASS()
class SQUADLEADER_API UGA_Jump : public UGameplayAbilitySoldier
{
	GENERATED_BODY()
	
public:
	UGA_Jump();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* OwnerInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGE_StateJumping> JumpingGameplayEffect;
};
