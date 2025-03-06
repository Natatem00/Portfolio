// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NetworkedGuid.generated.h"

USTRUCT()
struct FNetworkedGuid : public FGuid
{
	bool Serialize(FArchive& Ar);
	GENERATED_BODY()

	FNetworkedGuid()
	    : FGuid() {}
	FNetworkedGuid(const FGuid& InGuid)
	    : FGuid(InGuid) {}

	FNetworkedGuid& operator=(const FGuid& InGuid)
	{
		this->A = InGuid.A;
		this->B = InGuid.B;
		this->C = InGuid.C;
		this->D = InGuid.D;
		return *this;
	}

	/**
	 * Exports the GUIDs value to a string.
	 *
	 * @param ValueStr Will hold the string value.
	 * @param DefaultValue The default value.
	 * @param Parent Not used.
	 * @param PortFlags Not used.
	 * @param ExportRootScope Not used.
	 * @return true on success, false otherwise.
	 * @see ImportTextItem
	 */
	bool ExportTextItem(FString& ValueStr, FNetworkedGuid const& DefaultValue, UObject* Parent, int32 PortFlags, class UObject* ExportRootScope) const;

	/**
	 * Imports the GUIDs value from a text buffer.
	 *
	 * @param Buffer The text buffer to import from.
	 * @param PortFlags Not used.
	 * @param Parent Not used.
	 * @param ErrorText The output device for error logging.
	 * @return true on success, false otherwise.
	 * @see ExportTextItem
	 */

	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, class UObject* Parent, FOutputDevice* ErrorText);

	FString ToString(EGuidFormats Format = EGuidFormats::DigitsWithHyphens) const;
	FGuid ToGuid() const;
};

template<>
struct TIsPODType<FNetworkedGuid>
{
	enum
	{
		Value = true
	};
};

template<>
struct TStructOpsTypeTraits<FNetworkedGuid> : public TStructOpsTypeTraitsBase2<FNetworkedGuid>
{
	enum
	{
		WithExportTextItem = true,
		WithImportTextItem = true,
		WithZeroConstructor = true,
		WithSerializer = true
	};
};