// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

namespace DragoncallNetwork
{
inline static const FVector_NetQuantize10 InvalidLocation = FVector(FLT_MAX);

FORCEINLINE bool IsValidLocation(const FVector_NetQuantize10& TestLocation)
{
	return -InvalidLocation.X < TestLocation.X && TestLocation.X < InvalidLocation.X && -InvalidLocation.Y < TestLocation.Y && TestLocation.Y < InvalidLocation.Y && -InvalidLocation.Z < TestLocation.Z && TestLocation.Z < InvalidLocation.Z;
}
} // DragoncallNetwork
