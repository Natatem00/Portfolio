// Fill out your copyright notice in the Description page of Project Settings.

#include "EOEQuestTriggerBoxBase.h"

#include "Components/EOEQuestObjectComponent.h"
AEOEQuestTriggerBoxBase::AEOEQuestTriggerBoxBase()
{
	QuestObjectComponent = CreateDefaultSubobject<UEOEQuestObjectComponent>("QuestObjectComponent");

	bTriggeredByPlayer = true;
}
void AEOEQuestTriggerBoxBase::SetTriggerActor(AActor* InTriggerActor)
{
	TriggerActor = InTriggerActor;
}
bool AEOEQuestTriggerBoxBase::CanFinishQuestObject(const AActor* InNotifyActor) const
{
	return TriggerActor.Get() == InNotifyActor && QuestObjectComponent;
}
void AEOEQuestTriggerBoxBase::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld())
		return;

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();

	if (bTriggeredByPlayer && playerController)
	{
		SetTriggerActor(GetWorld()->GetFirstPlayerController()->GetPawn());
	}
}
