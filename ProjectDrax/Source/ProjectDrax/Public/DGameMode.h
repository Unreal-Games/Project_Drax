// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTDRAX_API ADGameMode : public AGameModeBase
{
	GENERATED_BODY()
protected:
	FTimerHandle TimerHandle_BotSpawner;
	FTimerHandle TimerHandle_NextWaveStart;


	int32 NoOfBotToSpawn;
	int32 WaveCount;
	UPROPERTY(EditDefaultsOnly,Category="GameMode")
	float TimeBetweenWaves;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
		void SpawnNewBot();

	void SpawnBotTimerElapsed();
	
	void StartWave();

	void EndWave();

	void PrepareNextWave();

	void CheckWaveState();

public:

	ADGameMode();
	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;
};
