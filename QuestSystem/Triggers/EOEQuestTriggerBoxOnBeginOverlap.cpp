// Fill out your copyright notice in the Description page of Project Settings.

#include "Triggers/EOEQuestTriggerBoxOnBeginOverlap.h"

#include "Components/EOEQuestObjectComponent.h"

void AEOEQuestTriggerBoxOnBeginOverlap::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (CanFinishQuestObject(OtherActor))
	{
		QuestObjectComponent->FinishQuestActor();
	}
}