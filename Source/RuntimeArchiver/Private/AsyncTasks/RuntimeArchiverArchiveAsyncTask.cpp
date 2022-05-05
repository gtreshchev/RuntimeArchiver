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

	OperationResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnAsyncResult);

	Archiver->AddEntriesFromStorage_Recursively(OperationResult, DirectoryPath, bAddParentDirectory, CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::StartFiles(FString ArchivePath, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!Archiver->CreateArchiveInStorage(ArchivePath))
	{
		OnFail.Broadcast();
	}

	OperationResult.BindDynamic(this, &URuntimeArchiverArchiveAsyncTask::OnAsyncResult);

	Archiver->AddEntriesFromStorage(OperationResult, MoveTemp(FilePaths), CompressionLevel);
}

void URuntimeArchiverArchiveAsyncTask::OnAsyncResult(bool bSuccess)
{
	OperationResult.Clear();
	
	if (!bSuccess || !Archiver->CloseArchive())
	{
		OnFail.Broadcast();
		return;
	}

	OnSuccess.Broadcast();
}
