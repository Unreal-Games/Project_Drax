// Fill out your copyright notice in the Description page of Project Settings.


#include "DCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DWeapon.h"
#include "Components/BoxComponent.h"
#include "ProjectDrax.h"
#include "Components/UDHealthComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Containers/Array.h"
#include "UnrealNetwork.h"

// Sets default values
ADCharacter::ADCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT( "CollisionComp"));
	CollisionComp->SetupAttachment(RootComponent);
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	HealthComp = CreateDefaultSubobject<UUDHealthComponent>(TEXT("HealthComp"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CamerComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	WeaponAttachSocketName = "GunSocket";

	Inventory.Init(nullptr, 3);
	PrimaryWeaponSocket = "Weapon1";
	
	SecondaryWeaponSocket = "Weapon2";
	bPrimarySocketEquiped = false;
	bSecondarySocketEquiped = false;
}

void ADCharacter::AddControllerPitchInput(float Val)
{
	Super::AddControllerPitchInput(Val);
	
}

void ADCharacter::AddControllerYawInput(float Val)
{
	
	Super::AddControllerYawInput(Val);
	FRotator PlayerRotation = GetControlRotation();
	if (PlayerRotation.Pitch> 90.f||PlayerRotation.Pitch<-90)
	{
		PlayerRotation.Pitch = 0;
		PlayerRotation.Roll = 0;
		SetActorRotation(PlayerRotation);
	}
}

// Called when the game starts or when spawned
void ADCharacter::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnHealthChanged.AddDynamic(this, &ADCharacter::OnHealthChanged);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//FVector Loc = GetMesh()->GetSocketLocation("GunSocket");
	//FRotator Rot = GetControlRotation();
	//UE_LOG(LogTemp,Warning,TEXT("Location:%s\nRotation%s"),*(Loc.ToString()),*(Rot.ToString()))
	CurrentWeapon = GetWorld()->SpawnActor<ADWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		//UE_LOG(LogTemp, Warning, TEXT("Socket:%s"), *(WeaponAttachSocketName.ToString()))
	}
	
	bProning=false;
	bProne=false;
SprintValue=2;
	
}

void ADCharacter::MoveForward(float Value)
{
	
	AddMovementInput(CameraComp->GetForwardVector()*FVector(1.f,1.f,0.f) * Value);
}

void ADCharacter::MoveRight(float Value)
{
	//Value *= PlayerRotation.Yaw;
	AddMovementInput(CameraComp->GetRightVector() * FVector(1.f, 1.f, 0.f) * Value);
}

void ADCharacter::BeginCrouch()
{
	Crouch();
}

void ADCharacter::BeginFire()
{
	if (CurrentWeapon)
	{
		bFire = true;
		CurrentWeapon->StartFire();
	}
}

void ADCharacter::EndFire()
{
	if (CurrentWeapon)
	{
		bFire = false;
		CurrentWeapon->StopFire();
	}
}

void ADCharacter::EndCrouch()
{
	UnCrouch();
}

void ADCharacter::BeginProne()
{
	bProning=true;
	GetWorld()->GetTimerManager().SetTimer(InputTimeHandle,this, &ADCharacter::ProneDown, 1.7f, true);
	//@TODO stand to prone should not get cancelled
}

void ADCharacter::ProneDown()
{
		bProne=true;
}

void ADCharacter::StandUp()
{
		bProne=false  ;
}


void ADCharacter::EndProne()
{
	//GetWorld()->GetTimerManager().ClearTimer(InputTimeHandle);
	if(bProne==false)
		bProne=true;
	bProning=false;
	
	GetWorld()->GetTimerManager().SetTimer(InputTimeHandle,this, &ADCharacter::StandUp, 1.5f, true);
}

void ADCharacter::BeginZoom()
{
	if(CurrentWeapon)
	{
		CameraComp->SetFieldOfView(10.f);
		CurrentWeapon->ToggleADS();
	}
}

void ADCharacter::ProcessWeaponPickup(ADWeapon* Weapon)
{
	UE_LOG(LogTemp,Warning,TEXT("asd"))
	if (Weapon != NULL)
	{
		if (Inventory[Weapon->WeaponConfig.Priority] == NULL)
		{
			ADWeapon* Spawner = GetWorld()->SpawnActor<ADWeapon>(Weapon->GetClass());
			if (Spawner)
			{
				Inventory[Spawner->WeaponConfig.Priority] = Spawner;
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "You Just picked up a " + Inventory[Spawner->WeaponConfig.Priority]->WeaponConfig.Name);
			}
			if(!CurrentWeapon)
			{
				Weapon->SetOwner(this);
				Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
				bPrimarySocketEquiped = true;
			}
			else if(!bPrimarySocketEquiped)
			{
				Weapon->SetOwner(this);
				Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,PrimaryWeaponSocket);
				bPrimarySocketEquiped = true;
			}
			else if(!bSecondarySocketEquiped)
			{
				Weapon->SetOwner(this);
				Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);
				bSecondarySocketEquiped = true;
			}
		}
		
	}
}


void ADCharacter::NextWeapon()
{
	if (Inventory[CurrentWeapon->WeaponConfig.Priority]->WeaponConfig.Priority != 2)
	{
		if (Inventory[CurrentWeapon->WeaponConfig.Priority + 1] == NULL)
		{
			for (int32 i = CurrentWeapon->WeaponConfig.Priority + 1; i < Inventory.Num(); i++)
			{
				if (Inventory[i] && Inventory[i]->IsA(ADWeapon::StaticClass()))
				{
					EquipWeapon(Inventory[i]);
				}
			}
		}
		else
		{
			EquipWeapon(Inventory[CurrentWeapon->WeaponConfig.Priority + 1]);
		}
	}
	else
	{
		EquipWeapon(Inventory[CurrentWeapon->WeaponConfig.Priority]);
	}
}

void ADCharacter::PrevWeapon()
{
	if (Inventory[CurrentWeapon->WeaponConfig.Priority]->WeaponConfig.Priority != 0)
	{
		if (Inventory[CurrentWeapon->WeaponConfig.Priority - 1] == NULL)
		{
			for (int32 i = CurrentWeapon->WeaponConfig.Priority - 1; i >= 0; i--)
			{
				if (Inventory[i] && Inventory[i]->IsA(ADWeapon::StaticClass()))
				{
					EquipWeapon(Inventory[i]);
				}
			}
		}
		else
		{
			EquipWeapon(Inventory[CurrentWeapon->WeaponConfig.Priority - 1]);
		}
	}
	else
	{
		EquipWeapon(Inventory[CurrentWeapon->WeaponConfig.Priority]);
	}
}

void ADCharacter::EquipWeapon(ADWeapon * Weapon)
{
	if (CurrentWeapon != NULL)
	{
		CurrentWeapon = Inventory[CurrentWeapon->WeaponConfig.Priority];
		CurrentWeapon->OnUnEquip();
		CurrentWeapon = Weapon;
		Weapon->SetOwningPawn(this);
		Weapon->OnEquip();
	}
	else
	{
		CurrentWeapon = Weapon;
		CurrentWeapon = Inventory[CurrentWeapon->WeaponConfig.Priority];
		CurrentWeapon->SetOwningPawn(this);
		Weapon->OnEquip();
	}
}

void ADCharacter::EndZoom()
{
	if (CurrentWeapon)
	{
		CameraComp->SetFieldOfView(90.f);
		CurrentWeapon->ToggleADS();
		
	}
}

void ADCharacter::PickUP()
{
	FHitResult Hit;
	FVector Start;
	FRotator Rot;
	GetActorEyesViewPoint(Start, Rot);
	FVector End = Start + Rot.Vector() * 1000;
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End,ECC_Visibility);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, true);
	if(Hit.GetActor())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("%s"),*Hit.GetActor()->GetName()));
		ADWeapon* Weapon = Cast<ADWeapon>(Hit.GetActor());
		if (Weapon)
		{
			ProcessWeaponPickup(Weapon);
		}
	}
}

void ADCharacter::BeginSprint()
{
	GetCharacterMovement()->MaxWalkSpeed *= SprintValue;
}

void ADCharacter::EndSprint()
{
	float speed=GetCharacterMovement()->MaxWalkSpeed /= SprintValue;
	UE_LOG(LogTemp, Warning, TEXT("Speed= %f"), speed);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Speed: %f"), speed));
}



// Called every frame
void ADCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ADCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}


void ADCharacter::OnHealthChanged(UUDHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
                                  class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;
		UE_LOG(LogTemp, Log, TEXT("Died:%d"),bDied );
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(10.0f);
	}
}

void ADCharacter::ReloadWeapon()
{
	if(CurrentWeapon)
	{
		
		bFire = false;
		
			//GetWorldTimerManager().SetTimer(TimerHandle_ReloadTime, CurrentWeapon, &ADWeapon::ReloadWeapon, 3.0f, false);
			CurrentWeapon->ReloadWeapon();
	}
}

// Called to bind functionality to input
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

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ADCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ADCharacter::EndFire);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ADCharacter::BeginZoom);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ADCharacter::EndZoom);

	PlayerInputComponent->BindAction("PickUP", IE_Pressed, this, &ADCharacter::PickUP);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ADCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PrevWeapon", IE_Pressed, this, &ADCharacter::PrevWeapon);
	
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ADCharacter::ReloadWeapon);
	
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ADCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ADCharacter::EndSprint);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

}


void ADCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADCharacter, CurrentWeapon);
	DOREPLIFETIME(ADCharacter, bDied);
}