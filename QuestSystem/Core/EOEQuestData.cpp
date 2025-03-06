// Fill out your copyright notice in the Description page of Project Settings.

#include "EOEQuestData.h"

#include "Components/EOEQuestObjectComponent.h"
#include "Interfaces/EOEQuestActor.h"

bool FEOEQuestObjectBase::CheckQuestObjectData(const FEOEQuestObjectData& Object) const
{
	return Object.ObjectTag == ObjectTag;
}
bool FEOEQuestObjectBase::ValidateQuestObejct() const
{
	return !ObjectTag.IsNone();
}
FString FEOEQuestObjectBase::ToString() const
{
	return FString::Printf(TEXT("ObjectTag: %s"), *ObjectTag.ToString());
}
bool FEOEQuestObjectWithActor::CheckQuestObjectData(const FEOEQuestObjectData& Object) const
{
	bool breturnBool = true;

	if (Object.bShouldCheckTag)
	{
		breturnBool = Super::CheckQuestObjectData(Object);
	}

	breturnBool = breturnBool && Object.Object == Actor.Get();
	return breturnBool;
}
bool FEOEQuestObjectWithActor::ValidateQuestObejct() const
{
	AActor* questObject = Actor.Get();
	if (questObject == nullptr)
	{
		return false;
	}

	UEOEQuestObjectComponent* questComponent = questObject->FindComponentByClass<UEOEQuestObjectComponent>();
	if (questComponent == nullptr)
	{
		return false;
	}

	if (questObject->Implements<UEOEQuestSkipCondition>() && IEOEQuestSkipCondition::Execute_CanSkipQuestActor(const_cast<AActor*>(questObject)) == true)
	{
		return false;
	}

	return true;
}
FString FEOEQuestObjectWithActor::ToString() const
{
	return "Actor name: " + Actor.GetAssetName() + " " + FEOEQuestObjectBase::ToString();
}