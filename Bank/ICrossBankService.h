#pragma once

// standard library
#include "string"

// project library
#include "TypeDefine.h"
#include "BankCode.h"
#include "CurrencyCode.h"

//! @brief Collection of methods that banks communicate with each other.
class ICrossBankService
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~ICrossBankService() {}

	//! @brief Let another bank to transfer some money into this bank without login.
	//! @param account 
	//! @param currencyCode 
	//! @param cashString 
	//! @return 
	BOOl virtual TransferIn( const std::string& account, ECurrencyCode currencyCode, const std::string& cashString ) = 0;
};
