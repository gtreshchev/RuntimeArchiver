// Georgy Treshchev 2022.

#pragma once

#include "UnrealArchiverTypes.generated.h"

/** Possible archiver errors */
UENUM(Blueprintable, Category = "Unreal Archiver")
enum class EUnrealArchiverErrorCode : uint8
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

/** Archive format */
UENUM(Blueprintable, Category = "Unreal Archiver")
enum class EUnrealArchiveFormat : uint8
{
	ZIP
};

/** Archive entry compression level. The higher the level, the more compression */
UENUM(Blueprintable, Category = "Unreal Archiver")
enum class EUnrealEntryCompressionLevel : uint8
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
enum class EUnrealArchiveMode : uint8
{
	Undefined,
	Read,
	Write
};

/** Archive location */
UENUM()
enum class EUnrealArchiveLocation : uint8
{
	Undefined,
	Storage,
	Memory
};

/** Information about archive entry. Used to search for files/directories in an archive to extract data. Do not fill it in manually */
USTRUCT(BlueprintType, Category = "Unreal Archiver")
struct FUnrealArchiveEntry
{
	GENERATED_BODY()

	/** Entry index. Used to identify entry */
	UPROPERTY(BlueprintReadOnly)
	int32 Index;

	/** Entry name */
	UPROPERTY(BlueprintReadOnly)
	FString Name;

	/** Whether this entry is a directory (folder) or not */
	UPROPERTY(BlueprintReadOnly)
	bool bIsDirectory;

	/** Uncompressed entry size */
	UPROPERTY(BlueprintReadOnly)
	int64 UncompressedSize;

	/** Compressed entry size */
	UPROPERTY(BlueprintReadOnly)
	int64 CompressedSize;

	/** Entry creation time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime CreationTime;

	/** Default constructor */
	FUnrealArchiveEntry()
		: Index(0)
	  , Name(FString())
	  , bIsDirectory(false)
	  , UncompressedSize(0)
	  , CompressedSize(0)
	  , CreationTime(FDateTime())
	{
	}

	/** Custom constructor */
	FUnrealArchiveEntry(int32 Index)
		: Index(Index)
	  , Name(FString())
	  , bIsDirectory(false)
	  , UncompressedSize(0)
	  , CompressedSize(0)
	  , CreationTime(FDateTime())
	{
	}
};
