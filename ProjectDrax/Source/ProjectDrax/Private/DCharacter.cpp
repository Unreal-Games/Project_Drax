// Fill out your copyright notice in the Description page of Project Settings.


#include "DCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"


ADCharacter::ADCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CamerComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	//Initialing components
	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SprintSpeedMultiplier = 2.0f;
	ProneSpeedMultiplier = 5.0f;
	BeginProneAnimTime = 1.7f;
	EndProneAnimTime = 1.5f;
	bProne = false;
	bProning = false;
}

// Called when the game starts or when spawned
void ADCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void ADCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void ADCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void ADCharacter::BeginCrouch()
{
	Crouch();
}

void ADCharacter::EndCrouch()
{
	UnCrouch();
}

void ADCharacter::BeginProne()
{
	bProning=true;
	if (GetCharacterMovement()->MaxWalkSpeed >DefaultWalkSpeed)
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed /= ProneSpeedMultiplier;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("prone!!")));
	GetWorld()->GetTimerManager().SetTimer(InputTimeHandle,this, &ADCharacter::ProneDown, BeginProneAnimTime, false);
	//@TODO stand to prone should not get cancelled
}

void ADCharacter::ProneDown()
{
		bProne=true;
}

void ADCharacter::StandUp()
{
		bProne=false  ;
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("walking!!")));
}


void ADCharacter::EndProne()
{
	//GetWorld()->GetTimerManager().ClearTimer(InputTimeHandle);
	if(bProne==false)
		bProne=true;
	bProning=false;
	
	GetWorld()->GetTimerManager().SetTimer(InputTimeHandle,this, &ADCharacter::StandUp, BeginProneAnimTime, false);
}

void ADCharacter::BeginZoom()
{
	//@TODO perform zoom
}

void ADCharacter::EndZoom()
{
	//@TODO perform endzoom
}

void ADCharacter::BeginSprint()
{
	if(!bProning)
		GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("multiplied!!")));
}

void ADCharacter::EndSprint()
{
	if(!bProning)
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Called!!")));
	//UE_LOG(LogTemp, Warning, TEXT("Speed= %f"), speed);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Speed: %f"), speed));
}

// Called every frame
void ADCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto speed= GetCharacterMovement()->MaxWalkSpeed;
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Speed: %f"), speed));
}


void ADCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ADCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ADCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ADCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ADCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ADCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ADCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ADCharacter::EndZoom);

	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &ADCharacter::BeginProne);
	PlayerInputComponent->BindAction("Prone", IE_Released, this, &ADCharacter::EndProne);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ADCharacter::EndSprint);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

}

