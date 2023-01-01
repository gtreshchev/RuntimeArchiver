// Georgy Treshchev 2023.

#pragma once

#include "RuntimeArchiver.h"
#include "Subsystems/EngineSubsystem.h"
#include "RuntimeArchiverTypes.h"
#include "RuntimeArchiverSubsystem.generated.h"


/** Delegate broadcast of any archiver errors */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRuntimeArchiverError, ERuntimeArchiverErrorCode, ErrorCode, const FString&, ErrorString);

/**
 * Archiver subsystem. Used for singleton access of generic stuff
 */
UCLASS()
class RUNTIMEARCHIVER_API URuntimeArchiverSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	/** Bind to know when an error occurs while running the archiver */
	UPROPERTY(BlueprintAssignable, Category = "Runtime Archiver Subsystem|Delegates")
	FRuntimeArchiverError OnError;

	/** A little helper for getting a subsystem */
	static URuntimeArchiverSubsystem* GetArchiveSubsystem();
};
