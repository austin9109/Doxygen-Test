#include "string"
#include "vector"
#include "gtest/gtest.h"
#include "ValidInputChecker.h"

using namespace std;

class ValidInputCheckerTest : public testing::Test
{

protected:

	void SetUp() override
	{

	}
};


/// <summary>
/// This test is testing for legacy string checker checking for string that contains invalid characters.
/// </summary>
TEST_F( ValidInputCheckerTest, LegacyStringCheckWithInvalidCharacters )
{
	EXPECT_FALSE( CValidInputChecker::IsLegacyString( "!@#$%^&&*()_" ) );
}

/// <summary>
/// This test is testing for legacy string checker checking for string that only contains legacy characters.
/// </summary>
TEST_F( ValidInputCheckerTest, LegacyStringCheckWithOnlyLegacyCharacters )
{
	EXPECT_TRUE( CValidInputChecker::IsLegacyString( "abcdefghijklmnopqrstuvwxyz" ) );
	EXPECT_TRUE( CValidInputChecker::IsLegacyString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ) );
	EXPECT_TRUE( CValidInputChecker::IsLegacyString( "0123456789" ) );
	EXPECT_TRUE( CValidInputChecker::IsLegacyString( "A456SDF34adsfsdb87q5461POIsf" ) );
}

/// <summary>
/// This test is testing for numerical string checker checking for string that contains invalid characters.
/// </summary>
TEST_F( ValidInputCheckerTest, NumericalStringCheckWithIncorrectFormatString )
{
	EXPECT_FALSE( CValidInputChecker::IsNumercialString( "abcdefghijklmnopqrstuvwxyz" ) );
	EXPECT_FALSE( CValidInputChecker::IsNumercialString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ) );
	EXPECT_FALSE( CValidInputChecker::IsNumercialString( "!@#$%^&&*()_" ) );
	EXPECT_FALSE( CValidInputChecker::IsNumercialString( "12.245.87" ) );
}

/// <summary>
/// This test is testing for numerical string checker checking for string that contains correct format.
/// </summary>
TEST_F( ValidInputCheckerTest, NumericalStringCheckWithCorrectFormatString )
{
	EXPECT_TRUE( CValidInputChecker::IsNumercialString( "19.54" ) );
	EXPECT_TRUE( CValidInputChecker::IsNumercialString( ".5489" ) );
	EXPECT_TRUE( CValidInputChecker::IsNumercialString( "8089." ) );
	EXPECT_TRUE( CValidInputChecker::IsNumercialString( "00057" ) );
}