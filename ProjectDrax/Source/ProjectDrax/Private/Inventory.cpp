// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "DPickUp.h"
#include <ProjectDrax\Public\DWeapon.h>
#include "DCharacter.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UInventory::UInventory()
{
	InventoryWeight = 120;
	CurrentInventoryWeight = 0;
}


// Called when the game starts
void UInventory::BeginPlay()
{
	Super::BeginPlay();

}

void UInventory::FindNearByItems()
{
	NearByItems.Empty();
	TArray<FHitResult> Hit;
	FVector Loc = GetOwner()->GetActorLocation();
	ECollisionChannel Col=ECollisionChannel::ECC_Pawn;

	FCollisionQueryParams ColParams;
	//ColParams.AddIgnoredActor(this);
	ColParams.AddIgnoredActor(GetOwner());
	FCollisionResponseParams ResParams;


	FCollisionShape ColShape = FCollisionShape::MakeSphere(500.f);
	DrawDebugSphere(GetWorld(), Loc, ColShape.GetSphereRadius(), 50, FColor::Purple, true, 999);
	bool IsHit = GetWorld()->SweepMultiByChannel(Hit, Loc, Loc + FVector(0, 0, 1), FQuat::Identity, ECC_Visibility, ColShape);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit !!!%d"), Hit.Num()));
	
	if (IsHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit !!!")));

		for (FHitResult Impact : Hit)
		{
			
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Result: %s"), *Impact.Actor->GetName()));
			//// uncommnet to see more info on sweeped actor
			// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("All Hit Information: %s"), *Impact.ToString()));
		
			AActor* Actor = Impact.GetActor();
			ADPickUp* PickUp = Cast<ADPickUp>(Actor);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Result: %s"), *Actor->GetName()));
			if (PickUp)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("PickUP Hit Result: %s"), *PickUp->GetName()));
				NearByItems.Add(PickUp);
			}
			ADWeapon* Weapon = Cast<ADWeapon>(Actor);
			if (Weapon)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("PickUP Hit Result: %s"), *Weapon->GetName()));
				NearByItems.Add(Weapon);
			}
		}
	}
	
	
	
}

ADWeapon* UInventory::Pick(class AActor* Item)
{
	ADPickUp* PickUp = Cast<ADPickUp>(Item);
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Hit Result: %s"), *Actor->GetName()));
	if (PickUp)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("PickUP Hit Result: %s"), *PickUp->GetName()));
		PickUp->SetActorEnableCollision(false);
		PickUp->SetActorHiddenInGame(true);
		NearByItems.Remove(PickUp);
		AddItem(PickUp);
	}
	ADWeapon* Weapon = Cast<ADWeapon>(Item);
	if (Weapon)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("PickUP Hit Result: %s"), *PickUp->GetName()));
		NearByItems.Remove(Weapon);
		return Weapon;
	}
	return nullptr;
}

bool UInventory::AddItem(ADPickUp* Item)
{
	if (Item->PickUP.Weight + CurrentInventoryWeight < InventoryWeight)
	{
		CurrentInventoryWeight += Item->PickUP.Weight;
		if (Items.Contains(Item))
		{
			
			 auto PrevItemIndex= Items.Find(Item);
			 Items[PrevItemIndex]->PickUP.Amount += Item->PickUP.Amount;
		}
		else
		Items.Add(Item);
	}
	
	for (ADPickUp* Ite : Items)
	{
		UE_LOG(LogTemp,Warning,TEXT("\n%s"),*Ite->GetName())
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("%s"), *Ite->GetName()));
	}

	return false;
}

void UInventory::DropItem(ADPickUp* Item)
{
	FVector loc= GetOwner()->GetActorLocation();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("%s"), *loc.ToString()));
	loc.X = FMath::RandRange(-50.0f, 100.0f);
	loc.Y = FMath::RandRange(-50.0f, 100.0f);
	loc.Z += 50;
	/*FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GetWorld()->SpawnActor<ADPickUp>(InventoryItem, loc, FRotator::ZeroRotator, SpawnParams);*/
	Item->SetActorEnableCollision(true);
	Item->SetActorLocation(loc);
	Item->SetActorHiddenInGame(false);
	
	RemoveItem(Item);
}

void UInventory::RemoveItem(ADPickUp* Item)
{
	Items.Remove(Item);
}

