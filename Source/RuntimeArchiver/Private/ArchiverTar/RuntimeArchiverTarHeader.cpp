// Georgy Treshchev 2023.

#include "ArchiverTar/RuntimeArchiverTarHeader.h"

#include "RuntimeArchiverDefines.h"
#include "RuntimeArchiverTypes.h"
#include "ArchiverTar/RuntimeArchiverTarOperations.h"

/**
 * Helper that handles type flags
 */
class FTarTypeFlagHelper
{
	/** File type flags */
	static const ANSICHAR FileTypeFlag;
	static const ANSICHAR FileTypeFlag1;

	/** Directory type flag */
	static const ANSICHAR DirectoryTypeFlag;

	/** Unsupported type flags */
	static const ANSICHAR HardLinkTypeFlag;
	static const ANSICHAR SymbolicLinkTypeFlag;
	static const ANSICHAR CharacterDeviceTypeFlag;
	static const ANSICHAR BlockDeviceTypeFlag;
	static const ANSICHAR FIFOTypeFlag;

	/** Array of string representation of type flags */
	static const TMap<ANSICHAR, FString> Strings;

public:
	/**
	 * Get a string representation of the specified type flag
	 */
	static FString ToString(ANSICHAR TypeFlag)
	{
		FString const* String = Strings.Find(TypeFlag);

		if (!String)
		{
			return TEXT("Unrecognized type flag");
		}

		return *String;
	}

	/**
	 * Check if the specified type flag applies to a directory
	 */
	static bool IsDirectory(ANSICHAR TypeFlag)
	{
		if (TypeFlag == FileTypeFlag || TypeFlag == FileTypeFlag1)
		{
			return false;
		}

		if (TypeFlag == DirectoryTypeFlag)
		{
			return true;
		}

		UE_LOG(LogRuntimeArchiver, Error, TEXT("The type flag %hc (%s) is not supported. Supported type flags are %hc/%hc (%s) and %hc (%s)"),
		       TypeFlag, *ToString(TypeFlag),
		       FileTypeFlag, FileTypeFlag1, *ToString(FileTypeFlag),
		       DirectoryTypeFlag, *ToString(DirectoryTypeFlag));
		return false;
	}

	/**
	 * Get type flag
	 */
	static ANSICHAR GetTypeFlag(bool bIsDirectory)
	{
		return bIsDirectory ? DirectoryTypeFlag : FileTypeFlag;
	}
};

const ANSICHAR FTarTypeFlagHelper::FileTypeFlag{'0'};
const ANSICHAR FTarTypeFlagHelper::FileTypeFlag1{'\0'};
const ANSICHAR FTarTypeFlagHelper::DirectoryTypeFlag{'5'};
const ANSICHAR FTarTypeFlagHelper::HardLinkTypeFlag{'1'};
const ANSICHAR FTarTypeFlagHelper::SymbolicLinkTypeFlag{'2'};
const ANSICHAR FTarTypeFlagHelper::CharacterDeviceTypeFlag{'3'};
const ANSICHAR FTarTypeFlagHelper::BlockDeviceTypeFlag{'4'};
const ANSICHAR FTarTypeFlagHelper::FIFOTypeFlag{'6'};

const TMap<ANSICHAR, FString> FTarTypeFlagHelper::Strings{
	{FileTypeFlag, TEXT("File")}, {FileTypeFlag1, TEXT("File")},
	{DirectoryTypeFlag, TEXT("Directory")},
	{HardLinkTypeFlag, TEXT("Hard link")},
	{SymbolicLinkTypeFlag, TEXT("Symbolic link")},
	{CharacterDeviceTypeFlag, TEXT("Character device")},
	{BlockDeviceTypeFlag, TEXT("Block device")},
	{FIFOTypeFlag, TEXT("FIFO")}
};

/**
 * Helper that handles checksum
 */
class FTarChecksumHelper
{
public:
	/**
	 * Check if the specified tar header has a valid checksum
	 *
	 * @param Header Header to check
	 * @return Whether the checksum is valid or not
	 */
	static bool IsValid(const FTarHeader& Header)
	{
		if (*Header.Checksum == '\0')
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Checksum in tar header '%hs' is NULL"), Header.GetName());
			return false;
		}

		if (BuildChecksum(Header) != Header.GetChecksum())
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Checksum in tar header %hs' is invalid"), Header.GetName())
			return false;
		}

		return true;
	}

	/**
	 * Build a checksum based primarily on tar header offsets
	 *
	 * @param Header Tar header based on which to build the checksum
	 * @return Built checksum
	 */
	static uint32 BuildChecksum(const FTarHeader& Header)
	{
		const uint8* HeaderPtr = reinterpret_cast<const uint8*>(&Header);

		uint32 Checksum = 256;

		for (size_t Index = 0; Index < STRUCT_OFFSET(FTarHeader, Checksum); ++Index)
		{
			Checksum += HeaderPtr[Index];
		}

		for (size_t Index = STRUCT_OFFSET(FTarHeader, TypeFlag); Index < sizeof(Header); ++Index)
		{
			Checksum += HeaderPtr[Index];
		}

		return Checksum;
	}
};

FTarHeader::FTarHeader()
	: Name{}
  , Mode{}
  , Owner{}
  , Group{}
  , Size{}
  , Time{}
  , Checksum{}
  , TypeFlag(0)
  , LinkName{}
  , Padding{}
{
	RuntimeArchiverTarOperations::FillString(Mode,UE_ARRAY_COUNT(Mode), '0');
	RuntimeArchiverTarOperations::FillString(Owner,UE_ARRAY_COUNT(Owner), '0');
	RuntimeArchiverTarOperations::FillString(Group,UE_ARRAY_COUNT(Group), '0');
	RuntimeArchiverTarOperations::FillString(Size,UE_ARRAY_COUNT(Size), '0');
	RuntimeArchiverTarOperations::FillString(Time,UE_ARRAY_COUNT(Time), '0');
	Checksum[0] = '\0';
}

bool FTarHeader::ToEntry(const FTarHeader& Header, int32 Index, FRuntimeArchiveEntry& Entry)
{
	if (!FTarChecksumHelper::IsValid(Header))
	{
		return false;
	}

	Entry.Index = Index;
	Entry.Name = StringCast<TCHAR>(Header.GetName()).Get();
	Entry.bIsDirectory = FTarTypeFlagHelper::IsDirectory(Header.GetTypeFlag());
	Entry.UncompressedSize = Header.GetSize();
	Entry.CompressedSize = RuntimeArchiverTarOperations::RoundUp<int64>(Entry.UncompressedSize, 512);
	Entry.CreationTime = FDateTime::FromUnixTimestamp(Header.GetTime());

	return true;
}

bool FTarHeader::FromEntry(const FRuntimeArchiveEntry& Entry, FTarHeader& Header)
{
	return GenerateHeader(Entry.Name, Entry.UncompressedSize, Entry.CreationTime, Entry.bIsDirectory, Header);
}

bool FTarHeader::GenerateHeader(const FString& Name, int64 Size, const FDateTime& CreationTime, bool bIsDirectory, FTarHeader& Header)
{
	Header = {};

	// Setting all possible permissions for Unix
	Header.SetMode(0777);

	Header.SetSize(Size);
	Header.SetTime(CreationTime.ToUnixTimestamp());
	Header.SetTypeFlag(FTarTypeFlagHelper::GetTypeFlag(bIsDirectory));
	Header.SetName(StringCast<ANSICHAR>(*Name).Get());
	Header.SetChecksum(FTarChecksumHelper::BuildChecksum(Header));

	return true;
}

const ANSICHAR* FTarHeader::GetName() const
{
	return Name;
}

void FTarHeader::SetName(const ANSICHAR* InName)
{
	RuntimeArchiverTarOperations::StringCopy(Name, InName);
}

uint32 FTarHeader::GetMode() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<uint32>(Mode, UE_ARRAY_COUNT(Mode));
}

void FTarHeader::SetMode(uint32 InMode)
{
	RuntimeArchiverTarOperations::DecimalToOctal(InMode, Mode);

	RuntimeArchiverTarOperations::ResizeStringWithZeroBytes(Mode, 8);
}

uint32 FTarHeader::GetOwner() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<uint32>(Owner, UE_ARRAY_COUNT(Owner));
}

void FTarHeader::SetOwner(uint32 InOwner)
{
	RuntimeArchiverTarOperations::DecimalToOctal(InOwner, Owner);
}

const ANSICHAR* FTarHeader::GetGroup() const
{
	return Group;
}

void FTarHeader::SetGroup(const ANSICHAR* InGroup)
{
	RuntimeArchiverTarOperations::StringCopy(Group, InGroup);
}

int64 FTarHeader::GetSize() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<int64>(Size, UE_ARRAY_COUNT(Size));
}

void FTarHeader::SetSize(int64 InSize)
{
	RuntimeArchiverTarOperations::DecimalToOctal(InSize, Size);
	RuntimeArchiverTarOperations::ResizeStringWithZeroBytes(Size, 12);
}

int64 FTarHeader::GetTime() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<int64>(Time, UE_ARRAY_COUNT(Time));
}

void FTarHeader::SetTime(int64 InTime)
{
	RuntimeArchiverTarOperations::DecimalToOctal(InTime, Time);
}

uint32 FTarHeader::GetChecksum() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<uint32>(Checksum, UE_ARRAY_COUNT(Checksum));
}

void FTarHeader::SetChecksum(uint32 InChecksum)
{
	RuntimeArchiverTarOperations::DecimalToOctal(InChecksum, Checksum);

	RuntimeArchiverTarOperations::ResizeStringWithZeroBytes(Checksum, 7);
	Checksum[7] = ' ';
}

ANSICHAR FTarHeader::GetTypeFlag() const
{
	return TypeFlag;
}

void FTarHeader::SetTypeFlag(ANSICHAR InTypeFlag)
{
	TypeFlag = InTypeFlag;
}

const ANSICHAR* FTarHeader::GetLinkName() const
{
	return LinkName;
}

void FTarHeader::SetLinkName(const ANSICHAR* InLinkName)
{
	RuntimeArchiverTarOperations::StringCopy(LinkName, InLinkName);
}
