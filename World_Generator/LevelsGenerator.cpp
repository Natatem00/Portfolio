// Fill out your copyright notice in the Description page of Project Settings.

#include "Levels/Generator/DragoncallLevelsGenerator.h"

#include "Data/DragoncallLevelInfoData.h"
#include "DragoncallDebug/Level/DragoncallLevelDebug.h"
#include "DragoncallHexSpawnPoint.h"
#include "DragoncallLog/DragoncallLog.h"
#include "Engine/LevelStreamingDynamic.h"
#include "LoadingScreenManager.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Net/UnrealNetwork.h"
#include "Subsystems/DragoncallLevelGeneratorSubsystem.h"

ADragoncallLevelsGenerator::FDragoncallLevelIndexCache::FDragoncallLevelIndexCache()
{
	Reset();
}
bool ADragoncallLevelsGenerator::FDragoncallLevelIndexCache::IsIndexesFull(uint8 FullValue) const
{
	while (FullValue)
	{
		if (!ContainsLevelInfoIndex(FullValue--))
		{
			return false;
		}
	}
	return ContainsLevelInfoIndex(0);
}

ADragoncallLevelsGenerator::ADragoncallLevelsGenerator()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetPriority = 3.f;
}

void ADragoncallLevelsGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		ULoadingScreenManager* loadingScreenManager = GetGameInstance()->GetSubsystem<ULoadingScreenManager>();
		loadingScreenManager->RegisterLoadingProcessor(this);
	}

	if (UDragoncallLevelGeneratorSubsystem* levelGenerator = GetWorld()->GetSubsystem<UDragoncallLevelGeneratorSubsystem>())
	{
		levelGenerator->SetLevelGenerator(this);
	}
}

void ADragoncallLevelsGenerator::Server_LoadLevels()
{
	if (HasAuthority())
	{
#if WITH_EDITOR
		Editor_GenerateLevels();
#else
		RandomSeed = FMath::Rand32();
		LoadLevels();
#endif // WITH_EDITOR
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RandomSeed, this);
	}
}
void ADragoncallLevelsGenerator::UnloadLevels()
{
	for (ULevelStreamingDynamic* dynamicLevelStreaming : LoadedLevelsStreamingDynamics)
	{
		dynamicLevelStreaming->SetIsRequestingUnloadAndRemoval(true);
	}

	if (!LoadedLevelsStreamingDynamics.IsEmpty())
	{
		GetWorld()->FlushLevelStreaming(EFlushLevelStreamingType::Full);
	}
	LoadedLevelsStreamingDynamics.Reset();
	LevelsIndexCache.Reset();

	LevelsToLoad = 0;
	bServerLoaded = false;
	bAllClientsLoaded = false;
}

void ADragoncallLevelsGenerator::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams doRepLifetimeParams;
	doRepLifetimeParams.bIsPushBased = true;
	doRepLifetimeParams.RepNotifyCondition = REPNOTIFY_Always;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RandomSeed, doRepLifetimeParams);
}
bool ADragoncallLevelsGenerator::ShouldShowLoadingScreen(FString& OutReason) const
{
	return !bAllClientsLoaded;
}

uint8 ADragoncallLevelsGenerator::GetRandomLevelInfoDataIndex(const FRandomStream& InRandomStream) const
{
	uint8 randomIndex;
	if (LevelsIndexCache.IsIndexesFull(LevelsInfo.Num() - 1))
	{
		return UINT8_ERROR;
	}

	do
	{
		randomIndex = InRandomStream.RandRange(0, LevelsInfo.Num() - 1);
	} while (LevelsIndexCache.ContainsLevelInfoIndex(randomIndex));

	LevelsIndexCache.AddLevelInfoIndex(randomIndex);
	return randomIndex;
}
void ADragoncallLevelsGenerator::OnRep_RandomSeed()
{
	LoadLevels();
}
void ADragoncallLevelsGenerator::LoadLevels()
{
	SCOPED_NAMED_EVENT(LevelGeneration, FColor::Green);

	FRandomStream randomStream = FRandomStream(RandomSeed);
	UE_LOG(LogDragoncallLevelGenerator, Log, TEXT("GENERATION SEED = %d"), RandomSeed);

	MaxRotationMultiplier = 360 / RotationDegrees;
	for (uint8 index = 0; index < LevelSpawnPointsInOrder.Num(); index++)
	{
		if (LevelSpawnPointsInOrder[index])
		{
			const uint8 levelInfoIndex = GetRandomLevelInfoDataIndex(randomStream);
			if (levelInfoIndex == UINT8_ERROR)
			{
				break;
			}
			if (LevelsInfo.IsValidIndex(levelInfoIndex))
			{
				FTransform spawnTransform = LevelSpawnPointsInOrder[index]->GetActorTransform();
				FRotator spawnRotation = spawnTransform.GetRotation().Rotator();
				const uint8 randRotation = randomStream.FRandRange(0, MaxRotationMultiplier);
				spawnRotation.Yaw += RotationDegrees * randRotation;
				spawnTransform.SetRotation(spawnRotation.Quaternion());

				if (CVarShowLevelGeneratorDebug.GetValueOnAnyThread())
				{
					UE_LOG(LogDragoncallLevelGenerator, Log, TEXT("SPAWN LEVEL INDEX = %d, LEVEL ROTATION = %s"), levelInfoIndex, *spawnRotation.ToString());
				}
				const UDragoncallLevelInfoData* level = LevelsInfo[levelInfoIndex];
				for (const TSoftObjectPtr<UWorld>& world : level->GetAllLevelToLoad())
				{
					bool bsuccess = false;
					const FString UniqueName = FString::Printf(TEXT("%s_%d"), *world.GetAssetName(), RandomSeed);
					ULevelStreamingDynamic* dynamicLevelStreaming = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(GetWorld(), world, spawnTransform, bsuccess, UniqueName);
					if (bsuccess)
					{
						LevelsToLoad++;
						dynamicLevelStreaming->OnLevelShown.AddUniqueDynamic(this, &ThisClass::OnLevelLoaded);
						LoadedLevelsStreamingDynamics.Add(dynamicLevelStreaming);
					}
				}
			}
		}
	}
}

#if WITH_EDITOR
void ADragoncallLevelsGenerator::Editor_GenerateLevels()
{
	UnloadLevels();

	if (RandomSeed == 0)
	{
		RandomSeed = FMath::Rand32();
	}
	LoadLevels();
}
#endif // WITH_EDITOR

void ADragoncallLevelsGenerator::OnLevelLoaded()
{
	LevelsToLoad--;
	if (LevelsToLoad <= 0)
	{
		if (HasAuthority())
		{
			OnServerLevelLoadedDelegate.Broadcast();
			bServerLoaded = true;
		}
		else
		{
			OnClientLevelLoadedDelegate.Broadcast();
			bAllClientsLoaded = true;
		}
	}
}