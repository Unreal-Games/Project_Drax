// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraShake.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "ProjectDrax.h"
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
		FName MuzzleSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UParticleSystem* MuzzleEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		UTexture2D* SplashArt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 Priority;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision)
		UBoxComponent* CollisionComp;
	UPROPERTY(EditAnywhere)
		TSubclassOf<AActor> Projectile;
	UPROPERTY(BlueprintReadWrite)
	bool bIsADS;
	bool flag;
	UFUNCTION(BlueprintImplementableEvent)
		void ShowADSOverlay();
	UFUNCTION(BlueprintImplementableEvent)
		void HideADSOverlay();
	void ToggleADS();
	UPROPERTY(BlueprintReadWrite, Category = "Fire")
		bool bReload;
	FTimerHandle TimerHandle_ReloadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		FWeaponData WeaponConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Config)
		TEnumAsByte<EWeaponProjectile::ProjectileType> ProjectileType;

	void AttachToPlayer();
	void DetachFromPlayer();
	void OnEquip();
	void OnUnEquip();
	UPROPERTY(EditDefaultsOnly, Category = Config)
		class USoundCue* FireSound;
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);
	
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
		int32 CurrentClip;
	void PlayFireEffects(FVector TraceEnd);

	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	UFUNCTION()
		void Instant_Fire();

	UFUNCTION()
		virtual void ProjectileFire();

	void SetOwningPawn(ADCharacter* NewOwner);
	


	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FMuzzle HitScanTrace;

	UFUNCTION()
		void OnRep_HitScanTrace();

	void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;
	

	float LastFireTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	
	

public:

	void StartFire();

	void StopFire();

	void ReloadWeapon();

	void AutoReloadWeapon();


protected:

	
	virtual void BeginPlay() override;
	ADCharacter* MyPawn;

};
