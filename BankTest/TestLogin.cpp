#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Bank.h"
#include "MockDataSerializer.h"
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
/// Login Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

class LoginTest : public testing::Test
{

protected:

	void SetUp() override
	{
		/// 
		/// Create mock m_pSerializer
		/// 
		m_pDataSerializer = make_unique<CMockDataSerializer>();

		// For this test we do login multiple times,
		// and we expect that even user login multiple times, bank will
		// only load user's data when constructed
		EXPECT_CALL( *m_pDataSerializer, Deserialize( _, _ ) )
			.WillRepeatedly(
				DoAll(	// Below 2 method has same effect:
						// Method1:
						testing::Invoke( [this]( const std::string& filePath, std::vector<TBankAccount>& bankAccounts ) {
							bankAccounts = m_mockBankAccounts;
						} ),
						// Method2:
						//SetArgReferee<1>( m_mockBankAccounts ),
						Return( true ) ) );

		// For this test we don't logout with multiple times,
		// and we expect that bank object only serialize user's data
		// when bank is destroyed.
		EXPECT_CALL( *m_pDataSerializer, Serialize( _, _ ) )
			.Times( 1 )
			.WillRepeatedly(
				DoAll(	SaveArg<1>( &m_mockBankAccounts ), // keep user's data in mock accounts
						Return( true ) ) );

		// create bank instance
		m_pBank = make_unique<CBank>( EBankCode::EBC_Alishan, std::move( m_pDataSerializer ) );

		/// 
		/// pre regist for this test
		/// 
		m_Account = "ThisIsMyAccount1234";
		m_Password = "ThisIsMyPassword5678";
		m_Deposit = "123.";

		m_pBank->Regist( m_Account, m_Password, m_Deposit );
	}

	void Disconnect()
	{
		printf( "Disconnect...\n" );
		m_pBank.reset();
	}

	void Reconnect()
	{
		printf( "Reconnect...\n" );
		m_pDataSerializer = make_unique<CMockDataSerializer>();

		// For this test we do login multiple times,
		// and we expect that even user login multiple times, bank will
		// only load user's data when constructed
		EXPECT_CALL( *m_pDataSerializer, Deserialize( _, _ ) )
			.WillRepeatedly(
				DoAll(	SetArgReferee<1>( m_mockBankAccounts ),
						Return( true ) ) );

		// For this test we don't logout with multiple times,
		// and we expect that bank object only serialize user's data
		// when bank is destroyed.
		EXPECT_CALL( *m_pDataSerializer, Serialize( _, _ ) )
			.Times( 1 )
			.WillRepeatedly(
				DoAll(	SaveArg<1>( &m_mockBankAccounts ), // keep user's data in mock accounts
						Return( true ) ) );

		// Here if not use method2 when constructed mock behaviour "GetBankService", when we call "GetBankService", it
		// will still give us the origin bank object, which will not go through Deserialize stage.
		m_pBank = make_unique<CBank>( EBankCode::EBC_Alishan, std::move( m_pDataSerializer ) );
	}

	//! @brief 
	unique_ptr<CBank> m_pBank;
	//! @brief 
	string m_Token;
	//! @brief pre-regist user's account
	string m_Account;
	//! @brief pre-regist user's password
	string m_Password;
	//! @brief pre-regist user's balance
	string m_Deposit;

private:

	unique_ptr<CMockDataSerializer> m_pDataSerializer;
	vector<TBankAccount> m_mockBankAccounts;

};

TEST_F( LoginTest, LoginWithNonExistAccount )
{
	EXPECT_TRUE( m_pBank->GetAccessToken( "ThisIsAnotherAccount", "ThisIsMyPassword5678", m_Token ) == ERetLogin::ERL_NoAccount );
}

TEST_F( LoginTest, LoginWithIncorrectAccountPassowordCombination )
{
	EXPECT_TRUE( m_pBank->GetAccessToken( m_Account, "ThisIsWrongPassword5678", m_Token ) == ERetLogin::ERL_WrongPassword );
}

TEST_F( LoginTest, LoginWithCorrectPassword )
{
	EXPECT_TRUE( m_pBank->GetAccessToken( m_Account, m_Password, m_Token ) == ERetLogin::ERL_Valid );
}

TEST_F( LoginTest, LoginWithMultipleSuccessTry )
{
	// first time login
	EXPECT_TRUE( m_pBank->GetAccessToken( m_Account, m_Password, m_Token ) == ERetLogin::ERL_Valid );
	// login again
	EXPECT_TRUE( m_pBank->GetAccessToken( m_Account, m_Password, m_Token ) == ERetLogin::ERL_AlreadyLogin );
}

TEST_F( LoginTest, DoInquireWithoutLogin )
{
	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balance ) );
}

TEST_F( LoginTest, DoDepositWithoutLogin )
{
	EXPECT_FALSE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, "123" ) );
}

TEST_F( LoginTest, DoWithdrawWithoutLogin )
{
	// try to withdraw without login
	DOUBLE balance = 0;
	EXPECT_FALSE( m_pBank->Withdraw( m_Token, ECurrencyCode::ECC_NTD, m_Deposit, balance ) );
}

TEST_F( LoginTest, DoCurrencyExchangeWithoutLogin )
{
	EXPECT_FALSE( m_pBank->Exchange( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_USD, "1" ) );
}

TEST_F( LoginTest, DoTransferWithoutLogin )
{
	// register another different account
	EXPECT_TRUE( m_pBank->Regist( "ThisIsAnotherAccount1234", "ThisIsPassword5678", "456" ) );

	// directly transer without login
	EXPECT_FALSE( m_pBank->Transfer( m_Token, ECurrencyCode::ECC_NTD, ECurrencyCode::ECC_NTD, EBankCode::EBC_Alishan, "ThisIsAnotherAccount1234", "23" ) );
}

TEST_F( LoginTest, LogoutWithSuccessfullySaveCurrentInformation )
{
	// Login First
	EXPECT_TRUE( m_pBank->GetAccessToken( m_Account, m_Password, m_Token ) == ERetLogin::ERL_Valid );

	// Deposit Money
	EXPECT_TRUE( m_pBank->Deposit( m_Token, ECurrencyCode::ECC_NTD, "1" ) );

	// check balance before logout
	DOUBLE balanceBeforeLogout = 0;
	m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceBeforeLogout );

	// logout, disconnect and reconnect
	m_pBank->ReturnAccessToken( m_Token );
	Disconnect();
	Reconnect();

	// check balance after login, and compare
	DOUBLE balanceAfterLoginAgain = 0;
	EXPECT_TRUE( m_pBank->GetAccessToken( m_Account, m_Password, m_Token) == ERetLogin::ERL_Valid );
	EXPECT_TRUE( m_pBank->Inquire( m_Token, ECurrencyCode::ECC_NTD, balanceAfterLoginAgain ) );
	EXPECT_DOUBLE_EQ( balanceAfterLoginAgain, balanceBeforeLogout );
	EXPECT_DOUBLE_EQ( balanceAfterLoginAgain, 124 );
}