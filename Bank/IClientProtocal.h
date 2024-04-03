#pragma once

// standard library
#include "string"

// project library
#include "TypeDefine.h"
#include "BankCode.h"

//! @brief Client protocal interface for sending request to server
class IClientProtocal
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~IClientProtocal() {}

	//! @brief Launch client protocal functionality.
	//! @param bankCode Specific bank. This is used to launch corresponding connection port.
	//! @return 
	virtual BOOl Launch( EBankCode bankCode ) = 0;

	//! @brief Send message to server.
	//! @param context Message to server.
	//! @param timeout 
	//! @param result Operation result from server.
	//! @return 
	virtual BOOl Send( std::string&& context, INT timeout /*sec*/, std::string& result ) = 0;


	//! @brief Close client protocal functionality.
	//! @return 
	virtual BOOl Close() = 0;
};
