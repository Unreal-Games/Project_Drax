// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/CameraShake.h"
#include "CoreMinimal.h"
#include "ProjectDrax.h"
#include "GameFramework/Actor.h"
#include "DWeapon.generated.h"

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

	UPROPERTY()
		USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* MeshComp;
	void PlayFireEffects(FVector TraceEnd);


	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		int DefaultMagSize;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	 	int CurrentMagSize;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		int TotalBullets;


	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* MuzzleEffect;


	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FMuzzle HitScanTrace;

	UFUNCTION()
		void OnRep_HitScanTrace();

	void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;
	

	float LastFireTime;

	/* RPM - Bullets per minute fired by weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float RateOfFire;

	/* Bullet Spread in Degrees */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
		float BulletSpread;

	// Derived from RateOfFire
	float TimeBetweenShots;


public:

	void StartFire();

	void StopFire();

	void ReloadWeapon();

	void AutoReloadWeapon();

	//void ResetMagBullets();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
