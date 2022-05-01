// Georgy Treshchev 2022.

#pragma once

#include "UnrealArchiver.h"
#include "Subsystems/EngineSubsystem.h"
#include "UnrealArchiverTypes.h"
#include "UnrealArchiverSubsystem.generated.h"


/** Delegate broadcast of any archiver errors */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUnrealArchiverError, EUnrealArchiverErrorCode, ErrorCode, const FString&, ErrorString);

/**
 * Archiver subsystem. Used for singleton access of generic stuff
 */
UCLASS()
class UNREALARCHIVER_API UUnrealArchiverSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	/** Bind to know when an error occurs while running the archiver */
	UPROPERTY(BlueprintAssignable, Category = "Unreal Archiver Subsystem|Delegates")
	FUnrealArchiverError OnError;

	/** A little helper for getting a subsystem */
	static UUnrealArchiverSubsystem* GetArchiveSubsystem();
};
