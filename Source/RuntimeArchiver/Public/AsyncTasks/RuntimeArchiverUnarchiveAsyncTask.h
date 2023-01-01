// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverBase.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RuntimeArchiverUnarchiveAsyncTask.generated.h"

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
	static URuntimeArchiverUnarchiveAsyncTask* UnarchiveDirectory(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite = true);

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
	static URuntimeArchiverUnarchiveAsyncTask* UnarchiveFiles(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite = true);

	/** Unarchiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnSuccess;

	/** Archiving in progress */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnProgress;

	/** Unarchiving failed */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnFail;

protected:
	//~ Begin UBlueprintAsyncActionBase Interface
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface

private:
	/** Information about the operation to unarchive the directory */
	struct
	{
		FString ArchivePath;
		FString EntryName;
		FString DirectoryPath;
		bool bAddParentDirectory;
		bool bForceOverwrite;
	} DirectoryInfo;

	/** Information about the operation to unarchive files */
	struct
	{
		FString ArchivePath;
		TArray<FString> EntryNames;
		FString DirectoryPath;
		bool bForceOverwrite;
	} FilesInfo;

	/** Specific unarchiving operation */
	enum class EOperationType : uint8
	{
		Directory,
		Files
	} OperationType;

	/** Used archiver */
	UPROPERTY()
#if ENGINE_MAJOR_VERSION >= 5
	TObjectPtr<URuntimeArchiverBase> Archiver;
#else
	URuntimeArchiverBase* Archiver;
#endif

	/** Operation result delegate */
	FRuntimeArchiverAsyncOperationResult OperationResult;

	/** Operation in progress delegate */
	FRuntimeArchiverAsyncOperationProgress OperationProgress;

	/** Start unarchiving directory operation */
	void StartDirectory();

	/** Start unarchiving files operation */
	void StartFiles();

	/**
	 * Execute the result of the operation
	 *
	 * @param bSuccess Whether the result is successful or not
	 */
	UFUNCTION()
	void OnResult_Callback(bool bSuccess);

	/**
	 * Execute the progress of the operation
	 *
	 * @param Percentage Current operation percentage
	 */
	UFUNCTION()
	void OnProgress_Callback(int32 Percentage);
};
