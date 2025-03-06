// Fill out your copyright notice in the Description page of Project Settings.

#include "DragoncallMeleeWeaponEquipActor.h"


bool operator==(const FHitResult& lhs, const FHitResult& rhs)
{
	return lhs.GetActor() == rhs.GetActor() && lhs.PenetrationDepth == rhs.PenetrationDepth;
}

#pragma region SUBSTEPING
ADragoncallMeleeWeaponEquipActor::FDragoncallZoneData ADragoncallMeleeWeaponEquipActor::GetZoneData(const TObjectPtr<UBoxComponent>& InZoneComponent)
{
	FDragoncallZoneData data;
	data.LocationOffset = InZoneComponent->GetRelativeLocation();
	data.RotationOffset = InZoneComponent->GetRelativeRotation();
	data.CollisionShape = InZoneComponent->GetCollisionShape();
	data.ScaledBoxExtent = InZoneComponent->GetScaledBoxExtent();

	return data;
}
void ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::GetActorsToIgnore(ADragoncallMeleeWeaponEquipActor* InActor, TArray<AActor*>& actorsToIgnore)
{
	actorsToIgnore.Add(InActor->GetOwner());
	actorsToIgnore.Add(InActor);

	TArray<AActor*> attachedActors;
	InActor->GetOwner()->GetAttachedActors(attachedActors);
	actorsToIgnore.Append(MoveTemp(attachedActors));
}
FTransform ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::GetZoneTransform(const ADragoncallMeleeWeaponEquipActor* InActor, const FDragoncallZoneData& InZonesSettings) const
{
	FTransform RootComponentTransform = InActor->GetRootComponent()->GetComponentTransform();
	FTransform ZoneTransformRed = FTransform(InZonesSettings.RotationOffset, InZonesSettings.LocationOffset);
	ZoneTransformRed = ZoneTransformRed * RootComponentTransform;

	return MoveTemp(ZoneTransformRed);
}
bool ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::ProceedHitResults(ADragoncallMeleeWeaponEquipActor* InActor, TArray<FHitResult>& InHitResults) const
{
	return InActor->ProceedHitResults(InHitResults);
}
uint8 ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::CalculateNumSubsteps(float DeltaTime) const
{
	return FMath::Clamp((DeltaTime / FixedDeltaTime) * SubstepInterval.Min, SubstepInterval.Min, SubstepInterval.Max);
}
void ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::DoSubSteps(ADragoncallMeleeWeaponEquipActor* InActor, TArrayView<FDragoncallLastFrameAttackData> InOutLastFrameData, const TMapZone& InZonesSettings, float DeltaTime) const
{
	if (!InActor)
	{
		return;
	}

	TArray<AActor*> actorsToIgnore;
	GetActorsToIgnore(InActor, actorsToIgnore);

	FRotator startRotation;
	FVector totalHalfSize;
	FVector startLocation;
	FVector finalPosition;
	FVector TracePoint;

	CalculatePoints(InActor, InZonesSettings, totalHalfSize, startLocation, finalPosition, TracePoint, startRotation);

	auto traceFunction = [](const UWorld* InWorld, const FVector& InTracePointStart, const FVector& InTracePointEnd, const FVector& InHalfSize, const FRotator& InTraceRotation, const FName& InProfileName, const TArray<AActor*>& ActorsToIgnore, TArray<FHitResult>& OutHitResult)
	{
		UKismetSystemLibrary::BoxTraceMultiByProfile(
			InWorld,
			InTracePointStart,
			InTracePointEnd,
			InHalfSize,
			InTraceRotation,
			InProfileName,
			false,
			ActorsToIgnore,
			CVarShowDebugCollision.GetValueOnGameThread() ? EDrawDebugTrace::Type::ForOneFrame : EDrawDebugTrace::Type::None,
			OutHitResult,
			true,
			FLinearColor::Red);
	};
	TArray<FHitResult> hitResult;

	if (InOutLastFrameData[EDragoncallTraceIndex::RED].bHasLastFrameData)
	{
		const uint8 currentNumSubsteps = CalculateNumSubsteps(DeltaTime);
		FVector startPosition = InOutLastFrameData[EDragoncallTraceIndex::RED].LastFramePosition;
		for (uint8 i = 0; i < currentNumSubsteps; ++i)
		{
			const FVector point = FMath::Lerp(InOutLastFrameData[EDragoncallTraceIndex::RED].LastFramePosition, TracePoint, i / static_cast<float>(currentNumSubsteps));
			const FRotator rotation = FMath::Lerp(InOutLastFrameData[EDragoncallTraceIndex::RED].LastFrameRotation, startRotation, (i) / static_cast<float>(currentNumSubsteps));

			traceFunction(InActor->GetWorld(), startPosition, point, totalHalfSize, rotation, InActor->ImpactZoneCollisionProfileName.Name, actorsToIgnore, hitResult);
			if (ProceedHitResults(InActor, hitResult))
			{
				break;
			}
			startPosition = point;
		}
	}
	else
	{
		traceFunction(InActor->GetWorld(), TracePoint, TracePoint, totalHalfSize, startRotation, InActor->ImpactZoneCollisionProfileName.Name, actorsToIgnore, hitResult);
		ProceedHitResults(InActor, hitResult);
	}
	InOutLastFrameData[EDragoncallTraceIndex::RED].LastFramePosition = TracePoint;
	InOutLastFrameData[EDragoncallTraceIndex::RED].LastFrameRotation = startRotation;
	InOutLastFrameData[EDragoncallTraceIndex::RED].bHasLastFrameData = true;
}
void ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::CalculatePoints(const ADragoncallMeleeWeaponEquipActor* InActor, const ADragoncallMeleeWeaponEquipActor::TMapZone& InZonesSettings, FVector& OutTotalHalfSize, FVector& OutStartLocation, FVector& OutFinalPosition, FVector& OutTracePoint, FRotator& OutRotation) const
{
	const FDragoncallZoneData zoneDataRed = InZonesSettings[DragoncallTags.Dragoncall.Technical.Zones.Red];
	const FDragoncallZoneData zoneDataGreen = InZonesSettings[DragoncallTags.Dragoncall.Technical.Zones.Green];
	const FDragoncallZoneData zoneDataOrange = InZonesSettings[DragoncallTags.Dragoncall.Technical.Zones.Orange];

	FTransform ZoneTransformRed = GetZoneTransform(InActor, zoneDataRed);

	FTransform ZoneTransformGreen = GetZoneTransform(InActor, zoneDataGreen);

	OutTotalHalfSize = zoneDataRed.CollisionShape.GetBox();
	OutTotalHalfSize.Z += zoneDataOrange.CollisionShape.GetBox().Z + zoneDataGreen.CollisionShape.GetBox().Z;

	const FVector halfSizeGreen = zoneDataGreen.CollisionShape.GetBox();
	const FVector halfSizeRed = zoneDataRed.CollisionShape.GetBox();

	const FVector direction = (ZoneTransformGreen.GetLocation() - ZoneTransformRed.GetLocation()).GetSafeNormal();
	OutStartLocation = ZoneTransformRed.GetLocation() - (direction * halfSizeRed.Size());
	OutFinalPosition = ZoneTransformGreen.GetLocation() + (direction * halfSizeGreen.Size());

	constexpr float middleMultiplier = 0.5f;
	OutTracePoint = (OutStartLocation + OutFinalPosition) * middleMultiplier;

	OutRotation = ZoneTransformRed.GetRotation().Rotator();
}
#if ENABLE_DRAW_DEBUG
void ADragoncallMeleeWeaponEquipActor::FDragoncallAttackSubstepCalculation::DrawDebug(const ADragoncallMeleeWeaponEquipActor* InActor, TArrayView<FDragoncallLastFrameAttackData> InOutLastFrameData, const TMapZone& InZonesSettings, float DeltaTime) const
{
	FRotator startRotation;
	FVector totalHalfSize;
	FVector startLocation;
	FVector finalPosition;
	FVector TracePoint;

	CalculatePoints(InActor, InZonesSettings, totalHalfSize, startLocation, finalPosition, TracePoint, startRotation);

	DrawDebugSphere(InActor->GetWorld(), startLocation, 2, 20, FColor::Red, false);
	DrawDebugSphere(InActor->GetWorld(), finalPosition, 2, 20, FColor::Red, false);

	auto debugTraceFunction = [](const UWorld* InWorld, const FVector& InTracePointStart, const FVector& InTracePointEnd, const FVector& InHalfSize, const FRotator& InTraceRotation)
	{
		TArray<FHitResult> hitResult;
		DrawDebugBoxTraceMulti(InWorld, InTracePointStart, InTracePointEnd, InHalfSize, InTraceRotation, EDrawDebugTrace::Type::ForOneFrame, false, hitResult, FColor::Blue, FColor::Blue, -1);
	};

	const uint8 currentNumSubsteps = CalculateNumSubsteps(DeltaTime);
	UKismetSystemLibrary::PrintString(InActor->GetWorld(), FString::Printf(TEXT("Num Of Substeps = %d"), currentNumSubsteps), true, false, FLinearColor::Red, 0.f);
	FVector startPosition = InOutLastFrameData[EDragoncallTraceIndex::RED].LastFramePosition;
	for (uint8 i = 0; i < currentNumSubsteps; ++i)
	{
		const FVector point = FMath::Lerp(InOutLastFrameData[EDragoncallTraceIndex::RED].LastFramePosition, TracePoint, i / static_cast<float>(currentNumSubsteps));
		const FRotator rotation = FMath::Lerp(InOutLastFrameData[EDragoncallTraceIndex::RED].LastFrameRotation, startRotation, (i) / static_cast<float>(currentNumSubsteps));

		debugTraceFunction(InActor->GetWorld(), startPosition, point, totalHalfSize, rotation);
		startPosition = point;
	}

	InOutLastFrameData[EDragoncallTraceIndex::RED].LastFramePosition = TracePoint;
	InOutLastFrameData[EDragoncallTraceIndex::RED].LastFrameRotation = startRotation;
}
#endif
#pragma endregion SUBSTEPING