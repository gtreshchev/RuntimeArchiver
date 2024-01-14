// Georgy Treshchev 2024.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverBase.h"
#include "RuntimeArchiverZip.generated.h"

/**
 * Zip archiver class. Works with zip archives
 */
UCLASS(BlueprintType, Category = "Runtime Archiver")
class RUNTIMEARCHIVER_API URuntimeArchiverZip : public URuntimeArchiverBase
{
	GENERATED_BODY()

public:
	/** Default constructor */
	URuntimeArchiverZip();

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

public:
	/**
	 * Open an archive from storage to append
	 *
	 * @param ArchivePath Path to open an archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Open")
	bool OpenArchiveFromStorageToAppend(FString ArchivePath);

private:
	/** Whether to use append mode or not */
	bool bAppendMode;

	/** Miniz archiver */
	void* MinizArchiver;
};
