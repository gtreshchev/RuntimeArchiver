// Georgy Treshchev 2022.

#include "RuntimeArchiverUtilities.h"

#include "RuntimeArchiverDefines.h"

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

bool URuntimeArchiverUtilities::CompressRawData(FName FormatName, const TArray64<uint8>& UncompressedData, TArray64<uint8>& CompressedData)
{
	int32 CompressedSize = FCompression::CompressMemoryBound(FormatName, UncompressedData.Num());

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
