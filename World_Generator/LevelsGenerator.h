// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "LoadingProcessInterface.h"

#include "DragoncallLevelsGenerator.generated.h"

class ADragoncallHexSpawnPoint;
class UDragoncallLevelInfoData;
class ULevelStreamingDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelStateChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelStateChangedForClientSignature, APlayerController*, InController);

UCLASS(Blueprintable)
class DRAGONCALL_API ADragoncallLevelsGenerator : public AInfo
	, public ILoadingProcessInterface
{
	GENERATED_BODY()

	struct FDragoncallLevelIndexCache
	{
		FDragoncallLevelIndexCache();
		FORCEINLINE bool ContainsLevelInfoIndex(const uint8 LevelInfoIndex) const
		{
			return (UsedLevelsInfoIndexes & (1 << LevelInfoIndex)) != 0;
		}

		FORCEINLINE void Reset()
		{
			UsedLevelsInfoIndexes = 0;
		}
		bool IsIndexesFull(uint8 FullValue) const;

		FORCEINLINE bool IsIndexesFull() const
		{
			return UsedLevelsInfoIndexes == UINT8_MAX;
		}

		FORCEINLINE void AddLevelInfoIndex(const uint8 LevelInfoIndex)
		{
			UsedLevelsInfoIndexes |= (1 << LevelInfoIndex);
		}

	private:
		// local cash of loaded levels indexes
		uint8 UsedLevelsInfoIndexes;
	};

public:
	ADragoncallLevelsGenerator();
	void Server_LoadLevels();

	UFUNCTION(CallInEditor, Category = "Levels")
	void UnloadLevels();

	FORCEINLINE bool IsServerLoaded() const
	{
		return bServerLoaded;
	}

	FORCEINLINE bool IsEveryoneLoaded() const
	{
		return bServerLoaded && bAllClientsLoaded;
	}

	UPROPERTY(BlueprintAssignable)
	FOnLevelStateChangedSignature OnServerLevelLoadedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnLevelStateChangedSignature OnClientLevelLoadedDelegate;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;

private:
	uint8 GetRandomLevelInfoDataIndex(const FRandomStream& InRandomStream) const;

	UFUNCTION()
	void OnRep_RandomSeed();

	void LoadLevels();
#if WITH_EDITOR
	UFUNCTION(CallInEditor, DisplayName = "Generate Levels", Category = "Levels")
	void Editor_GenerateLevels();
#endif

	UFUNCTION()
	void OnLevelLoaded();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Levels", meta = (AllowPrivateAccess = "true"))
	TArray<UDragoncallLevelInfoData*> LevelsInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Levels", meta = (AllowPrivateAccess = "true"))
	TArray<ADragoncallHexSpawnPoint*> LevelSpawnPointsInOrder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Levels", meta = (AllowPrivateAccess = "true", ClampMin = "0", UIMin = "0", ClampMax = "360", UIMax = "360"))
	float RotationDegrees = 60.f;

	UPROPERTY(EditAnywhere, Transient, Category = "Levels", ReplicatedUsing = OnRep_RandomSeed)
	int32 RandomSeed = 0;

	UPROPERTY(Transient)
	TArray<ULevelStreamingDynamic*> LoadedLevelsStreamingDynamics;

	mutable FDragoncallLevelIndexCache LevelsIndexCache;

	uint8 MaxRotationMultiplier = 5;
	uint16 LevelsToLoad = 0;
	uint8 bServerLoaded : 1;
	uint8 bAllClientsLoaded : 1;
};