// Fill out your copyright notice in the Description page of Project Settings.


#include "DWeapon.h"

#include "Bullet.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Components/SkeletalMeshComponent.h"
#include "DCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectDrax.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "UnrealNetwork.h"
// Sets default values
static int32 DebugWeaponDrawing = 1;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);

///@todo adding annimation for pickups and weapon equip
///@todo addingsound damage effects
///@todo Replicating for Multiplayer

ADWeapon::ADWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it
	//Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	CollisionComp =CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	WeaponConfig.MuzzleSocketName = "MuzzleSocket";
	
	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	//BaseDamage = 20.f;
	
	bReload = false;
}

void ADWeapon::ToggleADS()
{
	if(bIsADS)
	{
		bIsADS = false;
		HideADSOverlay();
		MeshComp->SetVisibility(true);
		
	}
	else
	{
		bIsADS = true;
		ShowADSOverlay();
		MeshComp->SetVisibility(false);
	}
}

void ADWeapon::AttachToPlayer()
{
	if (MyPawn)
	{
		DetachFromPlayer();

		USkeletalMeshComponent* Character = MyPawn->GetMesh();
		
		MeshComp->AttachTo(Character, "GunSocket");
	}
}

void ADWeapon::DetachFromPlayer()
{

	Super::DetachRootComponentFromParent();
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	

}

void ADWeapon::OnEquip()
{
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachToPlayer();
}

void ADWeapon::OnUnEquip()
{

	DetachFromPlayer();
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ADWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
		return;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, TEXT("Fire called"));
	if (ProjectileType == EWeaponProjectile::EBullet)
	{
		if (CurrentClip > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, TEXT("Bullet"));
			Instant_Fire();
			PlayWeaponSound(FireSound);
			CurrentClip -= WeaponConfig.ShotCost;
		}
		else
		{
			ReloadWeapon();
		}
	}
	if (ProjectileType == EWeaponProjectile::ESpread)
	{
		if (CurrentClip > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, TEXT("Spread"));
			for (int32 i = 0; i <= WeaponConfig.WeaponSpread; i++)
			{
				Instant_Fire();
				PlayWeaponSound(FireSound);
			}
			
			CurrentClip -= WeaponConfig.ShotCost;
		}
		else
		{
			ReloadWeapon();
		}
	}
	if (ProjectileType == EWeaponProjectile::EProjectile)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, TEXT("Projection"));
		if (CurrentClip > 0)
		{
			PlayWeaponSound(FireSound);
			ProjectileFire();
			CurrentClip -= WeaponConfig.ShotCost;
		}
		else
		{
			ReloadWeapon();
		}
	}LastFireTime = GetWorld()->TimeSeconds;
}

void ADWeapon::ProjectileFire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{

		FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName); //+FVector(100.f);
		FRotator MuzzleRotation = MeshComp->GetSocketRotation(WeaponConfig.MuzzleSocketName);
		GetOwner()->GetActorEyesViewPoint(MuzzleLocation, MuzzleRotation);
		//GetOwner()->GetActorEyesViewPoint(MuzzleLocation, MuzzleRotation);
		MuzzleRotation.Pitch -= 90;
		UGameplayStatics::SpawnEmitterAttached(WeaponConfig.MuzzleEffect, MeshComp, NAME_None, MuzzleLocation, MuzzleRotation);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto Bullet = GetWorld()->SpawnActor<ABullet>(Projectile, MuzzleLocation, MuzzleRotation, SpawnParams);
		Bullet->Inst = MyOwner->GetInstigatorController();
		Bullet->MyOwner = MyOwner;

	}

}


void ADWeapon::Instant_Fire()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = WeaponConfig.WeaponSpread;
	const float SpreadCone = FMath::DegreesToRadians(WeaponConfig.WeaponSpread * 0.5);
	FRotator AimDirc;
	FVector StartTrace;
	GetOwner()->GetActorEyesViewPoint(StartTrace, AimDirc);
	//AimDirc.Pitch -= 90;
	FVector AimDir = AimDirc.Vector();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, SpreadCone, SpreadCone);
	const FVector EndTrace = StartTrace + ShootDir * WeaponConfig.WeaponRange;

	FVector TraceTo = EndTrace;

	//UE_LOG(LogTemp, Warning, TEXT("Aim Dir : % s\nStart Tarce : % s\nShoot dir : % s\nEndTarce : % s"), *AimDir.ToString(),*StartTrace.ToString(),*ShootDir.ToString(),*EndTrace.ToString())

	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.AddIgnoredActor(this);

	FHitResult Hit(ForceInit);
	FHitResult Impact;
	if (GetWorld()->LineTraceSingleByChannel(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams))
	{
		Impact = Hit;
		TraceTo = Impact.ImpactPoint;
		ProcessInstantHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	}
	if (WeaponConfig.TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponConfig.TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;
			TracerComp->SetVectorParameter(WeaponConfig.TracerTargetName, EndPoint);
			//UE_LOG(LogTemp, Warning, TEXT("Tracing Effects:%s"), *HitScanTrace.TraceTo.ToString())
			DrawDebugLine(this->GetWorld(), MuzzleLocation, EndPoint, FColor::Black, true, 10000, 10.f);
		}

	}

	HitScanTrace.TraceTo = TraceTo;
	UE_LOG(LogTemp, Warning, TEXT("Trace:%s"), *TraceTo.ToString())
		HitScanTrace.SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Impact.PhysMaterial.Get());
	PlayWeaponSound(FireSound);
	PlayFireEffects(EndTrace);
	PlayImpactEffects(HitScanTrace.SurfaceType, TraceTo);
	/*if(Role==ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnREp"))
		HitScanTrace.TraceTo = TraceTo;
		HitScanTrace.SurfaceType= UPhysicalMaterial::DetermineSurfaceType(Impact.PhysMaterial.Get());
	}*/
}

void ADWeapon::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir,
	int32 RandomSeed, float ReticleSpread)
{
	const FVector EndTrace = Origin + ShootDir * WeaponConfig.WeaponRange;
	const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;
	//DrawDebugLine(this->GetWorld(), Origin, Impact.TraceEnd, FColor::Black, true, 10000, 10.f);

	ADCharacter* Enemy = Cast<ADCharacter>(Impact.GetActor());
	if (Enemy)
	{
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Impact.PhysMaterial.Get());

		float ActualDamage = WeaponConfig.BaseDamage;
		if (SurfaceType == SURFACE_FLESHVULNERABLE)
		{
			ActualDamage *= 3.0f;
		}
		if (SurfaceType == SURFACE_FLESHLIMBS)
		{
			ActualDamage /= 3;
		}
		if (Role == ROLE_Authority)
		{
			HitScanTrace.SurfaceType = SurfaceType;
		}
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, "YOU HIT AN ENEMY!!");
		AController* cont;
		if (Instigator)
			cont = Instigator->GetController();
		else
			cont =GetOwner()->GetInstigatorController();
		UGameplayStatics::ApplyPointDamage(Enemy, ActualDamage, Origin, Impact,cont, GetOwner(), DamageType);
	}
}



UAudioComponent* ADWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UE_LOG(LogTemp, Warning, TEXT("SoundEffects"))
	UAudioComponent* AC = NULL;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
		UE_LOG(LogTemp, Warning, TEXT("Sound Playing"))
	}

	return AC;
}

void ADWeapon::PlayFireEffects(FVector TraceEnd)
{
	UE_LOG(LogTemp,Warning,TEXT("FireEffects"))
	if (WeaponConfig.MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(WeaponConfig.MuzzleEffect, MeshComp, WeaponConfig.MuzzleSocketName);
	}

	/*if (WeaponConfig.TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);

		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponConfig.TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(WeaponConfig.TracerTargetName, HitScanTrace.TraceTo);
			UE_LOG(LogTemp, Warning, TEXT("Tracing Effects:%s"),*HitScanTrace.TraceTo.ToString())
		}
	}*/

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(WeaponConfig.FireCamShake);
		}
	}
}

void ADWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UE_LOG(LogTemp, Warning, TEXT("ImpactEffects"))
		UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
	case SURFACE_FLESHLIMBS:
		SelectedEffect = WeaponConfig.FleshImpactEffect;
		break;
	default:
		SelectedEffect = WeaponConfig.DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(WeaponConfig.MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ADWeapon::SetOwningPawn(ADCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		SetOwner(NewOwner);
	}
}


void ADWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
	
	
}

void ADWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ADWeapon::ServerFire_Validate()
{
	return true;
}
	
void ADWeapon::StartFire()
{
	if(GetOwner())
	{
		
		float FirstDelay = FMath::Max(LastFireTime + WeaponConfig.TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ADWeapon::Fire, WeaponConfig.TimeBetweenShots, true, FirstDelay);

	}
}

void ADWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ADWeapon::ReloadWeapon()
{
	bReload = true;
	if (CurrentAmmo > 0||CurrentClip<WeaponConfig.MaxClip)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ADWeapon::AutoReloadWeapon, 2.1f, false);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0, FColor::Blue, "NO AMMO");
	}
}

void ADWeapon::AutoReloadWeapon()
{
	
		if (CurrentAmmo < WeaponConfig.MaxClip)
		{
			CurrentClip = CurrentAmmo;
			CurrentAmmo = 0;
		}
		else
		{
			CurrentAmmo -= WeaponConfig.MaxClip-CurrentClip;
			CurrentClip = WeaponConfig.MaxClip;
		}
		bReload = false;
	
}

// Called when the game starts or when spawned
void ADWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ADWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADWeapon,HitScanTrace, COND_SkipOwner);
}
