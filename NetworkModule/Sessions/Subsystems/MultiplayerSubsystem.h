// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "MultiplayerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDragoncallOnSessionCreatedSignature, bool, bSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDragoncallOnFindSessionsSignature, const TArray<FOnlineSessionSearchResult>&, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDragoncallOnJoinSessionSignature, const EOnJoinSessionCompleteResult::Type, const FString&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDragoncallOnDestroySessionSignature, bool, bSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDragoncallOnStartSessionSignature, bool, bSuccessful);
/**
 *
 */
UCLASS()
class DRAGONCALLNETWORK_API UDragoncallMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDragoncallMultiplayerSubsystem();

	void CreateSession(const uint16 NumPublicConnections, const FString& InMatchType);
	void FindSession(const uint16 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& InSessionResult);
	void DestroySession();
	void StartSession();

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer Delegates")
	FDragoncallOnSessionCreatedSignature OnSessionCreatedDelegate;

	FDragoncallOnFindSessionsSignature OnFindSessionsDelegate;

	FDragoncallOnJoinSessionSignature OnJoinSessionDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer Delegates")
	FDragoncallOnDestroySessionSignature OnDestroySessionDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer Delegates")
	FDragoncallOnStartSessionSignature OnStartSessionDelegate;

private:
	void OnCreateSessionComplete(const FName SessionName, const bool bSuccessful);
	void OnFindSessionsComplete(const bool bSuccessful);
	void OnJoinSessionComplete(const FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(const FName SessionName, const bool bSuccessful);
	void OnStartSessionComplete(const FName SessionName, const bool bSuccessful);

private:
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FDelegateHandle OnCreateSessionCompleteDelegate_Handle;

	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegate_Handle;

	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegate_Handle;

	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FDelegateHandle OnDestroySessionCompleteDelegate_Handle;

	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FDelegateHandle OnStartSessionCompleteDelegate_Handle;

	FOnlineSessionSettings OnlineSessionSetting;

	TSharedPtr<FOnlineSessionSearch> OnlineSessionSearch;
	TWeakPtr<IOnlineSession> OnlineSessionPtr;
};
