// Georgy Treshchev 2023.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeArchiverBase.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Launch/Resources/Version.h"
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
	static URuntimeArchiverArchiveAsyncTask* ArchiveDirectory(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

	/**
	 * Asynchronously archive entries from file paths
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param FilePaths File paths to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Runtime Archiver|Async")
	static URuntimeArchiverArchiveAsyncTask* ArchiveFiles(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, ERuntimeArchiverCompressionLevel CompressionLevel = ERuntimeArchiverCompressionLevel::Compression6);

	/** Archiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnSuccess;

	/** Archiving in progress */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnProgress;

	/** Archiving failed */
	UPROPERTY(BlueprintAssignable)
	FRuntimeArchiverAsyncActionResult OnFail;

protected:
	//~ Begin UBlueprintAsyncActionBase Interface
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface

private:
	/** Information about the operation to archive the directory */
	struct
	{
		FString ArchivePath;
		FString DirectoryPath;
		bool bAddParentDirectory;
		ERuntimeArchiverCompressionLevel CompressionLevel;
	} DirectoryInfo;

	/** Information about the operation to archive files */
	struct
	{
		FString ArchivePath;
		TArray<FString> FilePaths;
		ERuntimeArchiverCompressionLevel CompressionLevel;
	} FilesInfo;

	/** Specific archiving operation */
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
	void OnResult_Callback(bool bSuccess);

	/**
	 * Execute the progress of the operation
	 *
	 * @param Percentage Current operation percentage
	 */
	UFUNCTION()
	void OnProgress_Callback(int32 Percentage);
};
