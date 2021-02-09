#pragma once

#include "Core.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "camera/cameracomponent.h"
#include "AbilitySystemInterface.h"
#include "../../AbilitySystem/Soldiers/AttributeSetSoldier.h"
#include "../../AbilitySystem/Soldiers/AbilitySystemSoldier.h"
#include "../../Weapons/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Soldier.generated.h"

UCLASS()
class SQUADLEADER_API ASoldier : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASoldier();

protected:
	virtual void BeginPlay() override;
	void PossessedBy(AController* _newController) override;
	void OnRep_PlayerState() override;

public:
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

//////////////// Inits
protected:
	void initCameras();
	void initMeshes();
	void initStats();
	void initMovements();
	virtual void initWeapons();

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
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributeEffects;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbilitySoldier>> CharacterDefaultAbilities;

	//UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Abilities")
	//TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;

	UPROPERTY()
	bool bAbilitiesInitialized;

	bool ASCInputBound;
	void SetAbilitySystemComponent();
	virtual void InitializeAttributes();
	void InitializeAbilities();
	void BindASCInput();

//////////////// Attributes
public:
	UPROPERTY(BluePrintReadWrite, Category = "Attributes")
	float fieldOfViewNormal;

	UPROPERTY(BluePrintReadWrite, Category = "Attributes")
	float fieldOfViewAim;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMoveSpeed() const;

//////////////// Cameras
private:
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
	void onMoveForward(const float _val);

	UFUNCTION()
	void onMoveRight(const float _val);

	UFUNCTION(BlueprintCallable, Category = "Sight")
	FVector lookingAtPosition();

//////////////// Weapons
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool wantsToFire = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool GetWantsToFire() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWantsToFire(const bool _want);

	void SetWantsToFire(const bool _want, const FGameplayEffectSpecHandle _damageEffectSpecHandle);

protected:
	// Default inventory
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<class AWeapon>> DefaultWeaponClasses;

	// Current inventory
	UPROPERTY(Transient, Replicated)
	TArray<class AWeapon*> Inventory;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	AWeapon* currentWeapon;

	void addToInventory(AWeapon* _weapon);

	void SetCurrentWeapon(class AWeapon* _newWeapon, class AWeapon* _previousWeapon = nullptr);

	UFUNCTION()
	void OnRep_CurrentWeapon(class AWeapon* _lastWeapon);

public:
	AWeapon* getCurrentWeapon() const noexcept { return currentWeapon; }
};