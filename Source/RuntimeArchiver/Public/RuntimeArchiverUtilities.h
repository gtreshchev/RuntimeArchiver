// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverTypes.h"
#include "UObject/Object.h"
#include "RuntimeArchiverUtilities.generated.h"

/**
 * Provides general purpose utility functions for archivers
 */
UCLASS()
class RUNTIMEARCHIVER_API URuntimeArchiverUtilities : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * Parse directories from path. It is assumed that the path is given to the file.
	 * For example the path "Folder/SubFolder/File.txt" will return an array of two elements, "Folder/" and "Folder/SubFolder/"
	 *
	 * @param FilePath The path to parse directories from
	 * @return Parsed directories
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Utilities")
	static TArray<FString> ParseDirectories(const FString& FilePath);
};
