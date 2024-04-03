#pragma once

// standard library
#include "functional"
#include "string"

// project library
#include "TypeDefine.h"
#include "BankCode.h"

//! @brief Server protocal interface for receiving requests from client.
class IServerProtocal
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~IServerProtocal() {}

	//! @brief Launch server protocal functionality.
	//! @param bankCode Specific bank. This is used to launch corresponding connection port.
	//! @return 
	virtual BOOl Launch( EBankCode bankCode ) = 0;

	//! @brief Close server protocal functionality.
	//! @return 
	virtual BOOl Close() = 0;

	//! @brief Binding with the protocal received event. When there is message receieved from client, 
	//! we push these message into callback and attain the operation result to send back to client.
	//! @param event 
	//! @return 
	virtual BOOl RegistCB( std::function<BOOl( std::string&&, std::string& )>&& callback ) = 0;
};