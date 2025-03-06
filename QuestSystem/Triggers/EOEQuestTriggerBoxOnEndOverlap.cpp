// Fill out your copyright notice in the Description page of Project Settings.

#include "Triggers/EOEQuestTriggerBoxOnEndOverlap.h"

#include "Components/EOEQuestObjectComponent.h"

void AEOEQuestTriggerBoxOnEndOverlap::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (CanFinishQuestObject(OtherActor))
	{
		QuestObjectComponent->FinishQuestActor();
	}
}