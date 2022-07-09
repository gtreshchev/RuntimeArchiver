#pragma once
#include "RuntimeArchiverBaseStream.h"

/**
 * File tar stream. Manages data at the file system level
 */
class RUNTIMEARCHIVER_API FRuntimeArchiverFileStream : public FRuntimeArchiverBaseStream
{
public:
	/** It should be impossible to create this object by the default constructor */
	FRuntimeArchiverFileStream() = delete;

	/**
	 * Open a tar archive file stream
	 *
	 * @param ArchivePath Path to open an archive
	 * @param bWrite Whether to open for writing or not
	 */
	explicit FRuntimeArchiverFileStream(const FString& ArchivePath, bool bWrite);

	virtual ~FRuntimeArchiverFileStream() override;

	//~ Begin FRuntimeArchiverBaseStream Interface
	virtual bool IsValid() const override;
	virtual bool Read(void* Data, int64 Size) override;
	virtual bool Write(const void* Data, int64 Size) override;
	virtual bool Seek(int64 NewPosition) override;
	virtual int64 Size() override;
	//~ End FRuntimeArchiverBaseStream Interface

private:
	/** The file handle used to read or write */
	IFileHandle* FileHandle;
};
