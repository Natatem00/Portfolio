// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "NativeGameplayTags.h"
#include "QuestSystem/Core/EOEQuestData.h"

#include "EOEBaseQuest.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(FEOEMessage_Quest_Update_Status);
/**
 *
 */

class UEOEQuestObjectComponent;

UCLASS()
class QUESTSYSTEM_API AEOEBaseQuest : public AInfo
{
	GENERATED_BODY()
	friend class UEOEQuestWorldSubsystem;

public:
	bool IsQuestCanceled() const;
	bool IsQuestFinished() const;

protected:
	virtual void BeginPlay() override;
	void ActivateQuestObject();

private:
	bool TryStartQuest();
	void FinishQuest();

	void OnFinishedQuestObject(const FGameplayTag MessageTag, const FEOEQuestObjectData& ObjectData);

	bool UpdateNextQuestObejctIndex();

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true", BaseStruct = "EOEQuestObjectBase"))
	TArray<FInstancedStruct> QuestObjects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	EEOEQuestStatus QuestStatus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	FGameplayTag QuestTag;

	int8 CurrentQuestObjectIndex = -1;

	FEOEQuestData QuestData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess = "true"))
	float TimeToDisplay = 60.f;
};
