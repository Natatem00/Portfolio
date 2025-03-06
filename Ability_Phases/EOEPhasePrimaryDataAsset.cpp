// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Phases/EOEPhasePrimaryDataAsset.h"

#include "AbilitySystemComponent.h"

void FEOEPhaseData::ApplyToAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (AbilitySystemComponent)
	{
		RemovedAbilities.Reset();
		for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.InputID == static_cast<int32>(AbilityInput))
			{
				RemovedAbilities.Add(Spec);
			}
		}
		AbilitySystemComponent->ClearAllAbilitiesWithInputID(static_cast<int32>(AbilityInput));
	}
}
void FEOEPhaseData::RemoveFromAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ClearAllAbilitiesWithInputID(static_cast<int32>(AbilityInput));
		for (const FGameplayAbilitySpec& Spec : RemovedAbilities)
		{
			if (Spec.InputID == static_cast<int32>(AbilityInput))
			{
				AbilitySystemComponent->GiveAbility(Spec);
			}
		}
		RemovedAbilities.Reset();
	}
}
void FEOEPhaseDataWithAbility::ApplyToAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const
{
	Super::ApplyToAbilityComponent(AbilitySystemComponent);

	if (AbilitySystemComponent)
	{
		FGameplayAbilitySpec Spec = FGameplayAbilitySpec(AbilityToGive, 1, static_cast<int32>(AbilityInput), AbilitySystemComponent->GetOwner());
		AbilitySystemComponent->GiveAbility(Spec);
	}
}
void UEOEPhasePrimaryDataAsset::ApplyToAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!AbilitySystemComponent)
		return;

	for (const FInstancedStruct& InstancedStruct : AbilitiesData)
	{
		InstancedStruct.Get<FEOEPhaseData>().ApplyToAbilityComponent(AbilitySystemComponent);
	}

	AbilitySystemComponent->BlockAbilitiesWithTags(AbilityTagsToBlock);
}
void UEOEPhasePrimaryDataAsset::RemoveFromAbilityComponent(UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (!AbilitySystemComponent)
		return;

	for (const FInstancedStruct& InstancedStruct : AbilitiesData)
	{
		InstancedStruct.Get<FEOEPhaseData>().RemoveFromAbilityComponent(AbilitySystemComponent);
	}

	AbilitySystemComponent->UnBlockAbilitiesWithTags(AbilityTagsToBlock);
}