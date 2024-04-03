// standard header
#include<string> 

// project header
#include "OutDebug.h"
#include "ValidInputChecker.h"
#include "StringCoverter.h"

FLOAT CStringCoverter::String2Float( const std::string& str )
{
	// These method will produce some computation error when the decimal point reaches a high number of decimal places
	/*if( CValidInputChecker::IsNumercialString( str ) == false ) {
		return 0;
	}

	FLOAT result = 0;
	int nDecimal = 0;

	for( auto const& c : str ) {
		if( CValidInputChecker::IsDecimalChar( c ) == true ) {
			nDecimal = -1;
		}
		else {
			if( nDecimal >= 0 ) {
				result *= 10;
				result += c - '0';
			}
			else {
				result += (FLOAT)( c - '0' ) * powf( 10, (FLOAT)nDecimal );
				nDecimal--;
			}
		}
	}*/

	try {
		return std::stof( str );
	}
	catch( ... ) {
		OUTDEBUG( "Wrong format while parse numerical string. Return as -1." );
		return -1;
	}
}

DOUBLE CStringCoverter::String2Double( const std::string& str )
{
	try {
		DOUBLE value = std::stod( str );
		return std::stod( str );
	}
	catch( ... ) {
		OUTDEBUG( "Wrong format while parse numerical string. Return as -1." );
		return -1;
	}
}