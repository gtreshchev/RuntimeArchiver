// Georgy Treshchev 2022.

#include "UnrealArchiverBase.h"
#include "UnrealArchiverSubsystem.h"
#include "UnrealArchiverDefines.h"

UUnrealArchiverBase::UUnrealArchiverBase()
	: Mode(EUnrealArchiveMode::Undefined)
  , Location(EUnrealArchiveLocation::Undefined)
{
}

UUnrealArchiverBase* UUnrealArchiverBase::CreateUnrealArchiver(UObject* WorldContextObject, TSubclassOf<UUnrealArchiverBase> ArchiverClass)
{
	return NewObject<UUnrealArchiverBase>(WorldContextObject, ArchiverClass);
}

void UUnrealArchiverBase::BeginDestroy()
{
	if (IsInitialized())
	{
		CloseArchive();
		Reset();
	}

	Super::BeginDestroy();
}

bool UUnrealArchiverBase::CreateArchiveInStorage(FString ArchivePath)
{
	if (!Initialize())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, FString::Printf(TEXT("Unable to initialize archiver in storage path '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	if (ArchivePath.IsEmpty())
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, TEXT("Archive name not specified"));
		Reset();
		return false;
	}

	Mode = EUnrealArchiveMode::Write;
	Location = EUnrealArchiveLocation::Storage;

	return true;
}

bool UUnrealArchiverBase::CreateArchiveInMemory(int32 InitialAllocationSize)
{
	if (!Initialize())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Unable to initialize archiver in memory"));
		Reset();
		return false;
	}

	Mode = EUnrealArchiveMode::Write;
	Location = EUnrealArchiveLocation::Memory;

	return true;
}

bool UUnrealArchiverBase::OpenArchiveFromStorage(FString ArchivePath)
{
	if (ArchivePath.IsEmpty() || !FPaths::FileExists(ArchivePath))
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Archive '%s' does not exist"), *ArchivePath));
		return false;
	}

	if (!Initialize())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, FString::Printf(TEXT("Unable to initialize archiver in path '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	Mode = EUnrealArchiveMode::Read;
	Location = EUnrealArchiveLocation::Storage;

	return true;
}

bool UUnrealArchiverBase::OpenArchiveFromMemory(TArray<uint8> ArchiveData)
{
	return OpenArchiveFromMemory(TArray64<uint8>(MoveTemp(ArchiveData)));
}

bool UUnrealArchiverBase::OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData)
{
	if (!Initialize())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Unable to initialize archiver in memory"));
		Reset();
		return false;
	}

	Mode = EUnrealArchiveMode::Read;
	Location = EUnrealArchiveLocation::Memory;

	return true;
}

bool UUnrealArchiverBase::CloseArchive()
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	return true;
}

bool UUnrealArchiverBase::GetArchiveDataFromMemory(TArray<uint8>& ArchiveData)
{
	TArray64<uint8> ArchiveData64;

	if (!GetArchiveDataFromMemory(ArchiveData64))
	{
		return false;
	}

	if (ArchiveData64.Num() > MAX_int32)
	{
		ReportError(EUnrealArchiverErrorCode::ExtractError, FString::Printf(TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), MAX_int32, ArchiveData64.Num()));
		return false;
	}

	ArchiveData = TArray<uint8>(MoveTemp(ArchiveData64));

	return true;
}

bool UUnrealArchiverBase::GetArchiveDataFromMemory(TArray64<uint8>& ArchiveData)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != EUnrealArchiveMode::Write || Location != EUnrealArchiveLocation::Memory)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode and '%s' location are supported to get archive data (using mode: '%s', using location: '%s')"),
		                                                                       *UEnum::GetValueAsName(EUnrealArchiveMode::Write).ToString(), *UEnum::GetValueAsName(EUnrealArchiveLocation::Memory).ToString(),
		                                                                       *UEnum::GetValueAsName(Mode).ToString(), *UEnum::GetValueAsName(Location).ToString()));
		return false;
	}

	return true;
}

int32 UUnrealArchiverBase::GetArchiveEntries()
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return -1;
	}

	return 0;
}

bool UUnrealArchiverBase::GetArchiveEntryInfoByName(FString EntryName, FUnrealArchiveEntry& EntryInfo)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (EntryName.IsEmpty())
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, TEXT("Entry name not specified"));
		return false;
	}

	return true;
}

bool UUnrealArchiverBase::GetArchiveEntryInfoByIndex(int32 EntryIndex, FUnrealArchiveEntry& EntryInfo)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (EntryIndex < 0)
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Unsupported entry index. Must be >= 0 (provided index: %d)"), EntryIndex));
		return false;
	}

	return true;
}

bool UUnrealArchiverBase::AddEntryFromStorage(FString EntryName, FString FilePath, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != EUnrealArchiveMode::Write)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for adding entries (using mode: '%s')"), *UEnum::GetValueAsName(EUnrealArchiveMode::Write).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryName.IsEmpty())
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, TEXT("Entry name not specified"));
		return false;
	}

	if (!FPaths::FileExists(FilePath) && !FPaths::DirectoryExists(FilePath))
	{
		ReportError(EUnrealArchiverErrorCode::AddError, FString::Printf(TEXT("Path '%s' does not contain a directory or file"), *FilePath));
		return false;
	}

	return true;
}

void UUnrealArchiverBase::AddEntryFromStorage_Recursively(FUnrealArchiverRecursiveResult OnResult, FString DirectoryPath, bool bAddParentDirectory, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		OnResult.ExecuteIfBound(false);
		return;
	}

	// Normalizing directory path
	FPaths::NormalizeDirectoryName(DirectoryPath);

	// Making sure the directory exists
	if (!FPaths::DirectoryExists(DirectoryPath))
	{
		ReportError(EUnrealArchiverErrorCode::AddError, FString::Printf(TEXT("Directory '%s' does not exist"), *DirectoryPath));
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
		const bool bResult{AddEntryFromStorage_Recursively_Internal(BaseDirectoryPathToExclude, DirectoryPath, CompressionLevel)};

		if (bResult)
		{
			UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully added entries from directory '%s'"), *DirectoryPath);
		}

		AsyncTask(ENamedThreads::GameThread, [bResult, OnResult]()
		{
			OnResult.ExecuteIfBound(bResult);
		});
	});
}

bool UUnrealArchiverBase::AddEntryFromStorage_Recursively_Internal(FString BaseDirectoryPathToExclude, FString DirectoryPath, EUnrealEntryCompressionLevel CompressionLevel)
{
	class FDirectoryVisitor_EntryAppender : public IPlatformFile::FDirectoryVisitor
	{
		UUnrealArchiverBase* UnrealArchiver;
		const FString& BaseDirectoryPathToExclude;
		EUnrealEntryCompressionLevel CompressionLevel;
	public:
		FDirectoryVisitor_EntryAppender(UUnrealArchiverBase* UnrealArchiver, const FString& BaseDirectoryPathToExclude, EUnrealEntryCompressionLevel CompressionLevel)
			: UnrealArchiver(UnrealArchiver)
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

			if (!UnrealArchiver->AddEntryFromStorage(EntryName, FilenameOrDirectory, CompressionLevel))
			{
				UnrealArchiver->ReportError(EUnrealArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot add '%s' entry. Aborting recursive adding entries"), *EntryName));
				return false;
			}

			return true;
		}
	};

	FDirectoryVisitor_EntryAppender DirectoryVisitor_EntryAppender(this, BaseDirectoryPathToExclude, CompressionLevel);

	return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*DirectoryPath, DirectoryVisitor_EntryAppender);
}

bool UUnrealArchiverBase::AddEntryFromMemory(FString EntryName, TArray<uint8> DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel)
{
	return AddEntryFromMemory(MoveTemp(EntryName), TArray64<uint8>(MoveTemp(DataToBeArchived)), CompressionLevel);
}

bool UUnrealArchiverBase::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != EUnrealArchiveMode::Write)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for adding entries (using mode: '%s')"), *UEnum::GetValueAsName(EUnrealArchiveMode::Write).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryName.IsEmpty())
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, TEXT("Entry name not specified"));
		return false;
	}

	return true;
}

bool UUnrealArchiverBase::ExtractEntryToStorage(const FUnrealArchiveEntry& EntryInfo, FString FilePath, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != EUnrealArchiveMode::Read)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for extracting the entry (using mode: '%s')"), *UEnum::GetValueAsName(EUnrealArchiveMode::Read).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryInfo.bIsDirectory)
	{
		FPaths::NormalizeDirectoryName(FilePath);

		if (FPaths::DirectoryExists(FilePath))
		{
			if (!bForceOverwrite)
			{
				ReportError(EUnrealArchiverErrorCode::ExtractError, FString::Printf(TEXT("Directory '%s' already exists"), *FilePath));
				return false;
			}

			UE_LOG(LogUnrealArchiver, Warning, TEXT("Directory '%s' already exists. It will be overwritten"), *FilePath);
		}
	}
	else
	{
		FPaths::NormalizeFilename(FilePath);

		if (FPaths::FileExists(FilePath))
		{
			if (!bForceOverwrite)
			{
				ReportError(EUnrealArchiverErrorCode::ExtractError, FString::Printf(TEXT("File '%s' already exists"), *FilePath));
				return false;
			}

			UE_LOG(LogUnrealArchiver, Warning, TEXT("File '%s' already exists. It will be overwritten"), *FilePath);
		}
	}

	return true;
}

void UUnrealArchiverBase::ExtractEntryToStorage_Recursively(FUnrealArchiverRecursiveResult OnResult, FString EntryName, FString DirectoryPath, bool bAddParentDirectory, bool bForceOverwrite)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
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
			FUnrealArchiveEntry ArchiveEntry;

			if (!GetArchiveEntryInfoByIndex(EntryIndex, ArchiveEntry))
			{
				ReportError(EUnrealArchiverErrorCode::AddError, FString::Printf(TEXT("Cannot get '%d' entry to extract. Aborting recursive extracting entries"), EntryIndex));
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
			UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully extracted entries from '%s'"), *EntryName);
		}

		AsyncTask(ENamedThreads::GameThread, [OnResult, bResult]()
		{
			OnResult.ExecuteIfBound(bResult);
		});
	});
}

bool UUnrealArchiverBase::ExtractEntryToMemory(const FUnrealArchiveEntry& EntryInfo, TArray<uint8>& UnarchivedData)
{
	TArray64<uint8> UnarchivedData64;

	if (!ExtractEntryToMemory(EntryInfo, UnarchivedData64))
	{
		return false;
	}

	if (UnarchivedData64.Num() > MAX_int32)
	{
		ReportError(EUnrealArchiverErrorCode::ExtractError, FString::Printf(TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), MAX_int32, UnarchivedData64.Num()));
		return false;
	}

	UnarchivedData = TArray<uint8>(MoveTemp(UnarchivedData64));

	return true;
}

bool UUnrealArchiverBase::ExtractEntryToMemory(const FUnrealArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData)
{
	if (!IsInitialized())
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Archiver is not initialized"));
		return false;
	}

	if (Mode != EUnrealArchiveMode::Read)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for extracting the entry (using mode: '%s')"), *UEnum::GetValueAsName(EUnrealArchiveMode::Read).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	if (EntryInfo.bIsDirectory)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedLocation, TEXT("It is impossible to extract directory entry into memory"));
		return false;
	}

	return true;
}

bool UUnrealArchiverBase::IsInitialized() const
{
	return Mode != EUnrealArchiveMode::Undefined && Location != EUnrealArchiveLocation::Undefined;
}

void UUnrealArchiverBase::Reset()
{
	Mode = EUnrealArchiveMode::Undefined;
	Location = EUnrealArchiveLocation::Undefined;
}

void UUnrealArchiverBase::ReportError(EUnrealArchiverErrorCode ErrorCode, const FString& ErrorString) const
{
	// Making sure we are in the game thread
	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [this, ErrorCode, ErrorString]() { ReportError(ErrorCode, ErrorString); });
		return;
	}

	UUnrealArchiverSubsystem* ArchiveSubsystem = UUnrealArchiverSubsystem::GetArchiveSubsystem();

	if (ArchiveSubsystem != nullptr)
	{
		if (ArchiveSubsystem->OnError.IsBound())
		{
			ArchiveSubsystem->OnError.Broadcast(ErrorCode, ErrorString);
		}
	}

	UE_LOG(LogUnrealArchiver, Error, TEXT("%s"), *ErrorString);
}
