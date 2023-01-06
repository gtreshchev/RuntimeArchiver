// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverTypes.h"
#include "UObject/Object.h"
#include "RuntimeArchiverRaw.generated.h"

DECLARE_DELEGATE_OneParam(FRuntimeArchiverRawMemoryResultNative, const TArray64<uint8>&);
DECLARE_DYNAMIC_DELEGATE_OneParam(FRuntimeArchiverRawMemoryResult, const TArray<uint8>&, Data);

/**
 * Raw archiver class. Works with various archives, especially those specific to the engine, such as Oodle, LZ4, GZip, etc.
 */
UCLASS()
class RUNTIMEARCHIVER_API URuntimeArchiverRaw : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Asynchronously compress raw data
	 *
	 * @param RawFormat Raw format
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @param UncompressedData Uncompressed data
	 * @param OnResult Delegate broadcasting the result
	 * @return Whether the compression was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Raw")
	static void CompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, TArray<uint8> UncompressedData, const FRuntimeArchiverRawMemoryResult& OnResult);

	/**
	 * Asynchronously compress raw data. Prefer to use this function if possible
	 *
	 * @param RawFormat Raw format
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @param UncompressedData Uncompressed data
	 * @param OnResult Delegate broadcasting the result
	 * @return Whether the compression was successful or not
	 */
	static void CompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, TArray64<uint8> UncompressedData, const FRuntimeArchiverRawMemoryResultNative& OnResult);

	/**
	 * Compress raw data
	 *
	 * @param RawFormat Raw format
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @param UncompressedData Uncompressed data
	 * @param CompressedData Out compressed data
	 * @return Whether the compression was successful or not
	 */
	static bool CompressRawData(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, const TArray64<uint8>& UncompressedData, TArray64<uint8>& CompressedData);

	/**
	 * Asynchronously uncompress raw data
	 *
	 * @param RawFormat Raw format
	 * @param CompressedData Compressed data
	 * @param OnResult Delegate broadcasting the result
	 * @return Whether the compression was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Raw")
	static void UncompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, TArray<uint8> CompressedData, const FRuntimeArchiverRawMemoryResult& OnResult);

	/**
	 * Asynchronously uncompress raw data. Prefer to use this function if possible
	 *
	 * @param RawFormat Raw format
	 * @param CompressedData Compressed data
	 * @param OnResult Delegate broadcasting the result
	 * @return Whether the compression was successful or not
	 */
	static void UncompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, TArray64<uint8> CompressedData, const FRuntimeArchiverRawMemoryResultNative& OnResult);

	/**
	 * Uncompress raw data
	 *
	 * @param RawFormat Raw format
	 * @param CompressedData Compressed data
	 * @param UncompressedData Out uncompressed data
	 * @return Whether the compression was successful or not
	 */
	static bool UncompressRawData(ERuntimeArchiverRawFormat RawFormat, TArray64<uint8> CompressedData, TArray64<uint8>& UncompressedData);

	/**
	 * Guess the size of the compressed raw data given the format. Returns the maximum possible size
	 *
	 * @param RawFormat Raw format
	 * @param UncompressedData Uncompressed data
	 * @return Guessed size
	 */
	static int64 GuessCompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& UncompressedData);

	/**
	 * Guess the size of the uncompressed raw data given the format. Returns the maximum possible size
	 *
	 * @param RawFormat Raw format
	 * @param CompressedData Compressed data
	 * @return Guessed size
	 */
	static int64 GuessUncompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& CompressedData);
};
