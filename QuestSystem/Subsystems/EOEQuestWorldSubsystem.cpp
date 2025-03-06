// Fill out your copyright notice in the Description page of Project Settings.

#include "EOEQuestWorldSubsystem.h"

#include "Components/EOEQuestObjectComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Quests/EOEBaseQuest.h"

class UGameplayMessageSubsystem;
void UEOEQuestWorldSubsystem::AddQuest(const FGameplayTag& InQuestTage, AEOEBaseQuest* InQuest)
{
	if (Quests.Contains(InQuestTage) == false)
	{
		Quests.Add(InQuestTage, InQuest);
	}
}
void UEOEQuestWorldSubsystem::RemoveQuest(const FGameplayTag& InQuestTage)
{
	if (Quests.Contains(InQuestTage) == true)
	{
		Quests.Remove(InQuestTage);
	}
}
bool UEOEQuestWorldSubsystem::TryActivateQuest(const FGameplayTag& InQuestTage)
{
	if (Quests.Contains(InQuestTage) == true)
	{
		return Quests[InQuestTage]->TryStartQuest();
	}
	return false;
}
void UEOEQuestWorldSubsystem::FinishQuestObject(const FName ObjectTag) const
{
	UGameplayMessageSubsystem& messageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());

	FEOEQuestObjectData questObjectData;
	questObjectData.Object = nullptr;
	questObjectData.bShouldCheckTag = true;
	questObjectData.ObjectTag = ObjectTag;

	messageSubsystem.BroadcastMessage<FEOEQuestObjectData>(FEOEMessage_Quest_Object_Finished, questObjectData);
}