#include "ValidInputChecker.h"

using namespace std;

/// <summary>
/// Judge if string is int correct numerical format.
/// For example: 19.5, 2.33, 1.
/// 
/// Invalid example: .5514, 068.551
/// </summary>
BOOl CValidInputChecker::IsNumercialString( const string& str )
{
	BOOl bFloatPoint = false;
	for( auto const& c : str ) {
		if( IsDecimalChar( c ) == true && bFloatPoint == false ) {
			bFloatPoint = true;
		}
		else if( IsDecimalChar( c ) == true && bFloatPoint == true ) {
			return false;
		}
		else if( IsNumercialChar( c ) == false ) {
			return false;
		}
	}

	return true;
}

/// <summary>
/// Judge if string that only contains alphabet or numerical character.
/// </summary>
BOOl CValidInputChecker::IsLegacyString( const string& str )
{
	for( auto const& c : str ) {
		if( IsAlphaChar( c ) == false && IsNumercialChar( c ) == false ) {
			return false;
		}
	}

	return true;
}

/// <summary>
/// Judge if character is alphabet.
/// </summary>
BOOl CValidInputChecker::IsAlphaChar( CHAR c )
{
	return ( c >= 65 && c <= 90 ) || ( c >= 97 && c <= 122 ); // ascii: A (65) ~ Z (90), a (97) ~ z (122)
}

/// <summary>
/// Judge if character is numerical.
/// </summary>
BOOl CValidInputChecker::IsNumercialChar( CHAR c )
{
	return c >= 48 && c <= 57; // ascii: 0 (48) ~ 9 (57)
}

/// <summary>
/// Judge if character is decimal point.
/// </summary>
BOOl CValidInputChecker::IsDecimalChar( CHAR c )
{
	return c == 46; // ascii: . (46)
}