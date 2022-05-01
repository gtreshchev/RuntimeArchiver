// Georgy Treshchev 2022.

#include "UnrealArchiverSubsystem.h"
#include "Engine.h"

UUnrealArchiverSubsystem* UUnrealArchiverSubsystem::GetArchiveSubsystem()
{
	return GEngine->GetEngineSubsystem<UUnrealArchiverSubsystem>();
}
