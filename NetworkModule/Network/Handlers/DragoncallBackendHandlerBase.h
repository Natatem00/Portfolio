#pragma once

#include "CoreMinimal.h"

#include "DragoncallBackendHandlerBase.generated.h"

USTRUCT()
struct DRAGONCALLNETWORK_API FDragoncallBackendHandlerBase
{
	GENERATED_BODY()
public:
	virtual ~FDragoncallBackendHandlerBase() = default;
	virtual void Init();
};
