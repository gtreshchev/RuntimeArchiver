// Georgy Treshchev 2022.

#include "ArchiverZip/UnrealArchiverZip.h"

#include "UnrealArchiverSubsystem.h"
#include "UnrealArchiverDefines.h"
#include "UnrealArchiverZipIncludes.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UUnrealArchiverZip::UUnrealArchiverZip()
	: Super::UUnrealArchiverBase()
  , bAppendMode(false)
  , MinizArchiver(nullptr)
{
}

bool UUnrealArchiverZip::CreateArchiveInStorage(FString ArchivePath)
{
	if (!Super::CreateArchiveInStorage(ArchivePath))
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	// Creating an archive in storage
	if (!mz_zip_writer_init_file_v2(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0, MZ_ZIP_FLAG_WRITE_ZIP64))
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while initializing zip archive '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully created zip archive '%s' in '%s'"), *GetName(), *ArchivePath);

	return true;
}

bool UUnrealArchiverZip::CreateArchiveInMemory(int32 InitialAllocationSize)
{
	if (!Super::CreateArchiveInMemory(InitialAllocationSize))
	{
		return false;
	}

	// Creating an archive in memory
	if (!mz_zip_writer_init_heap(static_cast<mz_zip_archive*>(MinizArchiver), 0, static_cast<size_t>(InitialAllocationSize)))
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Unable to initialize archive in memory"));
		Reset();
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully created zip archive '%s' in memory"), *GetName());

	return true;
}

bool UUnrealArchiverZip::OpenArchiveFromStorage(FString ArchivePath)
{
	if (!Super::OpenArchiveFromStorage(ArchivePath))
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	// Reading the archive from the file path
	if (!mz_zip_reader_init_file(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0))
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while opening zip archive '%s' to read"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully opened zip archive '%s' in '%s' to read"), *GetName(), *ArchivePath);

	return true;
}

bool UUnrealArchiverZip::OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData)
{
	if (!Super::OpenArchiveFromMemory(ArchiveData))
	{
		return false;
	}

	if (!mz_zip_reader_init_mem(static_cast<mz_zip_archive*>(MinizArchiver), ArchiveData.GetData(), ArchiveData.Num(), MZ_ZIP_FLAG_WRITE_ZIP64))
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized,TEXT("Unable to open archive in memory"));
		Reset();
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully opened zip archive '%s' in memory"), *GetName());

	return true;
}

bool UUnrealArchiverZip::CloseArchive()
{
	if (!Super::CloseArchive())
	{
		return false;
	}

	bool bResult{true};

	switch (Mode)
	{
	case EUnrealArchiveMode::Read:
		{
			bResult = static_cast<bool>(mz_zip_reader_end(static_cast<mz_zip_archive*>(MinizArchiver)));
			break;
		}
	case EUnrealArchiveMode::Write:
		{
			if (Location == EUnrealArchiveLocation::Storage)
			{
				bResult = static_cast<bool>(mz_zip_writer_finalize_archive(static_cast<mz_zip_archive*>(MinizArchiver)));
			}

			if (!mz_zip_writer_end(static_cast<mz_zip_archive*>(MinizArchiver)))
			{
				bResult = false;
			}

			break;
		}
	default:
		{
			bResult = false;
			break;
		}
	}

	if (!bResult)
	{
		ReportError(EUnrealArchiverErrorCode::CloseError, TEXT("Failed to close the archive"));
		return false;
	}

	Reset();

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully closed zip archive '%s'"), *GetName());

	return true;
}

int32 UUnrealArchiverZip::GetArchiveEntries()
{
	if (Super::GetArchiveEntries() < 0)
	{
		return false;
	}

	return static_cast<int32>(mz_zip_reader_get_num_files(static_cast<mz_zip_archive*>(MinizArchiver)));
}

bool UUnrealArchiverZip::GetArchiveEntryInfoByName(FString EntryName, FUnrealArchiveEntry& EntryInfo)
{
	if (!Super::GetArchiveEntryInfoByName(EntryName, EntryInfo))
	{
		return false;
	}

	FPaths::NormalizeFilename(EntryName);

	// Looking for the entry index depending on the entry name
	const int32 EntryIndex{static_cast<int32>(mz_zip_reader_locate_file(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*EntryName), nullptr, 0))};

	if (EntryIndex == -1)
	{
		ReportError(EUnrealArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to find the entry index under the entry name '%s'"), *EntryName));
		return false;
	}

	return GetArchiveEntryInfoByIndex(EntryIndex, EntryInfo);
}

bool UUnrealArchiverZip::GetArchiveEntryInfoByIndex(int32 EntryIndex, FUnrealArchiveEntry& EntryInfo)
{
	if (!Super::GetArchiveEntryInfoByIndex(EntryIndex, EntryInfo))
	{
		return false;
	}

	// Get file information
	mz_zip_archive_file_stat ArchiveFileStat;
	const bool bResult{static_cast<bool>(mz_zip_reader_file_stat(static_cast<mz_zip_archive*>(MinizArchiver), static_cast<mz_uint>(EntryIndex), &ArchiveFileStat))};

	if (!bResult)
	{
		ReportError(EUnrealArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to get entry information by index %d"), EntryIndex));
		return false;
	}

	// Filling in the entry data
	{
		EntryInfo.Index = static_cast<int32>(ArchiveFileStat.m_file_index);
		EntryInfo.CompressedSize = static_cast<int64>(ArchiveFileStat.m_comp_size);
		EntryInfo.UncompressedSize = static_cast<int64>(ArchiveFileStat.m_uncomp_size);
#ifndef MINIZ_NO_TIME
		EntryInfo.CreationTime = FDateTime::FromUnixTimestamp(ArchiveFileStat.m_time);
#endif
		EntryInfo.Name = UTF8_TO_TCHAR(ArchiveFileStat.m_filename);
		EntryInfo.bIsDirectory = static_cast<bool>(mz_zip_reader_is_file_a_directory(static_cast<mz_zip_archive*>(MinizArchiver), static_cast<mz_uint>(EntryIndex)));
	}

	return true;
}

bool UUnrealArchiverZip::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, EUnrealEntryCompressionLevel CompressionLevel)
{
	if (!Super::AddEntryFromMemory(EntryName, DataToBeArchived, CompressionLevel))
	{
		return false;
	}

	FPaths::NormalizeFilename(EntryName);

	// Writing data to the entry from memory
	const bool bResult = static_cast<bool>(mz_zip_writer_add_mem_ex(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*EntryName),
	                                                                DataToBeArchived.GetData(), static_cast<size_t>(DataToBeArchived.Num()),
	                                                                nullptr, 0,
	                                                                static_cast<mz_uint>(CompressionLevel), 0, 0));

	if (!bResult)
	{
		ReportError(EUnrealArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to add entry '%s' from memory"), *EntryName));
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully added entry '%s' from memory with size '%lld'"), *EntryName, static_cast<int64>(DataToBeArchived.Num()));

	return true;
}

bool UUnrealArchiverZip::ExtractEntryToMemory(const FUnrealArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData)
{
	if (!Super::ExtractEntryToMemory(EntryInfo, UnarchivedData))
	{
		return false;
	}

	if (Mode != EUnrealArchiveMode::Read)
	{
		ReportError(EUnrealArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for extracting zip entries (using mode: '%s')"), *UEnum::GetValueAsName(EUnrealArchiveMode::Read).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	const int32 NumOfArchiveEntries{GetArchiveEntries()};

	if (EntryInfo.Index > (NumOfArchiveEntries - 1) || EntryInfo.Index < 0)
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Entry index %d is invalid. Min index: 0, Max index: %d"), EntryInfo.Index, (NumOfArchiveEntries - 1)));
		return false;
	}

	// Extracting to memory
	size_t EntryInMemorySize;
	void* EntryInMemoryPtr{mz_zip_reader_extract_to_heap(static_cast<mz_zip_archive*>(MinizArchiver), static_cast<mz_uint>(EntryInfo.Index), &EntryInMemorySize, 0)};

	if (EntryInMemoryPtr == nullptr)
	{
		ReportError(EUnrealArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to extract entry '%s' into memory"), *EntryInfo.Name));
		return false;
	}

	UnarchivedData = TArray64<uint8>(static_cast<uint8*>(EntryInMemoryPtr), EntryInMemorySize);

	FMemory::Free(EntryInMemoryPtr);

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully extracted entry '%s' into memory"), *EntryInfo.Name);

	return true;
}

bool UUnrealArchiverZip::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// Creating Miniz archiver
	MinizArchiver = static_cast<mz_zip_archive*>(FMemory::Memzero(FMemory::Malloc(sizeof(mz_zip_archive)), sizeof(mz_zip_archive)));

	if (MinizArchiver == nullptr)
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, TEXT("Unable to allocate memory for Miniz archiver"));
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully initialized zip archiver '%s'"), *GetName());

	return true;
}

bool UUnrealArchiverZip::IsInitialized() const
{
	return Super::IsInitialized() && MinizArchiver != nullptr;
}

void UUnrealArchiverZip::Reset()
{
	if (MinizArchiver != nullptr)
	{
		FMemory::Free(MinizArchiver);
		MinizArchiver = nullptr;
	}

	Super::Reset();

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully uninitialized zip archiver '%s'"), *GetName());
}

void UUnrealArchiverZip::ReportError(EUnrealArchiverErrorCode ErrorCode, const FString& ErrorString) const
{
	FString ReadyErrorString{FString::Printf(TEXT("%s: %s."), *UEnum::GetValueAsName(ErrorCode).ToString(), *ErrorString)};

	if (IsInitialized())
	{
		mz_zip_archive* MinizArchiveReal{static_cast<mz_zip_archive*>(MinizArchiver)};

		const mz_zip_error LastMinizError{mz_zip_get_last_error(MinizArchiveReal)};
		const FString LastMinizErrorStr{UTF8_TO_TCHAR(mz_zip_get_error_string(LastMinizError))};

		// Cleaning last miniz error to avoid getting the same error next time
		if (LastMinizError != MZ_ZIP_NO_ERROR)
		{
			mz_zip_set_error(MinizArchiveReal, MZ_ZIP_NO_ERROR);
			ReadyErrorString += FString::Printf(TEXT("\nMiniz error details: '%s'."), *LastMinizErrorStr);
		}
	}

	Super::ReportError(ErrorCode, ReadyErrorString);
}

bool UUnrealArchiverZip::OpenArchiveFromStorageToAppend(FString ArchivePath)
{
	if (!Initialize())
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	if (ArchivePath.IsEmpty() || !FPaths::FileExists(ArchivePath))
	{
		ReportError(EUnrealArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Archive '%s' does not exist"), *ArchivePath));
		return false;
	}

	bAppendMode = true;

	Mode = EUnrealArchiveMode::Write;
	Location = EUnrealArchiveLocation::Storage;

	// Reading the archive from the file path
	if (!mz_zip_reader_init_file(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0))
	{
		ReportError(EUnrealArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while opening zip archive '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	// Initializing the archive for adding entries
	if (!mz_zip_writer_init_from_reader_v2(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0))
	{
		mz_zip_reader_end(static_cast<mz_zip_archive*>(MinizArchiver));
		ReportError(EUnrealArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while opening zip archive '%s' to append"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogUnrealArchiver, Log, TEXT("Successfully opened zip archive '%s' in '%s' to append"), *GetName(), *ArchivePath);

	return true;
}
