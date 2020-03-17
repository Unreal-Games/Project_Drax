// Fill out your copyright notice in the Description page of Project Settings.


#include "DWeapon.h"

// Sets default values
ADWeapon::ADWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

