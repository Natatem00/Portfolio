// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSubsystem.h"

#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystem.h"

UDragoncallMultiplayerSubsystem::UDragoncallMultiplayerSubsystem()
	: OnCreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)), OnFindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)), OnJoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)), OnDestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)), OnStartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();

	if (onlineSubsystem != nullptr)
	{
		OnlineSessionPtr = onlineSubsystem->GetSessionInterface();

		OnlineSessionSearch = MakeShareable(new FOnlineSessionSearch());
	}
}

void UDragoncallMultiplayerSubsystem::CreateSession(const uint16 NumPublicConnections, const FString& InMatchType)
{
	if (OnlineSessionPtr.IsValid())
	{
		const TSharedPtr onlineSession = OnlineSessionPtr.Pin();
		if (onlineSession->GetNamedSession(NAME_GameSession))
		{
			onlineSession->DestroySession(NAME_GameSession);
		}

		OnCreateSessionCompleteDelegate_Handle = onlineSession->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);

		OnlineSessionSetting.bAllowJoinInProgress = true;
		OnlineSessionSetting.bAllowJoinViaPresence = true;
		OnlineSessionSetting.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == TEXT("NULL");
		OnlineSessionSetting.bShouldAdvertise = true;
		OnlineSessionSetting.bUseLobbiesIfAvailable = true;
		OnlineSessionSetting.bUsesPresence = true;
		OnlineSessionSetting.NumPublicConnections = NumPublicConnections;
		OnlineSessionSetting.Set(FName(TEXT("MatchType")), InMatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!onlineSession->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, OnlineSessionSetting))
		{
			onlineSession->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate_Handle);
		}
	}
}

void UDragoncallMultiplayerSubsystem::FindSession(const uint16 MaxSearchResults)
{
	if (OnlineSessionPtr.IsValid() && OnlineSessionSearch)
	{
		const TSharedPtr onlineSession = OnlineSessionPtr.Pin();
		OnlineSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == TEXT("NULL");
		OnlineSessionSearch->MaxSearchResults = MaxSearchResults;
		OnlineSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

		OnFindSessionsCompleteDelegate_Handle = onlineSession->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

		ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!onlineSession->FindSessions(*localPlayer->GetPreferredUniqueNetId(), OnlineSessionSearch.ToSharedRef()))
		{
			onlineSession->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate_Handle);
			OnFindSessionsDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		}
	}
}

void UDragoncallMultiplayerSubsystem::JoinSession(const FOnlineSessionSearchResult& InSessionResult)
{
	if (OnlineSessionPtr.IsValid())
	{
		const TSharedPtr onlineSession = OnlineSessionPtr.Pin();
		OnJoinSessionCompleteDelegate_Handle = onlineSession->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!onlineSession->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, InSessionResult))
		{
			onlineSession->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate_Handle);
			OnJoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError, TEXT(""));
		}
	}
}

void UDragoncallMultiplayerSubsystem::DestroySession()
{
}

void UDragoncallMultiplayerSubsystem::StartSession()
{
}

void UDragoncallMultiplayerSubsystem::OnCreateSessionComplete(const FName SessionName, const bool bSuccessful)
{
	if (OnlineSessionPtr.IsValid())
	{
		const TSharedPtr onlineSession = OnlineSessionPtr.Pin();
		onlineSession->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate_Handle);
	}
	OnSessionCreatedDelegate.Broadcast(bSuccessful);
}

void UDragoncallMultiplayerSubsystem::OnFindSessionsComplete(const bool bSuccessful)
{
	if (OnlineSessionPtr.IsValid() && OnlineSessionSearch)
	{
		const TSharedPtr onlineSession = OnlineSessionPtr.Pin();
		onlineSession->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate_Handle);

		OnFindSessionsDelegate.Broadcast(OnlineSessionSearch->SearchResults, bSuccessful);
		return;
	}
	OnFindSessionsDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
}

void UDragoncallMultiplayerSubsystem::OnJoinSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (OnlineSessionPtr.IsValid())
	{
		const TSharedPtr onlineSession = OnlineSessionPtr.Pin();
		onlineSession->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate_Handle);
		FString connectionInfo;

		onlineSession->GetResolvedConnectString(SessionName, connectionInfo);
		OnJoinSessionDelegate.Broadcast(Result, connectionInfo);
	}

	OnJoinSessionDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError, TEXT(""));
}

void UDragoncallMultiplayerSubsystem::OnDestroySessionComplete(const FName SessionName, const bool bSuccessful)
{
}

void UDragoncallMultiplayerSubsystem::OnStartSessionComplete(const FName SessionName, const bool bSuccessful)
{
}
