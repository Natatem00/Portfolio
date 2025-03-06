// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

#include "EOEQuestObjectComponent.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(FEOEMessage_Quest_Object_Finished);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FEOEMessage_Quest_Object_Canceled);

UENUM(BlueprintType)
enum class EEOEQuestActorState : uint8
{
	UNACTIVE UMETA(DisplayName = "UNACTIVE"),
	ACTIVE UMETA(DisplayName = "ACTIVE"),
	SKIPED UMETA(DisplayName = "SKIPED"),
	FINISHED UMETA(DisplayName = "FINISHED"),
	CANCELED UMETA(DisplayName = "CANCELED")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestActivatedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class QUESTSYSTEM_API UEOEQuestObjectComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class AEOEBaseQuest;

public:
	UEOEQuestObjectComponent();

	UFUNCTION(BlueprintCallable, Category = "Quest Object")
	void FinishQuestActor(const bool ShouldCheckTag = false, const FName ObjectTag = FName(""));

	const EEOEQuestActorState& GetQuestActorState() const;

	UFUNCTION(BlueprintPure, Category = "Quest Object")
	bool IsQuestObjectActive() const;

private:
	void ActivateQuestActor();

public:
	UPROPERTY(BlueprintAssignable)
	FOnQuestActivatedSignature OnQuestActivatedDelegate;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EEOEQuestActorState QuestActorState = EEOEQuestActorState::UNACTIVE;
};
