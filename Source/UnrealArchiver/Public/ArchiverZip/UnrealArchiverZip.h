// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealArchiverBase.h"
#include "UnrealArchiverZip.generated.h"

/**
 * Zip archiver class. Works with zip archives
 */
UCLASS(BlueprintType, Category = "Unreal Archiver")
class UNREALARCHIVER_API UUnrealArchiverZip : public UUnrealArchiverBase
{
	GENERATED_BODY()

public:

	/** Default constructor */
	UUnrealArchiverZip();
	
	//~ Begin UUnrealArchiverBase Interface.
	virtual bool CreateArchiveInStorage(FString ArchivePath) override;
	virtual bool CreateArchiveInMemory(int32 InitialAllocationSize = 0) override;

	virtual bool OpenArchiveFromStorage(FString ArchivePath) override;
	virtual bool OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData) override;

	virtual bool CloseArchive() override;

	virtual int32 GetArchiveEntries() override;

	virtual bool GetArchiveEntryInfoByName(FString EntryName, FUnrealArchiveEntry& EntryInfo) override;
	virtual bool GetArchiveEntryInfoByIndex(int32 EntryIndex, FUnrealArchiveEntry& EntryInfo) override;
	
	virtual bool AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel) override;
	
	virtual bool ExtractEntryToMemory(const FUnrealArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData) override;

	virtual bool Initialize() override;
	virtual bool IsInitialized() const override;
	virtual void Reset() override;

	virtual void ReportError(EUnrealArchiverErrorCode ErrorCode, const FString& ErrorString) const override;
	//~ End UUnrealArchiverBase Interface.

public:
	/**
	 * Open an archive from storage to append
	 *
	 * @param ArchivePath Path to open an archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable)
	bool OpenArchiveFromStorageToAppend(FString ArchivePath);

private:
	/** Whether to use append mode or not */
	bool bAppendMode;
	void* MinizArchiver;
};
