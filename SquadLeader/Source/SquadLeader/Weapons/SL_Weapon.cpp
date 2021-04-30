#include "SL_Weapon.h"
#include "../AbilitySystem/Soldiers/AbilitySystemSoldier.h"
#include "../AbilitySystem/Soldiers/GameplayAbilitySoldier.h"
#include "../AbilitySystem/Soldiers/Trace/SL_LineTrace.h"
#include "../AbilitySystem/Soldiers/Trace/SL_SphereTrace.h"
#include "Net/UnrealNetwork.h"
#include "../Soldiers/Soldier.h"
#include "../Soldiers/Players/SoldierPlayer.h"
#include "../Soldiers/Players/SoldierPlayerController.h"
#include "../UI/SL_HUD.h"

ASL_Weapon::ASL_Weapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // TODO: Tweak it

	// Mesh
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));

	// Tags
	WeaponAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Skill.FireWeapon");
	WeaponIsFiringTag = FGameplayTag::RequestGameplayTag("Weapon.IsFiring");
	FireMode = FGameplayTag::RequestGameplayTag("Weapon.FireMode.Automatic");
}

void ASL_Weapon::BeginPlay()
{
	Super::BeginPlay();

	ForceUpdateAmmo();
}

void ASL_Weapon::EndPlay(EEndPlayReason::Type _EndPlayReason)
{
	if (LineTraceTargetActor)
		LineTraceTargetActor->Destroy();

	if (SphereTraceTargetActor)
		SphereTraceTargetActor->Destroy();

	Super::EndPlay(_EndPlayReason);
}

void ASL_Weapon::Destroyed()
{
	RemoveAbilities();
	Super::Destroyed();
}

void ASL_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASL_Weapon, OwningSoldier, COND_OwnerOnly);

	DOREPLIFETIME(ASL_Weapon, TimeBetweenShots);
	DOREPLIFETIME(ASL_Weapon, CurrentAmmo);
	DOREPLIFETIME(ASL_Weapon, MaxAmmo);
}

void ASL_Weapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
}

void ASL_Weapon::SetOwningSoldier(ASoldier* _InOwningCharacter)
{
	OwningSoldier = _InOwningCharacter;

	AbilitySystemComponent = OwningSoldier ? Cast<UAbilitySystemSoldier>(OwningSoldier->GetAbilitySystemComponent()) : nullptr;
	SetOwner(OwningSoldier);
	SetInstigator(OwningSoldier);
}

USkeletalMeshComponent* ASL_Weapon::GetWeaponMesh() const
{
	return WeaponMesh;
}

FName ASL_Weapon::GetMuzzleAttachPoint() const
{
	return MuzzleAttachPoint;
}

FName ASL_Weapon::GetRightHandAttachPoint() const
{
	return RightHandAttachPoint;
}

FName ASL_Weapon::GetLeftHandAttachPoint() const
{
	return LeftHandAttachPoint;
}

void ASL_Weapon::ResetWeapon()
{
	// Force HUD Update
	SetMaxAmmo(MaxAmmo);
	SetCurrentAmmo(MaxAmmo);
}

void ASL_Weapon::ForceUpdateAmmo()
{
	SetMaxAmmo(MaxAmmo);
	SetCurrentAmmo(CurrentAmmo);
}

UAbilitySystemComponent* ASL_Weapon::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASL_Weapon::AddAbilities()
{
	if (!OwningSoldier || GetLocalRole() != ROLE_Authority)
		return;

	UAbilitySystemSoldier* ASC = Cast<UAbilitySystemSoldier>(OwningSoldier->GetAbilitySystemComponent());
	
	if (!ASC)
		return;

	for (TSubclassOf<UGameplayAbilitySoldier>& Ability : Abilities)
	{
		if (!Ability)// Empty element from blueprint
		{
			UE_LOG(LogTemp, Error, TEXT("%s() Invalid granted ability in weapon. Check the blueprint of the weapon"), *FString(__FUNCTION__));
			continue;
		}
		AbilitySpecHandles.Add(ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, GetAbilityLevel(Ability.GetDefaultObject()->AbilityID), static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this)));
	}
}

void ASL_Weapon::RemoveAbilities()
{
	if (!IsValid(OwningSoldier) || GetLocalRole() != ROLE_Authority)
		return;

	UAbilitySystemSoldier* ASC = Cast<UAbilitySystemSoldier>(OwningSoldier->GetAbilitySystemComponent());

	if (!ASC)
		return;

	for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
		ASC->ClearAbility(SpecHandle);
}

float ASL_Weapon::GetMaxRange() const noexcept
{
	return MaxRange;
}

float ASL_Weapon::GetWeaponDamage() const noexcept
{
	return IsValid(OwningSoldier) ? Damage.GetValueAtLevel(OwningSoldier->GetCharacterLevel()) : 1.f;
}

void ASL_Weapon::SetWeaponDamage(const float _Damage) noexcept
{
	Damage = _Damage;
}

float ASL_Weapon::GetFieldOfViewAim() const noexcept
{
	return FieldOfViewAim;
}

int32 ASL_Weapon::GetAbilityLevel(ESoldierAbilityInputID _AbilityID)
{
	// TODO review this - All abilities for now are level 1
	return 1;
}

FGameplayTag ASL_Weapon::GetFireMode() const noexcept
{
	return FireMode;
}

float ASL_Weapon::GetTimeBetweenShots() const noexcept
{
	return TimeBetweenShots;
}

void ASL_Weapon::SetTimeBetweenShots(const float _Time) noexcept
{
	TimeBetweenShots = _Time;
}

int32 ASL_Weapon::GetCurrentAmmo() const noexcept
{
	return CurrentAmmo;
}

int32 ASL_Weapon::GetMaxAmmo() const noexcept
{
	return MaxAmmo;
}

bool ASL_Weapon::IsFullAmmo() const noexcept
{
	return CurrentAmmo == MaxAmmo;
}

bool ASL_Weapon::HasAmmo() const noexcept
{
	return CurrentAmmo > 0;
}

void ASL_Weapon::SetCurrentAmmo(const int32 _NewAmmo)
{
	CurrentAmmo = _NewAmmo;

	if (ASoldierPlayer* SP = GetOwner<ASoldierPlayer>(); SP)
	{
		if (ASoldierPlayerController* PC = SP->GetController<ASoldierPlayerController>(); PC)
		{
			if (ASL_HUD* HUD = PC->GetHUD<ASL_HUD>(); HUD)
			{
				HUD->OnAmmoChanged(CurrentAmmo);
			}
		}
	}
}

void ASL_Weapon::DecrementAmmo()
{
	if (bInfiniteAmmo)
		return;

	SetCurrentAmmo(CurrentAmmo - 1);

	if (!HasAmmo())
		OnOutOfAmmo();
}

void ASL_Weapon::SetMaxAmmo(const int32 _NewMaxAmmo)
{
	MaxAmmo = _NewMaxAmmo;

	if (ASoldierPlayer* SP = GetOwner<ASoldierPlayer>(); SP)
	{
		if (ASoldierPlayerController* PC = SP->GetController<ASoldierPlayerController>(); PC)
		{
			if (ASL_HUD* HUD = PC->GetHUD<ASL_HUD>(); HUD)
				HUD->OnMaxAmmoChanged(MaxAmmo);
		}
	}
}

void ASL_Weapon::ReloadWeapon()
{
	SetCurrentAmmo(MaxAmmo);
	
	if (GetLocalRole() != ROLE_Authority)
		ServerReloadWeapon();
}

void ASL_Weapon::ServerReloadWeapon_Implementation()
{
	ReloadWeapon();
}

bool ASL_Weapon::ServerReloadWeapon_Validate()
{
	return true;
}

void ASL_Weapon::SetHasInfiniteAmmo(const bool _hasInfiniteAmmo)
{
	bInfiniteAmmo = _hasInfiniteAmmo;
}

bool ASL_Weapon::HasInfiniteAmmo() const noexcept
{
	return bInfiniteAmmo;
}

float ASL_Weapon::GetBaseSpread() const noexcept
{
	return BaseSpread;
}

float ASL_Weapon::GetAimingSpreadMod() const noexcept
{
	return AimingSpreadMod;
}

float ASL_Weapon::GetTargetingSpreadIncrement() const noexcept
{
	return TargetingSpreadIncrement;
}

float ASL_Weapon::GetTargetingSpreadMax() const noexcept
{
	return TargetingSpreadMax;
}

ASL_LineTrace* ASL_Weapon::GetLineTraceTargetActor()
{
	if (LineTraceTargetActor)
		return LineTraceTargetActor;

	LineTraceTargetActor = GetWorld()->SpawnActor<ASL_LineTrace>();
	LineTraceTargetActor->SetOwner(this);
	return LineTraceTargetActor;
}

ASL_SphereTrace* ASL_Weapon::GetSphereTraceTargetActor()
{
	if (SphereTraceTargetActor)
		return SphereTraceTargetActor;

	SphereTraceTargetActor = GetWorld()->SpawnActor<ASL_SphereTrace>();
	SphereTraceTargetActor->SetOwner(this);
	return SphereTraceTargetActor;
}