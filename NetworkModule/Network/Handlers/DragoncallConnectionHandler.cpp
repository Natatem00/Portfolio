// Fill out your copyright notice in the Description page of Project Settings.

#include "Network/Handlers/DragoncallConnectionHandler.h"

#include "Helpers/JsonHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Network/DragoncallBackendSettings.h"
#include "Network/DragoncallBackendSubsystem.h"

const FString REQUEST = FString("api/v1/UserInfo/UserConnection");

void FDragoncallConnectionHandler::SendPlayerConnectedToServer(const UObject* InWorldContextObject, const FString& PlayerName, const FString& PlayerID, const uint8 TotalOnlineNum) const
{
	if (const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(InWorldContextObject))
	{
		const FDragoncallUserConnectionModel connectionModel = SetupConnectionModel(PlayerName, PlayerID, TotalOnlineNum, true);

		SendConnectionModel(connectionModel);
	}
}

void FDragoncallConnectionHandler::SendPlayerDisconnectedToServer(const UObject* InWorldContextObject, const FString& PlayerName, const FString& PlayerID, const uint8 TotalOnlineNum) const
{
	if (const UGameInstance* gameInstance = UGameplayStatics::GetGameInstance(InWorldContextObject))
	{
		const FDragoncallUserConnectionModel connectionModel = SetupConnectionModel(PlayerName, PlayerID, TotalOnlineNum, false);

		SendConnectionModel(connectionModel);
	}
}

FDragoncallUserConnectionModel FDragoncallConnectionHandler::SetupConnectionModel(const FString& PlayerName, const FString& PlayerID, const uint8 TotalOnlineNum, const bool bIsOnline)
{
	FDragoncallUserConnectionModel connectionModel;
	connectionModel.region = UDragoncallBackendSettings::GetSettings()->GetServerRegion();
	connectionModel.isConnected = bIsOnline;
	connectionModel.totalOnline = TotalOnlineNum;
	connectionModel.userName = PlayerName;
	connectionModel.dcUserId = PlayerID;

	return connectionModel;
}

void FDragoncallConnectionHandler::SendConnectionModel(const FDragoncallUserConnectionModel& InConnectionModel)
{
	UDragoncallBackendSubsystem* backendSubsystem = gameInstance->GetSubsystem<UDragoncallBackendSubsystem>();
	FString jsonOutput;
	DC::Json::GetJsonStringFromStruct<FDragoncallUserConnectionModel>(InConnectionModel, jsonOutput);
	backendSubsystem->Send(backendSubsystem->PostRequest(REQUEST, jsonOutput, EBackendService::Dev));
}