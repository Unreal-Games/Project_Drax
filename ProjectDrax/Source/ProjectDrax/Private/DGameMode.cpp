// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameMode.h"
#include "UDHealthComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "TimerManager.h"

void ADGameMode::SpawnBotTimerElapsed()
{
	SpawnNewBot();
	NoOfBotToSpawn--;
	if(NoOfBotToSpawn<=0)
	{
		EndWave();
	}
}

void ADGameMode::StartWave()
{
	WaveCount++;
	
	NoOfBotToSpawn = 2 * WaveCount;
	
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ADGameMode::SpawnNewBot, 1.f, true, 0.f);
}

void ADGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
}
 
void ADGameMode::PrepareNextWave()
{
	
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this,&ADGameMode::StartWave, TimeBetweenWaves, false);
}

void ADGameMode::CheckWaveState()
{
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	if(NoOfBotToSpawn>0||bIsPreparingForWave)
	{
		return; 
	}
	bool bIsAnyBotAlive = false;
	for(FConstPawnIterator It=GetWorld()->GetPawnIterator();It;It++)
	{
		APawn* TestPawn = It->Get();
		if(TestPawn==nullptr|| TestPawn->IsPlayerControlled())
		{
			continue;
		}
		UUDHealthComponent* HealthComp = Cast<UUDHealthComponent>(TestPawn->GetComponentByClass(UUDHealthComponent::StaticClass()));
		if(HealthComp&&HealthComp->GetHealth()>0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}
	if(!bIsAnyBotAlive)
	{
		PrepareNextWave();
	}
}

ADGameMode::ADGameMode()
{
	TimeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ADGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareNextWave();
}

void ADGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();
} 
