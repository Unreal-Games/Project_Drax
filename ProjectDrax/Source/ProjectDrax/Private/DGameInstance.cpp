// Fill out your copyright notice in the Description page of Project Settings.


#include "DGameInstance.h"

#include "OnlineSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"

UDGameInstance::UDGameInstance()
{
	MySessionName = FName("My Session");
}

void UDGameInstance::Init()
{
	if (IOnlineSubsystem * SubSystem = IOnlineSubsystem::Get())
	{
		SessionInterface = SubSystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			//Bind Delegates here
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UDGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UDGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UDGameInstance::OnJoinSessionComplete);
		}
	}
}

void UDGameInstance::CreateServer(FString ServerName,FString HostName)
{
	UE_LOG(LogTemp, Warning, TEXT("Create Server "))

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = true;
	if(IOnlineSubsystem::Get()->GetSubsystemName()!="NULL")
	{
		SessionSettings.bIsLANMatch = false;
	}
	else
		SessionSettings.bIsLANMatch = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 5;
	SessionSettings.Set(FName("SERVER_NAME_KEY"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings.Set(FName("SERVER_HOSTNAME_KEY"), HostName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	
	SessionInterface->CreateSession(0, MySessionName, SessionSettings);
}

void UDGameInstance::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionComplete :%d"), Succeeded)
		if (Succeeded)
		{
			//UGameplayStatics::OpenLevel()
			GetWorld()->ServerTravel("/Game/Maps/TestMap?listen");
		}
}

void UDGameInstance::FindServer()
{
	SearchForServers.Broadcast(true);
	UE_LOG(LogTemp, Warning, TEXT("Join Server "))

	SessionSeach = MakeShareable(new FOnlineSessionSearch());
	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
	{
		SessionSeach->bIsLanQuery = false;
	}
	else
		SessionSeach->bIsLanQuery = true;
	SessionSeach->MaxSearchResults = 10000;
	SessionSeach->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	
	SessionInterface->FindSessions(0,SessionSeach.ToSharedRef());
	
}


void UDGameInstance::OnFindSessionComplete(bool Suceeded)
{

	SearchForServers.Broadcast(false);

	
	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionComplete :%d"), Suceeded)
	if(Suceeded)
	{
		int32 ArrayIndex = -1;
		for(FOnlineSessionSearchResult Result: SessionSeach->SearchResults)
		{
			ArrayIndex++;
			if (!Result.IsValid())
			{
				continue;
			}
			FServerInfo Info;
			FString ServerName = "Empty Server Name";
			FString HostName = "Empty Host Name";

			Result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName);
			Result.Session.SessionSettings.Get(FName("SERVER_HOSTNAME_KEY"), HostName);

			
			Info.ServerName = ServerName;
			Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;
			Info.SetPlayerCount();
			Info.ServerArrayIndex = ArrayIndex;
			
			ServerListDel.Broadcast(Info);
			
		}
		UE_LOG(LogTemp, Warning, TEXT("Search count :%d"), SessionSeach->SearchResults.Num());
		
	}
}

void UDGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("Join Server Complete!! %s"),*SessionName.ToString())
	
	if(APlayerController* PController= UGameplayStatics::GetPlayerController(GetWorld(),0))
	{
		FString JoinAddress="";
		UE_LOG(LogTemp,Warning,TEXT("Controller:%s"),*PController->GetName())
		SessionInterface->GetResolvedConnectString(SessionName,JoinAddress);
		UE_LOG(LogTemp, Warning, TEXT("Join Address!! %s"), *JoinAddress)

		//if(JoinAddress!="")
			
			PController->ClientTravel(JoinAddress,ETravelType::TRAVEL_Absolute);
		UE_LOG(LogTemp, Warning, TEXT("Joined Server "))
	}
}

void UDGameInstance::JoinServer(int32 ServerIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Joining Server %d"),ServerIndex)
	FOnlineSessionSearchResult Result = SessionSeach->SearchResults[ServerIndex];
	if (Result.IsValid())
	{
		SessionInterface->JoinSession(0, MySessionName, Result);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Unable to Join Server invalid server"))

}


