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

	OperationResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnAsyncResult);

	Archiver->ExtractEntriesToStorage_Recursively(OperationResult, MoveTemp(EntryName), MoveTemp(DirectoryPath), bAddParentDirectory, bForceOverwrite);
}

void URuntimeArchiverUnarchiveAsyncTask::StartFiles(FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite)
{
	if (!Archiver->OpenArchiveFromStorage(ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}
	
	OperationResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnAsyncResult);

	TArray<FRuntimeArchiveEntry> Entries;
	
	// Getting archive entries by names
	{
		for (const FString& EntryName : EntryNames)
		{
			FRuntimeArchiveEntry Entry;
			if (!Archiver->GetArchiveEntryInfoByName(EntryName, Entry))
			{
				Archiver->ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to find '%s' archive entry"), *EntryName));
				OnAsyncResult(false);
				return;
			}
			
			Entries.Add(Entry);
		}
	}
	
	Archiver->ExtractEntriesToStorage(OperationResult, MoveTemp(Entries), MoveTemp(DirectoryPath), bForceOverwrite);
}

void URuntimeArchiverUnarchiveAsyncTask::OnAsyncResult(bool bSuccess)
{
	OperationResult.Clear();
	
	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
