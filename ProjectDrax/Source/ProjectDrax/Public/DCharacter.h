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
	
	ADCharacter();

protected:
	
	virtual void BeginPlay() override;

	//Movement Functions
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


	//ADS functions
	void BeginZoom();

	void EndZoom();


	//TimeHandle for animation
	FTimerHandle InputTimeHandle;


	//Default variables
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		float DefaultWalkSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		float ProneSpeedMultiplier;

	UPROPERTY(BlueprintReadOnly,Category="Movement")
		bool bProne;

	UPROPERTY(BlueprintReadOnly,Category="Movement")
		bool bProning;

	UPROPERTY(BlueprintReadOnly,Category="Movement")
		float SprintSpeedMultiplier;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		float BeginProneAnimTime;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
		float EndProneAnimTime;


	//Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* SpringArmComp;

public:	
	
	virtual void Tick(float DeltaTime) override;

	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
