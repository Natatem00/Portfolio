// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "UObject/Interface.h"

#include "EOEQuestActor.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEOEQuestSkipCondition : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class QUESTSYSTEM_API IEOEQuestSkipCondition
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanSkipQuestActor();
};
