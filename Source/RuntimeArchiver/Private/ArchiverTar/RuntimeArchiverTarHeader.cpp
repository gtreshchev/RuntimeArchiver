// Georgy Treshchev 2024.

#include "ArchiverTar/RuntimeArchiverTarHeader.h"

#include "RuntimeArchiverDefines.h"
#include "RuntimeArchiverTypes.h"
#include "HAL/UnrealMemory.h"
#include "Containers/StringConv.h"
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

		UE_LOG(LogRuntimeArchiver, Error, TEXT("The type flag %c (%s) is not supported. Supported type flags are %c/%c (%s) and %c (%s)"),
		       TCHAR(TypeFlag), *ToString(TypeFlag),
		       TCHAR(FileTypeFlag), TCHAR(FileTypeFlag1), *ToString(FileTypeFlag),
		       TCHAR(DirectoryTypeFlag), *ToString(DirectoryTypeFlag));
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
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Checksum in tar header '%s' is NULL"), StringCast<TCHAR>(Header.GetName()).Get());
			return false;
		}

		if (BuildChecksum(Header) != Header.GetChecksum())
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Checksum in tar header %s' is invalid"), StringCast<TCHAR>(Header.GetName()).Get())
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
{
	FMemory::Memset(this, 0, sizeof(FTarHeader));
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
	Header = FTarHeader();

	if (!Header.SetName(StringCast<RA_UTF8CHAR>(*Name).Get()))
	{
		return false;
	}

	// Setting all possible permissions for Unix
	Header.SetMode(0777);

	Header.SetSize(Size);
	Header.SetTime(CreationTime.ToUnixTimestamp());
	Header.SetTypeFlag(FTarTypeFlagHelper::GetTypeFlag(bIsDirectory));
	Header.SetChecksum(FTarChecksumHelper::BuildChecksum(Header));

	return true;
}

const RA_UTF8CHAR* FTarHeader::GetName() const
{
	return Name;
}

bool FTarHeader::SetName(const RA_UTF8CHAR* InName)
{
	// Check if length is within bounds
	if (TCString<RA_UTF8CHAR>::Strlen(InName) > UE_ARRAY_COUNT(Name))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Name '%s' is too long for tar header. Maximum length is %d"), StringCast<TCHAR>(InName).Get(), static_cast<uint32>(UE_ARRAY_COUNT(Name)));
		return false;
	}
	TCString<RA_UTF8CHAR>::Strncpy(Name, InName, UE_ARRAY_COUNT(Name));
	return true;
}

uint32 FTarHeader::GetMode() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<uint32>(Mode);
}

void FTarHeader::SetMode(uint32 InMode)
{
	RuntimeArchiverTarOperations::DecimalToOctal<uint32>(InMode, Mode, UE_ARRAY_COUNT(Mode));
}

uint32 FTarHeader::GetOwner() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<uint32>(Owner);
}

void FTarHeader::SetOwner(uint32 InOwner)
{
	RuntimeArchiverTarOperations::DecimalToOctal<uint32>(InOwner, Owner, UE_ARRAY_COUNT(Owner));
}

const ANSICHAR* FTarHeader::GetGroup() const
{
	return Group;
}

bool FTarHeader::SetGroup(const ANSICHAR* InGroup)
{
	// Check if length is within bounds
	if (FCStringAnsi::Strlen(InGroup) > UE_ARRAY_COUNT(Group))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Group '%s' is too long for tar header. Maximum length is %d"), StringCast<TCHAR>(InGroup).Get(), static_cast<uint32>(UE_ARRAY_COUNT(Group)));
		return false;
	}
	FCStringAnsi::Strncpy(Group, InGroup, UE_ARRAY_COUNT(Group));
	return true;
}

int64 FTarHeader::GetSize() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<int64>(Size);
}

void FTarHeader::SetSize(int64 InSize)
{
	RuntimeArchiverTarOperations::DecimalToOctal<int64>(InSize, Size, UE_ARRAY_COUNT(Size));
}

int64 FTarHeader::GetTime() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<int64>(Time);
}

void FTarHeader::SetTime(int64 InTime)
{
	RuntimeArchiverTarOperations::DecimalToOctal<int64>(InTime, Time, UE_ARRAY_COUNT(Time));
}

uint32 FTarHeader::GetChecksum() const
{
	return RuntimeArchiverTarOperations::OctalToDecimal<uint32>(Checksum);
}

void FTarHeader::SetChecksum(uint32 InChecksum)
{
	RuntimeArchiverTarOperations::DecimalToOctal<uint32>(InChecksum, Checksum, UE_ARRAY_COUNT(Checksum));
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

bool FTarHeader::SetLinkName(const ANSICHAR* InLinkName)
{
	if (FCStringAnsi::Strlen(InLinkName) > UE_ARRAY_COUNT(LinkName))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Link name '%s' is too long for tar header. Maximum length is %d"), StringCast<TCHAR>(InLinkName).Get(), static_cast<uint32>(UE_ARRAY_COUNT(LinkName)));
		return false;
	}
	FCStringAnsi::Strncpy(LinkName, InLinkName, UE_ARRAY_COUNT(LinkName));
	return true;
}
