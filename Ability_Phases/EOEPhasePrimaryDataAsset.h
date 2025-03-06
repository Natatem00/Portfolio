// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystem/EOEAbilityData.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"

#include "EOEPhasePrimaryDataAsset.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FEOEPhaseData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EOEAbilityInput AbilityInput;

	virtual void ApplyToAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const;
	virtual void RemoveFromAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const;

	virtual ~FEOEPhaseData() = default;

protected:
	mutable TArray<FGameplayAbilitySpec, TInlineAllocator<4>> RemovedAbilities;
};

USTRUCT(BlueprintType)
struct FEOEPhaseDataWithAbility : public FEOEPhaseData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityToGive;

	virtual void ApplyToAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const override;
};

/**
 *
 */
UCLASS()
class GAMECORE_API UEOEPhasePrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual void ApplyToAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const;
	virtual void RemoveFromAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BaseStruct = "EOEPhaseData", AllowPrivateAccess = "true"))
	TArray<FInstancedStruct> AbilitiesData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer AbilityTagsToBlock;
};
