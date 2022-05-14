// Georgy Treshchev 2022.

#include "AsyncTasks/RuntimeArchiverArchiveAsyncTask.h"

URuntimeArchiverArchiveAsyncTask* URuntimeArchiverArchiveAsyncTask::ArchiveDirectory(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
{
	URuntimeArchiverArchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	{
		ArchiveTask->OperationType = EOperationType::Directory;
		ArchiveTask->DirectoryInfo = {MoveTemp(ArchivePath), MoveTemp(DirectoryPath), bAddParentDirectory, CompressionLevel};
	}

	return ArchiveTask;
}

URuntimeArchiverArchiveAsyncTask* URuntimeArchiverArchiveAsyncTask::ArchiveFiles(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
{
	URuntimeArchiverArchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	{
		ArchiveTask->OperationType = EOperationType::Files;
		ArchiveTask->FilesInfo = {MoveTemp(ArchivePath), MoveTemp(FilePaths), CompressionLevel};
	}

	return ArchiveTask;
}

void URuntimeArchiverArchiveAsyncTask::Activate()
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

void URuntimeArchiverArchiveAsyncTask::StartDirectory()
{
	if (!Archiver->CreateArchiveInStorage(DirectoryInfo.ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnResult);

	Archiver->AddEntriesFromStorage_Directory(OperationResult, MoveTemp(DirectoryInfo.DirectoryPath), DirectoryInfo.bAddParentDirectory, DirectoryInfo.CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::StartFiles()
{
	if (!Archiver->CreateArchiveInStorage(FilesInfo.ArchivePath))
	{
		OnFail.Broadcast();
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnResult);

	Archiver->AddEntriesFromStorage(OperationResult, MoveTemp(FilesInfo.FilePaths), FilesInfo.CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::OnResult(bool bSuccess)
{
	OperationResult.Clear();

	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
