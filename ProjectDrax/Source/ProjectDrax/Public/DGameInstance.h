// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
//#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "DGameInstance.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
		FString ServerName;
	UPROPERTY(BlueprintReadOnly)
		FString PlayerCountStr;
	
	UPROPERTY(BlueprintReadOnly)
		int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly)
		int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly)
		int32 ServerArrayIndex;

	void SetPlayerCount()
	{
		PlayerCountStr=FString(FString::FromInt(CurrentPlayers) + "/" + FString::FromInt(MaxPlayers));
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerDel, FServerInfo, ServerListDel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerSearchDel, bool, SearchingForServers);

UCLASS()
class PROJECTDRAX_API UDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDGameInstance();

protected:

	FName MySessionName;
	
	UPROPERTY(BlueprintAssignable)
		FServerDel ServerListDel;
	UPROPERTY(BlueprintAssignable)
		FServerSearchDel SearchForServers;
	

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSeach;
	
	UFUNCTION(BlueprintCallable)
		void CreateServer(FString ServerName, FString HostName);

	UFUNCTION(BlueprintCallable)
		void FindServer();


	UFUNCTION(BlueprintCallable)
		void JoinServer(int32 ServerIndex);

	virtual void OnCreateSessionComplete(FName SessionName,bool Succeeded);
	virtual void OnFindSessionComplete(bool Suceeded);
	virtual void OnJoinSessionComplete(FName SessionName,EOnJoinSessionCompleteResult::Type Result);
	
	void Init() override;
};
