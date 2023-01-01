// Georgy Treshchev 2023.

#pragma once

#include "Engine/EngineBaseTypes.h"

class FTarChecksumHelper;
struct FRuntimeArchiveEntry;

/**
 * Tar header used to denote archived data 
 */
struct FTarHeader
{
	friend FTarChecksumHelper;

	FTarHeader();

private:
	/** Entry name */
	ANSICHAR Name[100];

	/** Permission bits */
	ANSICHAR Mode[8];

	/** Owner user ID (UID) */
	ANSICHAR Owner[8];

	/** Group ID (GID) */
	ANSICHAR Group[8];

	/** Size in bytes */
	ANSICHAR Size[12];

	/** Time of last modification */
	ANSICHAR Time[12];

	/** Sum of all bytes in the header block */
	ANSICHAR Checksum[8];

	/** Entry type (e.g. directory or file) */
	ANSICHAR TypeFlag;

	/** Name of target file name. Not used, but denotes some rarely used entry types */
	ANSICHAR LinkName[100];

	/** Unused padding */
	ANSICHAR Padding[255];

	// TODO: Replace Padding with additional fields (Magic, Version, UName, GName, DevMajor, DevMinor, Prefix) for better compatibility

public:
	/**
	 * Convert a tar header to an archive entry
	 *
	 * @param Header Tar header to convert from
	 * @param Index Tar header index
	 * @param Entry Filled archive entry
	 * @return Whether the conversion was successful or not
	 */
	static bool ToEntry(const FTarHeader& Header, int32 Index, FRuntimeArchiveEntry& Entry);

	/**
	 * Convert an archive entry to a tar header
	 *
	 * @param Entry Archive entry to convert from
	 * @param Header Filled tar header
	 * @return Whether the conversion was successful or not
	 */
	static bool FromEntry(const FRuntimeArchiveEntry& Entry, FTarHeader& Header);

	/**
	 * Generate tar header based on input
	 *
	 * @param Name Entry name
	 * @param Size Data size
	 * @param CreationTime Entry creation time
	 * @param bIsDirectory Whether the entry is a directory or a file
	 * @param Header Filled tar header
	 * @return Whether the conversion was successful or not
	 */
	static bool GenerateHeader(const FString& Name, int64 Size, const FDateTime& CreationTime, bool bIsDirectory, FTarHeader& Header);

	//~ Writing and reading tar header data

	const ANSICHAR* GetName() const;
	void SetName(const ANSICHAR* InName);

	uint32 GetMode() const;
	void SetMode(uint32 InMode);

	uint32 GetOwner() const;
	void SetOwner(uint32 InOwner);

	const ANSICHAR* GetGroup() const;
	void SetGroup(const ANSICHAR* InGroup);

	int64 GetSize() const;
	void SetSize(int64 InSize);

	int64 GetTime() const;
	void SetTime(int64 InTime);

	uint32 GetChecksum() const;
	void SetChecksum(uint32 InChecksum);

	ANSICHAR GetTypeFlag() const;
	void SetTypeFlag(ANSICHAR InTypeFlag);

	const ANSICHAR* GetLinkName() const;
	void SetLinkName(const ANSICHAR* InLinkName);
};
