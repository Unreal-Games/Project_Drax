// Fill out your copyright notice in the Description page of Project Settings.


#include "DWeapon.h"

#include "Bullet.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Components/SkeletalMeshComponent.h"
#include "DCharacter.h"
#include "DrawDebugHelpers.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectDrax.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"
// Sets default values
static int32 DebugWeaponDrawing = 1;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);



ADWeapon::ADWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it
	//Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	//BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600;

	SetReplicates(true);

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	DefaultMagSize = 40;
	TotalBullets = 120;

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

void ADWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}



	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ADWeapon::Fire()
{
	if (CurrentMagSize > 0)
	{
		CurrentMagSize -= 1;
		AActor* MyOwner = GetOwner();
		//if (MyOwner)
		//{
		//	FVector EyeLocation;
		//	FRotator EyeRotation;
		//	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		//	FVector ShotDirection = EyeRotation.Vector();

		//	// Bullet Spread
		//	float HalfRad = FMath::DegreesToRadians(BulletSpread);
		//	ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

		//	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		//	FCollisionQueryParams QueryParams;
		//	QueryParams.AddIgnoredActor(MyOwner);
		//	QueryParams.AddIgnoredActor(this);
		//	QueryParams.bTraceComplex = true;
		//	QueryParams.bReturnPhysicalMaterial = true;

		//	// Particle "Target" parameter
		//	FVector TracerEndPoint = TraceEnd;

		//	EPhysicalSurface SurfaceType = SurfaceType_Default;

		//	
		//	UE_LOG(LogTemp,Warning,TEXT("Current Mag:%d"),CurrentMagSize)
		//	FHitResult Hit;
		//if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		//	{
		//		// Blocking hit! Process damage
		//		AActor* HitActor = Hit.GetActor();

		//		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		//		float ActualDamage = BaseDamage;
		//		if (SurfaceType == SURFACE_FLESHVULNERABLE)
		//		{
		//			ActualDamage *= 3.0f;
		//		}

		//		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

		//		PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

		//		TracerEndPoint = Hit.ImpactPoint;

		//	}

		//	if (DebugWeaponDrawing > 0)
		//	{
		//		DrawDebugLine(GetWorld(), EyeLocation, TracerEndPoint, FColor::Yellow, false, 1.0f, 0, 1.0f);
		//	}

		//	PlayFireEffects(TracerEndPoint);

		//	if (Role == ROLE_Authority)
		//	{
		//		HitScanTrace.TraceTo = TracerEndPoint;
		//		HitScanTrace.SurfaceType = SurfaceType;
		//	}
		
			LastFireTime = GetWorld()->TimeSeconds;
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName); //+FVector(100.f);
			FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);
			GetOwner()->GetActorEyesViewPoint(MuzzleLocation, MuzzleRotation);
		//GetOwner()->GetActorEyesViewPoint(MuzzleLocation, MuzzleRotation);
			MuzzleRotation.Pitch -= 90;
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, NAME_None,MuzzleLocation,MuzzleRotation);
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto Bullet = GetWorld()->SpawnActor<ABullet>(Projectile, MuzzleLocation, MuzzleRotation,SpawnParams);
		Bullet->Inst= MyOwner->GetInstigatorController();
		Bullet->MyOwner = MyOwner;
		//}
	}
	else
	{
		flag = true;
		ReloadWeapon();
	}
}
void ADWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);
	
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
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ADWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ADWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ADWeapon::ReloadWeapon()
{
	if (TotalBullets > 0||CurrentMagSize!=DefaultMagSize)
	{
		
		bReload = true;
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, this, &ADWeapon::AutoReloadWeapon, 3.0f, false);
		
		
	}
}

void ADWeapon::AutoReloadWeapon()
{
	if ((CurrentMagSize + TotalBullets) < (DefaultMagSize))
	{
		CurrentMagSize += TotalBullets;
		TotalBullets = 0;
	}
	else
	{
		TotalBullets -= DefaultMagSize - CurrentMagSize;
		CurrentMagSize = DefaultMagSize;
	}
	bReload = false;
	
}

// Called when the game starts or when spawned
void ADWeapon::BeginPlay()
{
	Super::BeginPlay();
	flag = false;
	CurrentMagSize = DefaultMagSize;
	TimeBetweenShots = 60 / RateOfFire;
}

void ADWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADWeapon,HitScanTrace, COND_SkipOwner);
}