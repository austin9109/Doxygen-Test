#pragma once

// standard header
#include "string"
#include "functional"

// windows header
#include "winsock2.h"

// project header
#include "IServerProtocal.h"
#include "IClientProtocal.h"
#include "Bank.h"

class CSocketServerProtocal : public IServerProtocal
{

public:

	//! @brief Constructor.
	CSocketServerProtocal();

	//! @brief Destructor.
	~CSocketServerProtocal();

	//! @brief Launch protocal & server
	//! @return 
	BOOl Launch( EBankCode bankCode ) override;

	//! @brief Close the server.
	//! @return 
	BOOl Close() override;

	//! @brief A window for higher logic to regist callback when event comes in.
	BOOl RegistCB( std::function<BOOl( std::string&&, std::string& )>&& callback ) override;

private:

	//! @brief 
	BOOl m_launch = false;

	//! @brief 
	EBankCode m_bankCode;

	//! @brief Set datasturcture of socket descriptor
	fd_set m_readfds;

	//! @brief 
	std::function<BOOl( std::string&&, std::string& )> m_eventHandle;

};
