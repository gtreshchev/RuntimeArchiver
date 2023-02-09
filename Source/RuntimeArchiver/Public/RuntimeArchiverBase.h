// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverTypes.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "RuntimeArchiverBase.generated.h"

/**
 * The base class for the archiver. Do not create it manually!
 */
UCLASS(Abstract, HideDropdown, BlueprintType, Category = "Runtime Archiver")
class RUNTIMEARCHIVER_API URuntimeArchiverBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Default constructor
	 */
	URuntimeArchiverBase();

	/**
	 * Create an archiver to archive/unarchive files and directories
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver", meta = (WorldContext="WorldContextObject", DeterminesOutputType = "ArchiverClass"))
	static URuntimeArchiverBase* CreateRuntimeArchiver(UObject* WorldContextObject, UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<URuntimeArchiverBase> ArchiverClass);

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

public:
	/**
	 * Create an archive in the specified path. The file will be created after calling the "CloseArchive" function
	 *
	 * @param ArchivePath Path to create an archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Create")
	virtual bool CreateArchiveInStorage(FString ArchivePath);

	/**
	 * Create an archive in memory
	 *
	 * @param InitialAllocationSize Estimated archive size if known. Avoids unnecessary memory allocation
	 * @return Whether the operation was successful or not
	 * @note Call GetArchiveDataFromMemory to get the archive data
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Create")
	virtual bool CreateArchiveInMemory(int32 InitialAllocationSize = 0);

	/**
	 * Open an archive from storage
	 *
	 * @param ArchivePath Path to open an archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Open")
	virtual bool OpenArchiveFromStorage(FString ArchivePath);

	/**
	 * Open an archive from memory
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Open")
	bool OpenArchiveFromMemory(TArray<uint8> ArchiveData);

	/**
	 * Open an archive from memory. Prefer to use this function if possible
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 */
	virtual bool OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData);

	/**
	 * Close previously created/opened archive
	 *
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Close")
	virtual bool CloseArchive();

	/**
	 * Get archive data created in memory
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Get")
	bool GetArchiveData(TArray<uint8>& ArchiveData);

	/**
	 * Get archive data created in memory. Prefer to use this function if possible
	 *
	 * @param ArchiveData Binary archive data
	 * @return Whether the operation was successful or not
	 */
	virtual bool GetArchiveData(TArray64<uint8>& ArchiveData);

	/**
	 * Get the number of the archive entries
	 *
	 * @param NumOfArchiveEntries The number of entries in the archive
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Get")
	virtual bool GetArchiveEntries(int32& NumOfArchiveEntries);

	/**
	 * Get information about the archive entry by name
	 *
	 * @param EntryName Archive entry name
	 * @param EntryInfo Retrieved information about the entry
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Get")
	virtual bool GetArchiveEntryInfoByName(FString EntryName, FRuntimeArchiveEntry& EntryInfo);

	/**
	 * Get information about the archive entry by index
	 *
	 * @param EntryIndex Archive entry index. Must be >= 0
	 * @param EntryInfo Retrieved information about the entry
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Get")
	virtual bool GetArchiveEntryInfoByIndex(int32 EntryIndex, FRuntimeArchiveEntry& EntryInfo);

	/**
	 * Add entry from storage. In other words, import the file into the archive
	 *
	 * @param EntryName Entry name. In other words, the name of the file in the archive
	 * @param FilePath Path to the file to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Add")
	bool AddEntryFromStorage(FString EntryName, FString FilePath, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

	/**
	 * Add entries from storage. In other words, import files into the archive
	 *
	 * @param OnResult Delegate broadcasting the result
	 * @param OnProgress Delegate broadcasting the progress
	 * @param FilePaths File paths to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Add")
	void AddEntriesFromStorage(const FRuntimeArchiverAsyncOperationResult& OnResult, const FRuntimeArchiverAsyncOperationProgress& OnProgress, TArray<FString> FilePaths, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

	/**
	 * Add entries from storage. Must be used for directories only
	 *
	 * @param OnResult Delegate broadcasting the result
	 * @param DirectoryPath Directory to be archived
	 * @param bAddParentDirectory Whether to add the specified directory as a parent
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Add")
	void AddEntriesFromStorage_Directory(const FRuntimeArchiverAsyncOperationResult& OnResult, FString DirectoryPath, bool bAddParentDirectory, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

private:
	/**
	 * Internal function to recursively add entries from storage
	 *
	 * @param BaseDirectoryPathToExclude The base directory path to be used to exclude from the absolute file path
	 * @param DirectoryPath Directory to scan
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	bool AddEntriesFromStorage_Directory_Internal(FString BaseDirectoryPathToExclude, FString DirectoryPath, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

public:
	/**
	 * Add entry from memory. In other words, import the data in-memory into the archive
	 *
	 * @param EntryName Entry name. In other words, the name of the file in the archive
	 * @param DataToBeArchived Binary data to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Add")
	bool AddEntryFromMemory(FString EntryName, TArray<uint8> DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

	/**
	 * Add entry from memory. In other words, import the data in-memory into the archive. Prefer to use this function if possible
	 *
	 * @param EntryName Entry name. In other words, the name of the file in the archive
	 * @param DataToBeArchived Binary data to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 * @return Whether the operation was successful or not
	 */
	virtual bool AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

	/**
	 * Extract entry to storage. In other words, extract the file from the archive to storage
	 *
	 * @param EntryInfo Information about the entry
	 * @param FilePath Path to the file to extract to
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Extract")
	bool ExtractEntryToStorage(const FRuntimeArchiveEntry& EntryInfo, FString FilePath, bool bForceOverwrite = true);

	/**
	 * Extract entries to storage. In other words, extract the file from the archive to storage
	 *
	 * @param OnResult Delegate broadcasting the result
	 * @param OnProgress Delegate broadcasting the progress
	 * @param EntryInfo Array of all entries to extract
	 * @param DirectoryPath Path to the directory for exporting entries
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Extract")
	void ExtractEntriesToStorage(const FRuntimeArchiverAsyncOperationResult& OnResult, const FRuntimeArchiverAsyncOperationProgress& OnProgress, TArray<FRuntimeArchiveEntry> EntryInfo, FString DirectoryPath, bool bForceOverwrite = true);

	/**
	 * Extract entries to storage. Must be used for directories only
	 *
	 * @param OnResult Delegate broadcasting the result
	 * @param EntryName Path to the entry to extract to. Must be a directory only. Leave the field empty to use all entries
	 * @param DirectoryPath Path to the directory for exporting entries
	 * @param bAddParentDirectory Whether to add the specified directory as a parent
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Extract")
	void ExtractEntriesToStorage_Directory(const FRuntimeArchiverAsyncOperationResult& OnResult, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite = true);

	/**
	 * Extract entry into memory. In other words, extract the file from the archive into memory
	 *
	 * @param EntryInfo Information about the entry
	 * @param UnarchivedData Unarchived entry data
	 * @return Whether the operation was successful or not
	 */
	UFUNCTION(BlueprintCallable, Category = "Runtime Archiver|Extract")
	bool ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray<uint8>& UnarchivedData);

	/**
	 * Extract entry into memory. In other words, extract the file from the archive into memory. Prefer to use this function if possible
	 *
	 * @param EntryInfo Information about the entry
	 * @param UnarchivedData Unarchived entry data
	 * @return Whether the operation was successful or not
	 */
	virtual bool ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData);

	/**
	 * Initialize the archiver
	 */
	virtual bool Initialize();

	/**
	 * Check whether the archiver is initialized
	 */
	virtual bool IsInitialized() const;

	/**
	 * Reset the archiver. Here it is supposed to clear all the data
	 */
	virtual void Reset();

protected:
	/**
	 * Report an error in the archiver
	 *
	 * @param ErrorCode Archiver error code
	 * @param ErrorString Error details
	 */
	virtual void ReportError(ERuntimeArchiverErrorCode ErrorCode, const FString& ErrorString) const;

	/** Archive mode */
	ERuntimeArchiverMode Mode;

	/** Archive location */
	ERuntimeArchiverLocation Location;
};
