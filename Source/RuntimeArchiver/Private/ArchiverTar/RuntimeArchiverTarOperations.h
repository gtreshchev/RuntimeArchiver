// Georgy Treshchev 2023.

#pragma once

/**
 * Common functions that are used in tar operations
 */
namespace RuntimeArchiverTarOperations
{
	/**
	 * Get c-style string length
	 */
	FORCEINLINE uint32 StringLength(const ANSICHAR* String)
	{
		uint32 Length = 0;

		while (*String != '\0')
		{
			++Length;
			++String;
		}

		return Length;
	}

	/**
	 * Exchange the given characters. Works similar to std::swap
	 */
	FORCEINLINE void SwapChar(ANSICHAR& Char1, ANSICHAR& Char2)
	{
		ANSICHAR TempChar = MoveTemp(Char1);
		Char1 = MoveTemp(Char2);
		Char2 = MoveTemp(TempChar);
	}


	/**
	 * Reverse the specified string
	 */
	FORCEINLINE void ReverseString(ANSICHAR* String)
	{
		uint32 Start = 0;
		uint32 End = StringLength(String) - 1;

		while (Start < End)
		{
			SwapChar(*(String + Start), *(String + End));
			++Start;
			--End;
		}
	}

	/**
	 * Copy c-style string to destination. String sizes should be the same
	 */
	FORCEINLINE ANSICHAR* StringCopy(ANSICHAR* Destination, const ANSICHAR* Source)
	{
		if (Destination == nullptr)
		{
			return nullptr;
		}

		// Take a pointer pointing to the beginning of the destination string
		ANSICHAR* StartDestinationPtr = Destination;

		// Copy the string pointed by source into the array pointer by destination
		while (*Source != '\0')
		{
			*Destination = *Source;
			Destination++;
			Source++;
		}

		// Append the terminating null character
		*Destination = '\0';

		return StartDestinationPtr;
	}

	/**
	 * Resize the string
	 */
	FORCEINLINE bool ResizeStringWithZeroBytes(ANSICHAR* String, int32 RequiredSize)
	{
		int32 CurrentSize = StringLength(String) + 1;

		if (CurrentSize < RequiredSize)
		{
			while (CurrentSize != RequiredSize)
			{
				for (int32 i = RequiredSize - 2; i > 0; --i)
				{
					String[i] = String[i - 1];
				}

				String[0] = '0';
				++CurrentSize;
			}
		}

		String[RequiredSize - 1] = '\0';

		return true;
	}

	/**
	 * Fill the string with the specified value
	 */
	FORCEINLINE constexpr void FillString(ANSICHAR* String, int32 StringSize, ANSICHAR Value)
	{
		for (int32 Index = 0; Index < StringSize; ++Index)
		{
			String[Index] = Value;
		}

		String[StringSize - 1] = '\0';
	}

	/**
	 * Round a number up by the specified increment
	 */
	template <typename IntegralType>
	FORCEINLINE typename TEnableIf<TIsIntegral<IntegralType>::Value, IntegralType>::Type
	RoundUp(IntegralType Number, IntegralType Increment)
	{
		return Number + (Increment - Number % Increment) % Increment;
	}

	/**
	 * Convert string containing octal number to decimal number
	 */
	template <typename DecimalType>
	FORCEINLINE typename TEnableIf<TIsIntegral<DecimalType>::Value, DecimalType>::Type
	OctalToDecimal(const ANSICHAR* OctalString, uint32 OctalStringSize)
	{
		DecimalType Decimal = 0;
		uint32 Index = 0;

		while (Index < OctalStringSize && OctalString[Index])
		{
			Decimal = (Decimal << 3) | static_cast<DecimalType>(OctalString[Index++] - '0');
		}

		return Decimal;
	}

	/**
	 * Convert decimal number to string containing octal number
	 */
	template <typename DecimalType>
	FORCEINLINE void DecimalToOctal(DecimalType Decimal, ANSICHAR* Octal)
	{
		DecimalType Index = 0;

		// Handle 0 explicitly, otherwise empty string is printed for 0
		if (Decimal == 0)
		{
			Octal[Index++] = '0';
			Octal[Index] = '\0';
			return;
		}

		// Process individual digits
		while (Decimal != 0)
		{
			const DecimalType Rem = Decimal % 8;
			Octal[Index++] = (Rem > 9) ? (Rem - 10) + 'a' : Rem + '0';
			Decimal = Decimal / 8;
		}

		// Append the terminating null character
		Octal[Index] = '\0';

		// Reverse the string
		ReverseString(Octal);
	}
}
