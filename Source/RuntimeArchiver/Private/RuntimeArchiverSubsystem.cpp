// Georgy Treshchev 2022.

#include "RuntimeArchiverSubsystem.h"
#include "Engine.h"

URuntimeArchiverSubsystem* URuntimeArchiverSubsystem::GetArchiveSubsystem()
{
	return GEngine->GetEngineSubsystem<URuntimeArchiverSubsystem>();
}
