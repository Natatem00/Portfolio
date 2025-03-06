// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Phases/EOEPhaseWorldSubsystem.h"

#include "AbilitySystemGlobals.h"
#include "EOEPhasePrimaryDataAsset.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"

void UEOEPhaseWorldSubsystem::StartPhase(FPrimaryAssetId InPhasePrimaryAssetId)
{
	if (LoadAssetToApplyStreamableHandle && LoadAssetToApplyStreamableHandle->IsLoadingInProgress())
	{
		return;
	}

	UAssetManager& assetManager = UAssetManager::Get();

	LoadAssetToApplyStreamableHandle = assetManager.GetPrimaryAssetHandle(InPhasePrimaryAssetId);
	if (!LoadAssetToApplyStreamableHandle)
	{
		LoadAssetToApplyStreamableHandle = assetManager.LoadPrimaryAsset(InPhasePrimaryAssetId, TArray<FName>(), LoadAssetToApplyDelegate);
	}
	else
	{
		ApplyLoadedDataAsset();
	}
}
void UEOEPhaseWorldSubsystem::RemovePhase(FPrimaryAssetId InPhasePrimaryAssetId)
{
	UAssetManager& assetManager = UAssetManager::Get();

	if (const UEOEPhasePrimaryDataAsset* phaseDataAsset = assetManager.GetPrimaryAssetObject<UEOEPhasePrimaryDataAsset>(InPhasePrimaryAssetId))
	{
		if (!ActivePhasesPrimaryDataAssets.Contains(phaseDataAsset))
		{
			return;
		}
		UAbilitySystemComponent* abilitySystemComponentFromActor = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
		phaseDataAsset->RemoveFromAbilityComponent(abilitySystemComponentFromActor);
		ActivePhasesPrimaryDataAssets.RemoveNode(phaseDataAsset);
	}
}
void UEOEPhaseWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	LoadAssetToApplyDelegate.BindUObject(this, &UEOEPhaseWorldSubsystem::ApplyLoadedDataAsset);
}
void UEOEPhaseWorldSubsystem::ApplyLoadedDataAsset()
{
	if (!LoadAssetToApplyStreamableHandle)
	{
		return;
	}
	UAbilitySystemComponent* abilitySystemComponentFromActor = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	if (const UEOEPhasePrimaryDataAsset* phaseDataAsset = Cast<UEOEPhasePrimaryDataAsset>(LoadAssetToApplyStreamableHandle->GetLoadedAsset()))
	{
		if (!ActivePhasesPrimaryDataAssets.Contains(phaseDataAsset))
		{
			ActivePhasesPrimaryDataAssets.AddHead(phaseDataAsset);

			phaseDataAsset->ApplyToAbilityComponent(abilitySystemComponentFromActor);
		}
	}
}