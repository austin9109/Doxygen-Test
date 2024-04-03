#pragma once

// standard header
#include "string"
#include "functional"

// project header
#include "IClientProtocal.h"
#include "BankCode.h"

class CSocketClientProtocal : public IClientProtocal
{

public:

	//! @brief Constructor
	CSocketClientProtocal();

	//! @brief Destructor
	~CSocketClientProtocal();

public:

	//! @brief 
	//! @param bankCode 
	//! @return 
	BOOl Launch( EBankCode bankCode ) override;

	//! @brief Communciate with another bank
	//! @param context 
	//! @param timeout : (ms)
	//! @param result : server return words
	//! @return 
	BOOl Send( std::string&& context, INT timeout, std::string& result ) override;

	//! @brief 
	//! @param bankCode 
	//! @return 
	BOOl Close() override;

private:

	INT m_nClientSocket = 0;

	BOOl m_bLaunch = false;

};