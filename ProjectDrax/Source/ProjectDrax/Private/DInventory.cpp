// Fill out your copyright notice in the Description page of Project Settings.


#include "DInventory.h"

// Sets default values
ADInventory::ADInventory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADInventory::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADInventory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADInventory::AddItem(ADPickUp* Item)
{
	Items.Add(Item);
	return false;
}

void ADInventory::RemoveItem(ADPickUp* Item)
{
	
}

