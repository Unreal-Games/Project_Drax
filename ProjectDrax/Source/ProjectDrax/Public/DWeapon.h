// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ProjectDrax.h"

#include "Camera/CameraShake.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DWeapon.generated.h"

class ADCharacter;
UENUM(BlueprintType)
namespace EWeaponProjectile
{
	enum ProjectileType
	{
		EBullet			UMETA(DisplayName = "Bullet"),
		ESpread			UMETA(DisplayName = "Spread"),
		EProjectile		UMETA(DisplayName = "Projectile"),
	};
}


USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
		int32 MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 MaxClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float TimeBetweenShots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo)
		int32 ShotCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float WeaponRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float WeaponSpread;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float BaseDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float GravityDelay;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		float GravityScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		TSubclassOf<UCameraShake> FireCamShake;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FName TracerTargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UParticleSystem* FleshImpactEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UParticleSystem* TracerEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FName MuzzleSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UParticleSystem* MuzzleEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UTexture2D* SplashArt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 Priority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		class USkeletalMeshComponent* MeshComp;
};

USTRUCT()
struct FMuzzle
{
	GENERATED_BODY()

public:

	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
		FVector_NetQuantize TraceTo;
	
	
};
UCLASS()

class PROJECTDRAX_API ADWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	//AController* GetInst();
	
	// Sets default values for this actor's properties
	ADWeapon();

	//comp of weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision)
		UBoxComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USkeletalMeshComponent* MeshComp;
	
	UPROPERTY(BlueprintReadWrite)
		bool flag;


	//togle ADS
	bool bIsADS;
	UFUNCTION(BlueprintImplementableEvent)
		void ShowADSOverlay();
	UFUNCTION(BlueprintImplementableEvent)
		void HideADSOverlay();
	void ToggleADS();


	//reload weapon
	UPROPERTY(BlueprintReadWrite, Category = "Fire")
		bool bReload;
	FTimerHandle TimerHandle_ReloadTime;


	


	//weapon data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FWeaponData WeaponConfig;

	//choose projectile if present
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Config)
		TEnumAsByte<EWeaponProjectile::ProjectileType> ProjectileType;
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> Projectile;


	//Equiping and unequiping weapon
	void AttachToPlayer();
	void DetachFromPlayer();
	void OnEquip();
	void OnUnEquip();

	//playing firing sound and visual effects
	UPROPERTY(EditDefaultsOnly, Category = Config)
		class USoundCue* FireSound;
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);
	void PlayFireEffects(FVector TraceEnd);
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;
	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FMuzzle HitScanTrace;
	UFUNCTION()
		void OnRep_HitScanTrace();
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 CurrentClip;
	
	//processing fire
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);
	UFUNCTION()
		void Instant_Fire();
	UFUNCTION()
		virtual void ProjectileFire();
	void SetOwningPawn(ADCharacter* NewOwner);
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();
	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFireTime;
	void Fire();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	

public:
	//Fynctions for firing
	void StartFire();

	void StopFire();

	void ReloadWeapon();

	void AutoReloadWeapon();


protected:

	
	virtual void BeginPlay() override;
	
	//Initialising owner
	ADCharacter* MyPawn;

};
