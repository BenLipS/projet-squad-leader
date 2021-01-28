#include "Soldier.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"

ASoldier::ASoldier()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	initCameras();
	initMeshes();
}

void ASoldier::BeginPlay()
{
	Super::BeginPlay();

	if (bIsFirstPerson)
		setToFirstCameraPerson();
	else
		setToThirdCameraPerson();
}

void ASoldier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASoldier::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("MoveForward", this, &ASoldier::onMoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASoldier::MoveRight);

	InputComponent->BindAxis("Turn", this, &ASoldier::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &ASoldier::AddControllerPitchInput);
}

void ASoldier::initCameras()
{
	// 1st person camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(2.f, 0.f, BaseEyeHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetFieldOfView(fieldOfViewNormal);

	// 3rd person camera
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->TargetArmLength = 170;
	SpringArmComponent->SetRelativeLocation(FVector(-30.f, 40.f, BaseEyeHeight));
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->bEnableCameraRotationLag = true;
	SpringArmComponent->CameraLagSpeed = 10.0f;
	SpringArmComponent->CameraRotationLagSpeed = 10.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(SpringArmComponent);
	ThirdPersonCameraComponent->SetFieldOfView(fieldOfViewNormal);

	bIsFirstPerson = true;
}

void ASoldier::initMeshes()
{
	// 1st person mesh
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetOnlyOwnerSee(true); // Only the player sees it
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->bCastDynamicShadow = false;
	FirstPersonMesh->CastShadow = false;

	// 3rd person mesh - already defined with ACharacter
}

void ASoldier::initStats()
{
	fieldOfViewNormal = 90.f;
	fieldOfViewAim = 50.f;
}

void ASoldier::OnSwitchCamera()
{
	if (bIsFirstPerson)
		setToThirdCameraPerson();
	else
		setToFirstCameraPerson();
}

void ASoldier::setToFirstCameraPerson()
{
	ThirdPersonCameraComponent->Deactivate();
	GetMesh()->SetOwnerNoSee(true);
	FirstPersonCameraComponent->Activate();
	FirstPersonMesh->SetOwnerNoSee(false);

	bIsFirstPerson = true;
}

void ASoldier::setToThirdCameraPerson()
{
	FirstPersonCameraComponent->Deactivate();
	FirstPersonMesh->SetOwnerNoSee(true);
	ThirdPersonCameraComponent->Activate();
	GetMesh()->SetOwnerNoSee(false);

	bIsFirstPerson = false;
}

void ASoldier::onMoveForward(const float _val)
{
	if ((Controller != NULL) && (_val != 0.0f))
	{
		FRotator Rotation = Controller->GetControlRotation();

		// Ignore pitch
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
			Rotation.Pitch = 0.0f;

		AddMovementInput(FRotationMatrix(Rotation).GetScaledAxis(EAxis::X), _val);
	}
}

void ASoldier::MoveRight(const float _val) {
	if ((Controller != NULL) && (_val != 0.0f))
	{
		FRotator Rotation = Controller->GetControlRotation();
		AddMovementInput(FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y), _val);
	}
}