// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "QuestSystem/Core/Interfaces/EOEQuestActor.h"

#include "EOEQuestTriggerBoxBase.generated.h"

class UEOEQuestObjectComponent;
/**
 *
 */
UCLASS()
class QUESTSYSTEM_API AEOEQuestTriggerBoxBase : public ATriggerBox
{
	GENERATED_BODY()
public:
	AEOEQuestTriggerBoxBase();

	UFUNCTION(BlueprintCallable)
	void SetTriggerActor(AActor* InTriggerActor);

protected:
	bool CanFinishQuestObject(const AActor* InNotifyActor) const;

	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UEOEQuestObjectComponent> QuestObjectComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = TriggerBox)
	uint8 bTriggeredByPlayer : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TriggerBox, meta = (AllowPrivateAccess = "true", EditCondition = "bTriggeredByPlayer == false"))
	TSoftObjectPtr<AActor> TriggerActor;
};
