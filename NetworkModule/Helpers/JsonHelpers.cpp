#include "JsonHelpers.h"

#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace DC
{
namespace Json
{
TSharedPtr<FJsonObject> CreateJsonObject()
{
	return MakeShareable(new FJsonObject);
}

FString JsonToString(const TSharedPtr<FJsonObject>& InJson)
{
	FString OutString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
	FJsonSerializer::Serialize(InJson.ToSharedRef(), Writer);
	return OutString;
}

TSharedPtr<FJsonObject> StringToJson(const FString& InString)
{
	TSharedPtr<FJsonObject> OutJson;
	const auto Reader = TJsonReaderFactory<>::Create(InString);
	FJsonSerializer::Deserialize(Reader, OutJson);
	return OutJson;
}

TSharedPtr<FJsonObject> ObjectToJson(const UObject* InObject)
{
	return StringToJson(ObjectToJsonString(InObject));
}

FString ObjectToJsonString(const UObject* InObject)
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(InObject->GetClass(), InObject, JsonString);
	return JsonString;
}

TSharedPtr<FJsonObject> StructToJson(const UStruct* InStruct, const void* InStructMemory)
{
	return StringToJson(StructToJsonString(InStruct, InStructMemory));
}

FString StructToJsonString(const UStruct* InStruct, const void* InStructMemory)
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(InStruct, InStructMemory, JsonString);
	return JsonString;
}

bool LoadJsonToObject(TSharedPtr<FJsonObject> InJson, UObject* InObject)
{
	return FJsonObjectConverter::JsonObjectToUStruct(InJson.ToSharedRef(), InObject->GetClass(), InObject);
}

bool LoadJsonStringToObject(const FString& InJson, UObject* InObject)
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InJson);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		return LoadJsonToObject(JsonObject, InObject);
	}
	return false;
}

void WriteJsonToArchive(const TSharedPtr<FJsonObject>& InJson, FArchive& OutArchive)
{
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutArchive);
	FJsonSerializer::Serialize(InJson.ToSharedRef(), Writer);
}

void ReadJsonFromArchive(TSharedPtr<FJsonObject>& OutJson, FArchive& OutArchive)
{
	const auto Reader = TJsonReaderFactory<>::Create(&OutArchive);
	FJsonSerializer::Deserialize(Reader, OutJson);
}

void JsonToByteArray(TSharedPtr<FJsonObject> InJson, TArray<uint8>& OutByteArray)
{
}

void ByteArrayToJson(const TArray<uint8>& InByteArray, TSharedPtr<FJsonObject>& OutJson)
{
}

} // namespace Json
} // namespace DC