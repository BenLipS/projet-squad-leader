#pragma once

#include "Core.h"
#include "UObject/ObjectMacros.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "AttributeSetSoldier.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class SQUADLEADER_API UAttributeSetSoldier : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAttributeSetSoldier(const FObjectInitializer& _ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Add the same value to attribute when its max has changed
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, const float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	// Character level
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Character Level", ReplicatedUsing = OnRep_CharacterLevel)
	FGameplayAttributeData CharacterLevel = 1.f;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, CharacterLevel)

	UPROPERTY(BlueprintReadOnly)
	float MaxLevel = 10.f;

	// EXP
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|EXP", ReplicatedUsing = OnRep_EXP)
	FGameplayAttributeData EXP = 0.f;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, EXP)

	// EXPLevelUp
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|EXP", ReplicatedUsing = OnRep_EXPLevelUp)
	FGameplayAttributeData EXPLevelUp;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, EXPLevelUp)

	UFUNCTION()
	float GetRemainEXPForLevelUp() const;

protected:
	// Accumulated EXP from the previous levels
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|EXP", Replicated)
	float EXPAccumulated;

public:
	UFUNCTION()
	float GetEXPAccumulated() const;

	UFUNCTION()
	void LevelUp();

	UFUNCTION()
	void GrantEXP(const float _EXP);

public: // TODO: Should the attributes be public ?
	// Damage is a meta attribute used by the DamageExecution to calculate final damage, which then turns into -Health
	// Temporary value that only exists on the Server. Not replicated.
	UPROPERTY(BlueprintReadOnly, Category = "Damage", meta = (HideFromLevelInfos))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, Damage)

	// Health
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, Health)

	// Max health
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|MaxHealth", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, MaxHealth)

	// Health regen
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|HealthRegenRate", ReplicatedUsing = OnRep_HealthRegenRate)
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, HealthRegenRate)

	// Shield
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, Shield)

	// Max shield
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|MaxShield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, MaxShield)

	// Move Speed - Walk
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|MoveSpeedWalk", ReplicatedUsing = OnRep_MoveSpeedWalk)
	FGameplayAttributeData MoveSpeedWalk;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, MoveSpeedWalk)

	// Move Speed - Crouch
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|MoveSpeedCrouch", ReplicatedUsing = OnRep_MoveSpeedCrouch)
	FGameplayAttributeData MoveSpeedCrouch;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, MoveSpeedCrouch)

	// Move Speed - Multiplier (for buffs and debuffs)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|MoveSpeedMultiplier", ReplicatedUsing = OnRep_MoveSpeedMultiplier)
	FGameplayAttributeData MoveSpeedMultiplier = 1.f;
	ATTRIBUTE_ACCESSORS(UAttributeSetSoldier, MoveSpeedMultiplier)

protected:
	UFUNCTION()
	virtual void OnRep_CharacterLevel(const FGameplayAttributeData& _OldCharacterLevel);
	
	UFUNCTION()
	virtual void OnRep_EXP(const FGameplayAttributeData& _OldEXP);
	
	UFUNCTION()
	virtual void OnRep_EXPLevelUp(const FGameplayAttributeData& _OldEXPLevelUp);

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& _OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& _OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Shield(const FGameplayAttributeData& _OldShield);

	UFUNCTION()
	virtual void OnRep_MaxShield(const FGameplayAttributeData& _OldMaxShield);

	UFUNCTION()
	virtual void OnRep_HealthRegenRate(const FGameplayAttributeData& _OldHealthRegenRate);

	UFUNCTION()
	virtual void OnRep_MoveSpeedWalk(const FGameplayAttributeData& _OldMoveSpeedWalk);

	UFUNCTION()
	virtual void OnRep_MoveSpeedCrouch(const FGameplayAttributeData& _OldMoveSpeedCrouch);

	UFUNCTION()
	virtual void OnRep_MoveSpeedMultiplier(const FGameplayAttributeData& _OldMoveSpeedMultiplier);
};
