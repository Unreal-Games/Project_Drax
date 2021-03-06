// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DCharacter.generated.h"


class ADWeapon;
class USpringArmComponent;
class UCameraComponent;
struct FTimerHandle;

UCLASS()
class PROJECTDRAX_API ADCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADCharacter();
	
		virtual void AddControllerPitchInput(float Val) override;


		virtual void AddControllerYawInput(float Val) override;

		

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	

	void MoveRight(float Value);

	class UInventory* Inventory;

	void BeginCrouch();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowInventory();
	UFUNCTION(BlueprintImplementableEvent)
		void HideInventory();
	
	int32 CurrentWeaponIndex;
	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision)
		UBoxComponent* CollisionComp;*/

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UUDHealthComponent* HealthComp;

	void EndCrouch();

	void BeginProne();
UFUNCTION()
void ProneDown();
UFUNCTION()
void StandUp();

	void EndProne();

	void BeginSprint();

	void EndSprint();

	
	void BeginZoom();

	UFUNCTION(BlueprintCallable)
	void ProcessWeaponPickup(ADWeapon* Weapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory)
		TArray<class ADWeapon*> WeaponInventory;

	void EndZoom();

	FTimerHandle InputTimeHandle;

	void PickUP();

UPROPERTY(BlueprintReadOnly,Category="prone")
bool bProne;

UPROPERTY(BlueprintReadOnly,Category="prone")
bool bProning;

UPROPERTY(BlueprintReadOnly,Category="prone")
float SprintValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* SpringArmComp;

	void NextWeapon();
	void PrevWeapon();
	void EquipWeapon(ADWeapon* Weapon);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
		TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
		TSubclassOf<AActor> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		FName WeaponAttachSocketName;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		FName SecondaryWeaponSocket;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		FName PrimaryWeaponSocket;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		bool bPrimarySocketEquiped;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
		bool bSecondarySocketEquiped;
	virtual FVector GetPawnViewLocation() const override;
	UPROPERTY(Replicated,BlueprintReadWrite)
	class ADWeapon* CurrentWeapon;
	UPROPERTY(BlueprintReadOnly, Category = "prone")
		bool bFire ;
	
	bool bCurrentWeaponEquiped;

	UFUNCTION(BlueprintCallable, Category = "Player")
		void BeginFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void EndFire();
	
	UFUNCTION()
		void OnHealthChanged(UUDHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/* Pawn died previously */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
		bool bDied;
	void ReloadWeapon();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
