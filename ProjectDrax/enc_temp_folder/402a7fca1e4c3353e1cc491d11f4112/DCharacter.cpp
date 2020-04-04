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
	if(Role=ROLE_Authority)
	{
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
			CurrentWeaponIndex = 0;
			bCurrentWeaponEquiped = true;
			//	bPrimarySocketEquiped = true;
			Inventory[0] = CurrentWeapon;
		}
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

void ADCharacter::ShowInventory()
{
	for (int i = 0; i < 3; i++)
		if(Inventory[i])
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, i + "  .   " + Inventory[i]->GetName());
	
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
	UE_LOG(LogTemp, Warning, TEXT("Pickup called"))
	if (Weapon != NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponSpotted"))
		if (Weapon->GetOwner()==NULL)
		{
			ADWeapon* Spawner = GetWorld()->SpawnActor<ADWeapon>(Weapon->GetClass());
			if (Spawner)
			{
		
				if (!((bCurrentWeaponEquiped && bSecondarySocketEquiped) || (bCurrentWeaponEquiped&& bPrimarySocketEquiped)||(bPrimarySocketEquiped&&bSecondarySocketEquiped)))
				{
					if (!CurrentWeapon)
					{
						UE_LOG(LogTemp, Warning, TEXT("CurrentWeapon"))
							Inventory[0] = Spawner;
						Weapon->SetOwningPawn(this);
						if (!Inventory[0])
						{
							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
							
							Inventory[0] = Weapon;
						}
						else if (!Inventory[1])
						{
							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
							
							Inventory[1] = Weapon;
						}
						CurrentWeapon = Weapon;
						bCurrentWeaponEquiped = true;
					}
					else if (!Inventory[1])
					{
						UE_LOG(LogTemp, Warning, TEXT("Secondary"))
							Inventory[1] = Spawner;
						Weapon->SetOwningPawn(this);
						Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);
						bSecondarySocketEquiped = true;
					}
					else if (!Inventory[0])
					{
						UE_LOG(LogTemp, Warning, TEXT("Primary"))
							Inventory[0] = Spawner;
						Weapon->SetOwningPawn(this);
						Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryWeaponSocket);
						bPrimarySocketEquiped = true;
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Swap"))
						if (bCurrentWeaponEquiped)
						{
							UE_LOG(LogTemp, Warning, TEXT("CurrentSwap"))
								//Inventory[0]->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
								int Swap;
								for(int i=0;i<2;i++)
								{
									if(Inventory[i]==CurrentWeapon)
									{
										Swap = i;
									}
								}
								CurrentWeapon->OnUnEquip();
							CurrentWeapon->SetOwner(NULL);
							Weapon->SetOwningPawn(this);
							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
							CurrentWeapon = Weapon;
						
							Inventory[Swap] = Weapon;
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("SecSwap"))
								Inventory[1]->OnUnEquip();
							Inventory[1]->SetOwner(NULL);
							Weapon->SetOwningPawn(this);
							Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SecondaryWeaponSocket);

							Inventory[1] = Weapon;
						}
				}
			}
			
			
			
		}UE_LOG(LogTemp, Warning, TEXT("WEapon Dont have priority"))
		
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
	
	EquipWeapon(Inventory[CurrentWeaponIndex]);
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
	
	EquipWeapon(Inventory[CurrentWeaponIndex]);
}

void ADCharacter::EquipWeapon(ADWeapon * Weapon)
{
	if (Weapon)
	{
		if (CurrentWeapon != NULL)
		{
			//CurrentWeapon = Inventory[CurrentWeaponIndex];
			CurrentWeapon->OnUnEquip();
			if (!bPrimarySocketEquiped)
			{
				CurrentWeapon->OnUnEquip();
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Primary Swaping : " + Inventory[CurrentWeaponIndex]->WeaponConfig.Name+"with "+CurrentWeapon->WeaponConfig.Name);

				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryWeaponSocket);
				bPrimarySocketEquiped = true;
			}
			else
			{
				CurrentWeapon->OnUnEquip();
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Secondary Swaping : " + Inventory[CurrentWeaponIndex]->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

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
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "Current weapon initialsied : " + Inventory[CurrentWeaponIndex]->WeaponConfig.Name + "with " + CurrentWeapon->WeaponConfig.Name);

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

				CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryWeaponSocket);
				CurrentWeapon = Weapon;
				//CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);

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

	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &ADCharacter::ShowInventory);
	
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