#include "Streams/RuntimeArchiverBaseStream.h"

bool FRuntimeArchiverBaseStream::IsValid() const
{
	return false;
}

bool FRuntimeArchiverBaseStream::IsWrite() const
{
	return bWrite;
}

bool FRuntimeArchiverBaseStream::Seek(int64 InPosition)
{
	return false;
}


int64 FRuntimeArchiverBaseStream::Tell() const
{
	return Position;
}

bool FRuntimeArchiverBaseStream::Read(void* Data, int64 Size)
{
	return false;
}

bool FRuntimeArchiverBaseStream::Write(const void* Data, int64 Size)
{
	return false;
}

int64 FRuntimeArchiverBaseStream::Size()
{
	return 0;
}
