#include "GA_WallVision.h"
#include "../../../Soldiers/Players/SoldierPlayer.h"
#include "../../../Soldiers/Players/SoldierPlayerController.h"
#include "SquadLeader/AbilitySystem/Soldiers/GameplayEffects/States/GE_StateCastingSpell.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "SquadLeader/AbilitySystem/Soldiers/AbilityTasks/AbilityTask_PlayMontageAndWaitForEvent.h"
#include "GameFramework/Actor.h"
#include "AkAudioEvent.h"
#include "AkGameplayStatics.h"

UGA_WallVision::UGA_WallVision()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.WallVision")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.CastingSpell")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.ReloadWeapon")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.FireWeapon")));
	WallVisionReadyEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.WallVisionReady"));
}

bool UGA_WallVision::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;

	if (ASoldier* Soldier = Cast<ASoldier>(ActorInfo->AvatarActor.Get()); Soldier)
	{
		if (ASoldierPlayerController* PC = Soldier->GetController<ASoldierPlayerController>(); PC)
			return true;
	}
	return false;
}

void UGA_WallVision::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);

		SourceSoldier = Cast<ASoldierPlayer>(ActorInfo->AvatarActor);

		if (UAnimMontage* CastSpellMontage = SourceSoldier->CastSpellMontage; CastSpellMontage)
		{
			SourceSoldier->UseCurrentWeaponWithLeftHand(); // Because the animation use the right hand

			FGameplayTagContainer Tags;
			Tags.AddTag(WallVisionReadyEventTag);

			UAbilityTask_PlayMontageAndWaitForEvent* TaskPlayMontage = UAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(this, NAME_None, CastSpellMontage, Tags);

			TaskPlayMontage->OnCompleted.AddDynamic(this, &UGA_WallVision::MontageCompletedOrBlendedOut);
			TaskPlayMontage->OnBlendOut.AddDynamic(this, &UGA_WallVision::MontageCompletedOrBlendedOut);
			TaskPlayMontage->OnInterrupted.AddDynamic(this, &UGA_WallVision::MontageInterruptedOrCancelled);
			TaskPlayMontage->OnCancelled.AddDynamic(this, &UGA_WallVision::MontageInterruptedOrCancelled);
			TaskPlayMontage->OnEventReceived.AddDynamic(this, &UGA_WallVision::MontageSentEvent);
			TaskPlayMontage->ReadyForActivation();

			// State during the activation
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(UGE_StateCastingSpell::StaticClass(), 1.f, EffectContext);
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_WallVision::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ASoldierPlayerController* PC = SourceSoldier->GetController<ASoldierPlayerController>(); PC)
	{
		PC->OnWallVisionDeactivate();
		SourceSoldier->UpdateWallVisionPostEffect(0.f);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_WallVision::MontageCompletedOrBlendedOut()
{
	SourceSoldier->UseCurrentWeaponWithRightHand();

	FGameplayTagContainer EffectTagsToRemove;
	EffectTagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("State.CastingSpell")));
	GetAbilitySystemComponentFromActorInfo()->RemoveActiveEffectsWithGrantedTags(EffectTagsToRemove);
}

void UGA_WallVision::MontageInterruptedOrCancelled()
{
	SourceSoldier->UseCurrentWeaponWithRightHand();
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

	FGameplayTagContainer EffectTagsToRemove;
	EffectTagsToRemove.AddTag(FGameplayTag::RequestGameplayTag(FName("State.CastingSpell")));
	GetAbilitySystemComponentFromActorInfo()->RemoveActiveEffectsWithGrantedTags(EffectTagsToRemove);
}

void UGA_WallVision::MontageSentEvent(FGameplayTag _EventTag, FGameplayEventData _EventData)
{
	if (_EventTag == WallVisionReadyEventTag)
		StartWallVision();
}

void UGA_WallVision::StartWallVision()
{
	if (ASoldierPlayerController* PC = SourceSoldier->GetController<ASoldierPlayerController>(); PC)
	{
		PC->OnWallVisionActivate();
		UAbilityTask_WaitDelay* TaskWaitDelay = UAbilityTask_WaitDelay::WaitDelay(this, DurationVision);
		TaskWaitDelay->OnFinish.AddDynamic(this, &UGA_WallVision::EndWallVision);
		SourceSoldier->UpdateWallVisionPostEffect(1.f);
		TaskWaitDelay->ReadyForActivation();
		UAkGameplayStatics::PostEventByName("Ability_Termal_Vision", SourceSoldier);
	}
	else
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UGA_WallVision::EndWallVision()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}