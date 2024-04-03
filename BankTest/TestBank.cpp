#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ExchangeRate.h"
#include "Bank.h"
#include "sstream"
#include "iomanip"

#define	TEST_DEFAULT_DEPOSIT_CASH	( 123 )

using namespace std;

using ::testing::AtLeast;
using ::testing::Return;

//! @brief Test include: inquire, deposit, withdraw, exchange
class BankTest : public testing::Test
{

protected:

	void SetUp() override
	{

		m_pBank = make_shared<CBank>( EBankCode::EBC_Alishan );		
	}

	//! @brief 
	shared_ptr<CBank> m_pBank;
	//! @brief 
	string m_Token;
	//! @brief
	INT m_PreDeposit = TEST_DEFAULT_DEPOSIT_CASH;
};

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Balance Checking
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

TEST_F( BankTest, InquireWithIncorrectCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NODEFINE, balance ) );
}

TEST_F( BankTest, InquireWithCorrectBalanceResult )
{
	// regist an account and login
	EXPECT_TRUE( m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) ) );
	EXPECT_TRUE( m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token ) == ERetLogin::ERL_Valid );

	DOUBLE balance = 0;
	EXPECT_TRUE( m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance ) );
	EXPECT_TRUE( balance > 0 );
	EXPECT_DOUBLE_EQ( balance, 123 );
}

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Deposit Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

TEST_F( BankTest, DepositWithBalanceCheck )
{
	// regist an account and login
	EXPECT_TRUE( m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) ) );
	EXPECT_TRUE( m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token ) == ERetLogin::ERL_Valid );

	// deposit money
	EXPECT_TRUE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, "456." ) );

	// check currency balance
	DOUBLE balance = 0;
	EXPECT_TRUE( m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance ) );

	EXPECT_DOUBLE_EQ( balance, 579 );
}

TEST_F( BankTest, DepositWithDifferentCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_TRUE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_USD, "123" ) );

	DOUBLE balance = 0;
	EXPECT_TRUE( m_pBank->Inquire( m_Token, ECurrencyCode::ECC_USD, balance ) );
	EXPECT_TRUE( balance > 0 );
	EXPECT_DOUBLE_EQ( balance, 123 );
}

TEST_F( BankTest, DepositWithIncorrectCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NODEFINE, "456" ) );
}

TEST_F( BankTest, DepositWithIncorrectFormat )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, "-45616123" ) );
	EXPECT_FALSE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, "123.sfds" ) );
}

TEST_F( BankTest, DepositWithZeroCash )
{
	// regist an account
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, "0" ) );
}

TEST_F( BankTest, DepositWithLimitCash )
{
	// regist an account
	EXPECT_TRUE( m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) ) );
	EXPECT_TRUE( m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token ) == ERetLogin::ERL_Valid );

	// deposit with maximum value
	int maxInt = std::numeric_limits<int>::max() - m_PreDeposit;
	EXPECT_TRUE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, to_string( maxInt ) ) );

	// check balance
	DOUBLE balance = 0;

	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance );

	EXPECT_TRUE( balance > 0 );
	EXPECT_DOUBLE_EQ( balance, ( DOUBLE )( maxInt + 123 ) );
}

TEST_F( BankTest, DepositWithExceedLimitCashValue )
{
	// regist an account
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_TRUE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, to_string( std::numeric_limits<int>::max() - m_PreDeposit ) ) );
	EXPECT_FALSE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, to_string( std::numeric_limits<int>::min() ) ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Withdraw Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

TEST_F( BankTest, WithdrawWithCorrectOperation )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	DOUBLE balance = 0;
	EXPECT_TRUE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, "123", balance ) );
	EXPECT_DOUBLE_EQ( balance, 0 );
}

TEST_F( BankTest, WithdrawWithBalanceCheck )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	// withdraw money
	DOUBLE withdraw = 0;
	m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, "59", withdraw );

	// check currency balance
	DOUBLE balance = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance );

	EXPECT_NEAR( balance, 64, std::numeric_limits<DOUBLE>::min() );
}

TEST_F( BankTest, WithdrawWithDifferentCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );
	m_pBank->Deposit( m_Token, ECurrencyCode::ECC_USD, "456" );

	DOUBLE balance = 0;
	EXPECT_TRUE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_USD, "456", balance ) );
	EXPECT_DOUBLE_EQ( balance, 0 );
}

TEST_F( BankTest, WithdrawWithIncorrectCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NODEFINE, "1", balance ) );
}

TEST_F( BankTest, WithdrawWithIncorrectFormat )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, "-45", balance ) );
	EXPECT_FALSE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, "12.sfds", balance ) );
}

TEST_F( BankTest, WithdrawWithZeroCash )
{
	// regist an account
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, "0", balance ) );
}

TEST_F( BankTest, WithdrawWithExceedBalanceAmount )
{
	// regist an account
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, "124", balance ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Change Currenct Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

TEST_F( BankTest, CurrencyExchangeWithCorrectOperation )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_TRUE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, "1" ) );
}

TEST_F( BankTest, CurrencyExchangeWithCorrectBalance )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	// exchange all money in NTD
	m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, to_string( m_PreDeposit ) );

	// check balance
	DOUBLE balanceUSD = 0;
	DOUBLE balanceNTD = 0;

	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_USD, balanceUSD );
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceNTD );

	// compare summerize of balance is equal to regist money
	EXPECT_DOUBLE_EQ( balanceNTD, 0 );
	// sum of two currency balance should smaller than total deposit money. ( there is exchange fee )
	EXPECT_TRUE( balanceNTD + Currency::Value::NTD * balanceUSD <= m_PreDeposit );
}

TEST_F( BankTest, CurrencyExchangeWithSameCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, "1" ) );
}

TEST_F( BankTest, CurrencyExchangeWithIncorrectCurrency )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NODEFINE, "1" ) );
}

TEST_F( BankTest, CurrencyExchangeWithIncorrectFormat )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, "-45" ) );
	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, "12.sfds" ) );
}

TEST_F( BankTest, CurrencyExchangeWithZeroCash )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, "0" ) );
}

TEST_F( BankTest, CurrencyExchangeWithExceedBalanceAmount )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( m_PreDeposit ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, "124" ) );
	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_USD, ECurrencyCode::ECC_NTD, "1" ) );
}

TEST_F( BankTest, CurrencyExchangeWithExceedLimitCashValue )
{
	// regist an account and login
	m_pBank->Regist( "ThisIsMyAccount1234", "ThisIsMyPassword5678", to_string( std::numeric_limits<int>::max() ) );
	m_pBank->GetAccessToken( "ThisIsMyAccount1234", "ThisIsMyPassword5678", m_Token );

	// deposit max amount to USD account
	EXPECT_TRUE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_USD, to_string( m_PreDeposit ) ) );
	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_USD, ECurrencyCode::ECC_NTD, "1" ) );
}
