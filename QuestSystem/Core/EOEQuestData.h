// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "EOEQuestData.generated.h"

class AEOEBaseQuest;
class IEOEQuestSkipCondition;

UENUM(BlueprintType)
enum class EEOEQuestStatus : uint8
{
	NOT_ACTIVE UMETA(DisplayName = "NOT ACTIVE"),
	STARTED UMETA(DisplayName = "STARTED"),
	IN_PROGRESS UMETA(DisplayName = "IN PROGRESS"),
	FINISHED UMETA(DisplayName = "FINISHED"),
	CANCELED UMETA(DisplayName = "CANCELED")
};

USTRUCT(BlueprintType)
struct FEOEQuestObjectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ObjectTag;

	virtual bool CheckQuestObjectData(const FEOEQuestObjectData& Object) const;

	virtual bool ValidateQuestObejct() const;

	virtual FString ToString() const;

	virtual ~FEOEQuestObjectBase() = default;
};

USTRUCT(BlueprintType)
struct FEOEQuestObjectWithActor : public FEOEQuestObjectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<AActor> Actor;

	virtual bool CheckQuestObjectData(const FEOEQuestObjectData& Object) const;

	virtual bool ValidateQuestObejct() const;

	virtual FString ToString() const;
};

USTRUCT(BlueprintType)
struct FEOEQuestObjectData
{
	GENERATED_BODY()

	FName ObjectTag;
	TWeakObjectPtr<UObject> Object;
	uint8 bShouldCheckTag : 1;
};

USTRUCT(BlueprintType)
struct FEOEQuestData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<AEOEBaseQuest> Quest;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag QuestTag;

	UPROPERTY(BlueprintReadOnly)
	EEOEQuestStatus QuestStatus;

	FEOEQuestObjectBase QuestObject;
};