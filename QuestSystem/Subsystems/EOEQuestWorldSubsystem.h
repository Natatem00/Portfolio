// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "EOEQuestWorldSubsystem.generated.h"

class AEOEBaseQuest;
/**
 *
 */
UCLASS()
class QUESTSYSTEM_API UEOEQuestWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	friend class AEOEBaseQuest;

public:
	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
	bool TryActivateQuest(const FGameplayTag& InQuestTage);

	UFUNCTION(BlueprintCallable, Category = "QuestSystem")
	void FinishQuestObject(const FName ObjectTag) const;

private:
	void AddQuest(const FGameplayTag& InQuestTage, AEOEBaseQuest* InQuest);

	void RemoveQuest(const FGameplayTag& InQuestTage);

private:
	UPROPERTY()
	TMap<FGameplayTag, AEOEBaseQuest*> Quests;
};
