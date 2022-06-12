#include "Streams/RuntimeArchiverMemoryStream.h"

FRuntimeArchiverMemoryStream::FRuntimeArchiverMemoryStream(const TArray64<uint8>& ArchiveData)
	: FRuntimeArchiverBaseStream(false)
  , ArchiveData(ArchiveData)
{
}

FRuntimeArchiverMemoryStream::FRuntimeArchiverMemoryStream(int32 InitialAllocationSize)
	: FRuntimeArchiverBaseStream(true)
{
	ArchiveData.SetNum(InitialAllocationSize);
}

bool FRuntimeArchiverMemoryStream::IsValid() const
{
	return true;
}

bool FRuntimeArchiverMemoryStream::Read(void* Data, int64 Size)
{
	if (!IsValid())
	{
		return false;
	}

	if (Position + Size > ArchiveData.Num())
	{
		return false;
	}

	const bool bSuccess{FMemory::Memcpy(Data, ArchiveData.GetData() + Position, Size) != nullptr};
	Position += Size;

	return bSuccess;
}

bool FRuntimeArchiverMemoryStream::Write(const void* Data, int64 Size)
{
	ensureMsgf(bWrite, TEXT("Cannot write data to the stream because it is in read-only mode"));

	if (!IsValid())
	{
		return false;
	}

	const int64 NewPosition = Position + Size;

	if (NewPosition > ArchiveData.Num())
	{
		ArchiveData.SetNumUninitialized(NewPosition);
	}

	const bool bSuccess{FMemory::Memcpy(ArchiveData.GetData() + Position, Data, Size) != nullptr};
	Position = NewPosition;

	return bSuccess;
}

bool FRuntimeArchiverMemoryStream::Seek(int64 NewPosition)
{
	if (!IsValid())
	{
		return false;
	}

	if (NewPosition == Position)
	{
		return true;
	}

	if (NewPosition > ArchiveData.Num())
	{
		if (NewPosition != 0)
		{
			return false;
		}
	}

	Position = NewPosition;

	return true;
}

int64 FRuntimeArchiverMemoryStream::Size()
{
	if (!IsValid())
	{
		return -1;
	}

	return ArchiveData.Num();
}
