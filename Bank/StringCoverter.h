#pragma once

// standard library
#include "string"

// project library
#include "TypeDefine.h"

//! @brief Helper class for converting string into sub-element.
class CStringCoverter
{

public:

	//! @brief Convert string into numerical.
	//! @param str 
	//! @return 
	static FLOAT String2Float( const std::string& str );

	//! @brief Convert string into numerical.
	//! @param str 
	//! @return 
	static DOUBLE String2Double( const std::string& str );
};