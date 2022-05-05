// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RuntimeArchiverUnarchiveAsyncTask.generated.h"

class URuntimeArchiverBase;

/**
 * Async task which simplifies unarchiving from an archive
 */
UCLASS()
class RUNTIMEARCHIVER_API URuntimeArchiverUnarchiveAsyncTask : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Asynchronously unarchive entries in a directory to storage
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param EntryName Path to the entry to extract to. Must be a directory only. Leave the field empty to use all entries
	 * @param DirectoryPath Path to the directory for exporting entries
	 * @param bAddParentDirectory Whether to add the specified directory as a parent
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Archiver|Async")
	static URuntimeArchiverUnarchiveAsyncTask* UnarchiveDirectoryAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite = true);

	/**
	 * Asynchronously unarchive entries to storage
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param EntryNames Array of all entry names to extract
	 * @param DirectoryPath Path to the directory for exporting entries
	 * @param bForceOverwrite Whether to force a file to be overwritten if it exists or not
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Archiver|Async")
	static URuntimeArchiverUnarchiveAsyncTask* UnarchiveFilesAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite = true);

	/** Unarchiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnSuccess;

	/** Unarchiving was not successful */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnFail;

private:
	/** Used archiver */
	UPROPERTY()
	URuntimeArchiverBase* Archiver;

	/** Operation result delegate */
	FRuntimeArchiverAsyncOperationResult OperationResult;

	void StartDirectory(FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite);
	void StartFiles(FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite);

	UFUNCTION()
	void OnAsyncResult(bool bSuccess);
};
