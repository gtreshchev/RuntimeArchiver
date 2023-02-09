// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverBase.h"
#include "Launch/Resources/Version.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
class FRuntimeArchiverBaseStream;
#else
#include "Streams/RuntimeArchiverBaseStream.h"
#endif
#include "RuntimeArchiverTar.generated.h"

struct FTarHeader;
class FRuntimeArchiverTarEncapsulator;

/**
 * Tar archiver class. Works with tar archives. Inspired by Microtar
 */
UCLASS(BlueprintType, Category = "Runtime Archiver")
class RUNTIMEARCHIVER_API URuntimeArchiverTar : public URuntimeArchiverBase
{
	GENERATED_BODY()

public:
	//~ Begin URuntimeArchiverBase Interface
	virtual bool CreateArchiveInStorage(FString ArchivePath) override;
	virtual bool CreateArchiveInMemory(int32 InitialAllocationSize = 0) override;

	virtual bool OpenArchiveFromStorage(FString ArchivePath) override;
	virtual bool OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData) override;

	virtual bool CloseArchive() override;

	virtual bool GetArchiveData(TArray64<uint8>& ArchiveData) override;

	virtual bool GetArchiveEntries(int32& NumOfArchiveEntries) override;

	virtual bool GetArchiveEntryInfoByName(FString EntryName, FRuntimeArchiveEntry& EntryInfo) override;
	virtual bool GetArchiveEntryInfoByIndex(int32 EntryIndex, FRuntimeArchiveEntry& EntryInfo) override;

	virtual bool AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel) override;

	virtual bool ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData) override;

	virtual bool Initialize() override;
	virtual bool IsInitialized() const override;
	virtual void Reset() override;

	virtual void ReportError(ERuntimeArchiverErrorCode ErrorCode, const FString& ErrorString) const override;
	//~ End URuntimeArchiverBase Interface

private:
	/** Tar encapsulator */
	TUniquePtr<FRuntimeArchiverTarEncapsulator> TarEncapsulator;
};

/**
 * Encapsulator between archiver and stream that implements intermediate operations
 */
class FRuntimeArchiverTarEncapsulator
{
public:
	FRuntimeArchiverTarEncapsulator();
	virtual ~FRuntimeArchiverTarEncapsulator();

	/**
	 * Check if the archive is successfully opened or not
	 *
	 * @return Whether the archive is successfully opened or not
	 */
	bool IsValid();

	/**
	 * Test if tar archive contains valid data or not. Only the first header is checked for performance
	 * 
	 * @return Whether the archive contains valid data or not
	 */
	bool TestArchive();

	/**
	 * Open a tar archive from a file as a stream for reading or writing
	 *
	 * @param ArchivePath Path to archive to open
	 * @param bWrite Whether to open for writing or for reading
	 * @return Whether the archive was successfully opened or not
	 */
	bool OpenFile(const FString& ArchivePath, bool bWrite);

	/**
	 * Open a tar archive from memory as a stream for reading or writing
	 *
	 * @param ArchiveData Tar archive data. Must be empty if read-only mode is used
	 * @param InitialAllocationSize Estimated archive size if known. Avoids unnecessary memory allocation. Must be empty if read-only mode is used
	 * @param bWrite Whether to open for writing or for reading
	 * @return Whether the archive was successfully opened or not
	 */
	bool OpenMemory(const TArray64<uint8>& ArchiveData, int32 InitialAllocationSize, bool bWrite);

	/**
	 * Find header from the tar archive. Optionally updates the reading position of the found header. Works similar to the std::find_if algorithm
	 *
	 * @param ComparePredicate Compare predicate. Should return true when header is found, false otherwise. The first parameter is the header that can be the target, the second is its index in the archive
	 * @param Header Found header
	 * @param Index Found entry index
	 * @param bRemainPosition Whether to keep the previous read/write position, or update
	 * @return Whether the header was found or not
	 */
	bool FindIf(TFunctionRef<bool(const FTarHeader&, int32)> ComparePredicate, FTarHeader& Header, int32& Index, bool bRemainPosition);

	/**
	 * Get the number of tar archive entries
	 *
	 * @param NumOfArchiveEntries The number of entries in the archive
	 * @return Whether the operation was successful or not
	 */
	bool GetArchiveEntries(int32& NumOfArchiveEntries);

	/**
	 * Get tar archive data
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 */
	bool GetArchiveData(TArray64<uint8>& ArchiveData);

	/**
	 * Reset read/write position
	 * 
	 * @return Whether the operation was successful or not
	 */
	bool Rewind();

	/**
	 * Seek to the next header
	 *
	 * @return Whether the operation was successful or not
	 */
	bool Next();

	/**
	 * Read the header from the current position
	 *
	 * @param Header Read header
	 * @return Whether the operation was successful or not
	 */
	bool ReadHeader(FTarHeader& Header);

	/**
	 * Read the archived data from the current position
	 *
	 * @param UnarchivedData Unarchived data
	 * @return Whether the operation was successful or not
	 */
	bool ReadData(TArray64<uint8>& UnarchivedData);

	/**
	 * Write the header from the current position
	 *
	 * @param Header Header to write
	 * @return Whether the operation was successful or not
	 */
	bool WriteHeader(const FTarHeader& Header);

	/**
	 * Write the archived data from the current position
	 *
	 * @param DataToBeArchived Binary data to be archived. Leave it blank if it is a directory
	 * @return Whether the operation was successful or not
	 */
	bool WriteData(const TArray64<uint8>& DataToBeArchived);

	/**
	 * Write null bytes represented as null character '\0'
	 *
	 * @param NumOfBytes Number of null bytes to write
	 * @return Whether the operation was successful or not
	 */
	bool WriteNullBytes(int64 NumOfBytes) const;

	/**
	 * Write additional null bytes at the end to finalize the archive data
	 *
	 * @return Whether the operation was successful or not
	 */
	bool Finalize();

private:
	/** Used stream */
	TUniquePtr<FRuntimeArchiverBaseStream> Stream;

	/** Remaining read or write data size */
	int64 RemainingDataSize;

	/** Last header position */
	int64 LastHeaderPosition;

	/** Cached number of headers for a small optimization */
	int32 CachedNumOfHeaders;

	/** Whether the tar archive was finalized or not */
	bool bIsFinalized;
};
