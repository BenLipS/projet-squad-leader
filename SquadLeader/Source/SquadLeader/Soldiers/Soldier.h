﻿#pragma once

#include "Core.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "camera/cameracomponent.h"
#include "AbilitySystemInterface.h"
#include "../AbilitySystem/Soldiers/AttributeSetSoldier.h"
#include "../AbilitySystem/Soldiers/AbilitySystemSoldier.h"
#include "Interface/Teamable.h"
//
#include "SoldierTeam.h"
//
#include "Net/UnrealNetwork.h"
#include "Soldier.generated.h"

class ASL_Weapon;

USTRUCT()
struct SQUADLEADER_API FSoldier_Inventory
{
	GENERATED_USTRUCT_BODY()

	FSoldier_Inventory() = default;

	UPROPERTY()
	TArray<ASL_Weapon*> Weapons;

	// Consumable items ?
	// Grenade ?
};

UCLASS()
class SQUADLEADER_API ASoldier : public ACharacter, public IAbilitySystemInterface, public ITeamable
{
	GENERATED_BODY()

public:
	ASoldier(const FObjectInitializer& _ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

//////////////// Inits
protected:
	void InitCameras();
	void InitMeshes();
	void InitMovements();

//////////////// Resets
	void ResetWeapons();

//////////////// Controllers
protected:
	// Lock any interraction with this soldier
	UFUNCTION()
	virtual void LockControls();

	// Unlock interractions with this soldier
	UFUNCTION()
	virtual void UnLockControls();

//////////////// Ability System
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System Component", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemSoldier* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute Set", meta = (AllowPrivateAccess = "true"))
	UAttributeSetSoldier* AttributeSet;

public:
	UAbilitySystemSoldier* GetAbilitySystemComponent() const override;
	UAttributeSetSoldier* GetAttributeSet() const;

protected:
	// Define the default stats
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributeEffects;

	// Define the default abilities
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbilitySoldier>> CharacterDefaultAbilities;

	// Additional applied effect (for instance hp regen)
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	UPROPERTY()
	bool bAbilitiesInitialized;	
	virtual void InitializeAttributes();
	void InitializeAbilities();
	void AddStartupEffects();
	void InitializeTagChangeCallbacks();
	virtual void InitializeAttributeChangeCallbacks();

//////////////// Tag Change Callbacks
public:
	// States
	static FGameplayTag StateDeadTag;
	static FGameplayTag StateRunningTag;
	static FGameplayTag StateJumpingTag;
	static FGameplayTag StateCrouchingTag;
	static FGameplayTag StateFightingTag;
	static FGameplayTag StateAimingTag;
	static FGameplayTag StateGivingOrderTag;
	static FGameplayTag StateFiringTag;
	static FGameplayTag StateReloadingWeaponTag;
	static FGameplayTag StateDashingTag;

	// Abilities
	static FGameplayTag SkillRunTag;
	static FGameplayTag SkillJumpTag;
	static FGameplayTag SkillCrouchTag;
	static FGameplayTag SkillFireWeaponTag;
	static FGameplayTag SkillGrenadeTag;
	static FGameplayTag SkillAimTag;
	static FGameplayTag SkillAreaEffectFromSelfTag;
	static FGameplayTag SkillGiveOrderTag;
	static FGameplayTag SkillReloadWeaponTag;
	static FGameplayTag SkillQuickDashTag;

	// Weapon
	static FGameplayTag NoWeaponTag;
	FGameplayTag CurrentWeaponTag;

protected:
	virtual void DeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void RunningTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void JumpingTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void FightingTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void GivingOrderTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void FiringTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void ReloadingWeaponTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	virtual void DashingTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

public:
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateAbilities(const FGameplayTagContainer& _TagContainer);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateAbility(const FGameplayTag& _Tag);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void CancelAbilities(const FGameplayTagContainer& _TagContainer);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void CancelAbility(const FGameplayTag& _Tag);

//////////////// Attributes
public:
	UPROPERTY(BluePrintReadWrite, Category = "Attributes")
	uint8 InfluenceRadius = 2;

	UPROPERTY(BluePrintReadWrite, Category = "Attributes")
	float InfluenceWeight = 0.5f;

	// Getters
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMoveSpeedWalk() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMoveSpeedCrouch() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMoveSpeedMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	FVector GetLocation() const noexcept;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	uint8 GetInfluenceRadius() const noexcept;

	// Attribute changed callbacks
	FDelegateHandle HealthChangedDelegateHandle;
	virtual void HealthChanged(const FOnAttributeChangeData& _Data);

	virtual void Die();
	virtual void Respawn();

//////////////// Cameras
public:
	void setToFirstCameraPerson();
	void setToThirdCameraPerson();

// TODO: Change to protected and use getters/setters
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CurrentCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* ThirdPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComponent;

protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	FRotator SyncControlRotation;

public:
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetSyncControlRotation() const noexcept;

protected:
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSyncControlRotation(const FRotator& _Rotation);
	void ServerSyncControlRotation_Implementation(const FRotator& _Rotation);
	bool ServerSyncControlRotation_Validate(const FRotator& _Rotation);

	UFUNCTION(Reliable, NetMulticast, WithValidation)
	void MulticastSyncControlRotation(const FRotator& _Rotation);
	void MulticastSyncControlRotation_Implementation(const FRotator& _Rotation);
	bool MulticastSyncControlRotation_Validate(const FRotator& _Rotation);

protected:
	UPROPERTY(VisibleAnywhere, BluePrintReadWrite, Category = "Camera")
	bool bIsFirstPerson;

public:
	UFUNCTION()
	void onSwitchCamera();

////////////////  Meshes
public:
	UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
	USkeletalMeshComponent* FirstPersonMesh;

//////////////// Movement
	// Move direction
	UFUNCTION()
	void MoveForward(const float _Val);

	UFUNCTION()
	void MoveRight(const float _Val);

	// Looking direction
	UFUNCTION()
	virtual void LookUp(const float _Val);

	UFUNCTION()
	virtual void Turn(const float _Val);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual FVector GetLookingAtPosition();

	// Run
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool StartRunning();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopRunning();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool Walk();

	virtual void Landed(const FHitResult& _Hit) override;

//////////////// Inventory
protected:
	void SpawnDefaultInventory();
	
	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	FSoldier_Inventory Inventory;

	UFUNCTION()
	void OnRep_Inventory();

//////////////// Weapons
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Inventory | Weapon")
	TArray<TSubclassOf<ASL_Weapon>> DefaultInventoryWeaponClasses;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	ASL_Weapon* CurrentWeapon;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory | Weapon")
	ASL_Weapon* GetCurrentWeapon() const;

	bool bChangedWeaponLocally;

	// Adds a new weapon to the inventory.
	// Returns false if the weapon already exists in the inventory, true if it's a new weapon.
	UFUNCTION(BlueprintCallable, Category = "Inventory | Weapon")
	bool AddWeaponToInventory(ASL_Weapon* _NewWeapon, const bool _bEquipWeapon = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory | Weapon")
	void EquipWeapon(ASL_Weapon* _NewWeapon);

	UFUNCTION(Server, Reliable, Category = "Inventory | Weapon")
	void ServerEquipWeapon(ASL_Weapon* _NewWeapon);
	bool ServerEquipWeapon_Validate(ASL_Weapon* _NewWeapon);
	void ServerEquipWeapon_Implementation(ASL_Weapon* _NewWeapon);

protected:
	bool DoesWeaponExistInInventory(ASL_Weapon* _Weapon);

	void SetCurrentWeapon(ASL_Weapon* _NewWeapon, ASL_Weapon* _LastWeapon);

	// Unequips the specified weapon. Used when OnRep_CurrentWeapon fires.
	void UnEquipWeapon(ASL_Weapon* WeaponToUnEquip);

	UFUNCTION()
	void OnRep_CurrentWeapon(ASL_Weapon* _LastWeapon);

	// The CurrentWeapon is only automatically replicated to simulated clients.
	// The autonomous client can use this to request the proper CurrentWeapon from the server when it knows it may be
	// out of sync with it from predictive client-side changes.
	UFUNCTION(Server, Reliable, Category = "Inventory | Weapon")
	void ServerSyncCurrentWeapon();
	void ServerSyncCurrentWeapon_Implementation();
	bool ServerSyncCurrentWeapon_Validate();

	// The CurrentWeapon is only automatically replicated to simulated clients.
	// Use this function to manually sync the autonomous client's CurrentWeapon when we're ready to.
	// This allows us to predict weapon changes (changing weapons fast multiple times in a row so that the server doesn't replicate and clobber our CurrentWeapon).
	UFUNCTION(Client, Reliable, Category = "Inventory | Weapon")
	void ClientSyncCurrentWeapon(ASL_Weapon* _InWeapon);
	void ClientSyncCurrentWeapon_Implementation(ASL_Weapon* _InWeapon);
	bool ClientSyncCurrentWeapon_Validate(ASL_Weapon* _InWeapon);

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Stats")
	float FieldOfViewNormal;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartAiming();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopAiming();

//////////////// Soldier team
	UPROPERTY(EditAnywhere, Category = "PlayerTeam")
	ASoldierTeam* InitialTeam;  // for debug use

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCycleBetweenTeam();

	// Connected to the "L" key
	virtual void cycleBetweenTeam();

//////////////// Teamable
	virtual ASoldierTeam* GetTeam() override { return nullptr; };  // function overide in SoldierPlayer and Soldier AI
	virtual bool SetTeam(ASoldierTeam* _Team) override { return false; };  // function overide in SoldierPlayer and Soldier AI

/////////////// Respawn
public:
	UFUNCTION()
	virtual FVector GetRespawnPoint() { return FVector(0.f, 0.f, 1500.f); }  // function overide in SoldierPlayer and Soldier AI

//////////////// For AIPerception
private:
	class UAIPerceptionStimuliSourceComponent* stimulus;

	void setup_stimulus();

//////////////// Particles
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Animation | Particles")
	UParticleSystem* ImpactHitFX;

	UPROPERTY(EditDefaultsOnly, Category = "Animation | Particles")
	FVector ImpactHitFXScale;

//////////////// Montages
public:
	// Anim Montage
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation | Montages")
	UAnimMontage* StartGameMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation | Montages")
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation | Montages")
	UAnimMontage* RespawnMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation | Montages")
	UAnimMontage* WeaponFireMontage;

protected:
	// Callbacks
	FOnMontageEnded StartGame_SoldierMontageEndedDelegate;
	FOnMontageEnded Respawn_SoldierMontageEndedDelegate;

	UFUNCTION()
	virtual void OnStartGameMontageCompleted(UAnimMontage* _Montage, bool _bInterrupted);

	UFUNCTION()
	virtual void OnRespawnMontageCompleted(UAnimMontage* _Montage, bool _bInterrupted);

public:
	UFUNCTION()
	void ShowImpactHitEffect();
};