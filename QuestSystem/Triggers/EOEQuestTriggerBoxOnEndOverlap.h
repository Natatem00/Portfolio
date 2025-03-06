// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Triggers/EOEQuestTriggerBoxBase.h"

#include "EOEQuestTriggerBoxOnEndOverlap.generated.h"

/**
 *
 */
UCLASS()
class QUESTSYSTEM_API AEOEQuestTriggerBoxOnEndOverlap : public AEOEQuestTriggerBoxBase
{
	GENERATED_BODY()

protected:
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
};
