// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "DWeapon.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTDRAX_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventory();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY(BlueprintReadWrite)
		TArray<class ADPickUp*> Items;
	UPROPERTY(BlueprintReadWrite)
		TArray<class AActor*> NearByItems;
	int32 InventoryWeight;
	int32 CurrentInventoryWeight;
public:
	// Called every frame
	UFUNCTION(BlueprintCallable)
	void FindNearByItems();
	
	UFUNCTION(BlueprintCallable)
	ADWeapon* Pick(class AActor* Item);

	bool AddItem(class ADPickUp* Item);
	UFUNCTION(BlueprintCallable)
	void DropItem(class ADPickUp* Item);

	void RemoveItem(class ADPickUp* Item);
		
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		TSubclassOf<class ADPickUp> InventoryItem;
};
