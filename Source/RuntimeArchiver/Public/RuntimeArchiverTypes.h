// Georgy Treshchev 2023.

#pragma once

#include "RuntimeArchiverTypes.generated.h"

/** Possible archiver errors */
UENUM(Blueprintable, Category = "Runtime Archiver")
enum class ERuntimeArchiverErrorCode : uint8
{
	NotInitialized,
	UnsupportedMode,
	UnsupportedLocation,
	ExtractError,
	AddError,
	CloseError,
	GetError,
	InvalidArgument
};

/** Archive entry compression level. The higher the level, the more compression */
UENUM(Blueprintable, Category = "Runtime Archiver")
enum class ERuntimeArchiverCompressionLevel : uint8
{
	Compression0 = 0 UMETA(DisplayName = "0", ToolTip = "No compression"),
	Compression1 = 1 UMETA(DisplayName = "1", ToolTip = "Best speed compression"),
	Compression2 = 2 UMETA(DisplayName = "2", ToolTip = "Compression 2"),
	Compression3 = 3 UMETA(DisplayName = "3", ToolTip = "Compression 3"),
	Compression4 = 4 UMETA(DisplayName = "4", ToolTip = "Compression 4"),
	Compression5 = 5 UMETA(DisplayName = "5", ToolTip = "Compression 5"),
	Compression6 = 6 UMETA(DisplayName = "6", ToolTip = "Compression 6"),
	Compression7 = 7 UMETA(DisplayName = "7", ToolTip = "Compression 7"),
	Compression8 = 8 UMETA(DisplayName = "8", ToolTip = "Compression 8"),
	Compression9 = 9 UMETA(DisplayName = "9", ToolTip = "Compression 9"),
	Compression10 = 10 UMETA(DisplayName = "10", ToolTip = "Best compression")
};

/** Archive mode */
UENUM()
enum class ERuntimeArchiverMode : uint8
{
	Undefined,
	Read,
	Write
};

/** Archive location */
UENUM()
enum class ERuntimeArchiverLocation : uint8
{
	Undefined,
	Storage,
	Memory
};

/** RAW archive format */
UENUM()
enum class ERuntimeArchiverRawFormat : uint8
{
	Oodle,
	GZip,
	LZ4
};

/** Information about archive entry. Used to search for files/directories in an archive to extract data. Do not fill it in manually */
USTRUCT(BlueprintType, Category = "Runtime Archiver")
struct FRuntimeArchiveEntry
{
	GENERATED_BODY()

	/** Entry index. Used to identify entry */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime Archiver")
	int32 Index;

	/** Entry name */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime Archiver")
	FString Name;

	/** Whether this entry is a directory (folder) or not */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime Archiver")
	bool bIsDirectory;

	/** Uncompressed entry size */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime Archiver")
	int64 UncompressedSize;

	/** Compressed entry size */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime Archiver")
	int64 CompressedSize;

	/** Entry creation time */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime Archiver")
	FDateTime CreationTime;

	/** Default constructor */
	FRuntimeArchiveEntry()
		: Index(0)
	  , Name(FString())
	  , bIsDirectory(false)
	  , UncompressedSize(0)
	  , CompressedSize(0)
	  , CreationTime(FDateTime())
	{
	}

	/** Custom constructor */
	FRuntimeArchiveEntry(int32 Index)
		: Index(Index)
	  , Name(FString())
	  , bIsDirectory(false)
	  , UncompressedSize(0)
	  , CompressedSize(0)
	  , CreationTime(FDateTime())
	{
	}
};

/** Delegate broadcasting the result of asynchronous archive operations */
DECLARE_DYNAMIC_DELEGATE_OneParam(FRuntimeArchiverAsyncOperationResult, bool, bSuccess);

/** Delegate broadcasting the progress of asynchronous archive operations */
DECLARE_DYNAMIC_DELEGATE_OneParam(FRuntimeArchiverAsyncOperationProgress, int32, Percentage);

/** Dynamic delegate broadcasting the result of asynchronous archive actions */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRuntimeArchiverAsyncActionResult, int32, Percentage);
