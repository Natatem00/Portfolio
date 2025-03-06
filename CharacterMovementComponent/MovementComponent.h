// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionCore/Character/MotionCharacterMovementComponent.h"

#include "DragoncallMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDragoncallDashSignature);

USTRUCT(BlueprintType)
struct FDragoncallDashData
{
	GENERATED_BODY()

	float DefaultImpulse;
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float Impulse = 1000.f;

	float DefaultCrouchImpulse;
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float CrouchImpulse = 500.f;

	float DefaultGravityScale;
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float GravityScale = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashMaxTime = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float EndDashSpeedMultiplier = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float StartVectorUpFrictionMultiplier = 0.1f;
};

USTRUCT(BlueprintType)
struct FDragoncallBrakingFrictionFactorData
{
	GENERATED_BODY()

	float DefaultBrakingFrictionFactor;
	UPROPERTY(EditDefaultsOnly, Category = "Braking Friction")
	float BrakingFrictionFactorWhenLanded = 0.5f;

	float DefaultBrakingDecelerationWalking;
	UPROPERTY(EditDefaultsOnly, Category = "Braking Friction")
	float BrakingDecelerationWalkingWhenLanded = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Braking Friction")
	float TimeToResetBrakingFrictionToDefaults = 0.5f;
};

UENUM(BlueprintType)
enum class EDragoncallMovementState : uint8
{
	CROUCH_STATIONARY = 0,
	CROUCH_MOVE,

	STANDING_STATIONARY,
	STANDING_WALKING,
	STANDING_RUNNING,

	IN_AIR,

	CASTING_SPELL,
	HOLDING_ARROW,

	// add new stuff only before default
	DEFAULT
};

static uint8 ToUint8(EDragoncallMovementState State)
{
	return static_cast<uint8>(State);
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDragoncallOnChangedMovementState, EDragoncallMovementState, NewMovementState);

UCLASS()
class DRAGONCALL_API UDragoncallMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class IDragoncallMovementState;

#pragma region SAVE_MOVE
	class FSavedMove_DragoncallCharacter : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		float Saved_Speed;
		uint8 Saved_bWantsToDash : 1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;

	public:
		enum EDragoncallCompressedFlags
		{
			FLAG_DASH = 0x10
		};

		FSavedMove_DragoncallCharacter(const UCharacterMovementComponent* MovementComponent);
		FSavedMove_DragoncallCharacter() = default;
		TWeakObjectPtr<const UDragoncallMovementComponent> DragoncallMovementComponent;
	};
#pragma endregion SAVE_MOVE

#pragma region PREDICTION_DATA
	class FNetworkPredictionData_Client_DragoncallCharacter : public FNetworkPredictionData_Client_Character
	{
		typedef FNetworkPredictionData_Client_Character Super;

	public:
		FNetworkPredictionData_Client_DragoncallCharacter(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;

		TWeakObjectPtr<const UCharacterMovementComponent> DragoncallMovementComponent;
	};
#pragma endregion PREDICTION_DATA

public:
	UFUNCTION(BlueprintPure, Category = Movement)
	EDragoncallMovementState GetMovementState() const
	{
		return DCMovementState;
	}

	void SetMaxMovementSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable)
	void SetCrouch(const bool bNewVal);

	float GetStartFallingHeight() const
	{
		return StartFallingHeight;
	}

	float GetFallHeight() const;

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	void Dash();

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual bool CanAttemptJump() const override;
	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void UnCrouch(bool bClientSimulation = false) override;

private:
	void ChangeMovementState(const EDragoncallMovementState NewMovementState);
	inline void TryChangeSpeedState();
	inline void AddMovementStates();

	inline bool IsMoving() const;

	inline bool IsRunning() const;

	UFUNCTION(Server, Reliable)
	void SERVER_SetSafeSpeed(float NewSpeed);

	UFUNCTION(Server, Reliable)
	void SERVER_Crouch(uint8 bCrouching);

	void ResetBrakingFrictionFactorToDefault();

	bool CanDash() const;
	void PerfomDash();

	void StopDash();

	UFUNCTION()
	void OnRe_DashStart() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RunningSpeedThreshold = 400.0f;

	UPROPERTY(BlueprintAssignable, Category = Movement)
	FDragoncallOnChangedMovementState OnChangedMovementState;

	UPROPERTY(BlueprintAssignable, Category = Movement)
	FDragoncallDashSignature OnDashStartedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Movement)
	FDragoncallDashSignature OnDashEndedDelegate;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	EDragoncallMovementState DCMovementState = EDragoncallMovementState::DEFAULT;

private:
	TMap<EDragoncallMovementState, TSharedPtr<class IDragoncallMovementState>> MovementStates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FDragoncallBrakingFrictionFactorData BrakingFrictionFactorData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FDragoncallDashData DashData;

	UPROPERTY(ReplicatedUsing = OnRe_DashStart)
	uint8 Proxy_bDashStart : 1;

	float StartFallingHeight;

	float Safe_Speed = 0;
	uint8 Safe_bWantsToDash : 1;

	FTimerHandle DashTimerHandle;
};
