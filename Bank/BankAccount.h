#pragma once

// standard header
#include "string"
#include "map"

// project header
#include "TypeDefine.h"
#include "CurrencyCode.h"

//! @brief Structure used to store bank user's information.
struct TBankAccount
{

public:

	//! @brief Since we need to initiate balance with difference currency,
	//! we use custrom constructor here.
	TBankAccount();

public:

	//! @brief User's account
	std::string m_account;
	//! @brief User's password
	std::string m_password;
	//! @brief User's balance of each currency
	std::map<ECurrencyCode, INT> m_balance;
};