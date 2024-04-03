#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Bank.h"

using namespace std;

using ::testing::AtLeast;
using ::testing::Return;

///////////////////////////////////////////////////////////////////////////////////////////////
///
/// Regist Test
/// 
///////////////////////////////////////////////////////////////////////////////////////////////

class RegistTest : public testing::Test
{

protected:

	void SetUp() override
	{
		m_pBank = make_shared<CBank>( EBankCode::EBC_Alishan );
	}

	//! @brief 
	shared_ptr<CBank> m_pBank;
	//! @brief user's registering account
	string m_Account = "ThisIsMyAccount1234";
	//! @brief user's registering password
	string m_Password = "ThisIsMyPassword5678";

};

TEST_F( RegistTest, RegistWithInvalidCharacters )
{
	EXPECT_FALSE( m_pBank->Regist( "ThisIsCorrectAccount1234", "ThisIsInvalidPassword*&#@!", "1." ) );
	EXPECT_FALSE( m_pBank->Regist( "這不是正確帳號1234", "ThisIsCorrectPassword123", "1." ) );
}

TEST_F( RegistTest, RegistWithDuplicateAccount )
{
	// first time regist try
	EXPECT_TRUE( m_pBank->Regist( m_Account, m_Password, "1" ) );

	// duplicate regist
	EXPECT_FALSE( m_pBank->Regist( m_Account, m_Password, "1" ) );
}

TEST_F( RegistTest, RegistWithInvalidCashFormat )
{
	EXPECT_FALSE( m_pBank->Regist( m_Account, m_Password, "-123456" ) );
	EXPECT_FALSE( m_pBank->Regist( m_Account, m_Password, "!@#%^&23" ) );
}

TEST_F( RegistTest, RegistWithNoPreDepositCash )
{
	EXPECT_FALSE( m_pBank->Regist( m_Account, m_Password, "0." ) );
	EXPECT_FALSE( m_pBank->Regist( m_Account, m_Password, "-1." ) );
}
