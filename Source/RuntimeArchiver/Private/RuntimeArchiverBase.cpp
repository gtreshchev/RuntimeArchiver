// Georgy Treshchev 2022.

#include "RuntimeArchiverBase.h"

#include "RuntimeArchiverSubsystem.h"
#include "RuntimeArchiverDefines.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

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

bool URuntimeArchiverBase::GetArchiveDataFromMemory(TArray<uint8>& ArchiveData)
{
	TArray64<uint8> ArchiveData64;

	if (!GetArchiveDataFromMemory(ArchiveData64))
	{
		return false;
	}

	if (ArchiveData64.Num() > MAX_int32)
	{
		ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), MAX_int32, ArchiveData64.Num()));
		return false;
	}

	ArchiveData = TArray<uint8>(MoveTemp(ArchiveData64));

	return true;
}

bool URuntimeArchiverBase::GetArchiveDataFromMemory(TArray64<uint8>& ArchiveData)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Write || Location != ERuntimeArchiverLocation::Memory)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode and '%s' location are supported to get archive data (using mode: '%s', using location: '%s')"),
		                                                                        *UEnum::GetValueAsName(ERuntimeArchiverMode::Write).ToString(), *UEnum::GetValueAsName(ERuntimeArchiverLocation::Memory).ToString(),
		                                                                        *UEnum::GetValueAsName(Mode).ToString(), *UEnum::GetValueAsName(Location).ToString()));
		return false;
	}

	return true;
}

int32 URuntimeArchiverBase::GetArchiveEntries()
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return -1;
	}

	return 0;
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

bool URuntimeArchiverBase::AddEntryFromStorage(FString EntryName, FString FilePath, EUnrealEntryCompressionLevel CompressionLevel)
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
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to add entry '%s' from file '%s'"), *EntryName, *FilePath));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added entry '%s' from '%s'"), *EntryName, *FilePath);
	
	return true;
}

void URuntimeArchiverBase::AddEntriesFromStorage(FRuntimeArchiverAsyncOperationResult OnResult, TArray<FString> FilePaths, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}
	
	AsyncTask(ENamedThreads::AnyThread, [this, OnResult, FilePaths = MoveTemp(FilePaths), CompressionLevel]()
	{
		auto ExecuteResult = [this, OnResult](bool bResult)
		{
			AsyncTask(ENamedThreads::GameThread, [this, OnResult, bResult]()
			{
				OnResult.ExecuteIfBound(bResult);
			});
		};

		for (FString FilePath : FilePaths)
		{
			FPaths::NormalizeFilename(FilePath);

			const FString EntryName{FPaths::GetCleanFilename(FilePath)};

			if (!AddEntryFromStorage(EntryName, FilePath, CompressionLevel))
			{
				ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot add '%s' entry. Aborting async adding entries"), *EntryName));
				ExecuteResult(false);
				return;
			}
		}

		UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added '%d' entries"), FilePaths.Num());

		ExecuteResult(true);
	});
}

void URuntimeArchiverBase::AddEntriesFromStorage_Directory(FRuntimeArchiverAsyncOperationResult OnResult, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
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

		return MoveTemp(BasePath);
	}();

	AsyncTask(ENamedThreads::AnyThread, [this, OnResult, BaseDirectoryPathToExclude, DirectoryPath, CompressionLevel]()
	{
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

bool URuntimeArchiverBase::AddEntriesFromStorage_Directory_Internal(FString BaseDirectoryPathToExclude, FString DirectoryPath, EUnrealEntryCompressionLevel CompressionLevel)
{
	class FDirectoryVisitor_EntryAppender : public IPlatformFile::FDirectoryVisitor
	{
		URuntimeArchiverBase* RuntimeArchiver;
		const FString& BaseDirectoryPathToExclude;
		EUnrealEntryCompressionLevel CompressionLevel;
	public:
		FDirectoryVisitor_EntryAppender(URuntimeArchiverBase* RuntimeArchiver, const FString& BaseDirectoryPathToExclude, EUnrealEntryCompressionLevel CompressionLevel)
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

bool URuntimeArchiverBase::AddEntryFromMemory(FString EntryName, TArray<uint8> DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel)
{
	return AddEntryFromMemory(MoveTemp(EntryName), TArray64<uint8>(MoveTemp(DataToBeArchived)), CompressionLevel);
}

bool URuntimeArchiverBase::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel)
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

void URuntimeArchiverBase::ExtractEntriesToStorage(FRuntimeArchiverAsyncOperationResult OnResult, TArray<FRuntimeArchiveEntry> EntryInfo, FString DirectoryPath, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	FPaths::NormalizeDirectoryName(DirectoryPath);
	
	AsyncTask(ENamedThreads::AnyThread, [this, OnResult, EntryInfo = MoveTemp(EntryInfo), DirectoryPath = MoveTemp(DirectoryPath), bForceOverwrite]()
	{
		auto ExecuteResult = [this, OnResult](bool bResult)
		{
			AsyncTask(ENamedThreads::GameThread, [this, OnResult, bResult]()
			{
				OnResult.ExecuteIfBound(bResult);
			});
		};

		for (const FRuntimeArchiveEntry& Entry : EntryInfo)
		{
			const FString ExtractFilePath = [&Entry]()
			{
				FString FilePath = Entry.Name;
				FPaths::NormalizeDirectoryName(FilePath);
				return MoveTemp(FilePath);
			}();

			if (!ExtractEntryToStorage(Entry, FPaths::Combine(DirectoryPath, TEXT("/"), ExtractFilePath), bForceOverwrite))
			{
				ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot extract '%s' entry. Aborting async extracting entries"), *Entry.Name));
				ExecuteResult(false);
				return;
			}
		}

		UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted '%d' entries"), EntryInfo.Num());
		
		ExecuteResult(true);
	});
}

void URuntimeArchiverBase::ExtractEntriesToStorage_Directory(FRuntimeArchiverAsyncOperationResult OnResult, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	const int32 NumOfEntries{GetArchiveEntries()};

	FPaths::NormalizeDirectoryName(EntryName);
	FPaths::NormalizeDirectoryName(DirectoryPath);

	// Composing the base path to have the correct directory path
	// For example, if we scanned the "Folder" directory and found "Folder/File.ogg" entry name, then the directory file system path will be either "Folder/File.ogg" or "File.ogg" (depending on bAddParentDirectory)
	const FString BaseDirectoryPathToExclude = [&EntryName, bAddParentDirectory]()
	{
		FString BasePath{FPaths::GetPath(EntryName)};

		if (!bAddParentDirectory)
		{
			BasePath += TEXT("/") + FPaths::GetCleanFilename(EntryName) + TEXT("/");
		}

		return MoveTemp(BasePath);
	}();

	AsyncTask(ENamedThreads::AnyThread, [this, OnResult, NumOfEntries, EntryName, DirectoryPath, BaseDirectoryPathToExclude, bForceOverwrite]()
	{
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

			if (EntryName.IsEmpty() || ArchiveEntry.Name.StartsWith(EntryName))
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

	if (UnarchivedData64.Num() > MAX_int32)
	{
		ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), MAX_int32, UnarchivedData64.Num()));
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

	URuntimeArchiverSubsystem* ArchiveSubsystem = URuntimeArchiverSubsystem::GetArchiveSubsystem();

	if (ArchiveSubsystem != nullptr)
	{
		if (ArchiveSubsystem->OnError.IsBound())
		{
			ArchiveSubsystem->OnError.Broadcast(ErrorCode, ErrorString);
		}
	}

	UE_LOG(LogRuntimeArchiver, Error, TEXT("%s"), *ErrorString);
}
