// Fill out your copyright notice in the Description page of Project Settings.

#include "ServerConfigHelpers.h"

#include "DragoncallNetwork.h"
void DC::Config::EnsureBackendConfig()
{
	static const FString ConfigFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config/WindowsServer/BackendSettings.ini"));

	if (!FPaths::FileExists(ConfigFilePath))
	{
		const FString DefaultConfigContent = TEXT(
			"[/Script/DragoncallNetwork.DragoncallBackendSettings]\n"
			"ServerRegion="
			"\n");

		if (FFileHelper::SaveStringToFile(DefaultConfigContent, *ConfigFilePath))
		{
			UE_LOG(LogDragoncallNetwork, Log, TEXT("Config file created: %s"), *ConfigFilePath);
		}
		else
		{
			UE_LOG(LogDragoncallNetwork, Error, TEXT("Failed to create config file: %s"), *ConfigFilePath);
		}
	}
	else
	{
		UE_LOG(LogDragoncallNetwork, Log, TEXT("Config file already exists: %s"), *ConfigFilePath);
	}
}