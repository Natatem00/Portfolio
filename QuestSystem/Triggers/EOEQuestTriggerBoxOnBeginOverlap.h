// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Triggers/EOEQuestTriggerBoxBase.h"

#include "EOEQuestTriggerBoxOnBeginOverlap.generated.h"

/**
 *
 */
UCLASS()
class QUESTSYSTEM_API AEOEQuestTriggerBoxOnBeginOverlap : public AEOEQuestTriggerBoxBase
{
	GENERATED_BODY()

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
