// Georgy Treshchev 2023.

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
		OnFail.Broadcast(0);
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnResult_Callback);

	Archiver->ExtractEntriesToStorage_Directory(OperationResult, MoveTemp(DirectoryInfo.EntryName), MoveTemp(DirectoryInfo.DirectoryPath), DirectoryInfo.bAddParentDirectory, DirectoryInfo.bForceOverwrite);
}

void URuntimeArchiverUnarchiveAsyncTask::StartFiles()
{
	if (!Archiver->OpenArchiveFromStorage(FilesInfo.ArchivePath))
	{
		OnFail.Broadcast(0);
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnResult_Callback);
	OperationProgress.BindDynamic(this, &URuntimeArchiverUnarchiveAsyncTask::OnProgress_Callback);

	TArray<FRuntimeArchiveEntry> Entries;

	// Getting archive entries by names
	{
		for (const FString& EntryName : FilesInfo.EntryNames)
		{
			FRuntimeArchiveEntry Entry;
			if (!Archiver->GetArchiveEntryInfoByName(EntryName, Entry))
			{
				UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to find '%s' archive entry"), *EntryName);
				OnResult_Callback(false);
				return;
			}

			Entries.Add(Entry);
		}
	}

	Archiver->ExtractEntriesToStorage(OperationResult, OperationProgress, MoveTemp(Entries), MoveTemp(FilesInfo.DirectoryPath), MoveTemp(FilesInfo.bForceOverwrite));
}

void URuntimeArchiverUnarchiveAsyncTask::OnResult_Callback(bool bSuccess)
{
	OperationResult.Clear();

	bSuccess &= Archiver->CloseArchive();
	
	if (!bSuccess)
	{
		OnFail.Broadcast(0);
		return;
	}

	OnSuccess.Broadcast(100);
}

void URuntimeArchiverUnarchiveAsyncTask::OnProgress_Callback(int32 Percentage)
{
	OnProgress.Broadcast(Percentage);
}
