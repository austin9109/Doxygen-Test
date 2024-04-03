#include "BankAccount.h"

TBankAccount::TBankAccount()
{
	m_balance[ ECurrencyCode::ECC_NTD ] = 0;
	m_balance[ ECurrencyCode::ECC_USD ] = 0;
	m_balance[ ECurrencyCode::ECC_CNY ] = 0;
}