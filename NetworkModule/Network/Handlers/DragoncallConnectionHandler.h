// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Network/Handlers/DragoncallBackendHandlerBase.h"

#include "DragoncallConnectionHandler.generated.h"

/**
 *
 */

USTRUCT()
struct FDragoncallUserConnectionModel
{
	GENERATED_BODY()
	UPROPERTY()
	FString dcUserId;
	UPROPERTY()
	FString userName;
	UPROPERTY()
	FString region;
	UPROPERTY()
	uint8 isConnected : 1;
	UPROPERTY()
	uint8 totalOnline;
};

USTRUCT()
struct DRAGONCALLNETWORK_API FDragoncallConnectionHandler : public FDragoncallBackendHandlerBase
{
	GENERATED_BODY()
public:
	void SendPlayerConnectedToServer(const UObject* InWorldContextObject, const FString& PlayerName, const FString& PlayerID, const uint8 TotalOnlineNum) const;

	void SendPlayerDisconnectedToServer(const UObject* InWorldContextObject, const FString& PlayerName, const FString& PlayerID, const uint8 TotalOnlineNum) const;

protected:
	static FDragoncallUserConnectionModel SetupConnectionModel(const FString& PlayerName, const FString& PlayerID, uint8 TotalOnlineNum, const bool bIsOnline);
	static void SendConnectionModel(const FDragoncallUserConnectionModel& InConnectionModel);
};
