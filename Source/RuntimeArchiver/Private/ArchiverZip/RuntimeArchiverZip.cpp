// Georgy Treshchev 2023.

#include "ArchiverZip/RuntimeArchiverZip.h"

#include "RuntimeArchiverSubsystem.h"
#include "RuntimeArchiverDefines.h"
#include "RuntimeArchiverZipIncludes.h"
#include "Misc/Paths.h"

URuntimeArchiverZip::URuntimeArchiverZip()
	: Super::URuntimeArchiverBase()
  , bAppendMode(false)
  , MinizArchiver(nullptr)
{
}

bool URuntimeArchiverZip::CreateArchiveInStorage(FString ArchivePath)
{
	if (!Super::CreateArchiveInStorage(ArchivePath))
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	// Creating an archive in storage
	if (!mz_zip_writer_init_file_v2(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0, MZ_ZIP_FLAG_WRITE_ZIP64))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while initializing zip archive '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully created zip archive '%s' in '%s'"), *GetName(), *ArchivePath);

	return true;
}

bool URuntimeArchiverZip::CreateArchiveInMemory(int32 InitialAllocationSize)
{
	if (!Super::CreateArchiveInMemory(InitialAllocationSize))
	{
		return false;
	}

	// Creating an archive in memory
	if (!mz_zip_writer_init_heap(static_cast<mz_zip_archive*>(MinizArchiver), 0, static_cast<size_t>(InitialAllocationSize)))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to initialize archive in memory"));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully created zip archive '%s' in memory"), *GetName());

	return true;
}

bool URuntimeArchiverZip::OpenArchiveFromStorage(FString ArchivePath)
{
	if (!Super::OpenArchiveFromStorage(ArchivePath))
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	// Reading the archive from the file path
	if (!mz_zip_reader_init_file(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while opening zip archive '%s' to read"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened zip archive '%s' in '%s' to read"), *GetName(), *ArchivePath);

	return true;
}

bool URuntimeArchiverZip::OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData)
{
	if (!Super::OpenArchiveFromMemory(ArchiveData))
	{
		return false;
	}

	if (!mz_zip_reader_init_mem(static_cast<mz_zip_archive*>(MinizArchiver), ArchiveData.GetData(), ArchiveData.Num(), MZ_ZIP_FLAG_WRITE_ZIP64))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized,TEXT("Unable to open in-memory zip archive to read"));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened in-memory zip archive '%s' to read"), *GetName());

	return true;
}

bool URuntimeArchiverZip::CloseArchive()
{
	if (!Super::CloseArchive())
	{
		return false;
	}

	bool bResult{true};

	switch (Mode)
	{
	case ERuntimeArchiverMode::Read:
		{
			bResult = static_cast<bool>(mz_zip_reader_end(static_cast<mz_zip_archive*>(MinizArchiver)));
			break;
		}
	case ERuntimeArchiverMode::Write:
		{
			if (Location == ERuntimeArchiverLocation::Storage)
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
		ReportError(ERuntimeArchiverErrorCode::CloseError, TEXT("Failed to close the archive"));
		return false;
	}

	Reset();

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully closed zip archive '%s'"), *GetName());

	return true;
}

bool URuntimeArchiverZip::GetArchiveData(TArray64<uint8>& ArchiveData)
{
	if (!Super::GetArchiveData(ArchiveData))
	{
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Write || Location != ERuntimeArchiverLocation::Memory)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode and '%s' location are supported to get zip archive data (using mode: '%s', using location: '%s')"),
		                                                                        *UEnum::GetValueAsName(ERuntimeArchiverMode::Write).ToString(), *UEnum::GetValueAsName(ERuntimeArchiverLocation::Memory).ToString(),
		                                                                        *UEnum::GetValueAsName(Mode).ToString(), *UEnum::GetValueAsName(Location).ToString()));
		return false;
	}

	mz_zip_archive* MinizArchiverReal = static_cast<mz_zip_archive*>(MinizArchiver);

	if (!mz_zip_writer_finalize_archive(MinizArchiverReal))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to get zip archive data from memory")));
		return false;
	}

	const int64 ArchiveSize{static_cast<int64>(MinizArchiverReal->m_archive_size)};

	ArchiveData = TArray64<uint8>(static_cast<uint8*>(MinizArchiverReal->m_pState->m_pMem), ArchiveSize);

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved zip archive data from memory with size %lld bytes"), ArchiveSize);

	return true;
}

bool URuntimeArchiverZip::GetArchiveEntries(int32& NumOfArchiveEntries)
{
	if (!Super::GetArchiveEntries(NumOfArchiveEntries))
	{
		return false;
	}

	NumOfArchiveEntries = mz_zip_reader_get_num_files(static_cast<mz_zip_archive*>(MinizArchiver));
	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved %d zip entries"), NumOfArchiveEntries);
	return true;
}

bool URuntimeArchiverZip::GetArchiveEntryInfoByName(FString EntryName, FRuntimeArchiveEntry& EntryInfo)
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
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to find zip entry index under the entry name '%s'"), *EntryName));
		return false;
	}

	return GetArchiveEntryInfoByIndex(EntryIndex, EntryInfo);
}

bool URuntimeArchiverZip::GetArchiveEntryInfoByIndex(int32 EntryIndex, FRuntimeArchiveEntry& EntryInfo)
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
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to get zip entry information by index %d"), EntryIndex));
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

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved zip entry '%s'"), *EntryInfo.Name);

	return true;
}

bool URuntimeArchiverZip::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel)
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
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to add zip entry '%s' from memory"), *EntryName));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added zip entry '%s' from memory with size '%lld'"), *EntryName, static_cast<int64>(DataToBeArchived.Num()));

	return true;
}

bool URuntimeArchiverZip::ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData)
{
	if (!Super::ExtractEntryToMemory(EntryInfo, UnarchivedData))
	{
		return false;
	}

	if (Mode != ERuntimeArchiverMode::Read)
	{
		ReportError(ERuntimeArchiverErrorCode::UnsupportedMode, FString::Printf(TEXT("Only '%s' mode is supported for extracting zip entries (using mode: '%s')"), *UEnum::GetValueAsName(ERuntimeArchiverMode::Read).ToString(), *UEnum::GetValueAsName(Mode).ToString()));
		return false;
	}

	int32 NumOfArchiveEntries;
	if (!GetArchiveEntries(NumOfArchiveEntries) || EntryInfo.Index > (NumOfArchiveEntries - 1))
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Zip entry index %d is invalid. Min index: 0, Max index: %d"), EntryInfo.Index, (NumOfArchiveEntries - 1)));
		return false;
	}

	// Extracting to memory
	size_t EntryInMemorySize;
	void* EntryInMemoryPtr{mz_zip_reader_extract_to_heap(static_cast<mz_zip_archive*>(MinizArchiver), static_cast<mz_uint>(EntryInfo.Index), &EntryInMemorySize, 0)};

	if (!EntryInMemoryPtr)
	{
		ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to extract zip entry '%s' into memory"), *EntryInfo.Name));
		return false;
	}

	UnarchivedData = TArray64<uint8>(static_cast<uint8*>(EntryInMemoryPtr), EntryInMemorySize);

	FMemory::Free(EntryInMemoryPtr);

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted zip entry '%s' into memory"), *EntryInfo.Name);

	return true;
}

bool URuntimeArchiverZip::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// Creating Miniz archiver
	MinizArchiver = static_cast<mz_zip_archive*>(FMemory::Memzero(FMemory::Malloc(sizeof(mz_zip_archive)), sizeof(mz_zip_archive)));

	if (!MinizArchiver)
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to allocate memory for zip archiver"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully initialized zip archiver '%s'"), *GetName());

	return true;
}

bool URuntimeArchiverZip::IsInitialized() const
{
	return Super::IsInitialized() && MinizArchiver;
}

void URuntimeArchiverZip::Reset()
{
	if (MinizArchiver)
	{
		FMemory::Free(MinizArchiver);
		MinizArchiver = nullptr;
	}

	Super::Reset();

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully uninitialized zip archiver '%s'"), *GetName());
}

void URuntimeArchiverZip::ReportError(ERuntimeArchiverErrorCode ErrorCode, const FString& ErrorString) const
{
	FString ReadyErrorString{FString::Printf(TEXT("%s: %s."), *UEnum::GetValueAsName(ErrorCode).ToString(), *ErrorString)};

	if (IsInitialized())
	{
		mz_zip_archive* MinizArchiverReal{static_cast<mz_zip_archive*>(MinizArchiver)};

		const mz_zip_error LastMinizError{mz_zip_get_last_error(MinizArchiverReal)};
		const FString LastMinizErrorStr{UTF8_TO_TCHAR(mz_zip_get_error_string(LastMinizError))};

		// Cleaning last miniz error to avoid getting the same error next time
		if (LastMinizError != MZ_ZIP_NO_ERROR)
		{
			mz_zip_set_error(MinizArchiverReal, MZ_ZIP_NO_ERROR);
			ReadyErrorString += FString::Printf(TEXT("\nMiniz error details: '%s'."), *LastMinizErrorStr);
		}
	}

	Super::ReportError(ErrorCode, ReadyErrorString);
}

bool URuntimeArchiverZip::OpenArchiveFromStorageToAppend(FString ArchivePath)
{
	if (!Initialize())
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	if (ArchivePath.IsEmpty() || !FPaths::FileExists(ArchivePath))
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Archive '%s' does not exist"), *ArchivePath));
		return false;
	}

	bAppendMode = true;

	Mode = ERuntimeArchiverMode::Write;
	Location = ERuntimeArchiverLocation::Storage;

	// Reading the archive from the file path
	if (!mz_zip_reader_init_file(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while opening zip archive '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	// Initializing the archive for adding entries
	if (!mz_zip_writer_init_from_reader_v2(static_cast<mz_zip_archive*>(MinizArchiver), TCHAR_TO_UTF8(*ArchivePath), 0))
	{
		mz_zip_reader_end(static_cast<mz_zip_archive*>(MinizArchiver));
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("An error occurred while opening zip archive '%s' to append"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened zip archive '%s' in '%s' to append"), *GetName(), *ArchivePath);

	return true;
}
