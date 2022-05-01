// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnrealArchiver.h"
#include "UnrealArchiverDefines.h"

#define LOCTEXT_NAMESPACE "FUnrealArchiverModule"

void FUnrealArchiverModule::StartupModule()
{
}

void FUnrealArchiverModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealArchiverModule, UnrealArchiver)

DEFINE_LOG_CATEGORY(LogUnrealArchiver);
