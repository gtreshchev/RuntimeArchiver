// Georgy Treshchev 2024.

#pragma once

#include <string>
#include "Misc/CString.h"

/**
 * Common functions that are used in tar operations
 */
namespace RuntimeArchiverTarOperations
{

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
	 * This overload is for int32
	 */
	template <typename DecimalType, typename CharType>
	typename TEnableIf<TIsSame<DecimalType, int32>::Value, DecimalType>::Type
	OctalToDecimal(const CharType* OctalString)
	{
		return std::stoi(OctalString, 0, 8);
	}

	/**
	 * Convert string containing octal number to decimal number
	 * This overload is for int64
	 */
	template <typename DecimalType, typename CharType>
	typename TEnableIf<TIsSame<DecimalType, int64>::Value, DecimalType>::Type
	OctalToDecimal(const CharType* OctalString)
	{
		return std::stoll(OctalString, 0, 8);
	}

	/**
	 * Convert string containing octal number to decimal number
	 * This overload is for uint32
	 */
	template <typename DecimalType, typename CharType>
	typename TEnableIf<TIsSame<DecimalType, uint32>::Value, DecimalType>::Type
	OctalToDecimal(const CharType* OctalString)
	{
		return std::stoul(OctalString, 0, 8);
	}

	/**
	 * Convert decimal number to string containing octal number
	 * This overload is for int32
	 */
	template<typename DecimalType, typename CharType>
	typename TEnableIf<TIsSame<DecimalType, int64>::Value>::Type
	DecimalToOctal(DecimalType Decimal, CharType* Octal, int32 MaxLength)
	{
		TCString<CharType>::Sprintf(Octal, "%0*llo", MaxLength-1, Decimal);
	}

	/**
	 * Convert decimal number to string containing octal number
	 * This overload is for int32
	 */
	template<typename DecimalType, typename CharType>
	typename TEnableIf<TIsSame<DecimalType, uint32>::Value>::Type
	DecimalToOctal(DecimalType Decimal, CharType* Octal, int32 MaxLength)
	{
		TCString<CharType>::Sprintf(Octal, "%0*o", MaxLength-1, Decimal);
	}
}
