// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameMode.h"

#include "SPlayerState.h"
#include "UDHealthComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SGameState.h"
#include "TimerManager.h"


ADGameMode::ADGameMode()
{
	TimeBetweenWaves = 2.0f;

	PlayerStateClass = ASPlayerState::StaticClass();
	GameStateClass = ASGameState::StaticClass();
	//TimeBetweenWaves = 20.f;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
}

void ADGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckWaveState();
	CheckAnyPlayerAlive();
}
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

	SetWaveState(EWaveState::WaveInProgress);
}

void ADGameMode::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	SetWaveState(EWaveState::WaitingToComplete);
}
 
void ADGameMode::PrepareNextWave()
{
	
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this,&ADGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

	RestartDeadPlayers();
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
		SetWaveState(EWaveState::WaveComplete);

		PrepareNextWave();
	}
}

void ADGameMode::CheckAnyPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			UUDHealthComponent* HealthComp = Cast<UUDHealthComponent>(MyPawn->GetComponentByClass(UUDHealthComponent::StaticClass()));
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				// A player is still alive.
				return;
			}
		}
	}

	// No player alive
	GameOver();
}

void ADGameMode::GameOver()
{
	EndWave();

	// @TODO: Finish up the match, present 'game over' to players.

	SetWaveState(EWaveState::GameOver);

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died"));
}

void ADGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ADGameMode::RestartDeadPlayers()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}
}


void ADGameMode::StartPlay()
{
	Super::StartPlay();

	PrepareNextWave();
}

