// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/Player/Movement/DragoncallMovementComponent.h"

#include "Components/Player/Movement/DragoncallMovementState.h"
#include "DragoncallCharacter.h"
#include "DragoncallLog/DragoncallLog.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#pragma region SAVE_MOVE
bool UDragoncallMovementComponent::FSavedMove_DragoncallCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_DragoncallCharacter* NewVoidMove = static_cast<FSavedMove_DragoncallCharacter*>(NewMove.Get());

	if (Saved_bWantsToDash != NewVoidMove->Saved_bWantsToDash)
	{
		return false;
	}

	return Saved_Speed == NewVoidMove->Saved_Speed && Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UDragoncallMovementComponent::FSavedMove_DragoncallCharacter::Clear()
{
	Super::Clear();

	Saved_Speed = 0.f;
	Saved_bWantsToDash = false;
}

uint8 UDragoncallMovementComponent::FSavedMove_DragoncallCharacter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToDash)
	{
		Result |= FLAG_DASH;
	}

	return Result;
}

void UDragoncallMovementComponent::FSavedMove_DragoncallCharacter::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UDragoncallMovementComponent* MovmentComponent = Cast<UDragoncallMovementComponent>(C->GetCharacterMovement());

	Saved_Speed = MovmentComponent->Safe_Speed;
	Saved_bWantsToDash = MovmentComponent->Safe_bWantsToDash;
}

void UDragoncallMovementComponent::FSavedMove_DragoncallCharacter::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UDragoncallMovementComponent* MovmentComponent = Cast<UDragoncallMovementComponent>(C->GetCharacterMovement());

	MovmentComponent->Safe_Speed = Saved_Speed;
	MovmentComponent->Safe_bWantsToDash = Saved_bWantsToDash;
}
#pragma endregion SAVE_MOVE

#pragma region PREDICTION_DATA
UDragoncallMovementComponent::FNetworkPredictionData_Client_DragoncallCharacter::FNetworkPredictionData_Client_DragoncallCharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
	DragoncallMovementComponent = &ClientMovement;
}

FSavedMovePtr UDragoncallMovementComponent::FNetworkPredictionData_Client_DragoncallCharacter::AllocateNewMove()
{
	FSavedMovePtr movePtr = FSavedMovePtr(new FSavedMove_DragoncallCharacter(DragoncallMovementComponent.Get()));
	return movePtr;
}
#pragma endregion PREDICTION_DATA

void UDragoncallMovementComponent::BeginPlay()
{
	UCharacterMovementComponent::BeginPlay();

	AddMovementStates();

	BrakingFrictionFactorData.DefaultBrakingFrictionFactor = BrakingFrictionFactor;
	BrakingFrictionFactorData.DefaultBrakingDecelerationWalking = BrakingDecelerationWalking;
}

void UDragoncallMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	UCharacterMovementComponent::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TryChangeSpeedState();
}
void UDragoncallMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (Safe_bWantsToDash && CanDash())
	{
		PerfomDash();
		Safe_bWantsToDash = false;
		if (CharacterOwner->HasAuthority())
		{
			Proxy_bDashStart = true;
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Proxy_bDashStart, this);
		}
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

FNetworkPredictionData_Client* UDragoncallMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner);

	if (ClientPredictionData == nullptr)
	{
		UDragoncallMovementComponent* MutableThis = const_cast<UDragoncallMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_DragoncallCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}
void UDragoncallMovementComponent::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);

	if (!bClientSimulation)
	{
		bWantsToCrouch = false;
		Cast<ADragoncallCharacter>(CharacterOwner)->OnUncrouchedDelegate.Broadcast();
	}
}

void UDragoncallMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToDash = (Flags & FSavedMove_DragoncallCharacter::FLAG_DASH) != 0;
}

void UDragoncallMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	MaxWalkSpeed = Safe_Speed;
	MaxWalkSpeedCrouched = Safe_Speed;
}

bool UDragoncallMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() && (IsMovingOnGround() || IsFalling());
}

void UDragoncallMovementComponent::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	BrakingFrictionFactor = BrakingFrictionFactorData.BrakingFrictionFactorWhenLanded;
	BrakingDecelerationWalking = BrakingFrictionFactorData.BrakingDecelerationWalkingWhenLanded;

	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle, this, &ThisClass::ResetBrakingFrictionFactorToDefault, BrakingFrictionFactorData.TimeToResetBrakingFrictionToDefaults, false);
}

float UDragoncallMovementComponent::GetFallHeight() const
{
	if (IsMovingOnGround() == false)
	{
		const float CurrentHeight = GetActorLocation().Z;

		const float FallHeight = StartFallingHeight - CurrentHeight;

		return FallHeight;
	}
	return 0.0f;
}
void UDragoncallMovementComponent::Dash()
{
	Safe_bWantsToDash = true;
}
void UDragoncallMovementComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	static FDoRepLifetimeParams repLifeTime;
	repLifeTime.Condition = COND_SimulatedOnly;
	repLifeTime.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Proxy_bDashStart, repLifeTime);
}
void UDragoncallMovementComponent::ChangeMovementState(const EDragoncallMovementState NewMovementState)
{
	if (DCMovementState != NewMovementState)
	{
		DRAGONCALL_LOG(LogDragoncallMovementComponent, Verbose, "[%s] Changed Movement State to %s", *Log(this), *UEnum::GetValueAsString(NewMovementState));
	}
	DCMovementState = NewMovementState;

	OnChangedMovementState.Broadcast(DCMovementState);
}

void UDragoncallMovementComponent::TryChangeSpeedState()
{
	if (IsMovingOnGround())
	{
		StartFallingHeight = 0.f;

		uint8 Index = bWantsToCrouch ? ToUint8(EDragoncallMovementState::CROUCH_STATIONARY) : ToUint8(EDragoncallMovementState::STANDING_STATIONARY);

		Index += IsMoving() ? (IsRunning() ? 2 : 1) : 0;

		if (ToUint8(DCMovementState) != Index)
		{
			MovementStates[static_cast<EDragoncallMovementState>(Index)]->ChangeMovementState(this);
		}
	}
	else if (DCMovementState != EDragoncallMovementState::IN_AIR)
	{
		StartFallingHeight = GetActorLocation().Z;
		MovementStates[EDragoncallMovementState::IN_AIR]->ChangeMovementState(this);
	}
}

void UDragoncallMovementComponent::AddMovementStates()
{
	MovementStates.Reserve(ToUint8(EDragoncallMovementState::DEFAULT) + 1);

	MovementStates.Add(EDragoncallMovementState::CROUCH_STATIONARY, MakeShareable(new CrouchStationaryState()));
	MovementStates.Add(EDragoncallMovementState::CROUCH_MOVE, MakeShareable(new CrouchMoveState()));

	MovementStates.Add(EDragoncallMovementState::STANDING_STATIONARY, MakeShareable(new StandingStationaryState()));
	MovementStates.Add(EDragoncallMovementState::STANDING_WALKING, MakeShareable(new StandingWalkingState()));
	MovementStates.Add(EDragoncallMovementState::STANDING_RUNNING, MakeShareable(new StandingRunningState()));

	MovementStates.Add(EDragoncallMovementState::IN_AIR, MakeShareable(new InAirState()));
}

bool UDragoncallMovementComponent::IsMoving() const
{
	return Velocity.SizeSquared2D() > 0.01f;
}

bool UDragoncallMovementComponent::IsRunning() const
{
	return Velocity.SizeSquared2D() >= RunningSpeedThreshold * RunningSpeedThreshold;
}

void UDragoncallMovementComponent::SetMaxMovementSpeed(float NewSpeed)
{
	Safe_Speed = NewSpeed;

	SERVER_SetSafeSpeed(Safe_Speed);
}

void UDragoncallMovementComponent::SetCrouch(const bool bNewVal)
{
	bWantsToCrouch = bNewVal;
	SERVER_Crouch(bWantsToCrouch);
}

void UDragoncallMovementComponent::SERVER_SetSafeSpeed_Implementation(float NewSpeed)
{
	Safe_Speed = NewSpeed;
}

void UDragoncallMovementComponent::SERVER_Crouch_Implementation(uint8 bCrouching)
{
	bWantsToCrouch = bCrouching;
}
void UDragoncallMovementComponent::ResetBrakingFrictionFactorToDefault()
{
	BrakingFrictionFactor = BrakingFrictionFactorData.DefaultBrakingFrictionFactor;
	BrakingDecelerationWalking = BrakingFrictionFactorData.DefaultBrakingDecelerationWalking;
}
bool UDragoncallMovementComponent::CanDash() const
{
	return true;
}
void UDragoncallMovementComponent::PerfomDash()
{
	const TGuardValue<float> guardGravityScale(GravityScale, DashData.GravityScale);

	float impulse = DashData.Impulse;
	bIgnoreClientMovementErrorChecksAndCorrection = true;
	if (IsCrouching())
	{
		bWantsToCrouch = false;
		impulse = DashData.CrouchImpulse;
	}

	const FVector dashDirection = GetPawnOwner()->GetBaseAimRotation().Vector();

	// FVector::UpVector * DashData.StartVectorUpFrictionMultiplier is used to turn off friction for some time
	Velocity = impulse * (dashDirection + FVector::UpVector * DashData.StartVectorUpFrictionMultiplier);

	SetMovementMode(MOVE_Falling);
	OnDashStartedDelegate.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &ThisClass::StopDash, DashData.DashMaxTime, false);
}
void UDragoncallMovementComponent::StopDash()
{
	Velocity = Velocity * DashData.EndDashSpeedMultiplier;
	bIgnoreClientMovementErrorChecksAndCorrection = false;
	Proxy_bDashStart = false;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Proxy_bDashStart, this);
	OnDashEndedDelegate.Broadcast();
}
void UDragoncallMovementComponent::OnRe_DashStart() const
{
	if (Proxy_bDashStart)
	{
		OnDashStartedDelegate.Broadcast();
	}
	else
	{
		OnDashEndedDelegate.Broadcast();
	}
}

UDragoncallMovementComponent::FSavedMove_DragoncallCharacter::FSavedMove_DragoncallCharacter(const UCharacterMovementComponent* MovementComponent)
{
	DragoncallMovementComponent = Cast<UDragoncallMovementComponent>(MovementComponent);
}
