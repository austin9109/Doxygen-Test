#pragma once

// project header
#include "IBankClient.h"
#include "IClientProtocal.h"

//! @brief Object for communicating with bank through protocal.
class CBankClient : public IBankClient
{

public:

	//! @brief Constructor.
	//! @param bankCode Specific bank. This is used to attain server protocal.
	//! @param cprotocal Protocal used to communicate with bank.
	CBankClient( EBankCode bankCode, std::unique_ptr<IClientProtocal> cprotocal = NULL );
	~CBankClient();

public:

	// regist
	BOOl Regist( const std::string& account, const std::string& password, const std::string& preDepositCash ) override;
	
	// login & logout
	ERetLogin Login( const std::string& account, const std::string& password ) override;
	BOOl Logout() override;

	// Bank function
	BOOl Inquire( ECurrencyCode currencyCode, double& cashValue ) const override;
	BOOl Deposit( ECurrencyCode currencyCode, const std::string& cashString ) override;
	BOOl Withdraw( ECurrencyCode currencyCode, const std::string& cashString, double& withdrawValue ) override;
	BOOl Exchange( ECurrencyCode exportCode, ECurrencyCode importCode, const std::string& cashString ) override;
	BOOl Transfer( ECurrencyCode exportCode, ECurrencyCode importCode, EBankCode bankCode, const std::string& targetId, const std::string& cashString ) override;

	std::string GetBankName() const override;

private:

	//! @brief Split message received from bank server into recognizeable words.
	//! @param context 
	//! @param pattern 
	//! @return 
	std::vector<std::string> SplitContext( std::string&& context, const std::string& pattern ) const;

	//! @brief Protocal to bank server.
	std::unique_ptr<IClientProtocal> m_pClientProtocal;

	//! @brief The specific bank target of this client service.
	const EBankCode m_bankCode;

	//! @brief Passport used to access functionality of bank service
	std::string m_accessToken;
};
