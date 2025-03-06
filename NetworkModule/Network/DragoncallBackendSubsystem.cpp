#include "DragoncallBackendSubsystem.h"

#include "DragoncallBackendSettings.h"
#include "DragoncallNetwork.h"
#include "Handlers/DragoncallBackendHandlerBase.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UDragoncallBackendSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Http = &FHttpModule::Get();
}

void UDragoncallBackendSubsystem::SetRequestHeaders(const FHttpRequestRef& Request)
{
	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Accepts"), TEXT("application/json"));
}

FHttpRequestRef UDragoncallBackendSubsystem::CreateRequest(const FString& Subroute, const EBackendService& InBackendService) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
	Request->SetURL(UDragoncallBackendSettings::GetSettings()->GetUrlByType(InBackendService) + Subroute);
	SetRequestHeaders(Request);
	return Request;
}

FHttpRequestRef UDragoncallBackendSubsystem::GetRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const
{
	return SetupRequest("GET", Subroute, ContentJsonString, InBackendService);
}

FHttpRequestRef UDragoncallBackendSubsystem::PostRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const
{
	return SetupRequest("POST", Subroute, ContentJsonString, InBackendService);
}

FHttpRequestRef UDragoncallBackendSubsystem::DeleteRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const
{
	return SetupRequest("DELETE", Subroute, ContentJsonString, InBackendService);
}

FHttpRequestRef UDragoncallBackendSubsystem::PutRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const
{
	return SetupRequest("PUT", Subroute, ContentJsonString, InBackendService);
}

FHttpRequestRef UDragoncallBackendSubsystem::SetupRequest(const FString& InRequestType, const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateRequest(Subroute, InBackendService);
	Request->SetVerb(InRequestType);
	Request->SetContentAsString(ContentJsonString);
	UE_LOG(LogDragoncallNetwork, Log, TEXT("%s Request URL: %s %s"), *InRequestType, *Request->GetURL(), *ContentJsonString);
	return Request;
}

void UDragoncallBackendSubsystem::Send(const FHttpRequestRef& Request)
{
	Request->ProcessRequest();
}

bool UDragoncallBackendSubsystem::ResponseIsValid(const FHttpResponsePtr& Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
		return false;

	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
		return true;

	UE_LOG(LogDragoncallNetwork, Warning, TEXT("Http Response returned error code: %d"), Response->GetResponseCode());
	return false;
}