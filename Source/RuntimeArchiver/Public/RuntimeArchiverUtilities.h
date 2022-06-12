// Georgy Treshchev 2022.

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

	/**
	 * Compress raw data using a format supported by the engine (e.g. LZ4, GZip, etc.).
	 *
	 * @param RawFormat Raw format
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @param UncompressedData Uncompressed data
	 * @param CompressedData Out compressed data
	 * @return Whether the compression was successful or not
	 */
	static bool CompressRawData(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, const TArray64<uint8>& UncompressedData, TArray64<uint8>& CompressedData);
	
	/**
	 * Uncompress raw data using a format supported by the engine (e.g. LZ4, GZip, etc.).
	 *
	 * @param RawFormat Raw format
	 * @param UncompressedData Uncompressed data
	 * @param CompressedData Out compressed data
	 * @return Whether the compression was successful or not
	 */
	static bool UncompressRawData(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& CompressedData, TArray64<uint8>& UncompressedData);

	/**
	 * Guess the size of the compressed raw data given the format. Returns the maximum possible size
	 *
	 * @param RawFormat Raw format
	 * @param UncompressedData Uncompressed data
	 */
	static int64 GuessCompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& UncompressedData);

	/**
	 * Guess the size of the uncompressed raw data given the format. Returns the maximum possible size
	 *
	 * @param RawFormat Raw format
	 * @param CompressedData Compressed data
	 */
	static int64 GuessUncompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& CompressedData);
};
