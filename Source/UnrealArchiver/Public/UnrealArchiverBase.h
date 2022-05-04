// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
#include "UnrealArchiverTypes.h"
#include "UObject/Object.h"
#include "UnrealArchiverBase.generated.h"

/**
 * The base class for the archiver. Do not create it manually!
 */
UCLASS(Abstract, HideDropdown)
class UNREALARCHIVER_API UUnrealArchiverBase : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * Default constructor
	 */
	UUnrealArchiverBase();
	
	/**
	 * Create an archiver to archive/dearchive files and directories
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver", meta = (WorldContext="WorldContextObject", DeterminesOutputType = "ArchiverClass"))
	static UUnrealArchiverBase* CreateUnrealArchiver(UObject* WorldContextObject, TSubclassOf<UUnrealArchiverBase> ArchiverClass);

	//~ Begin UObject Interface.
	virtual void BeginDestroy() override;
	//~ End UObject Interface.

public:
	/**
	 * Create an archive in the specified path. The file will be created after calling the "CloseArchive" function
	 *
	 * @param ArchivePath Path to create an archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Create")
	virtual bool CreateArchiveInStorage(FString ArchivePath);

	/**
	 * Create an archive in memory
	 *
	 * @param InitialAllocationSize Estimated archive size if known. Avoids unnecessary memory allocation
	 * @return Whether the operation was successful or not
	 * @note Call GetArchiveDataFromMemory to get the archive data
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Create")
	virtual bool CreateArchiveInMemory(int32 InitialAllocationSize = 0);

	/**
	 * Open an archive from storage
	 *
	 * @param ArchivePath Path to open an archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Open")
	virtual bool OpenArchiveFromStorage(FString ArchivePath);

	/**
	 * Open an archive from memory
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Open")
	bool OpenArchiveFromMemory(TArray<uint8> ArchiveData);
	virtual bool OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData);

	/**
	 * Close previously created/opened archive
	 *
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Close")
	virtual bool CloseArchive();

	/**
	 * Get archive data created in memory
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 * @warning Call this only if you created the archive via CreateArchiveInMemory!
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Get")
	bool GetArchiveDataFromMemory(TArray<uint8>& ArchiveData);

	/**
	 * Get archive data created in memory. Prefer to use this function if possible
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 * @warning Call this only if you created the archive via CreateArchiveInMemory!
	 */
	virtual bool GetArchiveDataFromMemory(TArray64<uint8>& ArchiveData);

	/**
	 * Get the number of the archive entries
	 *
	 * @return Number of entries in the archive. -1 if nothing
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Get")
	virtual int32 GetArchiveEntries();

	/**
	 * Get information about the archive entry by name
	 *
	 * @param EntryName Archive entry name
	 * @param EntryInfo Retrieved information about the entry
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Get")
	virtual bool GetArchiveEntryInfoByName(FString EntryName, FUnrealArchiveEntry& EntryInfo);

	/**
	 * Get information about the archive entry by index
	 *
	 * @param EntryIndex Archive entry index. Must be >= 0
	 * @param EntryInfo Retrieved information about the entry
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Get")
	virtual bool GetArchiveEntryInfoByIndex(int32 EntryIndex, FUnrealArchiveEntry& EntryInfo);

	/**
	 * Add entry from storage. In other words, import the file into the archive
	 *
	 * @param EntryName Entry name. In other words, the name of the file in the archive
	 * @param FilePath Path to the file to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 * @note To add all files in a directory use "AddEntryFromStorage_Recursive" function
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Add")
	bool AddEntryFromStorage(FString EntryName, FString FilePath, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/**
	 * Recursively add entries from storage. Must be used for directories only
	 *
	 * @param OnResult Delegate broadcasting on result
	 * @param DirectoryPath Directory to be archived
	 * @param bAddParentDirectory Whether to add the specified directory as a parent
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Add")
	void AddEntryFromStorage_Recursively(FUnrealArchiverRecursiveResult OnResult, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

private:
	/**
	 * Internal function to recursively add entries from storage
	 *
	 * @param BaseDirectoryPathToExclude The base directory path to be used to exclude from the absolute file path
	 * @param DirectoryPath Directory to scan
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	bool AddEntryFromStorage_Recursively_Internal(FString BaseDirectoryPathToExclude, FString DirectoryPath, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

public:
	/**
	 * Add entry from memory. In other words, import the data in-memory into the archive
	 *
	 * @param EntryName Entry name. In other words, the name of the file in the archive
	 * @param DataToBeArchived Binary data to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Add")
	bool AddEntryFromMemory(FString EntryName, TArray<uint8> DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/**
	 * Add entry from memory. In other words, import the data in-memory into the archive. Prefer to use this function if possible
	 *
	 * @param EntryName Entry name. In other words, the name of the file in the archive
	 * @param DataToBeArchived Binary data to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	virtual bool AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/**
	 * Extract entry to storage. In other words, extract the file from the archive to storage
	 *
	 * @param EntryInfo Information about the entry
	 * @param FilePath Path to the file to extract to
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Extract")
	bool ExtractEntryToStorage(const FUnrealArchiveEntry& EntryInfo, FString FilePath, bool bForceOverwrite = true);

	/**
	 * Recursively extract entries to storage. Must be used for directories only
	 *
	 * @param OnResult Delegate broadcasting on result
	 * @param EntryName Path to the entry to extract to. Must be a directory only. Leave the field empty to use all entries
	 * @param DirectoryPath Path to the directory for exporting entries
	 * @param bAddParentDirectory Whether to add the specified directory as a parent
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Extract")
	void ExtractEntryToStorage_Recursively(FUnrealArchiverRecursiveResult OnResult, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite = true);

	/**
	 * Extract entry into memory. In other words, extract the file from the archive into memory
	 *
	 * @param EntryInfo Information about the entry
	 * @param UnarchivedData Unarchived entry data
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Unreal Archiver|Extract")
	bool ExtractEntryToMemory(const FUnrealArchiveEntry& EntryInfo, TArray<uint8>& UnarchivedData);

	/**
	 * Extract entry into memory. In other words, extract the file from the archive into memory. Prefer to use this function if possible
	 *
	 * @param EntryInfo Information about the entry
	 * @param UnarchivedData Unarchived entry data
	 * @return Whether the operation was successful or not
	 */
	virtual bool ExtractEntryToMemory(const FUnrealArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData);

	/**
	 * Initialize the archiver
	 */
	virtual bool Initialize() { return true; }

	/**
	 * Check whether the archiver is initialized
	 */
	virtual bool IsInitialized() const;

	/**
	 * Reset the archiver. Here it is supposed to clear all the data
	 */
	virtual void Reset();

	/**
	 * Report an error in the archiver
	 *
	 * @param ErrorCode Archiver error code
	 * @param ErrorString Error details
	 */
	virtual void ReportError(EUnrealArchiverErrorCode ErrorCode, const FString& ErrorString) const;

	/** Archive mode */
	EUnrealArchiveMode Mode;

	/** Archive location */
	EUnrealArchiveLocation Location;
};
