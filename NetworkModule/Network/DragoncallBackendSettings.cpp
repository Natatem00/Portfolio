#include "DragoncallBackendSettings.h"

FName UDragoncallBackendSettings::GetSectionName() const
{
	return "Backend";
}

FName UDragoncallBackendSettings::GetCategoryName() const
{
	return "Dragoncall";
}

FName UDragoncallBackendSettings::GetContainerName() const
{
	return "Project";
}
const FString& UDragoncallBackendSettings::GetUrlByType(EBackendService InService) const
{
	return ServerUrls[InService];
}

#if WITH_EDITOR
#define LOCTEXT_NAMESPACE "BackendSettings"

FText UDragoncallBackendSettings::GetSectionText() const
{
	return LOCTEXT("BackendSettingsName", "Backend");
}

FText UDragoncallBackendSettings::GetSectionDescription() const
{
	return LOCTEXT("BackendSettingsDescription", "Settings for Backend");
}

#undef LOCTEXT_NAMESPACE
#endif