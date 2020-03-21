// Fill out your copyright notice in the Description page of Project Settings.


#include "DWeapon.h"

#include "Bullet.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "CollisionQueryParams.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
// Sets default values
ADWeapon::ADWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	//RootComponent = Mesh;
	//BulletSpeed = 20.0f;
	//BaseDamage = 20.0f;
	//RateOfFire = 120.0f;

	//AActor* MyOwner = GetOwner();
	//QueryParams.AddIgnoredActor(MyOwner);
	//QueryParams.AddIgnoredActor(this);
	//QueryParams.bTraceComplex = true;
	//QueryParams.bReturnPhysicalMaterial = true;

}

void ADWeapon::Fire()
{
	FRotator SpawnRotation;
	FVector SpawnLocation;

	GetActorEyesViewPoint(SpawnLocation, SpawnRotation);
	//FVector Direction = SpawnRotation.GetNormalized();
	FVector Loc;
	FRotator Rot;
	FHitResult Hit;
	//GetController()->GetPlayerViewPoint(Loc, Rot);
	FVector Start = SpawnLocation;
	FVector End = Start + (SpawnRotation.Vector() * 1000);
	FCollisionQueryParams TraceParams;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

	DrawDebugLine(GetWorld(), Start, End, FColor::Orange, true);
	SpawnLocation += FVector(100.f, 100.f, 0.f);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(this);
	//	QueryParams.bTraceComplex = true;
		//DrawDebugLine(GetWorld(), SpawnLocation, End, FColor::Cyan, true);
	ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(ActorToSpawn, SpawnLocation, SpawnRotation);
	FVector NewVelocity = GetActorForwardVector() * 400.f;
	Bullet->Velocity = FVector(NewVelocity);
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

