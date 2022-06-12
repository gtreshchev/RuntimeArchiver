// Georgy Treshchev 2022.

#include "RuntimeArchiverUtilities.h"

#include "RuntimeArchiverDefines.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "Compression/OodleDataCompressionUtil.h"
#endif

TArray<FString> URuntimeArchiverUtilities::ParseDirectories(const FString& FilePath)
{
	TArray<FString> Directories;

	// All parts of directories on the left
	FString LeftDirectories;

	// Directory used to split
	FString DirectoryToSplit = FilePath;

	// Left side of split directory
	FString LeftDirectorySide;

	// Right side of split directory. It defines remaining directories
	FString RightDirectorySide;

	// Splitting the string by the forward slash (/) from left to right
	while (DirectoryToSplit.Split(TEXT("/"), &LeftDirectorySide, &RightDirectorySide, ESearchCase::IgnoreCase, ESearchDir::FromStart))
	{
		// Adding the left side of a directory and a forward slash at the end
		LeftDirectories += LeftDirectorySide + TEXT("/");

		// Adding the resulting string to the array
		Directories.Add(LeftDirectories);

		// Changing DirectoryToSplit to scan the right side of directories on the next loop
		DirectoryToSplit = RightDirectorySide;
	}

	return MoveTemp(Directories);
}

namespace
{
	/**
	 * Converting a raw format enum to a name used in the Unreal Engine compressor
	 */
	constexpr EName ToName(ERuntimeArchiverRawFormat Format)
	{
		switch (Format)
		{
#if ENGINE_MAJOR_VERSION >= 5
		case ERuntimeArchiverRawFormat::Oodle:
			{
				return NAME_Oodle;
			}
#endif
		case ERuntimeArchiverRawFormat::ZLib:
			{
				return NAME_Zlib;
			}
		case ERuntimeArchiverRawFormat::GZip:
			{
				return NAME_Gzip;
			}
		case ERuntimeArchiverRawFormat::LZ4:
			{
				return NAME_LZ4;
			}
		default:
			return NAME_None;
		}
	}

	/**
	 * Check if the specified format is valid
	 */
	bool IsFormatValid(const FName& FormatName)
	{
		// There is a bug in the engine that the LZ4 format cannot be found, which actually exists
		if (FormatName == NAME_LZ4)
		{
			return true;
		}

		if (!FCompression::IsFormatValid(FormatName))
		{
			return false;
		}

		return true;
	}
}

/**
 * Convert plugin-specific archiver data to Oodle compressor-specific data
 */
namespace OodleConversation
{
	FOodleDataCompression::ECompressor GetCompressor(ERuntimeArchiverCompressionLevel CompressionLevel)
	{
		switch (CompressionLevel)
		{
		case ERuntimeArchiverCompressionLevel::Compression0:
			return FOodleDataCompression::ECompressor::Selkie;
		case ERuntimeArchiverCompressionLevel::Compression1:
			return FOodleDataCompression::ECompressor::Selkie;
		case ERuntimeArchiverCompressionLevel::Compression2:
			return FOodleDataCompression::ECompressor::Mermaid;
		case ERuntimeArchiverCompressionLevel::Compression3:
			return FOodleDataCompression::ECompressor::Mermaid;
		case ERuntimeArchiverCompressionLevel::Compression4:
			return FOodleDataCompression::ECompressor::Mermaid;
		case ERuntimeArchiverCompressionLevel::Compression5:
			return FOodleDataCompression::ECompressor::Kraken;
		case ERuntimeArchiverCompressionLevel::Compression6:
			return FOodleDataCompression::ECompressor::Kraken;
		case ERuntimeArchiverCompressionLevel::Compression7:
			return FOodleDataCompression::ECompressor::Kraken;
		case ERuntimeArchiverCompressionLevel::Compression8:
			return FOodleDataCompression::ECompressor::Leviathan;
		case ERuntimeArchiverCompressionLevel::Compression9:
			return FOodleDataCompression::ECompressor::Leviathan;
		case ERuntimeArchiverCompressionLevel::Compression10:
			return FOodleDataCompression::ECompressor::Leviathan;
		default:
			return FOodleDataCompression::ECompressor::NotSet;
		}
	}

	FOodleDataCompression::ECompressionLevel GetCompressionLevel(ERuntimeArchiverCompressionLevel CompressionLevel)
	{
		switch (CompressionLevel)
		{
		case ERuntimeArchiverCompressionLevel::Compression0:
			return FOodleDataCompression::ECompressionLevel::HyperFast1;
		case ERuntimeArchiverCompressionLevel::Compression1:
			return FOodleDataCompression::ECompressionLevel::SuperFast;
		case ERuntimeArchiverCompressionLevel::Compression2:
			return FOodleDataCompression::ECompressionLevel::VeryFast;
		case ERuntimeArchiverCompressionLevel::Compression3:
			return FOodleDataCompression::ECompressionLevel::Fast;
		case ERuntimeArchiverCompressionLevel::Compression4:
			return FOodleDataCompression::ECompressionLevel::Normal;
		case ERuntimeArchiverCompressionLevel::Compression5:
			return FOodleDataCompression::ECompressionLevel::Fast;
		case ERuntimeArchiverCompressionLevel::Compression6:
			return FOodleDataCompression::ECompressionLevel::Normal;
		case ERuntimeArchiverCompressionLevel::Compression7:
			return FOodleDataCompression::ECompressionLevel::Optimal1;
		case ERuntimeArchiverCompressionLevel::Compression8:
			return FOodleDataCompression::ECompressionLevel::Optimal2;
		case ERuntimeArchiverCompressionLevel::Compression9:
			return FOodleDataCompression::ECompressionLevel::Optimal3;
		case ERuntimeArchiverCompressionLevel::Compression10:
			return FOodleDataCompression::ECompressionLevel::Optimal4;
		default:
			return FOodleDataCompression::ECompressionLevel::None;
		}
	}
}

bool URuntimeArchiverUtilities::CompressRawData(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, const TArray64<uint8>& UncompressedData, TArray64<uint8>& CompressedData)
{
	const FName& FormatName{ToName(RawFormat)};

	if (!IsFormatValid(FormatName))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("The specified format '%s' is not valid"), *FormatName.ToString());
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5
	if (FormatName.IsEqual(NAME_Oodle))
	{
		if (!FOodleCompressedArray::CompressTArray64(CompressedData, UncompressedData, OodleConversation::GetCompressor(CompressionLevel), OodleConversation::GetCompressionLevel(CompressionLevel)))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to compress data for '%s' format"), *FormatName.ToString());
			return false;
		}

		return true;
	}
#endif

	int32 CompressedSize = GuessCompressedSize(RawFormat, UncompressedData);

	if (CompressedSize <= 0)
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get compressed data size for '%s' format"), *FormatName.ToString());
		return false;
	}

	TArray64<uint8> TempCompressedData;

	TempCompressedData.SetNumUninitialized(CompressedSize);

	if (!FCompression::CompressMemory(FormatName, TempCompressedData.GetData(), CompressedSize, UncompressedData.GetData(), UncompressedData.Num()))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to compress data for '%s' format"), *FormatName.ToString());
		return false;
	}

	CompressedData = MoveTemp(TempCompressedData);
	CompressedData.SetNum(CompressedSize, false);

	return true;
}

bool URuntimeArchiverUtilities::UncompressRawData(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& CompressedData, TArray64<uint8>& UncompressedData)
{
	const FName& FormatName{ToName(RawFormat)};

	if (!IsFormatValid(FormatName))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("The specified format '%s' is not valid"), *FormatName.ToString());
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5
	if (FormatName.IsEqual(NAME_Oodle))
	{
		if (!FOodleCompressedArray::DecompressToTArray64(UncompressedData, CompressedData))
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to uncompress data for '%s' format"), *FormatName.ToString());
			return false;
		}

		return true;
	}
#endif

	const int64 UncompressedSize{GuessUncompressedSize(RawFormat, CompressedData)};

	if (UncompressedSize <= 0)
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to get compressed data size for '%s' format"), *FormatName.ToString());
		return false;
	}

	TArray64<uint8> TempUncompressedData;

	TempUncompressedData.SetNumUninitialized(UncompressedSize);

	if (!FCompression::UncompressMemory(FormatName, TempUncompressedData.GetData(), UncompressedSize, CompressedData.GetData(), CompressedData.Num()))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to uncompress data for '%s' format"), *FormatName.ToString());
		return false;
	}

	UncompressedData = MoveTemp(TempUncompressedData);
	UncompressedData.SetNum(UncompressedSize, false);

	return true;
}

int64 URuntimeArchiverUtilities::GuessCompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& UncompressedData)
{
	const FName& FormatName{ToName(RawFormat)};

	if (!IsFormatValid(FormatName))
	{
		UE_LOG(LogRuntimeArchiver, Error, TEXT("The specified format '%s' is not valid"), *FormatName.ToString());
		return false;
	}

#if ENGINE_MAJOR_VERSION >= 5
	return FCompression::GetMaximumCompressedSize(FormatName, UncompressedData.Num());
#else
	return FCompression::CompressMemoryBound(FormatName, UncompressedData.Num());
#endif
}

int64 URuntimeArchiverUtilities::GuessUncompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& CompressedData)
{
	if (CompressedData.Num() <= 0)
	{
		return 0;
	}

	switch (RawFormat)
	{
	case ERuntimeArchiverRawFormat::ZLib:
		{
			if (CompressedData.Num() < sizeof(uint64_t))
			{
				UE_LOG(LogRuntimeArchiver, Error, TEXT("The compressed buffer is ill-formed"));
				return false;
			}

			uint64_t UncompressedSize;
			FMemory::Memcpy(&UncompressedSize, CompressedData.GetData(), sizeof(uint64_t));

			return static_cast<int64>(UncompressedSize);
		}
	case ERuntimeArchiverRawFormat::GZip:
		{
			const uint8_t* DataPtr = CompressedData.GetData() + CompressedData.Num() - 4;

			return (static_cast<uint32_t>(DataPtr[0]) << 0) +
				(static_cast<uint32_t>(DataPtr[1]) << 8) +
				(static_cast<uint32_t>(DataPtr[2]) << 16) +
				(static_cast<uint32_t>(DataPtr[3]) << 24);
		}
	case ERuntimeArchiverRawFormat::LZ4:
		{
			return static_cast<int64>(CompressedData.Num() * 255);
		}
	default:
		{
			UE_LOG(LogRuntimeArchiver, Error, TEXT("The compressed buffer is ill-formed"));
		}
	}

	return 0;
}
