// Fill out your copyright notice in the Description page of Project Settings.

#include "EOEBaseQuest.h"

#include "Components/EOEQuestObjectComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "QuestSystem/Core/Interfaces/EOEQuestActor.h"
#include "QuestSystem/Subsystems/EOEQuestWorldSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG(FEOEMessage_Quest_Update_Status, "Message.Quest.Status.Update")

void AEOEBaseQuest::BeginPlay()
{
	Super::BeginPlay();

	if (IsQuestCanceled() == false && IsQuestFinished() == false)
	{
		if (UEOEQuestWorldSubsystem* questSubsystem = GetWorld()->GetSubsystem<UEOEQuestWorldSubsystem>())
		{
			questSubsystem->AddQuest(QuestTag, this);
		}
	}
}

bool AEOEBaseQuest::TryStartQuest()
{
	if (QuestStatus == EEOEQuestStatus::NOT_ACTIVE)
	{
		if (UpdateNextQuestObejctIndex())
		{
			QuestStatus = EEOEQuestStatus::STARTED;

			QuestData.Quest = this;
			QuestData.QuestStatus = QuestStatus;
			QuestData.QuestTag = QuestTag;
			ActivateQuestObject();

			UGameplayMessageSubsystem& messageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			messageSubsystem.RegisterListener<FEOEQuestObjectData>(FEOEMessage_Quest_Object_Finished, this, &AEOEBaseQuest::OnFinishedQuestObject);

			QuestStatus = EEOEQuestStatus::IN_PROGRESS;
			QuestData.QuestStatus = QuestStatus;
			messageSubsystem.BroadcastMessage<FEOEQuestData>(FEOEMessage_Quest_Update_Status, QuestData);

			GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, FColor::Red, FString::Printf(TEXT("Quest started! with tag = %s, next quest object = %s"), *QuestTag.ToString(), *QuestData.QuestObject.ToString()));
			return true;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, FColor::Red, FString::Printf(TEXT("Can't start quest with tag = %s!"), *QuestTag.ToString()));
	return false;
}
void AEOEBaseQuest::FinishQuest()
{
	UGameplayMessageSubsystem& messageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	QuestStatus = EEOEQuestStatus::FINISHED;
	QuestData.QuestStatus = QuestStatus;
	QuestData.QuestObject = FEOEQuestObjectBase();
	messageSubsystem.BroadcastMessage<FEOEQuestData>(FEOEMessage_Quest_Update_Status, QuestData);

	if (UEOEQuestWorldSubsystem* questSubsystem = GetWorld()->GetSubsystem<UEOEQuestWorldSubsystem>())
	{
		questSubsystem->RemoveQuest(QuestTag);
	}

	GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, FColor::Red, FString::Printf(TEXT("Quest FINISHED!")));
}
bool AEOEBaseQuest::IsQuestCanceled() const
{
	return QuestStatus == EEOEQuestStatus::CANCELED;
}
bool AEOEBaseQuest::IsQuestFinished() const
{
	return QuestStatus == EEOEQuestStatus::FINISHED;
}
void AEOEBaseQuest::ActivateQuestObject()
{
	UGameplayMessageSubsystem& messageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());

	QuestData.QuestObject = QuestObjects[CurrentQuestObjectIndex].Get<FEOEQuestObjectBase>();

	if (FEOEQuestObjectWithActor* questObjectWithActor = QuestObjects[CurrentQuestObjectIndex].GetMutablePtr<FEOEQuestObjectWithActor>())
	{
		questObjectWithActor->Actor.Get()->GetComponentByClass<UEOEQuestObjectComponent>()->ActivateQuestActor();
	}

	messageSubsystem.BroadcastMessage<FEOEQuestData>(FEOEMessage_Quest_Update_Status, QuestData);
}
void AEOEBaseQuest::OnFinishedQuestObject(const FGameplayTag MessageTag, const FEOEQuestObjectData& ObjectData)
{
	if (QuestObjects.IsValidIndex(CurrentQuestObjectIndex) && QuestObjects[CurrentQuestObjectIndex].Get<FEOEQuestObjectBase>().CheckQuestObjectData(ObjectData))
	{
		if (UpdateNextQuestObejctIndex())
		{
			ActivateQuestObject();
			GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, FColor::Red, FString::Printf(TEXT("Quest updated! next quest object = %s"), *QuestData.QuestObject.ToString()));
		}
		else
		{
			FinishQuest();
		}
	}
}
bool AEOEBaseQuest::UpdateNextQuestObejctIndex()
{
	CurrentQuestObjectIndex++;

	if (QuestObjects.Num() <= CurrentQuestObjectIndex || !QuestObjects[CurrentQuestObjectIndex].IsValid())
	{
		CurrentQuestObjectIndex = -1;
		return false;
	}

	const FEOEQuestObjectBase& questObjectData = QuestObjects[CurrentQuestObjectIndex].Get<FEOEQuestObjectBase>();

	if (!questObjectData.ValidateQuestObejct())
	{
		return UpdateNextQuestObejctIndex();
	}

	return true;
}