#pragma once
#include "RuntimeArchiverBaseStream.h"

/**
 * Memory tar stream. Manages data at the memory level
 */
class RUNTIMEARCHIVER_API FRuntimeArchiverMemoryStream : public FRuntimeArchiverBaseStream
{
public:
	/** It should be impossible to create this object by the default constructor */
	FRuntimeArchiverMemoryStream() = delete;
	
	virtual ~FRuntimeArchiverMemoryStream() override = default;
	
	/**
	 * Read-only constructor
	 *
	 * @param ArchiveData Binary archive data
	 */
	explicit FRuntimeArchiverMemoryStream(const TArray64<uint8>& ArchiveData);

	/**
	 * Write constructor
	 *
	 * @param InitialAllocationSize Estimated archive size if known. Avoids unnecessary memory allocation
	 */
	explicit FRuntimeArchiverMemoryStream(int32 InitialAllocationSize);

	//~ Begin FArchiverTarBaseStream Interface
	virtual bool IsValid() const override;
	virtual bool Read(void* Data, int64 Size) override;
	virtual bool Write(const void* Data, int64 Size) override;
	virtual bool Seek(int64 NewPosition) override;
	virtual int64 Size() override;
	//~ End FArchiverTarBaseStream Interface

protected:
	/** Binary archive data */
	TArray64<uint8> ArchiveData;
};
