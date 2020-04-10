// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "ProjectDrax.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "DCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealNetwork.h"

// Sets default values
ABullet::ABullet()
{
	BulletSpeed = 40000.f;
	GravityDelay = 0.1;
	GravityScale = 3.5f;
	ProjectileCollisions = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectileCollision"));
	ProjectileCollisions->InitBoxExtent(FVector(2.f));
	
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;
	
	ProjectileCollisions->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileCollisions->OnComponentHit.AddDynamic(this, &ABullet::OnProjectileHit);
	SetRootComponent(ProjectileCollisions);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = ProjectileCollisions;
	ProjectileMovement->InitialSpeed = BulletSpeed;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);

	ProjectileParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileParticle"));
	ProjectileParticles->SetupAttachment(RootComponent);

	InitialLifeSpan = 3.0f;
}

void ABullet::OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor->IsA(ACharacter::StaticClass()))
	{
		
			if (Hit.BoneName != NAME_None)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Bone:%s"), *Hit.BoneName.ToString()));

			}
				FVector ShotDirection = GetActorRotation().Vector();
				float BulletSpread = 2.0f;
			//	// Bullet Spread
				float HalfRad = FMath::DegreesToRadians(BulletSpread);
				ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);
			// Blocking hit! Process damage
			EPhysicalSurface SurfaceType = SurfaceType_Default;
				AActor* HitActor = Hit.GetActor();
				if (HitActor)
				{
					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

					float ActualDamage = BaseDamage;
					if (SurfaceType == SURFACE_FLESHVULNERABLE)
					{
						ActualDamage *= 3.0f;
					}
					if(SurfaceType==SURFACE_FLESHLIMBS)
					{
						ActualDamage /= 3;
					}
					//if(GetOwner()->GetInstigatorController())
					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, Inst, MyOwner, DamageType);

					PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

					//TracerEndPoint = Hit.ImpactPoint;
				}
		
			
			//OtherActor->Destroy()
	}
	//UE_LOG(LogTemp,Warning,TEXT("Actor:%s"),*GetOwner()->GetName())
		Destroy();
}


void ABullet::ApplyGravity()
{
	ProjectileMovement->ProjectileGravityScale = GravityScale;
}

void ABullet::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		//FVector MuzzleLocation = GetOwner()->GetActorForwardVector();

		FVector ShotDirection = ImpactPoint;// -MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ABullet::OnRep_HitScanTrace()
{
	//PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

void ABullet::BeginPlay()
{
	Super::BeginPlay();
	FTimerHandle GravityTimer;
	GetWorldTimerManager().SetTimer(GravityTimer, this, &ABullet::ApplyGravity, GravityDelay, false);
	
	
}
void ABullet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABullet, HitScanTrace, COND_SkipOwner);
}
