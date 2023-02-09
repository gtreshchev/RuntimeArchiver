// Georgy Treshchev 2023.

#include "ArchiverRaw/RuntimeArchiverRaw.h"
#include "RuntimeArchiverDefines.h"
#include "Async/Async.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Compression/OodleDataCompressionUtil.h"
#endif

namespace
{
	/**
	 * Converting a raw format enum to a name used in the Unreal Engine compressor
	 */
	EName ToName(ERuntimeArchiverRawFormat Format)
	{
		switch (Format)
		{
		case ERuntimeArchiverRawFormat::Oodle:
			{
#if ENGINE_MAJOR_VERSION >= 5
				return NAME_Oodle;
#else
				UE_LOG(LogRuntimeArchiver, Error, TEXT("Oodle format is not supported in %s %d"), TEXT(EPIC_PRODUCT_NAME), ENGINE_MAJOR_VERSION);
				return NAME_None;
#endif
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
		// There is a bug in the engine where the LZ4 format cannot be found, even though it exists
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

#if ENGINE_MAJOR_VERSION >= 5
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
#endif

void URuntimeArchiverRaw::CompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, TArray<uint8> UncompressedData, const FRuntimeArchiverRawMemoryResult& OnResult)
{
	CompressRawDataAsync(RawFormat, CompressionLevel, TArray64<uint8>(MoveTemp(UncompressedData)),
	                     FRuntimeArchiverRawMemoryResultNative::CreateLambda([OnResult](TArray64<uint8> CompressedData64)
	                     {
		                     if (CompressedData64.Num() > TNumericLimits<TArray<uint8>::SizeType>::Max())
		                     {
			                     UE_LOG(LogRuntimeArchiver, Error, TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), TNumericLimits<TArray<uint8>::SizeType>::Max(), CompressedData64.Num());
			                     OnResult.ExecuteIfBound(TArray<uint8>());
			                     return;
		                     }
		                     OnResult.ExecuteIfBound(TArray<uint8>(MoveTemp(CompressedData64)));
	                     }));
}

void URuntimeArchiverRaw::CompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, TArray64<uint8> UncompressedData, const FRuntimeArchiverRawMemoryResultNative& OnResult)
{
	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [RawFormat, CompressionLevel, UncompressedData = MoveTemp(UncompressedData), OnResult]() mutable
	{
		TArray64<uint8> CompressedData;
		CompressRawData(RawFormat, CompressionLevel, MoveTemp(UncompressedData), CompressedData);

		AsyncTask(ENamedThreads::GameThread, [OnResult, CompressedData = MoveTemp(CompressedData)]() mutable
		{
			OnResult.ExecuteIfBound(MoveTemp(CompressedData));
		});
	});
}

bool URuntimeArchiverRaw::CompressRawData(ERuntimeArchiverRawFormat RawFormat, ERuntimeArchiverCompressionLevel CompressionLevel, const TArray64<uint8>& UncompressedData, TArray64<uint8>& CompressedData)
{
	const FName FormatName{ToName(RawFormat)};
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

	TempCompressedData.SetNum(CompressedSize, true);
	CompressedData = MoveTemp(TempCompressedData);
	return true;
}

void URuntimeArchiverRaw::UncompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, TArray<uint8> CompressedData, const FRuntimeArchiverRawMemoryResult& OnResult)
{
	UncompressRawDataAsync(RawFormat, TArray64<uint8>(MoveTemp(CompressedData)),
	                       FRuntimeArchiverRawMemoryResultNative::CreateLambda([OnResult](TArray64<uint8> UncompressedData64)
	                       {
		                       if (UncompressedData64.Num() > TNumericLimits<TArray<uint8>::SizeType>::Max())
		                       {
			                       UE_LOG(LogRuntimeArchiver, Error, TEXT("Array with int32 size (max length: %d) cannot fit int64 size data (retrieved length: %lld)\nA standard byte array can hold a maximum of 2 GB of data"), TNumericLimits<TArray<uint8>::SizeType>::Max(), UncompressedData64.Num());
			                       OnResult.ExecuteIfBound(TArray<uint8>());
			                       return;
		                       }
		                       OnResult.ExecuteIfBound(TArray<uint8>(MoveTemp(UncompressedData64)));
	                       }));
}

void URuntimeArchiverRaw::UncompressRawDataAsync(ERuntimeArchiverRawFormat RawFormat, TArray64<uint8> CompressedData, const FRuntimeArchiverRawMemoryResultNative& OnResult)
{
	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [RawFormat, CompressedData = MoveTemp(CompressedData), OnResult]() mutable
	{
		TArray64<uint8> UncompressedData;
		UncompressRawData(RawFormat, CompressedData = MoveTemp(CompressedData), UncompressedData);

		AsyncTask(ENamedThreads::GameThread, [OnResult, UncompressedData = MoveTemp(UncompressedData)]() mutable
		{
			OnResult.ExecuteIfBound(MoveTemp(UncompressedData));
		});
	});
}

bool URuntimeArchiverRaw::UncompressRawData(ERuntimeArchiverRawFormat RawFormat, TArray64<uint8> CompressedData, TArray64<uint8>& UncompressedData)
{
	const FName FormatName{ToName(RawFormat)};

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
	return true;
}

int64 URuntimeArchiverRaw::GuessCompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& UncompressedData)
{
	const FName FormatName{ToName(RawFormat)};
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

int64 URuntimeArchiverRaw::GuessUncompressedSize(ERuntimeArchiverRawFormat RawFormat, const TArray64<uint8>& CompressedData)
{
	if (CompressedData.Num() <= 0)
	{
		return 0;
	}

	switch (RawFormat)
	{
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
			UE_LOG(LogRuntimeArchiver, Error, TEXT("Unable to determine the uncompressed size of the format '%s'"), *UEnum::GetValueAsString(RawFormat));
		}
	}

	return 0;
}
