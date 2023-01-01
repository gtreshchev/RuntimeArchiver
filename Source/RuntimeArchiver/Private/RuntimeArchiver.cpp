// Georgy Treshchev 2023.

#include "RuntimeArchiver.h"
#include "RuntimeArchiverDefines.h"

#define LOCTEXT_NAMESPACE "FRuntimeArchiverModule"

void FRuntimeArchiverModule::StartupModule()
{
}

void FRuntimeArchiverModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRuntimeArchiverModule, RuntimeArchiver)

DEFINE_LOG_CATEGORY(LogRuntimeArchiver);
