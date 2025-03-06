// Fill out your copyright notice in the Description page of Project Settings.

#include "EOEQuestObjectComponent.h"

#include "Core/EOEQuestData.h"
#include "GameFramework/GameplayMessageSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG(FEOEMessage_Quest_Object_Finished, "Message.Quest.Object.Finished");
UE_DEFINE_GAMEPLAY_TAG(FEOEMessage_Quest_Object_Canceled, "Message.Quest.Object.Canceled");

UEOEQuestObjectComponent::UEOEQuestObjectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UEOEQuestObjectComponent::ActivateQuestActor()
{
	QuestActorState = EEOEQuestActorState::ACTIVE;

	OnQuestActivatedDelegate.Broadcast();
}
void UEOEQuestObjectComponent::FinishQuestActor(const bool ShouldCheckTag, const FName ObjectTag)
{
	if (!IsQuestObjectActive())
		return;

	QuestActorState = EEOEQuestActorState::FINISHED;

	UGameplayMessageSubsystem& messageSubsystem = UGameplayMessageSubsystem::Get(GetOwner());

	FEOEQuestObjectData questObjectData;
	questObjectData.Object = GetOwner();
	questObjectData.bShouldCheckTag = ShouldCheckTag;
	questObjectData.ObjectTag = ObjectTag;

	messageSubsystem.BroadcastMessage<FEOEQuestObjectData>(FEOEMessage_Quest_Object_Finished, questObjectData);
}
const EEOEQuestActorState& UEOEQuestObjectComponent::GetQuestActorState() const
{
	return QuestActorState;
}
bool UEOEQuestObjectComponent::IsQuestObjectActive() const
{
	return QuestActorState == EEOEQuestActorState::ACTIVE;
}
