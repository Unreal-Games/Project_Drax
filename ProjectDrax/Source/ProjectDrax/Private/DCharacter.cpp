// Fill out your copyright notice in the Description page of Project Settings.


#include "DCharacter.h"
#include "DWeapon.h"
#include "DPickUp.h"
#include "Inventory.h"
#include "ProjectDrax.h"
#include "Components/UDHealthComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Containers/Array.h"
#include "UnrealNetwork.h"

// Sets default values
ADCharacter::ADCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	/*CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT( "CollisionComp"));
	CollisionComp->SetupAttachment(RootComponent);*/
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	HealthComp = CreateDefaultSubobject<UUDHealthComponent>(TEXT("HealthComp"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CamerComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	Inventory = CreateDefaultSubobject<UInventory>(TEXT("InventoryComp"));


	WeaponAttachSocketName = "GunSocket";

	WeaponInventory.Init(nullptr, 3);
	PrimaryWeaponSocket = "Weapon1";
	SecondaryWeaponSocket = "Weapon2";
	bPrimarySocketEquiped = false;
	bSecondarySocketEquiped = false;
}

// Called when the game starts or when spawned
void ADCharacter::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnHealthChanged.AddDynamic(this, &ADCharacter::OnHealthChanged);
	if (Role = ROLE_Authority)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ADWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwningPawn(this);
			CurrentWeapon->SetActorEnableCollision(false);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
			CurrentWeaponIndex = 0;
			bCurrentWeaponEquiped = true;
			
			WeaponInventory[0] = CurrentWeapon;
		}
	}


	bProning = false;
	bProne = false;
	SprintValue = 2;

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
	//UE_LOG(LogTemp,Warning,TEXT("%s"),*CurrentWeapon->WeaponConfig.Name);
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
	
	if (Weapon != NULL)
	{
		Weapon-> SetActorEnableCollision(false);
		if (Weapon->GetOwner()==NULL)
		{
			ADWeapon* Spawner = GetWorld()->SpawnActor<ADWeapon>(Weapon->GetClass());
			if (Spawner)
			{
		
				if (!((bCurrentWeaponEquiped && bSecondarySocketEquiped) || (bCurrentWeaponEquiped&& bPrimarySocketEquiped)||(bPrimarySocketEquiped&&bSecondarySocketEquiped)))
				{
					if (!CurrentWeapon)
					{
						
							//Inventory[0] = Spawner;
						Weapon->SetOwningPawn(this);
						if (!WeaponInventory[0])
						{
							GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Primary Pickup : " + Weapon->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
							
							WeaponInventory[0] = Weapon;
							
						}
						else if (!WeaponInventory[1])
						{
							//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Second Pickup : " + Weapon->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
							
							WeaponInventory[1] = Weapon;
						}
						CurrentWeapon = Weapon;
						bCurrentWeaponEquiped = true;
					}
					else if (!WeaponInventory[0])
					{

						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Primary : " + Weapon->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

						WeaponInventory[0] = Weapon;
						Weapon->SetOwningPawn(this);
						Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryWeaponSocket);
						bPrimarySocketEquiped = true;
					}
					else if (!WeaponInventory[1])
					{
						
						WeaponInventory[1] = Weapon;
						GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Second : " + Weapon->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

						Weapon->SetOwningPawn(this);
						Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);
						bSecondarySocketEquiped = true;
					}
				}
				else
				{
				
						if (bCurrentWeaponEquiped)
						{
							
								GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Swap : " + Weapon->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

								//WeaponInventory[0]->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
								int Swap;
								for(int i=0;i<2;i++)
								{
									if(WeaponInventory[i]->WeaponConfig.Name==CurrentWeapon->WeaponConfig.Name)
									{
										Swap = i;
									}
								}
								CurrentWeapon->OnUnEquip();
							CurrentWeapon->SetOwner(NULL);
							Weapon->SetOwningPawn(this);
							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
							CurrentWeapon = Weapon;
						
							WeaponInventory[Swap] = Weapon;
						}
						else
						{
							
								GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Second swap : " + Weapon->WeaponConfig.Name + "with " + WeaponInventory[1]->WeaponConfig.Name);

								WeaponInventory[1]->OnUnEquip();
								WeaponInventory[1]->SetOwner(NULL);
							Weapon->SetOwningPawn(this);
							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);

							WeaponInventory[1] = Weapon;
						}
				}
			}
		}	
	}
}


void ADCharacter::NextWeapon()
{
	CurrentWeaponIndex += 1;
	if(CurrentWeaponIndex>2)
	{
		CurrentWeaponIndex = 0;	
	}
	if (CurrentWeaponIndex == 0)
	{
		bPrimarySocketEquiped = false;

	}
	else
		bSecondarySocketEquiped = false;
	
	EquipWeapon(WeaponInventory[CurrentWeaponIndex]);
}

void ADCharacter::PrevWeapon()
{
	CurrentWeaponIndex -= 1;
	if (CurrentWeaponIndex < 0)
	{
		CurrentWeaponIndex = 2;
	}
	if (CurrentWeaponIndex == 0)
	{
		bPrimarySocketEquiped = false;

	}
	else
		bSecondarySocketEquiped = false;
	
	EquipWeapon(WeaponInventory[CurrentWeaponIndex]);
}

void ADCharacter::EquipWeapon(ADWeapon * Weapon)
{
	if (Weapon)
	{
		if (CurrentWeapon != NULL)
		{
			//CurrentWeapon = Inventory[CurrentWeaponIndex];
			CurrentWeapon->OnUnEquip();
			if (!(bPrimarySocketEquiped||WeaponInventory[0]!= CurrentWeapon))
			{
				CurrentWeapon->OnUnEquip();
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Primary Swaping : " + WeaponInventory[CurrentWeaponIndex]->WeaponConfig.Name+"with "+CurrentWeapon->WeaponConfig.Name);

				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryWeaponSocket);
				bPrimarySocketEquiped = true;
			}
			else if (!(bSecondarySocketEquiped || WeaponInventory[1] != CurrentWeapon))
			{
				CurrentWeapon->OnUnEquip();
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Secondary Swaping : " + WeaponInventory[CurrentWeaponIndex]->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);
				bSecondarySocketEquiped = true;
			}
			CurrentWeapon = Weapon;
			Weapon->SetOwningPawn(this);
			Weapon->OnUnEquip();
			//Weapon->OnEquip();
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);

			
		}
		else
		{
			Weapon->OnUnEquip();
			CurrentWeapon = Weapon;
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Current weapon initialsied : " + WeaponInventory[CurrentWeaponIndex]->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

			//CurrentWeapon = Inventory[CurrentWeaponIndex];
			CurrentWeapon->SetOwningPawn(this);
			//Weapon->OnEquip();
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);

			bCurrentWeaponEquiped = true;
		}
	}
	else
	{

		if (CurrentWeapon != NULL)
		{
			CurrentWeapon->OnUnEquip();
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Swaping Null with " + CurrentWeapon->WeaponConfig.Name);
			if (!(bPrimarySocketEquiped || WeaponInventory[0] != CurrentWeapon))
			{
				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryWeaponSocket);
				CurrentWeapon = Weapon;
				bPrimarySocketEquiped = true;
			}
				//CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
			else if (!(bSecondarySocketEquiped || WeaponInventory[1] != CurrentWeapon))
			{
				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);
				CurrentWeapon = Weapon;
				bSecondarySocketEquiped = true;
			}
				//Weapon->SetOwningPawn(this);
			bCurrentWeaponEquiped = false;
		}
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
			return;
		}
		ADPickUp* PickUp = Cast<ADPickUp>(Hit.GetActor());
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("%s"), *PickUp->GetName()));
		if (PickUp)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("piciking")));
			Inventory->AddItem(PickUp);
			PickUp->SetActorEnableCollision(false);
			PickUp->SetActorHiddenInGame(true);
			//PickUp->Destroy();
			//PickUp->DestroyPickUp();
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
		CurrentWeapon->OnUnEquip();
		CurrentWeapon->SetOwner(NULL);
		//CurrentWeapon = nullptr;
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

	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &ADCharacter::ShowInventory);
	//PlayerInputComponent->BindAction("Inventory", IE_Released, this, &ADCharacter::HideInventory);

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