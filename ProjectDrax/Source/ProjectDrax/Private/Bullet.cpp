// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Super::Tick(DeltaTime);
	FHitResult Hit;
	//Velocity = FVector(100.f, 100.f, 0.f);
	FVector Start = this->GetActorLocation();
	FVector End = (Velocity * DeltaTime) + Start;
	End.Z += this->GetActorRotation().Pitch;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	//UE_LOG(LogTemp, Warning, TEXT("Start:%s/nEnd:%s"),*(Start.ToString()),*(End.ToString()));
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		if (Hit.GetActor())
		{
			//ADestructibleActor Mesh = Cast<ADestructibleActor>(Hit.GetActor());

			//if(Mesh)
			//{
			//	Mesh->GetDestructibleComponent()->AppyRadiousDamage(10.0f, Hit.ImpactPoint, 32.f, 10.f, false);
			//}
			//
			//UE_LOG(LogTemp, Warning, TEXT("Hello"));
		//	Destroy();

		}
		//UE_LOG(LogTemp, Warning, TEXT("End"));
	}
	else
	{
		BulletExpiry += DeltaTime;
		DrawDebugLine(GetWorld(), Start, End, FColor(0.f, -BulletExpiry * 80.f, 100.f), true);
		SetActorLocation(End);
		Velocity += FVector(0.f, 0.f, -20.f) * DeltaTime;

	}
	if (BulletExpiry > 3)
	{
		//Destroy();
	}
}

