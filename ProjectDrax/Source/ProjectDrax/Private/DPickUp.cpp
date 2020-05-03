// Fill out your copyright notice in the Description page of Project Settings.


#include "DPickUp.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"


ADPickUp::ADPickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	///*CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	//CollisionComp;*/
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;// ->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADPickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADPickUp::DestroyPickUp()
{
	Destroy();
}



