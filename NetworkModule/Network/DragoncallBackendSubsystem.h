#pragma once

#include "DragoncallBackendSettings.h"
#include "Handlers/DragoncallBackendHandlerBase.h"
#include "HttpFwd.h"

#include "DragoncallBackendSubsystem.generated.h"

struct FDragoncallBackendHandlerBase;
class FHttpModule;
UCLASS()
class DRAGONCALLNETWORK_API UDragoncallBackendSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	template<class HandlerClass>
	typename TEnableIf<TIsDerivedFrom<HandlerClass, FDragoncallBackendHandlerBase>::Value, HandlerClass&>::Type
	GetOrCreateHandler()
	{
		auto LHandlerClass = HandlerClass::StaticStruct();
		if (auto LHandler = GlobalHandlers.Find(LHandlerClass))
		{
			return static_cast<HandlerClass&>(*LHandler->Get());
		}
		TSharedPtr<FDragoncallBackendHandlerBase> LNewHandler = GlobalHandlers.Emplace(LHandlerClass, MakeShared<HandlerClass>());
		LNewHandler->Init();
		return static_cast<HandlerClass&>(*LNewHandler.Get());
	}

	template<class HandlerClass>
	typename TEnableIf<TIsDerivedFrom<HandlerClass, FDragoncallBackendHandlerBase>::Value, HandlerClass&>::Type
	GetOrCreateHandlerFor(UObject* InObject)
	{
		auto LHandlerClass = HandlerClass::StaticStruct();
		if (const auto LHandler = HandlersForObjects.Find({ InObject, LHandlerClass }))
		{
			return static_cast<HandlerClass&>(*LHandler->Get());
		}

		auto LNewHandler = HandlersForObjects.Emplace({ InObject, LHandlerClass }, MakeShared<HandlerClass>());
		LNewHandler->Init();

		return static_cast<HandlerClass&>(*LNewHandler.Get());
	}
	FHttpRequestRef GetRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const;
	FHttpRequestRef PostRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const;
	FHttpRequestRef DeleteRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const;
	FHttpRequestRef PutRequest(const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const;
	static void Send(const FHttpRequestRef& Request);

	static bool ResponseIsValid(const FHttpResponsePtr& Response, bool bWasSuccessful);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
	FHttpRequestRef CreateRequest(const FString& Subroute, const EBackendService& InBackendService) const;
	static void SetRequestHeaders(const FHttpRequestRef& Request);
	FHttpRequestRef SetupRequest(const FString& InRequestType, const FString& Subroute, const FString& ContentJsonString, const EBackendService& InBackendService) const;

private:
	TMap<UScriptStruct*, TSharedPtr<FDragoncallBackendHandlerBase>> GlobalHandlers;
	TMap<TPair<TWeakObjectPtr<UObject>, UScriptStruct*>, TSharedPtr<FDragoncallBackendHandlerBase>> HandlersForObjects;

	FHttpModule* Http;
};
