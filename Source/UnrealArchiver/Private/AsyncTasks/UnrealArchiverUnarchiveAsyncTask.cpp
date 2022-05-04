// Georgy Treshchev 2022.

#include "AsyncTasks/UnrealArchiverUnarchiveAsyncTask.h"

#include "UnrealArchiverBase.h"
#include "Async/Async.h"
#include "Misc/Paths.h"

UUnrealArchiverUnarchiveAsyncTask* UUnrealArchiverUnarchiveAsyncTask::UnarchiveDirectoryAsync(TSubclassOf<UUnrealArchiverBase> ArchiverClass, FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	UUnrealArchiverUnarchiveAsyncTask* ArchiveTask = NewObject<UUnrealArchiverUnarchiveAsyncTask>();

	ArchiveTask->Archiver = UUnrealArchiverBase::CreateUnrealArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartDirectory(MoveTemp(ArchivePath), MoveTemp(EntryName), MoveTemp(DirectoryPath), bAddParentDirectory, bForceOverwrite);

	return ArchiveTask;
}

UUnrealArchiverUnarchiveAsyncTask* UUnrealArchiverUnarchiveAsyncTask::UnarchiveFilesAsync(TSubclassOf<UUnrealArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite)
{
	UUnrealArchiverUnarchiveAsyncTask* ArchiveTask = NewObject<UUnrealArchiverUnarchiveAsyncTask>();

	ArchiveTask->Archiver = UUnrealArchiverBase::CreateUnrealArchiver(ArchiveTask, ArchiverClass);

	ArchiveTask->StartFiles(MoveTemp(ArchivePath), MoveTemp(EntryNames), MoveTemp(DirectoryPath), bForceOverwrite);

	return ArchiveTask;
}

void UUnrealArchiverUnarchiveAsyncTask::StartDirectory(FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	if (!Archiver->OpenArchiveFromStorage(ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	FUnrealArchiverRecursiveResult RecursiveResult;
	RecursiveResult.BindDynamic(this, &UUnrealArchiverUnarchiveAsyncTask::OnRecursiveResult);

	Archiver->ExtractEntryToStorage_Recursively(RecursiveResult, MoveTemp(EntryName), MoveTemp(DirectoryPath), bAddParentDirectory, bForceOverwrite);
}

void UUnrealArchiverUnarchiveAsyncTask::StartFiles(FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite)
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

			FUnrealArchiveEntry EntryInfo;

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

void UUnrealArchiverUnarchiveAsyncTask::OnRecursiveResult(bool bSuccess)
{
	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
