// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"

#include "EOEPhaseWorldSubsystem.generated.h"

/**
 *
 */
class UEOEPhasePrimaryDataAsset;

UCLASS()
class GAMECORE_API UEOEPhaseWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void StartPhase(FPrimaryAssetId InPhasePrimaryAssetId);

	UFUNCTION(BlueprintCallable)
	void RemovePhase(FPrimaryAssetId InPhasePrimaryAssetId);

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	void ApplyLoadedDataAsset();

private:
	FStreamableDelegate LoadAssetToApplyDelegate;

	TSharedPtr<FStreamableHandle> LoadAssetToApplyStreamableHandle;

	TDoubleLinkedList<const UEOEPhasePrimaryDataAsset*> ActivePhasesPrimaryDataAssets;
};
