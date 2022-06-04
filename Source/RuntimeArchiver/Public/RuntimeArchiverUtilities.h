// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
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

	/**
	 * Compress data using a format supported by the engine (e.g. LZ4, GZip, etc.).
	 *
	 * @param FormatName Name of the format
	 * @param UncompressedData Uncompressed data
	 * @param CompressedData Out compressed data
	 * @return Whether the compression was successful or not
	 */
	RUNTIMEARCHIVER_API static bool CompressData(FName FormatName, const TArray64<uint8>& UncompressedData, TArray64<uint8>& CompressedData);
};
