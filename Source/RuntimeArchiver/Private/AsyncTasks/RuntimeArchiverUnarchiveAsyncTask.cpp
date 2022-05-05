// Georgy Treshchev 2022.

#include "AsyncTasks/RuntimeArchiverUnarchiveAsyncTask.h"

#include "RuntimeArchiverBase.h"
#include "Async/Async.h"
#include "Misc/Paths.h"

URuntimeArchiverUnarchiveAsyncTask* URuntimeArchiverUnarchiveAsyncTask::UnarchiveDirectoryAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	URuntimeArchiverUnarchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverUnarchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartDirectory(MoveTemp(ArchivePath), MoveTemp(EntryName), MoveTemp(DirectoryPath), bAddParentDirectory, bForceOverwrite);

	return ArchiveTask;
}

URuntimeArchiverUnarchiveAsyncTask* URuntimeArchiverUnarchiveAsyncTask::UnarchiveFilesAsync(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite)
{
	URuntimeArchiverUnarchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverUnarchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartFiles(MoveTemp(ArchivePath), MoveTemp(EntryNames), MoveTemp(DirectoryPath), bForceOverwrite);

	return ArchiveTask;
}

void URuntimeArchiverUnarchiveAsyncTask::StartDirectory(FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	if (!Archiver->OpenArchiveFromStorage(ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	FRuntimeArchiverRecursiveResult RecursiveResult;
	RecursiveResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnRecursiveResult);

	Archiver->ExtractEntryToStorage_Recursively(RecursiveResult, MoveTemp(EntryName), MoveTemp(DirectoryPath), bAddParentDirectory, bForceOverwrite);
}

void URuntimeArchiverUnarchiveAsyncTask::StartFiles(FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite)
{
	if (!Archiver->OpenArchiveFromStorage(ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	FPaths::NormalizeDirectoryName(DirectoryPath);

	AsyncTask(ENamedThreads::AnyThread, [this, EntryNames = MoveTemp(EntryNames), DirectoryPath = MoveTemp(DirectoryPath), bForceOverwrite]()
	{
		auto OnResult = [this](bool bResult)
		{
			AsyncTask(ENamedThreads::GameThread, [this, bResult]()
			{
				OnRecursiveResult(bResult);
			});
		};

		for (FString EntryName : EntryNames)
		{
			FPaths::NormalizeDirectoryName(EntryName);

			FRuntimeArchiveEntry EntryInfo;

			if (!Archiver->GetArchiveEntryInfoByName(EntryName, EntryInfo))
			{
				OnResult(false);
				return;
			}

			const bool bResult{Archiver->ExtractEntryToStorage(EntryInfo, FPaths::Combine(DirectoryPath, TEXT("/"), EntryName), bForceOverwrite)};

			if (!bResult)
			{
				OnResult(false);
				return;
			}
		}

		OnResult(true);
	});
}

void URuntimeArchiverUnarchiveAsyncTask::OnRecursiveResult(bool bSuccess)
{
	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
