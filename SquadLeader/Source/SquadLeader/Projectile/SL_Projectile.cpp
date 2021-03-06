#include "SL_Projectile.h"
#include "Components/SPhereComponent.h"
#include "../AreaEffect/AreaEffect.h"
#include "../Soldiers/Soldier.h"
#include "../Soldiers/AIs/SoldierAI.h"
#include "../AI/AIClassesController/AssaultAIBasicController.h"
#include "../AI/AIClassesController/AssaultAISquadController.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASL_Projectile::ASL_Projectile() : CollisionProfileNameMesh{ FName{"BlockAllDynamic"} }
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	CollisionComp->bShouldCollideWhenPlacing = false;

	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void ASL_Projectile::BeginPlay()
{
	Super::BeginPlay();

	if (OnContactPolicy == ContactPolicy::STICKY) {
		//CollisionComp->SetCollisionResponseToAllChannel(ECC_EngineTraceChannel1, ECR_Overlap);
		//CollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
	CollisionComp->InitSphereRadius(Radius); //physic collision radius

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ASL_Projectile::OnOverlapBegin);

	if (OnContactPolicy == ContactPolicy::EXPLODE || OnContactPolicy == ContactPolicy::STICKY)
		CollisionComp->OnComponentHit.AddDynamic(this, &ASL_Projectile::OnHit);

	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = OnContactPolicy == ContactPolicy::BOUNCE;
	ProjectileMovement->Bounciness = Bounciness;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	SetCollisionProfile(CollisionProfileNameMesh);

	InitVelocity();

	//if (auto temp = Cast<USphereComponent>(RootComponent))
	//	temp->IgnoreActorWhenMoving(GetOwner(), true);

	if (ExplosionDelay > 0.0f)
		GetWorldTimerManager().SetTimer(TimerExplosion, this, &ASL_Projectile::OnEndOfDelay, ExplosionDelay, true);
}

void ASL_Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASL_Projectile, CollisionProfileNameMesh);
}

USkeletalMeshComponent* ASL_Projectile::GetMesh() const
{
	return Mesh;
}

void ASL_Projectile::SetCollisionProfile(const FName& _Name)
{
	CollisionProfileNameMesh = _Name;
	CollisionComp->SetCollisionProfileName(CollisionProfileNameMesh);
	if (Mesh)
		Mesh->SetCollisionProfileName(CollisionProfileNameMesh);
}

FName ASL_Projectile::GetCollisionProfile() const
{
	return CollisionProfileNameMesh;
}

void ASL_Projectile::OnEndOfDelay()
{
	OnExplode();
}

void ASL_Projectile::OnExplode()
{
	ExplodeSoundEvent();
	for (auto AreaEffectClass : AreaEffectList)
	{
		AAreaEffect* AreaEffect = GetWorld()->SpawnActorDeferred<AAreaEffect>(AreaEffectClass, FTransform{ GetActorRotation(), GetActorLocation() }, this, GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (AreaEffect)
		{
#ifdef UE_BUILD_DEBUG
			AreaEffect->bDebugTrace = bDebugTraceExplosion;
#endif
			AreaEffect->bIgnoreBlock = false;
			AreaEffect->FinishSpawning(FTransform{ GetActorRotation(), GetActorLocation() });
		}

	}
	Destroy();
}

void ASL_Projectile::OnStick()
{
}

void ASL_Projectile::InitVelocity()
{
	FVector ForwardVector;
	ASoldier* soldier = Cast<ASoldier>(GetOwner());

	if (soldier) {
		ForwardVector = soldier->GetLookingDirection();
	}
	else if (GetOwner()) {
		ForwardVector = GetOwner()->GetActorForwardVector();
	}
	//Fixing radius to 1
	ForwardVector.Normalize();

	float Azimuth = UKismetMathLibrary::DegAcos(ForwardVector.Z);
	float Inclination = UKismetMathLibrary::DegAtan2(ForwardVector.Y, ForwardVector.X);

	if (ASoldierAI* AISoldier = Cast<ASoldierAI>(GetOwner()); AISoldier) {
		if (AAIGeneralController* AI = Cast<AAIGeneralController>(Cast<ASoldierAI>(GetOwner())->Controller); AI) {
			PitchAdjust = AI->LaunchProjectilePitchAdjust;
			YawAdjust = AI->LaunchProjectileYawAdjust;
		}
	}

	Azimuth += PitchAdjust;
	if (Azimuth > 180.f)
	{
		Azimuth = 180.f;
	}
	else if (Azimuth < 0.f)
	{
		Azimuth = 0.f;
	}

	Inclination += YawAdjust;
	Inclination = UKismetMathLibrary::GenericPercent_FloatFloat(Inclination, 360.f);

	ForwardVector = FVector(UKismetMathLibrary::DegSin(Azimuth) * UKismetMathLibrary::DegCos(Inclination), UKismetMathLibrary::DegSin(Azimuth) * UKismetMathLibrary::DegSin(Inclination), UKismetMathLibrary::DegCos(Azimuth));

	ProjectileMovement->Velocity = ForwardVector * ProjectileMovement->InitialSpeed;
}

void ASL_Projectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{

	}
	else
	{
		switch (OnContactPolicy) {
		case ContactPolicy::EXPLODE:
			OnExplode();
			break;
		case ContactPolicy::STICKY:
			/*CollisionComp->Deactivate();
			ProjectileMovement->Deactivate();*/
			AttachToActor(OtherActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
			OnStick();
			//AttachToActor(OtherActor, FAttachmentTransformRules::KeepRelativeTransform);
			//AttachToComponent(OtherComponent, FAttachmentTransformRules::KeepRelativeTransform);
			break;
		default:
			break;
		}
	}

}

void ASL_Projectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OnContactPolicy == ContactPolicy::EXPLODE)
	{
		ASoldier* soldierOwner = Cast<ASoldier>(GetOwner());
		ASoldier* soldier = Cast<ASoldier>(OtherActor);

		if (soldier)
		{
			if (soldierOwner)
			{
				if (soldierOwner->GetTeam() != soldier->GetTeam())
				{
					OnExplode();
				}
			}
			else
			{
				OnExplode();
			}
		}
		else
		{
			OnExplode();
		}
	}
	else if (OnContactPolicy == ContactPolicy::STICKY)
	{
		ASoldier* soldier = Cast<ASoldier>(OtherActor);
		if (soldier)
		{
		}
		else
		{
			//ProjectileMovement->Deactivate();
			AttachToActor(OtherActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
			OnStick();
		}
	}
}

//FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffect, GetAbilityLevel());