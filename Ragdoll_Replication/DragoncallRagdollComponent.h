// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Dragoncall/AbilitySystem/Effects/DragoncallEnableRagdollGameplayEffect.h"
#include "Misc/Types/NetworkTypes.h"

#include "DragoncallRagdollComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDragoncallOnRagdollStateChangedSiganture);

class UPhysicsAsset;
class UPhysicalAnimationComponent;
class ADragoncallCharacter;

USTRUCT()
struct FDragoncallRagdollData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, Category = Ragdoll)
	FDragoncallRagdollSettings Settings;
	TOptional<FHitResult> HitResult;
	float ImpulseMagnitude = 1.0f;
};

template<>
struct TIsPODType<FDragoncallRagdollData>
{
	enum
	{
		Value = true
	};
};

USTRUCT()
struct FDragoncallRagdollReplicationData
{
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize10 PelvisBoneLocation = DragoncallNetwork::InvalidLocation;

	UPROPERTY()
	FVector_NetQuantize10 PelvisBoneRotation = DragoncallNetwork::InvalidLocation;

	bool IsValid() const { return DragoncallNetwork ::IsValidLocation(PelvisBoneLocation); }
};

template<>
struct TStructOpsTypeTraits<FDragoncallRagdollReplicationData> : public TStructOpsTypeTraitsBase2<FDragoncallRagdollReplicationData>
{
	enum
	{
		WithNetSerializer = true
	};
};

inline bool operator!=(const FTransform& InTransform, const FTransform& InTransformToCompare);

UCLASS(Abstract, Blueprintable)
class DRAGONCALL_API UDragoncallRagdollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDragoncallRagdollComponent();

	bool GetIsRagdollEnabled() const;

	void EnableRagdoll();

	void DisableRagdoll();

	FTransform GetPelvisBoneTransform() const;

	UPROPERTY(BlueprintAssignable, Category = Ragdoll)
	FDragoncallOnRagdollStateChangedSiganture OnRagdollEnabled_Start;
	UPROPERTY(BlueprintAssignable, Category = Ragdoll)
	FDragoncallOnRagdollStateChangedSiganture OnRagdollEnabled_End;
	UPROPERTY(BlueprintAssignable, Category = Ragdoll)
	FDragoncallOnRagdollStateChangedSiganture OnRagdollDisabled_Start;
	UPROPERTY(BlueprintAssignable, Category = Ragdoll)
	FDragoncallOnRagdollStateChangedSiganture OnRagdollDisabled_End;

protected:
	void EnableRagdollImpl();
	void EnableClientRagdollPhysics() const;
	void CollectPelvisBoneData(const USkeletalMeshComponent* Mesh);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void DestroyComponent(bool bPromoteChildren /* = false */) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void UpdateMeshLocation();

	UFUNCTION()
	void OnRep_RagdollReplicationData(const FDragoncallRagdollReplicationData& InOldRagdollData);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PelvisBoneName = "pelvis";

	UPROPERTY(BlueprintReadOnly)
	bool bRagdollEnabled = false;

	uint8 bNeedRagdollInterpolation : 1;

	FName OriginMeshCollisionProfileName;

	UPROPERTY()
	TObjectPtr<UPhysicsAsset> OriginMeshPhysAsset;

	UPROPERTY(EditDefaultsOnly)
	FDragoncallRagdollData RagdollData;

	TWeakObjectPtr<ADragoncallCharacter> RagdollCharacter;

	FTransform MeshDelta;

	UPROPERTY(ReplicatedUsing = OnRep_RagdollReplicationData)
	FDragoncallRagdollReplicationData RagdollReplicationData;

	FTimerHandle EnableRagdollTimerHandle;
	FTimerHandle DisableRagdollTimerHandle;
	FTimerHandle ClientRagdollPhysicsTimerHandle;

	FTimerHandle ServerUpdateMeshLocationTimerHandle;

private:
	struct FDragoncallRagdollClientInterpolation
	{
		bool Interpolate(const float DeltaTime, const ACharacter* InCharacter) const;

		FTransform ServerPelvisTransformToInterpolate;

		float InterpolationSpeed = 30.f;
		float CorrectionPositionTrashHold = 35.f;

		float DesiredCorrectionTime = 0.3f;
		float RotationStiffness = 25000.f;

		bool IsServerPelvisTransformToInterpolateNotEmpty() const;

	private:
		FTransform CurrentInterpolationTransform;
	};

	FDragoncallRagdollClientInterpolation RagdollClientInterpolation;
};
