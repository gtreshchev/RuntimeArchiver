#pragma once

/**
 * Base tar archive stream. Do not create it directly
 */
class FRuntimeArchiverBaseStream
{
protected:
	/**
	 * Base constructor
	 *
	 * @param bWrite Whether to have write permission or not
	 */
	explicit FRuntimeArchiverBaseStream(bool bWrite)
		: Position(0)
	  , bWrite(bWrite)
	{
	}

public:
	/** It should be impossible to create this class from outside */
	FRuntimeArchiverBaseStream() = delete;

	/** Virtual destructor to properly destroy child classes */
	virtual ~FRuntimeArchiverBaseStream() = default;

	/**
	 * Check if the the stream seems to be valid or not
	 * 
	 * @return Whether the stream is valid or not
	 */
	virtual bool IsValid() const;

	bool IsWrite() const;

	/**
	 * Get the current write or read position
	 */
	int64 Tell() const;

	/**
	 * Seek archived data at a specified position. In other words, change the current write or read position
	 *
	 * @param Position Position for seeking the archived data
	 */
	virtual bool Seek(int64 Position);

	/**
	 * Read archived data from the current position
	 *
	 * @param Data In-memory data pointer to fill
	 * @param Size Data size
	 * @return Whether the operation was successful or not
	 */
	virtual bool Read(void* Data, int64 Size);

	/**
	 * Write archived data from the current position
	 *
	 * @param Data In-memory data pointer to retrieve
	 * @param Size Data size
	 * @return Whether the operation was successful or not
	 */
	virtual bool Write(const void* Data, int64 Size);

	/**
	 * Get the total size
	 */
	virtual int64 Size();

protected:

	/** Current read or write position */
	int64 Position;

	/** Whether there is write permission or not */
	bool bWrite;
};
