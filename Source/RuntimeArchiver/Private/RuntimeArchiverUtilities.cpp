// Georgy Treshchev 2023.

#include "RuntimeArchiverUtilities.h"

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