// Georgy Treshchev 2023.

#include "AsyncTasks/RuntimeArchiverArchiveAsyncTask.h"

URuntimeArchiverArchiveAsyncTask* URuntimeArchiverArchiveAsyncTask::ArchiveDirectory(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, FString DirectoryPath, bool bAddParentDirectory, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	URuntimeArchiverArchiveAsyncTask* ArchiveTask = NewObject<URuntimeArchiverArchiveAsyncTask>();

	ArchiveTask->Archiver = URuntimeArchiverBase::CreateRuntimeArchiver(ArchiveTask, ArchiverClass);

	{
		ArchiveTask->OperationType = EOperationType::Directory;
		ArchiveTask->DirectoryInfo = {MoveTemp(ArchivePath), MoveTemp(DirectoryPath), bAddParentDirectory, CompressionLevel};
	}

	return ArchiveTask;
}

URuntimeArchiverArchiveAsyncTask* URuntimeArchiverArchiveAsyncTask::ArchiveFiles(TSubclassOf<URuntimeArchiverBase> ArchiverClass, FString ArchivePath, TArray<FString> FilePaths, ERuntimeArchiverCompressionLevel CompressionLevel)
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
		OnResult_Callback(false);
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnResult_Callback);

	Archiver->AddEntriesFromStorage_Directory(OperationResult, MoveTemp(DirectoryInfo.DirectoryPath), DirectoryInfo.bAddParentDirectory, DirectoryInfo.CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::StartFiles()
{
	if (!Archiver->CreateArchiveInStorage(FilesInfo.ArchivePath))
	{
		OnResult_Callback(false);
		return;
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnResult_Callback);
	OperationProgress.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnProgress_Callback);

	Archiver->AddEntriesFromStorage(OperationResult, OperationProgress, MoveTemp(FilesInfo.FilePaths), FilesInfo.CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::OnResult_Callback(bool bSuccess)
{
	OperationResult.Clear();

	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast(0);
		return;
	}

	OnSuccess.Broadcast(100);

	SetReadyToDestroy();
}

void URuntimeArchiverArchiveAsyncTask::OnProgress_Callback(int32 Percentage)
{
	OnProgress.Broadcast(Percentage);
}
