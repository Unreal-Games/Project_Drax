// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DInventory.generated.h"

UCLASS()
class PROJECTDRAX_API ADInventory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADInventory();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	UPROPERTY()
		TArray<class ADPickUp*> Items;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool AddItem(class ADPickUp* Item);

	void RemoveItem(class ADPickUp* Item);
};
