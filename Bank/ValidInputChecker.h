#pragma once

// standard library
#include "string"

// project library
#include "TypeDefine.h"

//! @brief Helper class for checking if string format is legal.
class CValidInputChecker
{

public:

	friend class CStringCoverter;

public:

	//! @brief Detect if string is in legal numerical format. E.g., with only numerical words and one float point.
	//! @param str 
	//! @return 
	static BOOl IsNumercialString( const std::string& str );

	//! @brief Detect if string is in legal format. E.g., with only alphabet and numerical words.
	//! @param str 
	//! @return 
	static BOOl IsLegacyString( const std::string& str );

private:

	//! @brief Detect if a character is alphabet
	//! @param c 
	//! @return 
	static BOOl IsAlphaChar( CHAR c );

	//! @brief Detect if a character is numerical
	//! @param c 
	//! @return 
	static BOOl IsNumercialChar( CHAR c );

	//! @brief Detect if a character is float point symbol.
	//! @param c 
	//! @return 
	static BOOl IsDecimalChar( CHAR c );

};