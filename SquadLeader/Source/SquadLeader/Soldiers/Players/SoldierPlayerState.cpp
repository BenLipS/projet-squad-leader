#include "SoldierPlayerState.h"
#include "SoldierPlayerController.h"
#include "SquadLeader/UI/HUDWidget.h"

ASoldierPlayerState::ASoldierPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemSoldier>(TEXT("Ability System Component"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UAttributeSetSoldier>(TEXT("Attribute Set"));
}

void ASoldierPlayerState::BeginPlay()
{
	Super::BeginPlay();
	InitializeAttributeChangeCallbacks();
}

void ASoldierPlayerState::InitializeAttributeChangeCallbacks()
{
	if (AbilitySystemComponent)
	{
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute()).AddUObject(this, &ASoldierPlayerState::HealthChanged);
		MaxHealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &ASoldierPlayerState::MaxHealthChanged);
		ShieldChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetShieldAttribute()).AddUObject(this, &ASoldierPlayerState::ShieldChanged);
		MaxShieldChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetShieldAttribute()).AddUObject(this, &ASoldierPlayerState::MaxShieldChanged);
	}
}

UAbilitySystemSoldier* ASoldierPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAttributeSetSoldier* ASoldierPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

float ASoldierPlayerState::GetHealth() const
{
	return AttributeSet->GetHealth();
}

float ASoldierPlayerState::GetMaxHealth() const
{
	return AttributeSet->GetMaxHealth();
}

float ASoldierPlayerState::GetShield() const
{
	return AttributeSet->GetShield();
}

float ASoldierPlayerState::GetMaxShield() const
{
	return AttributeSet->GetMaxShield();
}

void ASoldierPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	if (ASoldierPlayerController* PC = Cast<ASoldierPlayerController>(GetOwner()); PC)
	{
		if (UHUDWidget* HUD = PC->GetHUD(); HUD)
			HUD->SetHealth(Data.NewValue);
	}
}

void ASoldierPlayerState::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	if (ASoldierPlayerController* PC = Cast<ASoldierPlayerController>(GetOwner()); PC)
	{
		if (UHUDWidget* HUD = PC->GetHUD(); HUD)
			HUD->SetMaxHealth(Data.NewValue);
	}
}

void ASoldierPlayerState::ShieldChanged(const FOnAttributeChangeData& Data)
{
	if (ASoldierPlayerController* PC = Cast<ASoldierPlayerController>(GetOwner()); PC)
	{
		if (UHUDWidget* HUD = PC->GetHUD(); HUD)
			HUD->SetShield(Data.NewValue);
	}
}

void ASoldierPlayerState::MaxShieldChanged(const FOnAttributeChangeData& Data)
{
	if (ASoldierPlayerController* PC = Cast<ASoldierPlayerController>(GetOwner()); PC)
	{
		if (UHUDWidget* HUD = PC->GetHUD(); HUD)
			HUD->SetMaxShield(Data.NewValue);
	}
}