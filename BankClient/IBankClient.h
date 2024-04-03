#pragma once

// standard header
#include "string"

// project header
#include "TypeDefine.h"
#include "BankCode.h"
#include "CurrencyCode.h"

//! @brief Collection of methods that provided bank functionality to user.
class IBankClient
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~IBankClient() {}

	// regist
	BOOl virtual Regist( const std::string& account, const std::string& password, const std::string& preDepositCash ) = 0;

	// login & logout
	ERetLogin virtual Login( const std::string& account, const std::string& password ) = 0;
	BOOl virtual Logout() = 0;

	// Bank function
	BOOl virtual Inquire( ECurrencyCode currencyCode, double& cashValue ) const = 0;
	BOOl virtual Deposit( ECurrencyCode currencyCode, const std::string& cashString ) = 0;
	BOOl virtual Withdraw( ECurrencyCode currencyCode, const std::string& cashString, double& withdrawValue ) = 0;
	BOOl virtual Exchange( ECurrencyCode importCode, ECurrencyCode exportCode, const std::string& cashString ) = 0;
	BOOl virtual Transfer( ECurrencyCode importCode, ECurrencyCode exportCode, EBankCode bankCode, const std::string& targetId, const std::string& cashString ) = 0;

	// other
	std::string virtual GetBankName() const = 0;
};