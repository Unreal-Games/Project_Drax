// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UDHealthComponent.h"



#include "DGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UUDHealthComponent::UUDHealthComponent()
{
	
	DefaultHealth = 100;
	bIsDead = false;

	//TeamNum = 255;

	SetIsReplicated(true);
	
}


// Called when the game starts
void UUDHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UUDHealthComponent::HandleTakeAnyDamage);
		}
	}

		Health = DefaultHealth;
}
//
void UUDHealthComponent::OnRep_Health(float OldHealth)
{
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

//
void UUDHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("Health Changed:%s"), *(FString::SanitizeFloat(Health)));
	if (Damage <= 0.0f || bIsDead)
		return;
	UE_LOG(LogTemp, Log, TEXT("H"))
	if (DamageCauser == DamagedActor && IsFriendly(DamagedActor, DamageCauser))
	{
		UE_LOG(LogTemp, Log, TEXT("Health "))
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);
	UE_LOG(LogTemp, Log, TEXT("Health Changed:%s"), *( FString::SanitizeFloat(Health)));
	bIsDead = Health <= 0.0f;
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	if (bIsDead)
	{
		ADGameMode* GM = Cast<ADGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
}

float UUDHealthComponent::GetHealth() const
{
	return Health;
}

void UUDHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

bool UUDHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		// Assume Friendly
		return true;
	}

	UUDHealthComponent* HealthCompA = Cast<UUDHealthComponent>(ActorA->GetComponentByClass(UUDHealthComponent::StaticClass()));
	UUDHealthComponent* HealthCompB = Cast<UUDHealthComponent>(ActorB->GetComponentByClass(UUDHealthComponent::StaticClass()));
	
	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		// Assume friendly
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("Team No.A:%d \nTeam no. B:%d"), HealthCompA->TeamNum, HealthCompB->TeamNum);
	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

//
//void UUDHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	DOREPLIFETIME(UUDHealthComponent, Health);
//}
