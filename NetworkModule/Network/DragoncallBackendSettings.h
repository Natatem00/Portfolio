#pragma once

#include "DragoncallBackendSettings.generated.h"

UENUM(BlueprintType)
enum class EBackendService : uint8
{
	Auth,
	Game,
	Dev,
	End UMETA(Hidden)
};

UCLASS(Config = BackendSettings, defaultconfig, BlueprintType)
class UDragoncallBackendSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetSectionName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetContainerName() const override;

	UFUNCTION(BlueprintPure)
	const FString& GetUrlByType(EBackendService InService = EBackendService::Dev) const;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

	UFUNCTION(BlueprintPure, DisplayName = "BackendSettings")
	static const UDragoncallBackendSettings* GetSettings()
	{
		return GetDefault<UDragoncallBackendSettings>();
	}

	const FString& GetServerRegion() const
	{
		return ServerRegion;
	}

private:
	UPROPERTY(Config, EditAnywhere, Category = "Environment")
	TMap<EBackendService, FString> ServerUrls;

	UPROPERTY(Config)
	FString ServerRegion;
};