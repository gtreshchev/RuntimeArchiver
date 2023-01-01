// Georgy Treshchev 2023.

#include "RuntimeArchiverSubsystem.h"
#include "Engine.h"

URuntimeArchiverSubsystem* URuntimeArchiverSubsystem::GetArchiveSubsystem()
{
	return GEngine->GetEngineSubsystem<URuntimeArchiverSubsystem>();
}
