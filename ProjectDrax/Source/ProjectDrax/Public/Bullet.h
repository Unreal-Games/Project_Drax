// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"


USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
		FVector_NetQuantize TraceTo;

	


};
UCLASS()
class PROJECTDRAX_API ABullet : public AActor
{
	GENERATED_BODY()
		
public:	
	// Sets default values for this actor's properties
	ABullet();
	AController* Inst;
	AActor* MyOwner;
	//UPROPERTY(BlueprintReadWrite)
	FCollisionQueryParams QueryParams;
	UPROPERTY(EditDefaultsOnly)
	class	UBoxComponent* ProjectileCollisions;

	UPROPERTY(EditDefaultsOnly)
		class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly)
		class UStaticMeshComponent* ProjectileMesh;
	UPROPERTY(EditDefaultsOnly)
		class UParticleSystemComponent* ProjectileParticles;

	//float BulletExpiry = 0;

	UPROPERTY(EditAnywhere, Category = "Bullet")
		float BulletSpeed;
	UPROPERTY(EditAnywhere, Category = "Bullet")
		float GravityDelay;
	UPROPERTY(EditAnywhere, Category = "Bullet")
		float GravityScale;
	UFUNCTION()
		void OnProjectileHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyGravity();


	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* DefaultImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		float BaseDamage;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

	UFUNCTION()
		void OnRep_HitScanTrace();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
