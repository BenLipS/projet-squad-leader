#include "NavLinkProxy_V1.h"
#include "NavLinkCustomComponent.h"
#include "../../Soldiers/Soldier.h"
#include "Kismet/KismetMathLibrary.h"
#include "SquadLeader/Soldiers/SoldierMovementComponent.h"

ANavLinkProxy_V1::ANavLinkProxy_V1() {
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ANavLinkProxy_V1::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	if(!this->GetSmartLinkComp()->IsEnabled())
		this->GetSmartLinkComp()->SetEnabled(true);
}

void ANavLinkProxy_V1::Jump(AActor* Agent, const FVector& Destination) {
	//if (GEngine)
	//	//GEngine->AddOnScreenDebugMessage(20, 1.f, FColor::Blue, TEXT("Start to jump."));

	this->GetSmartLinkComp()->SetEnabled(false);

	ASoldier* _soldier = Cast<ASoldier>(Agent);
	auto _location = _soldier->GetActorLocation();

	FRotator rotation_actor = UKismetMathLibrary::FindLookAtRotation(_location, Destination);
	rotation_actor.Pitch = 0.f;
	rotation_actor.Roll = 0.f;

	_soldier->SetActorRotation(rotation_actor);
	_soldier->GetCharacterMovement()->Velocity = _soldier->GetActorForwardVector() * _soldier->GetCharacterMovement()->Velocity.Size();
	_soldier->GetCharacterMovement()->JumpZVelocity = JumpHeight;

	_soldier->ActivateAbility(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Jump")));
}