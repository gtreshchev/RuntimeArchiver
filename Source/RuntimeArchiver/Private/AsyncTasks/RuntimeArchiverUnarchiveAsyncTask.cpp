// Georgy Treshchev 2022.

#include "AsyncTasks/RuntimeArchiverUnarchiveAsyncTask.h"

#include "RuntimeArchiverDefines.h"

URuntimeArchiverUnarchiveAsyncTask* URuntimeArchiverUnarchiveAsyncTask::UnarchiveDirectory(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	URuntimeArchiverUnarchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverUnarchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	{
		ArchiveTask->OperationType = EOperationType::Directory;
		ArchiveTask->DirectoryInfo = {MoveTemp(ArchivePath), MoveTemp(EntryName), MoveTemp(DirectoryPath), bAddParentDirectory, bForceOverwrite};
	}

	return ArchiveTask;
}

URuntimeArchiverUnarchiveAsyncTask* URuntimeArchiverUnarchiveAsyncTask::UnarchiveFiles(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> EntryNames, FString DirectoryPath, bool bForceOverwrite)
{
	URuntimeArchiverUnarchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverUnarchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	{
		ArchiveTask->OperationType = EOperationType::Files;
		ArchiveTask->FilesInfo = {MoveTemp(ArchivePath), MoveTemp(EntryNames), MoveTemp(DirectoryPath), bForceOverwrite};
	}

	return ArchiveTask;
}

void URuntimeArchiverUnarchiveAsyncTask::Activate()
{
	Super::Activate();
	
	switch (OperationType)
	{
	case EOperationType::Directory:
		{
			StartDirectory();
			break;
		}
	case EOperationType::Files:
		{
			StartFiles();
			break;
		}
	}
}

void URuntimeArchiverUnarchiveAsyncTask::StartDirectory()
{
	if (!Archiver->OpenArchiveFromStorage(DirectoryInfo.ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnResult);

	Archiver->ExtractEntriesToStorage_Directory(OperationResult, MoveTemp(DirectoryInfo.EntryName), MoveTemp(DirectoryInfo.DirectoryPath), DirectoryInfo.bAddParentDirectory, DirectoryInfo.bForceOverwrite);
}

void URuntimeArchiverUnarchiveAsyncTask::StartFiles()
{
	if (!Archiver->OpenArchiveFromStorage(FilesInfo.ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnResult);

	TArray<FRuntimeArchiveEntry> Entries;

	// Getting archive entries by names
	{
		for (const FString& EntryName : FilesInfo.EntryNames)
		{
			FRuntimeArchiveEntry Entry;
			if (!Archiver->GetArchiveEntryInfoByName(EntryName, Entry))
			{
				UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to find '%s' archive entry"), *EntryName);
				OnResult(false);
				return;
			}

			Entries.Add(Entry);
		}
	}

	Archiver->ExtractEntriesToStorage(OperationResult, MoveTemp(Entries), MoveTemp(FilesInfo.DirectoryPath), MoveTemp(FilesInfo.bForceOverwrite));
}

void URuntimeArchiverUnarchiveAsyncTask::OnResult(bool bSuccess)
{
	OperationResult.Clear();

	bSuccess &= Archiver->CloseArchive();
	
	if (!bSuccess)
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
