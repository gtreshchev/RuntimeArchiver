// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
#include "UnrealArchiverTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UnrealArchiverArchiveAsyncTask.generated.h"

class UUnrealArchiverBase;

/**
 * Async task which simplifies archiving from an archive
 */
UCLASS()
class UUnrealArchiverArchiveAsyncTask : public UBlueprintAsyncActionBase
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
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Unreal Archiver|Async")
	static UUnrealArchiverArchiveAsyncTask* ArchiveDirectoryAsync(TSubclassOf<UUnrealArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/**
	 * Asynchronously archive entries from file paths
	 *
	 * @param ArchiverClass Archiver class
	 * @param ArchivePath Path to open an archive
	 * @param FilePaths File paths to be archived
	 * @param CompressionLevel Compression level. The higher the level, the more compression
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "Unreal Archiver|Async")
	static UUnrealArchiverArchiveAsyncTask* ArchiveFilesAsync(TSubclassOf<UUnrealArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel = EUnrealEntryCompressionLevel::Compression6);

	/** Archiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FUnrealArchiverAsyncResult OnSuccess;

	/** Unarchiving completed successfully */
	UPROPERTY(BlueprintAssignable)
	FUnrealArchiverAsyncResult OnFail;

private:
	/** Used archiver */
	UPROPERTY()
	UUnrealArchiverBase* Archiver;

	void StartDirectory(FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel);
	void StartFiles(FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel);

	UFUNCTION()
	void OnRecursiveResult(bool bSuccess);
};
