// Georgy Treshchev 2022.

#include "AsyncTasks/RuntimeArchiverArchiveAsyncTask.h"

#include "RuntimeArchiverBase.h"
#include "Async/Async.h"
#include "Misc/Paths.h"

URuntimeArchiverArchiveAsyncTask* URuntimeArchiverArchiveAsyncTask::ArchiveDirectoryAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
{
	URuntimeArchiverArchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartDirectory(MoveTemp(ArchivePath), MoveTemp(DirectoryPath), bAddParentDirectory, CompressionLevel);

	return ArchiveTask;
}

URuntimeArchiverArchiveAsyncTask* URuntimeArchiverArchiveAsyncTask::ArchiveFilesAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
{
	URuntimeArchiverArchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartFiles(MoveTemp(ArchivePath), MoveTemp(FilePaths), CompressionLevel);

	return ArchiveTask;
}

void URuntimeArchiverArchiveAsyncTask::StartDirectory(FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!Archiver->CreateArchiveInStorage(ArchivePath))
	{
		OnFail.Broadcast();
	}

	FRuntimeArchiverRecursiveResult RecursiveResult;
	RecursiveResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnRecursiveResult);

	Archiver->AddEntryFromStorage_Recursively(RecursiveResult, DirectoryPath, bAddParentDirectory, CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::StartFiles(FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
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

void URuntimeArchiverArchiveAsyncTask::OnRecursiveResult(bool bSuccess)
{
	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
