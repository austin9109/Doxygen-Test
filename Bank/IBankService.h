#pragma once

// standard library
#include "string"

// project library
#include "BankCode.h"
#include "CurrencyCode.h"
#include "ICrossBankService.h"

//! @brief Collection of methods that bank offered to bank user.
class IBankService
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~IBankService() {}

	//! @brief Entry for user to regist an account to this bank. Pre deposit money should larger than 0,
	//!		   any account should not be repeat with alread registered user's account.
	//! @param account User's account. Should be combination of alphabet + numerical.
	//! @param password User's password. Should be combination of alphabet + numerical.
	//! @param preDepositCash User's pre deposit cash.
	//! @return 
	BOOl virtual Regist( const std::string& account, const std::string& password, const std::string& preDepositCash ) = 0;

	//! @brief Inquire user's remain balance in this bank.
	//! @param token Access token of corresponding user.
	//! @param currencyCode 
	//! @param cashValue 
	//! @return 
	BOOl virtual Inquire( const std::string& token, ECurrencyCode currencyCode, DOUBLE& cashValue ) const = 0;

	//! @brief Depositi money into user's bank account.
	//! @param token 
	//! @param currencyCode 
	//! @param cashString 
	//! @return 
	BOOl virtual Deposit( const std::string& token, ECurrencyCode currencyCode, const std::string& cashString ) = 0;

	//! @brief 
	//! @param token 
	//! @param currencyCode 
	//! @param cashString 
	//! @param remainBalance 
	//! @return 
	BOOl virtual Withdraw( const std::string& token, ECurrencyCode currencyCode, const std::string& cashString, DOUBLE& remainBalance ) = 0;

	//! @brief 
	//! @param token 
	//! @param importCode 
	//! @param exportCode 
	//! @param cashString 
	//! @return 
	BOOl virtual Exchange( const std::string& token, ECurrencyCode importCode, ECurrencyCode exportCode, const std::string& cashString ) = 0;

	//! @brief 
	//! @param token 
	//! @param importCode 
	//! @param exportCode 
	//! @param exportBank 
	//! @param targetId 
	//! @param cashString 
	//! @return 
	BOOl virtual Transfer( const std::string& token, ECurrencyCode importCode, ECurrencyCode exportCode, EBankCode exportBank, const std::string& targetId, const std::string& cashString ) = 0;

	//! @brief 
	//! @param account 
	//! @param password 
	//! @param token 
	//! @return 
	ERetLogin virtual GetAccessToken( const std::string& account, const std::string& password, std::string& token ) = 0;

	//! @brief 
	//! @param token 
	//! @return 
	ERetLogin virtual ReturnAccessToken( const std::string& token ) = 0;

	//! @brief 
	//! @return 
	EBankCode virtual GetBankCode() const = 0;
};
