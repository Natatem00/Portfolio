// Fill out your copyright notice in the Description page of Project Settings.

#include "DragoncallRagdollComponent.h"

#include "Components/CapsuleComponent.h"
#include "Dragoncall/Dragoncall.h"
#include "DragoncallCharacter.h"
#include "DragoncallLog/DragoncallLog.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

bool FDragoncallRagdollReplicationData::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << PelvisBoneLocation;
	Ar << PelvisBoneRotation;

	bOutSuccess = true;
	return true;
}
bool operator!=(const FTransform& InTransform, const FTransform& InTransformToCompare)
{
	return InTransform.GetLocation() != InTransformToCompare.GetLocation() || InTransform.GetRotation() != InTransformToCompare.GetRotation() || InTransform.GetScale3D() != InTransformToCompare.GetScale3D();
}
UDragoncallRagdollComponent::UDragoncallRagdollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UDragoncallRagdollComponent::GetIsRagdollEnabled() const
{
	return bRagdollEnabled;
}

void UDragoncallRagdollComponent::EnableRagdoll()
{
	if (bRagdollEnabled)
	{
		return;
	}

	DRAGONCALL_LOG(LogDragoncallCharacter, Log, "[%s] Enable ragdoll", *Log(this));

	bRagdollEnabled = true;

	if (FMath::IsNearlyZero(RagdollData.Settings.Delay))
	{
		EnableRagdollImpl();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(EnableRagdollTimerHandle, this, &UDragoncallRagdollComponent::EnableRagdollImpl, RagdollData.Settings.Delay, false);
	}
}

void UDragoncallRagdollComponent::DisableRagdoll()
{
	GetWorld()->GetTimerManager().ClearTimer(ServerUpdateMeshLocationTimerHandle);
	OnRagdollDisabled_Start.Broadcast();

	if (!bRagdollEnabled)
	{
		return;
	}

	DRAGONCALL_LOG(LogDragoncallCharacter, Log, "[%s] Disable ragdoll", *Log(this));

	auto ragdollCharacter = CastChecked<ADragoncallCharacter>(GetOwner());

	const auto mesh = ragdollCharacter->GetMesh();
	if (RagdollCharacter->HasAuthority())
	{
		UpdateMeshLocation();
	}
	mesh->SetAllBodiesSimulatePhysics(false);
	mesh->SetSimulatePhysics(false);
	mesh->PutAllRigidBodiesToSleep();
	mesh->bBlendPhysics = false;

	bRagdollEnabled = false;

	OnRagdollDisabled_End.Broadcast();

	UpdateMeshLocation();

	SetComponentTickEnabled(false);
}

FTransform UDragoncallRagdollComponent::GetPelvisBoneTransform() const
{
	const auto Rotation = FRotator::MakeFromEuler(RagdollReplicationData.PelvisBoneRotation);
	const auto Translation = RagdollReplicationData.PelvisBoneLocation;
	return FTransform(Rotation, Translation, FVector::OneVector);
}

void UDragoncallRagdollComponent::EnableRagdollImpl()
{
	OnRagdollEnabled_Start.Broadcast();

	RagdollCharacter = CastChecked<ADragoncallCharacter>(GetOwner());

	USkeletalMeshComponent* mesh = RagdollCharacter->GetMesh();

	OriginMeshCollisionProfileName = mesh->GetCollisionProfileName();
	mesh->SetIsReplicated(true);
	mesh->SetCollisionProfileName(RagdollData.Settings.RagdollCollisionProfileName.Name);

	if (RagdollCharacter->HasAuthority())
	{
		mesh->SetAllBodiesSimulatePhysics(true);
		mesh->SetSimulatePhysics(true);
		mesh->WakeAllRigidBodies();
		mesh->bBlendPhysics = true;
	}
	else
	{
		if (!RagdollReplicationData.IsValid())
		{
			// Data is not replicated yet, so use client's data to start ragdoll animation correctly
			bNeedRagdollInterpolation = true;
			CollectPelvisBoneData(mesh);
			EnableClientRagdollPhysics();
			// TODO: remove this. Need some logic to disable ragdoll when body is dormant for some time
			GetWorld()->GetTimerManager().SetTimer(DisableRagdollTimerHandle, this, &UDragoncallRagdollComponent::DisableRagdoll, RagdollData.Settings.DisableDelay, false);
		}
		else
		{
			// If we have replicated data - it means we are spawning character in the middle or end of ragdoll.
			// In this case enable physics with a delay (after animations syncs pose with replicated data),
			// otherwise limbs gains too much velocity.
			auto& TimeManager = GetWorld()->GetTimerManager();
			TimeManager.SetTimer(
				ClientRagdollPhysicsTimerHandle,
				this,
				&ThisClass::EnableClientRagdollPhysics,
				RagdollData.Settings.ClientPhysicsDelay);
			// TODO: remove this. Need some logic to disable ragdoll when body has become relevant
			static constexpr float timeToDisable = 3.f;
			GetWorld()->GetTimerManager().SetTimer(DisableRagdollTimerHandle, this, &UDragoncallRagdollComponent::DisableRagdoll, timeToDisable, false);
		}
	}

	UCharacterMovementComponent* characterMovementComp = RagdollCharacter->GetCharacterMovement();
	characterMovementComp->StopMovementImmediately();
	characterMovementComp->DisableMovement();

	RagdollCharacter->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");

	OnRagdollEnabled_End.Broadcast();

	if (RagdollCharacter->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(ServerUpdateMeshLocationTimerHandle, this, &UDragoncallRagdollComponent::UpdateMeshLocation, 0.1f, true);
	}
}

void UDragoncallRagdollComponent::EnableClientRagdollPhysics() const
{
	if(RagdollCharacter)
	{
		if(USkeletalMeshComponent* mesh = RagdollCharacter->GetMesh())
		{
			RagdollCharacter->SetHidden(false);
			mesh->SetAllBodiesSimulatePhysics(true);
			mesh->SetSimulatePhysics(true);
			mesh->WakeAllRigidBodies();
			mesh->bBlendPhysics = true;
		}
	}
}

void UDragoncallRagdollComponent::CollectPelvisBoneData(const USkeletalMeshComponent* Mesh)
{
	RagdollReplicationData = FDragoncallRagdollReplicationData{
		Mesh->GetSocketLocation(PelvisBoneName),
		Mesh->GetSocketRotation(PelvisBoneName).Euler()
	};
}

void UDragoncallRagdollComponent::DestroyComponent(bool bPromoteChildren /* = false */)
{
	Super::DestroyComponent(bPromoteChildren);

	if (const auto* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
}

void UDragoncallRagdollComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams params;
	params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RagdollReplicationData, params);
}

void UDragoncallRagdollComponent::UpdateMeshLocation()
{
	const auto* Mesh = RagdollCharacter.IsValid() ? RagdollCharacter->GetMesh() : nullptr;

	if (Mesh != nullptr)
	{
		CollectPelvisBoneData(Mesh);
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RagdollReplicationData, this);
	}
}
void UDragoncallRagdollComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!RagdollClientInterpolation.Interpolate(DeltaTime, RagdollCharacter.Get()))
	{
		// TODO: now DisableRagdoll disables Tick
		// SetComponentTickEnabled(false);
	}
}

void UDragoncallRagdollComponent::OnRep_RagdollReplicationData(const FDragoncallRagdollReplicationData& InOldRagdollData)
{
	RagdollClientInterpolation.ServerPelvisTransformToInterpolate = GetPelvisBoneTransform();
	if (RagdollClientInterpolation.IsServerPelvisTransformToInterpolateNotEmpty())
	{
		SetComponentTickEnabled(true);
	}
}

bool UDragoncallRagdollComponent::FDragoncallRagdollClientInterpolation::Interpolate(const float DeltaTime, const ACharacter* InCharacter) const
{
	if (!InCharacter)
		return false;

	USkeletalMeshComponent* mesh = InCharacter->GetMesh();
	if (!mesh)
		return false;

	FBodyInstance* pelvisBody = mesh->GetBodyInstance("pelvis");
	if (!pelvisBody)
		return false;

	const FTransform currentPelvisTransform = pelvisBody->GetUnrealWorldTransform();

	const FTransform targetPelvisTransform = ServerPelvisTransformToInterpolate;
	if (!IsServerPelvisTransformToInterpolateNotEmpty())
	{
		return false;
	}

	const FVector currentLocation = currentPelvisTransform.GetLocation();
	const FVector targetLocation = targetPelvisTransform.GetLocation();
	const FVector deltaLocation = targetLocation - currentLocation;
	const float distance = deltaLocation.Size();

	// If the distance is less than the threshold - do not make corrections
	if (distance < CorrectionPositionTrashHold)
	{
		return false;
	}

	// location
	const float desiredSpeed = distance / FMath::Max(DesiredCorrectionTime, KINDA_SMALL_NUMBER);
	const FVector dir = deltaLocation.GetSafeNormal();
	const FVector desiredVelocity = dir * desiredSpeed;

	const FVector currentVelocity = pelvisBody->GetUnrealWorldVelocity();
	const float pelvisMass = pelvisBody->GetBodyMass();

	// Impulse = (V_target - V_current) * Mass
	const FVector velocityChange = (desiredVelocity - currentVelocity);
	const FVector linearImpulse = velocityChange * pelvisMass;

	pelvisBody->AddImpulse(linearImpulse, false);

	// rotation
	const FQuat currentQuat = currentPelvisTransform.GetRotation();
	const FQuat targetQuat = targetPelvisTransform.GetRotation();

	const FQuat deltaQuat = targetQuat * currentQuat.Inverse();
	FVector deltaAxis;
	float deltaAngle;
	deltaQuat.ToAxisAndAngle(deltaAxis, deltaAngle);
	// [-pi, pi]
	deltaAngle = FMath::UnwindRadians(deltaAngle);

	// The simplest variant is a small “elastic” torque:
	// Torque = K * DeltaAngle * (axis of rotation)
	// where K is a constant, selected experimentally
	constexpr float minDeltaAngle = 0.01f;
	if (FMath::Abs(deltaAngle) > minDeltaAngle)
	{
		const FVector angularTorque = RotationStiffness * deltaAngle * deltaAxis.GetSafeNormal();
		pelvisBody->AddTorqueInRadians(angularTorque, false);
	}

	return true;
}
bool UDragoncallRagdollComponent::FDragoncallRagdollClientInterpolation::IsServerPelvisTransformToInterpolateNotEmpty() const
{
	return ServerPelvisTransformToInterpolate != FTransform::Identity;
}
