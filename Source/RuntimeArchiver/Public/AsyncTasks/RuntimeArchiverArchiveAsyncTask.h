// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverBase.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RuntimeArchiverArchiveAsyncTask.generated.h"

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
	static URuntimeArchiverArchiveAsyncTask* ArchiveDirectory(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/**
	 * Asynchronously archive entries from file paths
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param FilePaths File paths to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Archiver|Async")
	static URuntimeArchiverArchiveAsyncTask* ArchiveFiles(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/** Archiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnSuccess;

	/** Unarchiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnFail;

protected:
	//~ Begin UBlueprintAsyncActionBase interface.
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase interface.

private:
	/** Information about the operation to archive the directory */
	struct
	{
		FString ArchivePath;
		FString DirectoryPath;
		bool bAddParentDirectory;
		EUnrealEntryCompressionLevel CompressionLevel;
	} DirectoryInfo;

	/** Information about the operation to archive files */
	struct
	{
		FString ArchivePath;
		TArray<FString> FilePaths;
		EUnrealEntryCompressionLevel CompressionLevel;
	} FilesInfo;

	/** Specific archiving operation */
	enum class EOperationType : uint8
	{
		Directory,
		Files
	} OperationType;

	/** Used archiver */
	UPROPERTY()
	URuntimeArchiverBase* Archiver;

	/** Operation result delegate */
	FRuntimeArchiverAsyncOperationResult OperationResult;

	/** Start archiving directory operation */
	void StartDirectory();

	/** Start archiving files operation */
	void StartFiles();

	/**
	 * Execute the result of the operation
	 *
	 * @param bSuccess Whether the result is successful or not
	 */
	UFUNCTION()
	void OnResult(bool bSuccess);
};
