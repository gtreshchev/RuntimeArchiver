#include "Streams/RuntimeArchiverFileStream.h"

#include "RuntimeArchiverDefines.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFileManager.h"

FRuntimeArchiverFileStream::FRuntimeArchiverFileStream(const FString& ArchivePath, bool bWrite)
	: FRuntimeArchiverBaseStream(bWrite)
{
	IPlatformFile& PlatformFile{FPlatformFileManager::Get().GetPlatformFile()};
	FileHandle = bWrite ? PlatformFile.OpenWrite(*ArchivePath, false, true) : PlatformFile.OpenRead(*ArchivePath, false);

	UE_LOG(LogRuntimeArchiver, Log, TEXT("File opened at '%s', bWrite: %s. Validity: %s"),
	       *ArchivePath, bWrite ? TEXT("true") : TEXT("false"), FRuntimeArchiverFileStream::IsValid() ? TEXT("true") : TEXT("false"));
}

FRuntimeArchiverFileStream::~FRuntimeArchiverFileStream()
{
	if (FRuntimeArchiverFileStream::IsValid())
	{
		delete FileHandle;
		FileHandle = nullptr;
	}
}

bool FRuntimeArchiverFileStream::IsValid() const
{
	return FileHandle != nullptr;
}

bool FRuntimeArchiverFileStream::Read(void* Data, int64 Size)
{
	if (!IsValid())
	{
		return false;
	}

	const bool bSuccess{FileHandle->Read(static_cast<uint8*>(Data), Size)};
	Position = FileHandle->Tell();
	return bSuccess;
}

bool FRuntimeArchiverFileStream::Write(const void* Data, int64 Size)
{
	ensureMsgf(bWrite, TEXT("Cannot write data to the stream because it is in read-only mode"));

	if (!IsValid())
	{
		return false;
	}

	const bool bSuccess{FileHandle->Write(static_cast<const uint8*>(Data), Size)};
	Position = FileHandle->Tell();
	return bSuccess;
}

bool FRuntimeArchiverFileStream::Seek(int64 NewPosition)
{
	if (!IsValid())
	{
		return false;
	}

	const bool bSuccess{FileHandle->Seek(NewPosition)};
	Position = FileHandle->Tell();
	return bSuccess;
}

int64 FRuntimeArchiverFileStream::Size()
{
	if (!IsValid())
	{
		return -1;
	}

	return FileHandle->Size();
}
