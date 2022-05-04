// Georgy Treshchev 2022.

#include "AsyncTasks/UnrealArchiverArchiveAsyncTask.h"

#include "UnrealArchiverBase.h"
#include "Async/Async.h"
#include "Misc/Paths.h"

UUnrealArchiverArchiveAsyncTask* UUnrealArchiverArchiveAsyncTask::ArchiveDirectoryAsync(TSubclassOf<UUnrealArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
{
	UUnrealArchiverArchiveAsyncTask* ArchiveTask = NewObject<UUnrealArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = UUnrealArchiverBase::CreateUnrealArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartDirectory(MoveTemp(ArchivePath), MoveTemp(DirectoryPath), bAddParentDirectory, CompressionLevel);

	return ArchiveTask;
}

UUnrealArchiverArchiveAsyncTask* UUnrealArchiverArchiveAsyncTask::ArchiveFilesAsync(TSubclassOf<UUnrealArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
{
	UUnrealArchiverArchiveAsyncTask* ArchiveTask = NewObject<UUnrealArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = UUnrealArchiverBase::CreateUnrealArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartFiles(MoveTemp(ArchivePath), MoveTemp(FilePaths), CompressionLevel);

	return ArchiveTask;
}

void UUnrealArchiverArchiveAsyncTask::StartDirectory(FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!Archiver->CreateArchiveInStorage(ArchivePath))
	{
		OnFail.Broadcast();
	}

	FUnrealArchiverRecursiveResult RecursiveResult;
	RecursiveResult.BindDynamic(this, &UUnrealArchiverArchiveAsyncTask::OnRecursiveResult);

	Archiver->AddEntryFromStorage_Recursively(RecursiveResult, DirectoryPath, bAddParentDirectory, CompressionLevel);
}

void UUnrealArchiverArchiveAsyncTask::StartFiles(FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!Archiver->CreateArchiveInStorage(ArchivePath))
	{
		OnFail.Broadcast();
	}

	AsyncTask(ENamedThreads::AnyThread, [this, FilePaths = MoveTemp(FilePaths), CompressionLevel]()
	{
		auto OnResult = [this](bool bResult)
		{
			AsyncTask(ENamedThreads::GameThread, [this, bResult]()
			{
				OnRecursiveResult(bResult);
			});
		};

		for (FString FilePath : FilePaths)
		{
			FPaths::NormalizeFilename(FilePath);

			const bool bResult{Archiver->AddEntryFromStorage(FPaths::GetCleanFilename(FilePath), FilePath, CompressionLevel)};

			if (!bResult)
			{
				OnResult(false);
				return;
			}
		}

		OnResult(true);
	});
}

void UUnrealArchiverArchiveAsyncTask::OnRecursiveResult(bool bSuccess)
{
	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
