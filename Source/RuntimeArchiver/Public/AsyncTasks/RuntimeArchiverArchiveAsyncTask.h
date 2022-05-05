// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RuntimeArchiverArchiveAsyncTask.generated.h"

class URuntimeArchiverBase;

/**
 * Async task which simplifies archiving from an archive
 */
UCLASS()
class URuntimeArchiverArchiveAsyncTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Asynchronously archive entries from a directory
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param DirectoryPath Directory to be archived
	 * @param bAddParentDirectory Whether to add the specified directory as a parent
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Archiver|Async")
	static URuntimeArchiverArchiveAsyncTask* ArchiveDirectoryAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/**
	 * Asynchronously archive entries from file paths
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param FilePaths File paths to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Archiver|Async")
	static URuntimeArchiverArchiveAsyncTask* ArchiveFilesAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/** Archiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnSuccess;

	/** Unarchiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnFail;

private:
	/** Used archiver */
	UPROPERTY()
	URuntimeArchiverBase* Archiver;

	/** Operation result delegate */
	FRuntimeArchiverAsyncOperationResult OperationResult;

	void StartDirectory(FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel);
	void StartFiles(FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel);

	UFUNCTION()
	void OnAsyncResult(bool bSuccess);
};
