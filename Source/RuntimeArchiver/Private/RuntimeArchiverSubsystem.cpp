// Georgy Treshchev 2024.

#include "RuntimeArchiverSubsystem.h"
#include "Engine.h"

URuntimeArchiverSubsystem* URuntimeArchiverSubsystem::GetArchiveSubsystem()
{
	return GEngine->GetEngineSubsystem<URuntimeArchiverSubsystem>();
}
