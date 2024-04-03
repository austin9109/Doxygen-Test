#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Bank.h"
#include "ExchangeRate.h"
#include "MockClientProtocal.h"
#include "MockServerProtocal.h"
#include <type_traits>

using namespace std;

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::ReturnRefOfCopy;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
using ::testing::_;

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Transfer Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

class TransferTest : public testing::Test
{

protected:

	void SetUp() override
	{
		/// 
		/// Create mock client protocal
		/// 
		unique_ptr<CMockClientProtocal> m_pClientProtocal = make_unique<CMockClientProtocal>();

		// Protocal should be launched only once if operate correctly.
		EXPECT_CALL( *m_pClientProtocal, Launch( _ ) )
			.WillRepeatedly( Return( true ) );

		// Protocal should be closed only once if operate correctly.
		EXPECT_CALL( *m_pClientProtocal, Close() )
			.WillRepeatedly( Return( true ) );

		// Set mock behaviour for different send usuage.
		// This is for test "TransferToNonExistBank" and "TransferToNonExistBankAccount"
		EXPECT_CALL( *m_pClientProtocal, Send( _, _, _ ) )
			.WillRepeatedly(
				DoAll(
					testing::Invoke( [ this ]( std::string&& context, INT timeout /*sec*/, std::string& result ) {
							m_anotherBankCallback( move( context ), result );
							return true;
						} ) ) );

		/// 
		/// Create mock server protocal
		/// 
		unique_ptr<CMockServerProtocal> m_pServerProtocal = make_unique<CMockServerProtocal>();

		// Protocal should be launched only once if operate correctly.
		EXPECT_CALL( *m_pServerProtocal, Launch( _ ) )
			.WillOnce( Return( true ) );

		// Protocal should be closed only once if operate correctly.
		EXPECT_CALL( *m_pServerProtocal, Close() )
			.WillOnce( Return( true ) );

		// Set mock behaviour for different send usuage.
		// This is for test "TransferToNonExistBank" and "TransferToNonExistBankAccount"
		EXPECT_CALL( *m_pServerProtocal, RegistCB( _ ) )
			.WillOnce(
				DoAll(
					testing::Invoke( [this]( std::function<bool( std::string&&, std::string& )>&& callback ) {
						m_anotherBankCallback = move( callback );
						return true;
		} ) ) );


		// create bank instance
		m_pBank = make_unique<CBank>( EBankCode::EBC_Alishan, nullptr, nullptr, std::move( m_pClientProtocal ) );
		m_pAnotherBank = make_unique<CBank>( EBankCode::EBC_GreenIsland, nullptr, std::move( m_pServerProtocal ), nullptr );
		m_pAnotherBank->LaunchServer();

		// pre regist for this test
		m_Account = "ThisIsMyAccount1234";
		m_Anotherccount = "ThisIsAnotherAccount1234";
		m_Password = "ThisIsMyPassword5678";
		m_Deposit = 123;

		// pre regist account for Alishan bank
		m_pBank->Regist( m_Account, m_Password, to_string( m_Deposit ) );
		m_pBank->Regist( m_Anotherccount, m_Password, to_string( m_Deposit ) );

		// pre regist acount for GreenIsland bank
		m_pAnotherBank->Regist( m_Anotherccount, m_Password, to_string( m_Deposit ) );
	}

	void TearDown() override
	{
		m_pAnotherBank->CloseServer();
	}

	//! @brief 
	unique_ptr<CBank> m_pBank;
	//! @brief 
	unique_ptr<CBank> m_pAnotherBank;
	//! @brief 
	string m_Token;
	//! @brief pre-regist user's account
	string m_Account;
	//! @brief pre-regist user's account
	string m_Anotherccount;
	//! @brief pre-regist user's password
	string m_Password;
	//! @brief
	INT m_Deposit;
	//! @brief
	std::function<bool( std::string&&, std::string& )> m_anotherBankCallback;

};

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Transfer Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

TEST_F( TransferTest, TransferWithCorrectOperation )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transfer and check balance
	EXPECT_TRUE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, EBankCode::EBC_Alishan, m_Anotherccount, "100" ) );
}

TEST_F( TransferTest, TransferWithCorrectRemainBalance )
{
	// Login and save some USD in another account
	m_pBank->GetAccessToken( m_Anotherccount, m_Password, m_Token );
	m_pBank->Deposit( m_Token, ECurrencyCode::ECC_USD, "123" );
	m_pBank->ReturnAccessToken( m_Token );

	// login to my account
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transfer and check balance
	EXPECT_TRUE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, EBankCode::EBC_Alishan, m_Anotherccount, "100" ) );

	DOUBLE balance = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance );

	// check my account's balance is equal to regist money minus transfer amount
	EXPECT_NEAR( balance + 100.0, 123.0, std::numeric_limits<DOUBLE>::min() );
	EXPECT_TRUE( balance + 100.0 <= 123.0 );

	// check another user's account
	m_pBank->ReturnAccessToken( m_Token );
	m_pBank->GetAccessToken( m_Anotherccount, m_Password, m_Token );

	balance = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_USD, balance );

	// EXPECT total amount of current balance must smaller than transfer amount plus origin cash
	// Notice the DOUBLE numerical...
	EXPECT_TRUE( balance * Currency::Value::NTD <= ( 123.0 * Currency::Value::NTD ) + 100.0 );
}

TEST_F( TransferTest, TransferToSameBank )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transfer and check balance
	EXPECT_TRUE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, "23" ) );

	DOUBLE balance = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance );

	EXPECT_NEAR( balance + 23.0, 123.0, std::numeric_limits<DOUBLE>::min() );
}

TEST_F( TransferTest, TransferToDifferentBank )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transfer and check balance
	EXPECT_TRUE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_GreenIsland, m_Anotherccount, "23" ) );

	DOUBLE balance = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance );

	EXPECT_DOUBLE_EQ( balance + trunc( 23.0 * ( 1 + ChargeFee::Alishan ) ), 123.0 );

	// transfer half of remain money
	EXPECT_TRUE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, EBankCode::EBC_GreenIsland, m_Anotherccount, "50" ) );

	// logout and inquire
	string anotherToken;
	EXPECT_TRUE( m_pAnotherBank->GetAccessToken( m_Anotherccount, m_Password, anotherToken ) == ERetLogin::ERL_Valid );
	EXPECT_TRUE( m_pAnotherBank->Inquire( anotherToken, ECurrencyCode::ECC_USD, balance ) );

	// expect money transfer from NTD to USD should be equal
	EXPECT_NEAR( 50.0 / Currency::Value::NTD, balance, 1 );

}

TEST_F( TransferTest, TransferWithInvalidOperationChangeNothing )
{
	// login and attain currenty balance
	DOUBLE balanceIn = 0, balanceOut = 0;
	m_pBank->GetAccessToken( m_Anotherccount, m_Password, m_Token );
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceOut );
	m_pBank->ReturnAccessToken( m_Token );

	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceIn );

	// make an fault transfer
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, to_string( m_Deposit + 1 ) ) );

	// check if balance in each account is correct
	DOUBLE balanceInCheck = 0, balanceOutCheck = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceInCheck );
	m_pBank->ReturnAccessToken( m_Token );

	m_pBank->GetAccessToken( m_Anotherccount, m_Password, m_Token );
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceOutCheck );

	// check balance if modified
	EXPECT_DOUBLE_EQ( balanceIn, balanceInCheck );
	EXPECT_DOUBLE_EQ( balanceOut, balanceOutCheck );
	EXPECT_DOUBLE_EQ( balanceIn, m_Deposit );
	EXPECT_DOUBLE_EQ( balanceOut, m_Deposit );
}

TEST_F( TransferTest, TransferToNonExistBank )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_NoneExist, m_Anotherccount, "23" ) );
}

TEST_F( TransferTest, TransferToNonExistBankAccount )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transer to non-exist account
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, "ThisIsANonExistAccount", "23" ) );
}

TEST_F( TransferTest, TransferToSelf )
{
	// login into one of created account
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Account, "23" ) );
}

TEST_F( TransferTest, TransferWithIncorrectCurrency )
{
	// login into one of created account
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NODEFINE, EBankCode::EBC_Alishan, m_Anotherccount, "23" ) );
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NODEFINE, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, "35" ) );
}

TEST_F( TransferTest, TransferWithIncorrectFormat )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transer to account with invalid format
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, "-57" ) );
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, "#@!#@#" ) );
}

TEST_F( TransferTest, TransferWithZeroCash )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transer to account with 0 cash
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, "0" ) );
}

TEST_F( TransferTest, TransferWithExceedBalanceAmount )
{
	// Login
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transer to account with cash larger than amount
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, m_Anotherccount, "124" ) );
}

TEST_F( TransferTest, TransferWithExceedLimitCashValue )
{
	// regist an account with maximum deposit
	m_pBank->Regist( "ThisIsAnotherAccount5678", m_Password, to_string( std::numeric_limits<INT>::max() ) );

	// login into one of created account
	m_pBank->GetAccessToken( m_Account, m_Password, m_Token );

	// transer to account that has max balance amount
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, "ThisIsAnotherAccount5678", "1" ) );
}
