// Georgy Treshchev 2023.

#include "ArchiverTar/RuntimeArchiverTar.h"
#include "RuntimeArchiverDefines.h"
#include "RuntimeArchiverTarOperations.h"
#include "RuntimeArchiverUtilities.h"
#include "ArchiverTar/RuntimeArchiverTarHeader.h"
#include "Streams/RuntimeArchiverFileStream.h"
#include "Streams/RuntimeArchiverMemoryStream.h"
#include "Misc/Paths.h"

bool URuntimeArchiverTar::CreateArchiveInStorage(FString ArchivePath)
{
	if (!Super::CreateArchiveInStorage(ArchivePath))
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	if (!TarEncapsulator->OpenFile(ArchivePath, true))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("Unable to open tar archive '%s' for writing"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully created tar archive '%s' in '%s'"), *GetName(), *ArchivePath);

	return true;
}

bool URuntimeArchiverTar::CreateArchiveInMemory(int32 InitialAllocationSize)
{
	if (!Super::CreateArchiveInMemory(InitialAllocationSize))
	{
		return false;
	}

	if (!TarEncapsulator->OpenMemory(TArray64<uint8>(), InitialAllocationSize, true))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to initialize tar archive in memory"));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully created tar archive '%s' in memory"), *GetName());

	return true;
}

bool URuntimeArchiverTar::OpenArchiveFromStorage(FString ArchivePath)
{
	if (!Super::OpenArchiveFromStorage(ArchivePath))
	{
		return false;
	}

	FPaths::NormalizeFilename(ArchivePath);

	if (!TarEncapsulator->OpenFile(ArchivePath, false))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, FString::Printf(TEXT("Unable to open tar archive '%s' for writing"), *ArchivePath));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened tar archive '%s' in '%s' to read"), *GetName(), *ArchivePath);

	return true;
}

bool URuntimeArchiverTar::OpenArchiveFromMemory(const TArray64<uint8>& ArchiveData)
{
	if (!Super::OpenArchiveFromMemory(ArchiveData))
	{
		return false;
	}

	if (!TarEncapsulator->OpenMemory(ArchiveData, 0, false))
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to open in-memory tar archive to read"));
		Reset();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully opened in-memory tar archive '%s' to read"), *GetName());

	return true;
}

bool URuntimeArchiverTar::CloseArchive()
{
	if (!Super::CloseArchive())
	{
		return false;
	}

	Reset();

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully closed tar archive '%s'"), *GetName());

	return true;
}

bool URuntimeArchiverTar::GetArchiveData(TArray64<uint8>& ArchiveData)
{
	if (!Super::GetArchiveData(ArchiveData))
	{
		return false;
	}

	const bool bSuccess{TarEncapsulator->GetArchiveData(ArchiveData)};

	if (!bSuccess)
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Unable to get tar archive data from memory"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved tar archive data from memory with size '%lld'"), ArchiveData.Num());

	return true;
}

bool URuntimeArchiverTar::GetArchiveEntries(int32& NumOfArchiveEntries)
{
	if (!Super::GetArchiveEntries(NumOfArchiveEntries))
	{
		return false;
	}

	if (!TarEncapsulator->GetArchiveEntries(NumOfArchiveEntries))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, TEXT("Unable to get number of tar entries"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved %d tar entries"), NumOfArchiveEntries);
	return true;
}

bool URuntimeArchiverTar::GetArchiveEntryInfoByName(FString EntryName, FRuntimeArchiveEntry& EntryInfo)
{
	if (!Super::GetArchiveEntryInfoByName(EntryName, EntryInfo))
	{
		return false;
	}

	FTarHeader Header;
	int32 EntryIndex;

	// Searching for a header by the entry name
	const bool bFound = TarEncapsulator->FindIf([&EntryName](const FTarHeader& PossibleHeader, int32 PossibleIndex)
	{
		return EntryName.Equals(StringCast<TCHAR>(PossibleHeader.GetName()).Get());
	}, Header, EntryIndex, true);

	if (!bFound)
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to find the entry index under the entry name '%s'"), *EntryName));
		return false;
	}

	if (!FTarHeader::ToEntry(Header, EntryIndex, EntryInfo))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Failed to convert tar header to entry with entry name '%s'"), *EntryName));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved tar entry '%s' by name"), *EntryInfo.Name);

	return true;
}

bool URuntimeArchiverTar::GetArchiveEntryInfoByIndex(int32 EntryIndex, FRuntimeArchiveEntry& EntryInfo)
{
	if (!Super::GetArchiveEntryInfoByIndex(EntryIndex, EntryInfo))
	{
		return false;
	}

	FTarHeader Header;

	// Searching for a header by the entry index
	const bool bFound = TarEncapsulator->FindIf([EntryIndex](const FTarHeader& PossibleHeader, int32 PossibleIndex)
	{
		return EntryIndex == PossibleIndex;
	}, Header, EntryIndex, true);

	if (!bFound)
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Unable to find tar header at index %d"), EntryIndex));
		return false;
	}

	if (!FTarHeader::ToEntry(Header, EntryIndex, EntryInfo))
	{
		ReportError(ERuntimeArchiverErrorCode::GetError, FString::Printf(TEXT("Failed to convert tar header to entry with entry index '%d'"), EntryIndex));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully retrieved tar entry '%s' by index"), *EntryInfo.Name);

	return true;
}

bool URuntimeArchiverTar::AddEntryFromMemory(FString EntryName, const TArray64<uint8>& DataToBeArchived, ERuntimeArchiverCompressionLevel CompressionLevel)
{
	if (!Super::AddEntryFromMemory(EntryName, DataToBeArchived, CompressionLevel))
	{
		return false;
	}

	// Adding directory entries separately as required by the tar specification
	{
		// Parsing directories from entry name
		TArray<FString> Directories{URuntimeArchiverUtilities::ParseDirectories(EntryName)};

		for (const FString& Directory : Directories)
		{
			FTarHeader Header;
			int32 Index;

			// Skip if the archive already contains this directory entry
			const bool bFound = TarEncapsulator->FindIf([&Directory](const FTarHeader& PotentialHeader, int32 Index)
			{
				return Directory.Equals(StringCast<TCHAR>(PotentialHeader.GetName()).Get());
			}, Header, Index, true);

			if (!bFound)
			{
				if (!FTarHeader::GenerateHeader(Directory, 0, FDateTime::Now(), true, Header))
				{
					ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to generate directory header for for entry '%s' to write from memory"), *Directory));
					return false;
				}

				// Directories only require writing a header, no data
				if (!TarEncapsulator->WriteHeader(Header))
				{
					ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to write header for entry '%s' from memory"), *Directory));
					return false;
				}
			}
		}
	}

	FTarHeader Header;
	if (!FTarHeader::GenerateHeader(EntryName, DataToBeArchived.Num(), FDateTime::Now(), false, Header))
	{
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to generate file header for for entry '%s' to write from memory"), *EntryName));
		return false;
	}

	// Archiving data
	{
		if (!TarEncapsulator->WriteHeader(Header))
		{
			ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to write header for entry '%s' from memory"), *EntryName));
			return false;
		}

		if (!TarEncapsulator->WriteData(DataToBeArchived))
		{
			ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to write data for entry '%s' from memory"), *EntryName));
			return false;
		}
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully added tar entry '%s' with size %lld bytes from memory"), *EntryName, DataToBeArchived.Num());

	return true;
}

bool URuntimeArchiverTar::ExtractEntryToMemory(const FRuntimeArchiveEntry& EntryInfo, TArray64<uint8>& UnarchivedData)
{
	if (!Super::ExtractEntryToMemory(EntryInfo, UnarchivedData))
	{
		return false;
	}

	int32 NumOfArchiveEntries;
	if (!GetArchiveEntries(NumOfArchiveEntries) || EntryInfo.Index > (NumOfArchiveEntries - 1))
	{
		ReportError(ERuntimeArchiverErrorCode::InvalidArgument, FString::Printf(TEXT("Tar entry index %d is invalid. Min index: 0, Max index: %d"), EntryInfo.Index, (NumOfArchiveEntries - 1)));
		return false;
	}

	FTarHeader Header;
	int32 Index;

	// Make sure we have such entry
	const bool bFound = TarEncapsulator->FindIf([&EntryInfo](const FTarHeader& PotentialHeader, int32 Index)
	{
		return EntryInfo.Name.Equals(StringCast<TCHAR>(PotentialHeader.GetName()).Get());
	}, Header, Index, false);

	if (!bFound)
	{
		ReportError(ERuntimeArchiverErrorCode::ExtractError, FString::Printf(TEXT("Unable to find tar entry '%s' to write into memory"), *EntryInfo.Name));
		return false;
	}

	UnarchivedData.SetNumUninitialized(Header.GetSize());

	if (!TarEncapsulator->ReadData(UnarchivedData))
	{
		ReportError(ERuntimeArchiverErrorCode::AddError, FString::Printf(TEXT("Unable to read data from tar entry '%s' to write into memory"), *EntryInfo.Name));
		UnarchivedData.Empty();
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully extracted tar entry '%s' into memory"), *EntryInfo.Name);

	return true;
}

bool URuntimeArchiverTar::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	TarEncapsulator.Reset(new FRuntimeArchiverTarEncapsulator);

	if (!TarEncapsulator)
	{
		ReportError(ERuntimeArchiverErrorCode::NotInitialized, TEXT("Unable to allocate memory for tar archiver"));
		return false;
	}

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully initialized tar archiver '%s'"), *GetName());

	return true;
}

bool URuntimeArchiverTar::IsInitialized() const
{
	return Super::IsInitialized() && TarEncapsulator.IsValid() && TarEncapsulator->IsValid();
}

void URuntimeArchiverTar::Reset()
{
	TarEncapsulator.Reset();

	Super::Reset();

	UE_LOG(LogRuntimeArchiver, Log, TEXT("Successfully uninitialized tar archiver '%s'"), *GetName());
}

void URuntimeArchiverTar::ReportError(ERuntimeArchiverErrorCode ErrorCode, const FString& ErrorString) const
{
	Super::ReportError(ErrorCode, ErrorString);
}

FRuntimeArchiverTarEncapsulator::FRuntimeArchiverTarEncapsulator()
	: RemainingDataSize{0}
  , LastHeaderPosition{0}
  , CachedNumOfHeaders{0}
  , bIsFinalized{false}
{
}

FRuntimeArchiverTarEncapsulator::~FRuntimeArchiverTarEncapsulator()
{
	if (Stream.IsValid())
	{
		if (Stream->IsWrite())
		{
			Finalize();
		}

		Stream.Reset();
	}
}

bool FRuntimeArchiverTarEncapsulator::IsValid()
{
	return Stream.IsValid() && Stream->IsValid();
}

bool FRuntimeArchiverTarEncapsulator::TestArchive()
{
	if (Stream->IsWrite())
	{
		return true;
	}

	static FTarHeader Header{};

	if (!Rewind())
	{
		return false;
	}

	return ReadHeader(Header);
}

bool FRuntimeArchiverTarEncapsulator::OpenFile(const FString& ArchivePath, bool bWrite)
{
	if (Stream.IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open tar stream because it has already been opened"));
		return false;
	}

	Stream.Reset(new FRuntimeArchiverFileStream(ArchivePath, bWrite));

	if (!Stream->IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open tar stream because it is not valid"));
		return false;
	}

	return TestArchive();
}

bool FRuntimeArchiverTarEncapsulator::OpenMemory(const TArray64<uint8>& ArchiveData, int32 InitialAllocationSize, bool bWrite)
{
	if (Stream.IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open tar stream because it has already been opened"));
		return false;
	}

	Stream.Reset(bWrite ? new FRuntimeArchiverMemoryStream(InitialAllocationSize) : new FRuntimeArchiverMemoryStream(ArchiveData));

	if (!IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to open tar stream because it is not valid"));
		return false;
	}

	return TestArchive();
}

bool FRuntimeArchiverTarEncapsulator::FindIf(TFunctionRef<bool(const FTarHeader&, int32)> ComparePredicate, FTarHeader& Header, int32& Index, bool bRemainPosition)
{
	if (!IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to find tar entry because stream is invalid"));
		return false;
	}

	const int64 PreviousPosition{Stream->Tell()};

	// Make sure looking from the start
	if (!Rewind())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to rewind read/write position of tar archive to find tar entry"));
		return false;
	}

	FTarHeader TempHeader;
	int32 TempIndex{0};

	bool bFound{false};

	// Iterate all files until we hit an error or find the header
	while (ReadHeader(TempHeader))
	{
		if (ComparePredicate(TempHeader, TempIndex))
		{
			Header = TempHeader;
			Index = TempIndex;

			bFound = true;
			break;
		}

		if (!Next())
		{
			break;
		}

		TempIndex++;
	}

	if (bRemainPosition)
	{
		Stream->Seek(PreviousPosition);
	}

	return bFound;
}

bool FRuntimeArchiverTarEncapsulator::GetArchiveEntries(int32& NumOfArchiveEntries)
{
	if (!IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get tar archive entries because stream is invalid"));
		return false;
	}

	// TODO: verify the validity of the last entry

	// Read-only mode is supposed to have a fixed (immutable) number of entries, so if possible, return the cached number of entries in this mode to improve performance
	if (!Stream->IsWrite() && CachedNumOfHeaders > 0)
	{
		NumOfArchiveEntries = CachedNumOfHeaders;
		return true;
	}

	const int64 PreviousPosition{Stream->Tell()};

	// Making sure looking from the start
	if (!Rewind())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to rewind read/write position of tar archive to get the number of archive entries"));
		return false;
	}

	FTarHeader TempHeader;
	int32 NumOfHeaders{0};

	// Iterate all files
	while (ReadHeader(TempHeader))
	{
		NumOfHeaders++;

		if (!Next())
		{
			break;
		}
	}

	Stream->Seek(PreviousPosition);

	// Cache number of entries in read-only mode to improve performance
	if (!Stream->IsWrite())
	{
		CachedNumOfHeaders = NumOfHeaders;
	}

	NumOfArchiveEntries = NumOfHeaders;
	return true;
}

bool FRuntimeArchiverTarEncapsulator::GetArchiveData(TArray64<uint8>& ArchiveData)
{
	if (!IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get tar archive data because stream is invalid"));
		return false;
	}

	if (Stream->IsWrite() && !Finalize())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get tar archive data because finalization failed"));
		return false;
	}

	ArchiveData.SetNumUninitialized(Stream->Size());

	const int64 PrevRemainingDataSize = RemainingDataSize;
	const int64 PrevLastHeaderPosition = LastHeaderPosition;
	const int64 PrevStreamPosition = Stream->Tell();

	Rewind();

	if (!Stream->Read(ArchiveData.GetData(), ArchiveData.Num()))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to read tar archive data with size '%lld'"), ArchiveData.Num());
		return false;
	}

	RemainingDataSize = PrevRemainingDataSize;
	LastHeaderPosition = PrevLastHeaderPosition;

	if (!Stream->Seek(PrevStreamPosition))
	{
		return false;
	}

	return true;
}

bool FRuntimeArchiverTarEncapsulator::Rewind()
{
	if (!IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to rewind read/write position of tar archive because stream is invalid"));
		return false;
	}

	RemainingDataSize = 0;
	LastHeaderPosition = 0;
	return Stream->Seek(0);
}

bool FRuntimeArchiverTarEncapsulator::Next()
{
	if (!IsValid())
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get next tar archive data because stream is invalid"));
		return false;
	}

	FTarHeader Header;
	if (!ReadHeader(Header))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to read header for getting next tar entry data"));
		return false;
	}

	const int64 NextEntryOffset = RuntimeArchiverTarOperations::RoundUp<int64>(Header.GetSize(), 512) + sizeof(FTarHeader);

	return Stream->Seek(Stream->Tell() + NextEntryOffset);
}

bool FRuntimeArchiverTarEncapsulator::ReadHeader(FTarHeader& Header)
{
	LastHeaderPosition = Stream->Tell();

	// Make sure that we will read valid data (the last 2 entries in the archive are not valid)
	{
		const int64 PotentialMaxPosition = (sizeof(FTarHeader) * 2) + Stream->Tell() + sizeof(Header);

		if (PotentialMaxPosition > Stream->Size())
		{
			return false;
		}
	}

	if (!Stream->Read(&Header, sizeof(Header)))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to read header from stream"));
		return false;
	}

	// Seek back to start of header
	if (!Stream->Seek(LastHeaderPosition))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to seek back to start of header"));
		return false;
	}

	return true;
}

bool FRuntimeArchiverTarEncapsulator::ReadData(TArray64<uint8>& Data)
{
	// If there is no remaining data, then this is the first reading. Getting the size, setting the remaining data and seeking to the beginning of the data
	if (RemainingDataSize == 0)
	{
		FTarHeader Header;
		if (!ReadHeader(Header))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to read header for getting tar entry data"));
			return false;
		}

		if (!Stream->Seek(Stream->Tell() + sizeof(FTarHeader)))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to seek to next header for getting tar entry data"));
			return false;
		}

		RemainingDataSize = Header.GetSize();
	}

	if (!Stream->Read(Data.GetData(), Data.Num()))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to read tar entry data"));
		return false;
	}

	RemainingDataSize -= Data.Num();

	// If there is no remaining data, then we have finished reading and seek back to the header
	if (RemainingDataSize == 0)
	{
		return Stream->Seek(LastHeaderPosition);
	}

	return true;
}

bool FRuntimeArchiverTarEncapsulator::WriteHeader(const FTarHeader& Header)
{
	RemainingDataSize = Header.GetSize();
	return Stream->Write(&Header, sizeof(Header));
}

bool FRuntimeArchiverTarEncapsulator::WriteData(const TArray64<uint8>& DataToBeArchived)
{
	if (!Stream->Write(DataToBeArchived.GetData(), DataToBeArchived.Num()))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to write tar entry data"));
		return false;
	}

	RemainingDataSize -= DataToBeArchived.Num();

	// Write padding if all data for this entry has already been written
	if (RemainingDataSize == 0)
	{
		const int64 CurrentPosition{Stream->Tell()};
		return WriteNullBytes(RuntimeArchiverTarOperations::RoundUp<int64>(CurrentPosition, 512) - CurrentPosition);
	}

	return true;
}

bool FRuntimeArchiverTarEncapsulator::WriteNullBytes(int64 NumOfBytes) const
{
	constexpr ANSICHAR NullCharacter = '\0';

	for (int64 Index = 0; Index < NumOfBytes; ++Index)
	{
		if (!Stream->Write(&NullCharacter, 1))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to write null bytes to tar archive"));
			return false;
		}
	}

	return true;
}

bool FRuntimeArchiverTarEncapsulator::Finalize()
{
	if (bIsFinalized)
	{
		return true;
	}

	bIsFinalized = true;

	return WriteNullBytes(sizeof(FTarHeader) * 2);
}
