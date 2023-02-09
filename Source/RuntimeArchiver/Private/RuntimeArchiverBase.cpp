// Georgy Treshchev 2023.

#include "RuntimeArchiverBase.h"

#include "RuntimeArchiverSubsystem.h"
#include "RuntimeArchiverDefines.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "UObject/GCObjectScopeGuard.h"

URuntimeArchiverBase::URuntimeArchiverBase()
	: Mode(ERuntimeArchiverMode::Undefined)
  , Location(ERuntimeArchiverLocation::Undefined)
{
}

URuntimeArchiverBase* URuntimeArchiverBase::CreateRuntimeArchiver(UObject* WorldContextObject, TSubclassOf<URuntimeArchiverBase> ArchiverClass)
{
	return NewObject<URuntimeArchiverBase>(WorldContextObject, ArchiverClass);
}

void URuntimeArchiverBase::BeginDestroy()
{
	if (IsInitialized())
	{
		CloseArchive();
		Reset();
	}

	Super::BeginDestroy();
}

bool URuntimeArchiverBase::CreateArchiveInStorage(FString ArchivePath)
{
	if (!Initialize())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("Unable to initialize archiver in storage path '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	if (ArchivePath.IsEmpty())
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, TEXT("Archive name not specified"));
		Reset();
		return false;
	}

	Mode = ERuntimeArchiverMode::Write;
	Location = ERuntimeArchiverLocation::Storage;

	return true;
}

bool URuntimeArchiverBase::CreateArchiveInMemory(int32 InitialAllocationSize)
{
	if (!Initialize())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to initialize archiver in memory"));
		Reset();
		return false;
	}

	if (InitialAllocationSize < 0)
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("The initial allication size must be >= 0 (specified size: %d)"), InitialAllocationSize));
		Reset();
		return false;
	}

	Mode = ERuntimeArchiverMode::Write;
	Location = ERuntimeArchiverLocation::Memory;

	return true;
}

bool URuntimeArchiverBase::OpenArchiveFromStorage(FString ArchivePath)
{
	if (ArchivePath.IsEmpty() || !FPaths::FileExists(ArchivePath))
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Archive '%s' does not exist"), *ArchivePath));
		return false;
	}

	if (!Initialize())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("Unable to initialize archiver in path '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	Mode = ERuntimeArchiverMode::Read;
	Location = ERuntimeArchiverLocation::Storage;

	return true;
}

bool URuntimeArchiverBase::OpenArchiveFromMemory(TArray<uint8> ArchiveData)
{
	return OpenArchiveFromMemory(TArray64<uint8>(MoveTemp(ArchiveData)));
}

bool URuntimeArchiverBase::OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData)
{
	if (!Initialize())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to initialize archiver in memory"));
		Reset();
		return false;
	}

	Mode = ERuntimeArchiverMode::Read;
	Location = ERuntimeArchiverLocation::Memory;

	return true;
}

bool URuntimeArchiverBase::CloseArchive()
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::GetArchiveData(TArray<uint8>& ArchiveData)
{
	TArray64<uint8> ArchiveData64;

	if (!GetArchiveData(ArchiveData64))
	{
		return false;
	}

	if (ArchiveData64.Num() > TNumericLimits<TArray<uint8>::SizeType>::Max())
	{
		ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), static_cast<int32>(TNumericLimits<TArray<uint8>::SizeType>::Max()), ArchiveData64.Num()));
		return false;
	}

	ArchiveData = TArray<uint8>(MoveTemp(ArchiveData64));

	return true;
}

bool URuntimeArchiverBase::GetArchiveData(TArray64<uint8>& ArchiveData)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::GetArchiveEntries(int32& NumOfArchiveEntries)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::GetArchiveEntryInfoByName(FString EntryName, FRuntimeArchiveEntry& EntryInfo)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (EntryName.IsEmpty())
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, TEXT("Entry name not specified"));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::GetArchiveEntryInfoByIndex(int32 EntryIndex, FRuntimeArchiveEntry& EntryInfo)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (EntryIndex < 0)
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Unsupported entry index. Must be >= 0 (provided index: %d)"), EntryIndex));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::AddEntryFromStorage(FString EntryName, FString FilePath, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Write)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for adding entries (using mode: '%s')"), *UEnum::GetValueAsName(ERuntimeArchiverMode::Write).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryName.IsEmpty())
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, TEXT("Entry name not specified"));
		return false;
	}

	FPaths::NormalizeFilename(FilePath);

	if (!FPaths::FileExists(FilePath))
	{
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Path '%s' does not contain a file"), *FilePath));
		return false;
	}

	TArray64<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to load file '%s' for entry '%s'"), *FilePath, *EntryName));
		return false;
	}

	if (!AddEntryFromMemory(EntryName, FileData, CompressionLevel))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to add entry '%s' from file '%s'"), *EntryName, *FilePath);
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added entry '%s' from '%s'"), *EntryName, *FilePath);

	return true;
}

void URuntimeArchiverBase::AddEntriesFromStorage(const FRuntimeArchiverAsyncOperationResult& OnResult, const FRuntimeArchiverAsyncOperationProgress& OnProgress, TArray<FString> FilePaths, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, OnResult, OnProgress, FilePaths = MoveTemp(FilePaths), CompressionLevel]()
	{
		FGCObjectScopeGuard Guard(this);

		auto ExecuteResult = [OnResult](bool bResult)
		{
			AsyncTask(ENamedThreads::GameThread, [OnResult, bResult]()
			{
				OnResult.ExecuteIfBound(bResult);
			});
		};

		auto ExecuteProgress = [OnProgress](int32 Percentage)
		{
			AsyncTask(ENamedThreads::GameThread, [OnProgress, Percentage]()
			{
				OnProgress.ExecuteIfBound(Percentage);
			});
		};

		for (TArray<FString>::SizeType FilePathsIndex = 0; FilePathsIndex < FilePaths.Num(); ++FilePathsIndex)
		{
			FString FilePath = FilePaths[FilePathsIndex];

			FPaths::NormalizeFilename(FilePath);

			const FString EntryName{FPaths::GetCleanFilename(FilePath)};

			if (!AddEntryFromStorage(EntryName, FilePath, CompressionLevel))
			{
				ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot add '%s' entry. Aborting async adding entries"), *EntryName));
				ExecuteResult(false);
				return;
			}

			ExecuteProgress(static_cast<float>(FilePathsIndex + 1) / FilePaths.Num() * 100);
		}

		UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added '%d' entries"), FilePaths.Num());

		ExecuteResult(true);
	});
}

void URuntimeArchiverBase::AddEntriesFromStorage_Directory(const FRuntimeArchiverAsyncOperationResult& OnResult, FString DirectoryPath, bool bAddParentDirectory, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	// Normalizing directory path
	FPaths::NormalizeDirectoryName(DirectoryPath);

	// Making sure the directory exists
	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Directory '%s' does not exist"), *DirectoryPath));
		OnResult.ExecuteIfBound(false);
	}

	// Composing the base path to have the correct entry names
	// For example, if we scanned the "C:/Folder" directory and found "C:/Folder/File.ogg", then the entry name will be either "Folder/File.ogg" or "File.ogg" (depending on bAddParentDirectory)
	const FString BaseDirectoryPathToExclude = [&DirectoryPath, bAddParentDirectory]()
	{
		FString BasePath{FPaths::GetPath(DirectoryPath)};

		if (!BasePath.IsEmpty())
		{
			if (!bAddParentDirectory)
			{
				BasePath += TEXT("/") + FPaths::GetCleanFilename(DirectoryPath);
			}

			BasePath += TEXT("/");
		}

		return BasePath;
	}();

	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, OnResult, BaseDirectoryPathToExclude, DirectoryPath = MoveTemp(DirectoryPath), CompressionLevel]()
	{
		FGCObjectScopeGuard Guard(this);
		const bool bResult{AddEntriesFromStorage_Directory_Internal(BaseDirectoryPathToExclude, DirectoryPath, CompressionLevel)};

		if (bResult)
		{
			UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added entries from directory '%s'"), *DirectoryPath);
		}

		AsyncTask(ENamedThreads::GameThread, [bResult, OnResult]()
		{
			OnResult.ExecuteIfBound(bResult);
		});
	});
}

bool URuntimeArchiverBase::AddEntriesFromStorage_Directory_Internal(FString BaseDirectoryPathToExclude, FString DirectoryPath, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	class FDirectoryVisitor_EntryAppender : public IPlatformFile::FDirectoryVisitor
	{
		URuntimeArchiverBase* RuntimeArchiver;
		const FString BaseDirectoryPathToExclude;
		const ERuntimeArchiverCompressionLevel CompressionLevel;

	public:
		FDirectoryVisitor_EntryAppender(URuntimeArchiverBase* RuntimeArchiver, const FString& BaseDirectoryPathToExclude, ERuntimeArchiverCompressionLevel CompressionLevel)
			: RuntimeArchiver(RuntimeArchiver)
		  , BaseDirectoryPathToExclude(BaseDirectoryPathToExclude)
		  , CompressionLevel(CompressionLevel)
		{
		}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			// We are only interested in files (directories are supposed to be added automatically by the archiver when specifying the path to the file)
			if (bIsDirectory)
			{
				return true;
			}

			// Get the entry name by truncating the base directory from the found file
			const FString EntryName{FString(FilenameOrDirectory).RightChop(BaseDirectoryPathToExclude.Len())};

			if (!RuntimeArchiver->AddEntryFromStorage(EntryName, FilenameOrDirectory, CompressionLevel))
			{
				RuntimeArchiver->ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot add '%s' entry. Aborting recursive adding entries"), *EntryName));
				return false;
			}

			return true;
		}
	};

	FDirectoryVisitor_EntryAppender DirectoryVisitor_EntryAppender(this, BaseDirectoryPathToExclude, CompressionLevel);

	return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*DirectoryPath, DirectoryVisitor_EntryAppender);
}

bool URuntimeArchiverBase::AddEntryFromMemory(FString EntryName, TArray<uint8> DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	return AddEntryFromMemory(MoveTemp(EntryName), TArray64<uint8>(MoveTemp(DataToBeArchived)), CompressionLevel);
}

bool URuntimeArchiverBase::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Write)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for adding entries (using mode: '%s')"), *UEnum::GetValueAsName(ERuntimeArchiverMode::Write).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryName.IsEmpty())
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, TEXT("Entry name not specified"));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::ExtractEntryToStorage(const FRuntimeArchiveEntry& EntryInfo, FString FilePath, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Read)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for extracting the entry (using mode: '%s')"), *UEnum::GetValueAsName(ERuntimeArchiverMode::Read).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	// If this is a directory
	if (EntryInfo.bIsDirectory)
	{
		FPaths::NormalizeDirectoryName(FilePath);

		if (FPaths::DirectoryExists(FilePath))
		{
			if (!bForceOverwrite)
			{
				ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Directory '%s' already exists"), *FilePath));
				return false;
			}

			UE_LOG(LogRuntimeArchiver, Warning, TEXT("Directory '%s' already exists. It will be overwritten"), *FilePath);
		}

		IPlatformFile& PlatformFile{FPlatformFileManager::Get().GetPlatformFile()};

		// Ensure we have a valid directory to extract entry to
		{
			const FString DirectoryPath{FPaths::GetPath(FilePath)};
			if (!PlatformFile.CreateDirectoryTree(*DirectoryPath))
			{
				ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to create subdirectory '%s' to extract entry '%s'"), *DirectoryPath, *EntryInfo.Name));
				return false;
			}
		}

		if (!PlatformFile.CreateDirectory(*FilePath))
		{
			ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to extract entry '%s' to directory '%s'"), *EntryInfo.Name, *FilePath));
			return false;
		}
	}
	// If this is a file
	else
	{
		FPaths::NormalizeFilename(FilePath);

		if (FPaths::FileExists(FilePath))
		{
			if (!bForceOverwrite)
			{
				ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("File '%s' already exists"), *FilePath));
				return false;
			}

			UE_LOG(LogRuntimeArchiver, Warning, TEXT("File '%s' already exists. It will be overwritten"), *FilePath);
		}

		TArray64<uint8> EntryData;
		if (!ExtractEntryToMemory(EntryInfo, EntryData))
		{
			ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to extract the entry '%s' from archive to memory for file '%s'"), *EntryInfo.Name, *FilePath));
			return false;
		}

		if (!FFileHelper::SaveArrayToFile(EntryData, *FilePath))
		{
			ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to save the entry '%s' from memory to file '%s'"), *EntryInfo.Name, *FilePath));
			return false;
		}

		UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted entry '%s' to file '%s'"), *EntryInfo.Name, *FilePath);
	}

	return true;
}

void URuntimeArchiverBase::ExtractEntriesToStorage(const FRuntimeArchiverAsyncOperationResult& OnResult, const FRuntimeArchiverAsyncOperationProgress& OnProgress, TArray<FRuntimeArchiveEntry> EntryInfo, FString DirectoryPath, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	FPaths::NormalizeDirectoryName(DirectoryPath);

	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, OnResult, OnProgress, EntryInfo = MoveTemp(EntryInfo), DirectoryPath = MoveTemp(DirectoryPath), bForceOverwrite]()
	{
		FGCObjectScopeGuard Guard(this);

		auto ExecuteResult = [OnResult](bool bResult)
		{
			AsyncTask(ENamedThreads::GameThread, [OnResult, bResult]()
			{
				OnResult.ExecuteIfBound(bResult);
			});
		};

		auto ExecuteProgress = [OnProgress](int32 Percentage)
		{
			AsyncTask(ENamedThreads::GameThread, [OnProgress, Percentage]()
			{
				OnProgress.ExecuteIfBound(Percentage);
			});
		};

		for (int32 EntryIndex = 0; EntryIndex < EntryInfo.Num(); ++EntryIndex)
		{
			const FRuntimeArchiveEntry& Entry = EntryInfo[EntryIndex];

			const FString ExtractFilePath = [&Entry]()
			{
				FString FilePath = Entry.Name;
				FPaths::NormalizeDirectoryName(FilePath);
				return FilePath;
			}();

			if (!ExtractEntryToStorage(Entry, FPaths::Combine(DirectoryPath, TEXT("/"), ExtractFilePath), bForceOverwrite))
			{
				ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot extract '%s' entry. Aborting async extracting entries"), *Entry.Name));
				ExecuteResult(false);
				return;
			}

			ExecuteProgress(static_cast<float>(EntryIndex + 1) / EntryInfo.Num() * 100);
		}

		UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted '%d' entries"), EntryInfo.Num());

		ExecuteResult(true);
	});
}

namespace
{
	/**
	 * Check whether the entry name belongs to the base name. For example, the entry name "SubFolder/File.txt" belongs to the base name "SubFolder", but the entry name "SubFolderNew/File.txt" does not belong to the base name "SubFolder"
	 */
	bool CheckEntryNameBelongsToBaseName(const FString& BaseName, const FString& EntryName)
	{
		int32 BaseNameIndex, EntryNameIndex;

		for (BaseNameIndex = EntryNameIndex = 0; BaseNameIndex < BaseName.Len() && EntryNameIndex < EntryName.Len(); ++BaseNameIndex, ++EntryNameIndex)
		{
			const TCHAR& BaseNameCharacter{BaseName[BaseNameIndex]};
			const TCHAR& EntryNameCharacter{EntryName[EntryNameIndex]};

			if (BaseNameCharacter != EntryNameCharacter)
			{
				return false;
			}

			if (BaseNameIndex == BaseName.Len() - 1 &&
				EntryNameIndex + 1 < EntryName.Len() && EntryName[EntryNameIndex + 1] == TEXT('/'))
			{
				return true;
			}
		}

		return false;
	}
}

void URuntimeArchiverBase::ExtractEntriesToStorage_Directory(const FRuntimeArchiverAsyncOperationResult& OnResult, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	int32 NumOfEntries;
	if (!GetArchiveEntries(NumOfEntries))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Cannot get the number of archive entries. Aborting recursive extracting entries"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	FPaths::NormalizeDirectoryName(EntryName);
	FPaths::NormalizeDirectoryName(DirectoryPath);

	// Composing the base path to have the correct directory path
	// For example, if we scanned the "Folder" directory and found "Folder/File.ogg" entry name, then the directory file system path will be either "Folder/File.ogg" or "File.ogg" (depending on bAddParentDirectory)
	const FString BaseDirectoryPathToExclude = [&EntryName, bAddParentDirectory]()
	{
		FString BasePath{FPaths::GetPath(EntryName)};

		if (!BasePath.IsEmpty())
		{
			if (!bAddParentDirectory)
			{
				BasePath += TEXT("/") + FPaths::GetCleanFilename(EntryName);
			}
		}

		return BasePath;
	}();

	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [this, OnResult, NumOfEntries, EntryName, DirectoryPath, BaseDirectoryPathToExclude, bForceOverwrite]()
	{
		FGCObjectScopeGuard Guard(this);
		bool bResult{true};

		for (int32 EntryIndex = 0; EntryIndex < NumOfEntries; ++EntryIndex)
		{
			FRuntimeArchiveEntry ArchiveEntry;

			if (!GetArchiveEntryInfoByIndex(EntryIndex, ArchiveEntry))
			{
				ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot get '%d' entry to extract. Aborting recursive extracting entries"), EntryIndex));
				bResult = false;
				break;
			}

			if (EntryName.IsEmpty() || CheckEntryNameBelongsToBaseName(EntryName, ArchiveEntry.Name))
			{
				// Get the file path by truncating the base directory from the found entry
				const FString SpecificFilePath{FPaths::Combine(DirectoryPath, ArchiveEntry.Name.RightChop(BaseDirectoryPathToExclude.Len()))};

				if (!ExtractEntryToStorage(ArchiveEntry, SpecificFilePath, bForceOverwrite))
				{
					bResult = false;
					break;
				}
			}
		}

		if (bResult)
		{
			UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted entries from '%s'"), *EntryName);
		}

		AsyncTask(ENamedThreads::GameThread, [OnResult, bResult]()
		{
			OnResult.ExecuteIfBound(bResult);
		});
	});
}

bool URuntimeArchiverBase::ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray<uint8>& UnarchivedData)
{
	TArray64<uint8> UnarchivedData64;

	if (!ExtractEntryToMemory(EntryInfo, UnarchivedData64))
	{
		return false;
	}

	if (UnarchivedData64.Num() > TNumericLimits<TArray<uint8>::SizeType>::Max())
	{
		ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), static_cast<int32>(TNumericLimits<TArray<uint8>::SizeType>::Max()), UnarchivedData64.Num()));
		return false;
	}

	UnarchivedData = TArray<uint8>(MoveTemp(UnarchivedData64));

	return true;
}

bool URuntimeArchiverBase::ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Read)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for extracting the entry (using mode: '%s')"), *UEnum::GetValueAsName(ERuntimeArchiverMode::Read).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryInfo.bIsDirectory)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedLocation, TEXT("It is impossible to extract directory entry into memory"));
		return false;
	}

	return true;
}

bool URuntimeArchiverBase::Initialize()
{
	return true;
}

bool URuntimeArchiverBase::IsInitialized() const
{
	return Mode != ERuntimeArchiverMode::Undefined && Location != ERuntimeArchiverLocation::Undefined;
}

void URuntimeArchiverBase::Reset()
{
	Mode = ERuntimeArchiverMode::Undefined;
	Location = ERuntimeArchiverLocation::Undefined;
}

void URuntimeArchiverBase::ReportError(ERuntimeArchiverErrorCode ErrorCode, const FString& ErrorString) const
{
	// Making sure we are in the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [this, ErrorCode, ErrorString]() { URuntimeArchiverBase::ReportError(ErrorCode, ErrorString); });
		return;
	}

	if (const URuntimeArchiverSubsystem* ArchiveSubsystem{URuntimeArchiverSubsystem::GetArchiveSubsystem()})
	{
		if (ArchiveSubsystem->OnError.IsBound())
		{
			ArchiveSubsystem->OnError.Broadcast(ErrorCode, ErrorString);
		}
	}

	UE_LOG(LogRuntimeArchiver, Error, TEXT("%s"), *ErrorString);
}
