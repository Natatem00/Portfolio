// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DragoncallWeaponEquipActor.h"
#include "GameCore/Misc/CustomHitResults.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"

#include "DragoncallMeleeWeaponEquipActor.generated.h"

class FDragoncallAttackSubstepCalculation;
class UBoxComponent;
class UArrowComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FDragoncallImpactZoneHitDelegate, AActor*, OtherActor, const FGameplayTag, ZoneTag, const FHitResult&, HitResult, FVector_NetQuantizeNormal, HitDirection);

inline bool operator==(const FHitResult& lhs, const FHitResult& rhs);

UCLASS()
class DRAGONCALL_API ADragoncallMeleeWeaponEquipActor : public ADragoncallWeaponEquipActor
{
	GENERATED_BODY()

	friend FDragoncallAttackSubstepCalculation;

protected:
	struct FDragoncallZoneData
	{
		FVector LocationOffset;
		FRotator RotationOffset;
		FVector ScaledBoxExtent;

		FCollisionShape CollisionShape;
	};

	enum EDragoncallTraceIndex : uint8
	{
		RED = 0,
		ORANGE,
		GREEN
	};

	using TMapZonePair = TPair<FGameplayTag, FDragoncallZoneData>;
	using TMapZone = TMap<FGameplayTag, FDragoncallZoneData>;

public:
	ADragoncallMeleeWeaponEquipActor();

	virtual void BeginPlay() override;

	const FName& GetImpactZoneCollisionProfileName() const
	{
		return ImpactZoneCollisionProfileName.Name;
	}
	virtual void Tick(float DeltaTime) override;
#if !UE_BUILD_SHIPPING
	void DrawDebugZone(float DeltaTime) const;
#endif // !UE_BUILD_SHIPPING

protected:
	virtual void TryHit(float DeltaTime);

	bool OnHit(const FHitResult& HitResult, const TMapZonePair& HitZone);

	virtual ADragoncallMeleeWeaponEquipActor::TMapZonePair* GetDamageZone(const FHitResult& InHit);

	virtual bool ProceedHitResults(TArray<FHitResult> InHitResults);

	UFUNCTION(BlueprintCallable)
	void SetTraceEnabled(bool bEnable);

private:
	static FDragoncallZoneData GetZoneData(const TObjectPtr<UBoxComponent>& InZoneComponent);
	void SetupImpactZoneSettings();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ImpactZoneHeadingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> GreenZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> OrangeZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> RedZone;

	TMapZone ImpactZoneSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FCollisionProfileName ImpactZoneCollisionProfileName = FName(TEXT("HitTrace"));

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAttackPhase = false;

	UPROPERTY(BlueprintAssignable)
	FDragoncallImpactZoneHitDelegate OnImpactZoneHit;

	UPROPERTY(BlueprintAssignable)
	FDragoncallImpactZoneHitDelegate OnImpactZoneBlocked;

	TArray<FGameplayAbilitySpecHandle> GameplayAbilitySpecHandles;

private:
	FVector NormalizedVelocity;
	FVector ActorPrevLocation;

protected:
#pragma region SUBSTEPING
	struct FDragoncallLastFrameAttackData
	{
		FVector LastFramePosition;
		FRotator LastFrameRotation;

		uint8 bHasLastFrameData : 1;
	};
	struct FDragoncallAttackSubstepCalculation
	{
		virtual ~FDragoncallAttackSubstepCalculation() = default;
		virtual void DoSubSteps(ADragoncallMeleeWeaponEquipActor* InActor, TArrayView<FDragoncallLastFrameAttackData> InOutLastFrameData, const TMapZone& InZonesSettings, float DeltaTime) const;
#if ENABLE_DRAW_DEBUG
		virtual void DrawDebug(const ADragoncallMeleeWeaponEquipActor* InActor, TArrayView<FDragoncallLastFrameAttackData> InOutLastFrameData, const TMapZone& InZonesSettings, float DeltaTime) const;
#endif
	protected:
		virtual void CalculatePoints(const ADragoncallMeleeWeaponEquipActor* InActor, const ADragoncallMeleeWeaponEquipActor::TMapZone& InZonesSettings, FVector& OutTotalHalfSize, FVector& OutStartLocation, FVector& OutFinalPosition, FVector& OutTracePoint, FRotator& OutRotation) const;
		static void GetActorsToIgnore(ADragoncallMeleeWeaponEquipActor* InActor, TArray<AActor*>& actorsToIgnore);

		virtual FTransform GetZoneTransform(const ADragoncallMeleeWeaponEquipActor* InActor, const FDragoncallZoneData& InZonesSettings) const;

		virtual bool ProceedHitResults(ADragoncallMeleeWeaponEquipActor* InActor, TArray<FHitResult>& InHitResults) const;

		virtual uint8 CalculateNumSubsteps(float DeltaTime) const;

	protected:
		TInterval<uint8> SubstepInterval = TInterval<uint8>(3, 7);
		mutable float FixedDeltaTime = 1.0f / 60.0f;
	};

	TSharedPtr<FDragoncallAttackSubstepCalculation> SubstepCalculation;
	mutable TArray<FDragoncallLastFrameAttackData> LastFrameAttackData;
#pragma endregion SUBSTEPING
};
