#pragma once
#include "HttpFwd.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"

namespace DC
{
namespace Json
{
TSharedPtr<FJsonObject> CreateJsonObject();
FString JsonToString(const TSharedPtr<FJsonObject>& InJson);
TSharedPtr<FJsonObject> StringToJson(const FString& InString);

TSharedPtr<FJsonObject> ObjectToJson(const UObject* InObject);
FString ObjectToJsonString(const UObject* InObject);

template<class T>
TSharedPtr<FJsonObject> StructToJson(T& InStruct)
{
	return StructToJson(T::StaticStruct(), &InStruct);
}

template<class T>
FString StructToJsonString(T& InStruct)
{
	return StructToJsonString(T::StaticStruct(), &InStruct);
}

template<typename StructType>
void GetJsonStringFromStruct(StructType FilledStruct, FString& StringOutput)
{
	FJsonObjectConverter::UStructToJsonObjectString(StructType::StaticStruct(), &FilledStruct, StringOutput, 0, 0);
}
template<typename StructType>
void GetStructFromJsonString(FHttpResponsePtr Response, StructType& StructOutput)
{
	StructType StructData;
	FString JsonString = Response->GetContentAsString();
	FJsonObjectConverter::JsonObjectStringToUStruct<StructType>(JsonString, &StructOutput, 0, 0);
}

TSharedPtr<FJsonObject> StructToJson(const UStruct* InStruct, const void* InStructMemory);
FString StructToJsonString(const UStruct* InStruct, const void* InStructMemory);

bool LoadJsonToObject(TSharedPtr<FJsonObject> InJson, UObject* InObject);
bool LoadJsonStringToObject(const FString& InJson, UObject* InObject);

void WriteJsonToArchive(const TSharedPtr<FJsonObject>& InJson, FArchive& OutArchive);
void ReadJsonFromArchive(TSharedPtr<FJsonObject>& OutJson, FArchive& OutArchive);

void JsonToByteArray(TSharedPtr<FJsonObject> InJson, TArray<uint8>& OutByteArray);
void ByteArrayToJson(const TArray<uint8>& InByteArray, TSharedPtr<FJsonObject>& OutJson);
} // namespace Json
} // namespace DC
