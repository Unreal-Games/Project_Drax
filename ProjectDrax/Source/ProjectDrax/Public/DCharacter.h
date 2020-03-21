// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DCharacter.generated.h"

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	

	void MoveRight(float Value);

	void BeginCrouch();

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

	void EndZoom();

	FTimerHandle InputTimeHandle;


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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
