// Georgy Treshchev 2023.

#include "ArchiverOodle/RuntimeArchiverOodle.h"
#include "RuntimeArchiverDefines.h"
#include "RuntimeArchiverUtilities.h"
#include "ArchiverRaw/RuntimeArchiverRaw.h"
#include "ArchiverTar/RuntimeArchiverTar.h"
#include "Streams/RuntimeArchiverFileStream.h"
#include "Streams/RuntimeArchiverMemoryStream.h"

URuntimeArchiverOodle::URuntimeArchiverOodle()
	: LastCompressionLevel{ERuntimeArchiverCompressionLevel::Compression6}
{
}

bool URuntimeArchiverOodle::CreateArchiveInStorage(FString ArchivePath)
{
	if (!Super::CreateArchiveInStorage(ArchivePath))
	{
		return false;
	}

	CompressedStream.Reset(new FRuntimeArchiverFileStream(ArchivePath, true));
	if (!CompressedStream->IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open Oodle stream because it is not valid"));
		Reset();
		return false;
	}

	if (!TarArchiver->CreateArchiveInMemory(0))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to create Oodle archive in storage '%s' due to tar archiver error"), *ArchivePath);
		Reset();
		return false;
	}

	return true;
}

bool URuntimeArchiverOodle::CreateArchiveInMemory(int32 InitialAllocationSize)
{
	if (!Super::CreateArchiveInMemory(InitialAllocationSize))
	{
		return false;
	}

	CompressedStream.Reset(new FRuntimeArchiverMemoryStream(InitialAllocationSize));
	if (!CompressedStream->IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open Oodle stream because it is not valid"));
		Reset();
		return false;
	}

	if (!TarArchiver->CreateArchiveInMemory(InitialAllocationSize))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to create Oodle archive in memory due to tar archiver error"));
		Reset();
		return false;
	}

	return true;
}

bool URuntimeArchiverOodle::OpenArchiveFromStorage(FString ArchivePath)
{
	if (!Super::OpenArchiveFromStorage(ArchivePath))
	{
		return false;
	}

	CompressedStream.Reset(new FRuntimeArchiverFileStream(ArchivePath, false));
	if (!CompressedStream->IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open Oodle stream because it is not valid"));
		Reset();
		return false;
	}

	TArray64<uint8> CompressedArchiveData;
	CompressedArchiveData.SetNumUninitialized(CompressedStream->Size());

	if (!CompressedStream->Read(CompressedArchiveData.GetData(), CompressedArchiveData.Num()))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Unable to read Oodle compressed stream to get archive data"));
		Reset();
		return false;
	}

	TArray64<uint8> TarArchiveData;
	if (!URuntimeArchiverRaw::UncompressRawData(ERuntimeArchiverRawFormat::Oodle, MoveTemp(CompressedArchiveData), TarArchiveData))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to uncompress Oodle to tar data to open archive from storage '%s'"), *ArchivePath));
		Reset();
		return false;
	}

	if (!TarArchiver->OpenArchiveFromMemory(TarArchiveData))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open Oodle archive from storage due to tar archiver error"));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened Oodle archive '%s' in '%s' to read"), *GetName(), *ArchivePath);
	return true;
}

bool URuntimeArchiverOodle::OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData)
{
	if (!Super::OpenArchiveFromMemory(ArchiveData))
	{
		return false;
	}

	CompressedStream.Reset(new FRuntimeArchiverMemoryStream(ArchiveData));
	if (!CompressedStream->IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open Oodle stream because it is not valid"));
		Reset();
		return false;
	}

	TArray64<uint8> TarArchiveData;
	if (!URuntimeArchiverRaw::UncompressRawData(ERuntimeArchiverRawFormat::Oodle, ArchiveData, TarArchiveData))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Unable to uncompress Oodle to tar data to open archive from memory"));
		Reset();
		return false;
	}

	if (!TarArchiver->OpenArchiveFromMemory(TarArchiveData))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open Oodle archive from memory due to tar archiver error"));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened in-memory Oodle archive '%s' to read"), *GetName());
	return true;
}

bool URuntimeArchiverOodle::CloseArchive()
{
	if (!Super::CloseArchive())
	{
		return false;
	}

	if (Mode == ERuntimeArchiverMode::Write && Location == ERuntimeArchiverLocation::Storage)
	{
		TArray64<uint8> ArchiveData;
		if (!GetArchiveData(ArchiveData))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get archive data to close archive"));
			return false;
		}

		if (!CompressedStream->Seek(0))
		{
			ReportError(ERuntimeArchiverErrorCode::CloseError, TEXT("Unable to seek first position in Oodle archive to close archive"));
			return false;
		}

		if (!CompressedStream->Write(ArchiveData.GetData(), ArchiveData.Num()))
		{
			ReportError(ERuntimeArchiverErrorCode::CloseError, TEXT("Unable to write archive data to Oodle compressed stream to close archive"));
			return false;
		}
	}

	if (!TarArchiver->CloseArchive())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to close Oodle archive due to tar archiver error"));
		return false;
	}

	Reset();
	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully closed Oodle archive '%s'"), *GetName());
	return true;
}

bool URuntimeArchiverOodle::GetArchiveData(TArray64<uint8>& ArchiveData)
{
	if (!Super::GetArchiveData(ArchiveData))
	{
		return false;
	}

	if (Mode == ERuntimeArchiverMode::Read)
	{
		ArchiveData.SetNumUninitialized(CompressedStream->Size());

		if (!CompressedStream->Read(ArchiveData.GetData(), ArchiveData.Num()))
		{
			ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Unable to read Oodle compressed stream to get archive data"));
			return false;
		}
	}
	else if (Mode == ERuntimeArchiverMode::Write)
	{
		TArray64<uint8> TarArchiveData;
		if (!TarArchiver->GetArchiveData(TarArchiveData))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get Oodle archive data due to tar archiver error"));
			return false;
		}

		if (!URuntimeArchiverRaw::CompressRawData(ERuntimeArchiverRawFormat::Oodle, LastCompressionLevel, TarArchiveData, ArchiveData))
		{
			ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Unable to compress tar to Oodle data to get archive data"));
			return false;
		}
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved Oodle archive data from memory with size '%lld'"), ArchiveData.Num());
	return true;
}

bool URuntimeArchiverOodle::GetArchiveEntries(int32& NumOfArchiveEntries)
{
	if (!Super::GetArchiveEntries(NumOfArchiveEntries))
	{
		return false;
	}

	if (!TarArchiver->GetArchiveEntries(NumOfArchiveEntries))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to the get number of Oodle entries due to tar archiver error"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved %d Oodle entries"), NumOfArchiveEntries);
	return true;
}

bool URuntimeArchiverOodle::GetArchiveEntryInfoByName(FString EntryName, FRuntimeArchiveEntry& EntryInfo)
{
	if (!Super::GetArchiveEntryInfoByName(EntryName, EntryInfo))
	{
		return false;
	}

	if (!TarArchiver->GetArchiveEntryInfoByName(MoveTemp(EntryName), EntryInfo))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get Oodle entry by name due to tar archiver error"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved Oodle entry '%s' by name"), *EntryInfo.Name);
	return true;
}

bool URuntimeArchiverOodle::GetArchiveEntryInfoByIndex(int32 EntryIndex, FRuntimeArchiveEntry& EntryInfo)
{
	if (!Super::GetArchiveEntryInfoByIndex(EntryIndex, EntryInfo))
	{
		return false;
	}

	if (!TarArchiver->GetArchiveEntryInfoByIndex(EntryIndex, EntryInfo))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get Oodle entry by index due to tar archiver error"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved Oodle entry '%s' by index"), *EntryInfo.Name);
	return true;
}

bool URuntimeArchiverOodle::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	if (!Super::AddEntryFromMemory(EntryName, DataToBeArchived, CompressionLevel))
	{
		return false;
	}

	if (!TarArchiver->AddEntryFromMemory(EntryName, DataToBeArchived, CompressionLevel))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to add Oodle entry due to tar archiver error"));
		return false;
	}

	LastCompressionLevel = CompressionLevel;
	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added Oodle entry '%s' with size %lld bytes from memory"), *EntryName, DataToBeArchived.Num());
	return true;
}

bool URuntimeArchiverOodle::ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData)
{
	if (!Super::ExtractEntryToMemory(EntryInfo, UnarchivedData))
	{
		return false;
	}

	if (!TarArchiver->ExtractEntryToMemory(EntryInfo, UnarchivedData))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to extract Oodle entry due to tar archiver error"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted Oodle entry '%s' into memory"), *EntryInfo.Name);
	return true;
}

bool URuntimeArchiverOodle::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	TarArchiver.Reset(Cast<URuntimeArchiverTar>(CreateRuntimeArchiver(this, URuntimeArchiverTar::StaticClass())));
	if (!TarArchiver.IsValid())
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to allocate memory for Oodle archiver"));
		return false;
	}

	return true;
}

bool URuntimeArchiverOodle::IsInitialized() const
{
	return Super::IsInitialized() && TarArchiver.IsValid() && TarArchiver->IsInitialized();
}

void URuntimeArchiverOodle::Reset()
{
	if (TarArchiver.IsValid())
	{
		TarArchiver->Reset();
		TarArchiver.Reset();
	}

	CompressedStream.Reset();
	Super::Reset();
	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully uninitialized Oodle archiver '%s'"), *GetName());
}

void URuntimeArchiverOodle::ReportError(ERuntimeArchiverErrorCode ErrorCode, const FString& ErrorString) const
{
	Super::ReportError(ErrorCode, ErrorString);
}
