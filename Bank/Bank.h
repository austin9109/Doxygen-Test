#pragma once

// standard library
#include "memory"
#include "string"
#include "vector"

// project library
#include "TypeDefine.h"
#include "BankCode.h"
#include "BankAccount.h"
#include "IDataSerializer.h"
#include "IBankService.h"
#include "ICrossBankService.h"
#include "IServerProtocal.h"
#include "IClientProtocal.h"

//////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Predefined charge fee for each bank
/// </summary>
class ChargeFee
{

public:

	static const DOUBLE Alishan;
	static const DOUBLE XueMountain;
	static const DOUBLE GreenIsland;
	static const DOUBLE XiaoLiuqiu;
};


//////////////////////////////////////////////////////////////////////////////////

//! @brief 
//! For user, bank needs to offer "IBankService". For bank to bank
//! communication while transfering, another bank needs to offer
//! "ICrossBankService" for bank to transfer some cash.
//! Thus we design bank to inherit "IBankService" and "ICrossBankService".
//! 
//! When interact with bank API, user need to required for an access token first,
//! then access bank's other method with this token. Bank will check if this token
//! is valid and direct to corresponding account.
class CBank : public IBankService, private ICrossBankService
{

	typedef std::map<std::string, TBankAccount> AccountBook;

public:

	//! @brief Constructor
	CBank( EBankCode bankCode , std::unique_ptr<IDataSerializer> serializer = NULL, std::unique_ptr<IServerProtocal> sprotocal = NULL, std::unique_ptr<IClientProtocal> cprotocal = NULL );
	
	//! @brief Destructor
	~CBank();

public:

	//! @brief Entry for user to regist an account to this bank. Pre deposit money should larger than 0,
	//!		   any account should not be repeat with alread registered user's account.
	//! @param account User's account. Should be combination of alphabet + numerical.
	//! @param password User's password. Should be combination of alphabet + numerical.
	//! @param preDepositCash User's pre deposit cash.
	//! @return 	
	BOOl Regist( const std::string& account, const std::string& password, const std::string& preDepositCash ) override;

	//! @brief Inquire user's remain balance in this bank.
	//! @param token Access token of corresponding user.
	//! @param currencyCode 
	//! @param cashValue 
	//! @return 
	BOOl Inquire( const std::string& token, ECurrencyCode currencyCode, DOUBLE& cashValue ) const override;
	
	//! @brief Depositi money into user's bank account.
	//! @param token 
	//! @param currencyCode 
	//! @param cashString 
	//! @return 
	BOOl Deposit( const std::string& token, ECurrencyCode currencyCode, const std::string& cashString ) override;

	//! @brief 
	//! @param token 
	//! @param currencyCode 
	//! @param cashString 
	//! @param remainBalance 
	//! @return 
	BOOl Withdraw( const std::string& token, ECurrencyCode currencyCode, const std::string& cashString, DOUBLE& withdrawValue ) override;

	//! @brief 
	//! @param token 
	//! @param importCode 
	//! @param exportCode 
	//! @param cashString 
	//! @return 
	BOOl Exchange( const std::string& token, ECurrencyCode exportCode, ECurrencyCode importCode, const std::string& cashString ) override;

	//! @brief 
	//! @param token 
	//! @param importCode 
	//! @param exportCode 
	//! @param exportBank 
	//! @param targetId 
	//! @param cashString 
	//! @return 
	BOOl Transfer( const std::string& token, ECurrencyCode exportCode, ECurrencyCode importCode, EBankCode exportBank, const std::string& targetId, const std::string& cashString ) override;

	//! @brief Launch server communication protocal.
	void LaunchServer( void );

	//! @brief Close server communication protocal.
	void CloseServer( void );

	// {@ other

	//! @brief 
	//! @param account 
	//! @param password 
	//! @param token 
	//! @return 
	ERetLogin GetAccessToken( const std::string& account, const std::string& password, std::string& token ) override;

	//! @brief 
	//! @param token 
	//! @return 
	ERetLogin ReturnAccessToken( const std::string& token ) override;

	//! @brief 
	//! @return 
	EBankCode GetBankCode( void  ) const override;

	/// @}

public: 
	
	/// {@ Helper function.

	//! @brief Get the exchange rate of specific currency.
	//! 
	//! @param [in] currencyCode Specific currency. 
	//! @return Exchange rate. 
	static DOUBLE GetExchangeRate( ECurrencyCode currencyCode );

	//! @brief Get the charge fee of specific bank.
	//! 
	//! @param [in] bankCode Specific bank. 
	//! @return Charge fee.
	static DOUBLE GetChargeFee( EBankCode bankCode );

	//! @brief Get the Denomination of specific currency.
	//! 
	//! @param [in] currencyCode Specific currency.  
	//! @return Minimum denomination. 
	static DOUBLE GetDenomination( ECurrencyCode currencyCode );
 
	//! @brief Get bank name.
	//! 
	//! @param [in] bankCode Specific bank. 
	//! @return Bank name.
	static std::string GetBankName( EBankCode bankCode );

	/// @}

private:

	//! @brief 
	//! 
	//! @param account 
	//! @param currencyCode 
	//! @param cashString 
	//! @return BOOl 
	BOOl TransferIn( const std::string& account, ECurrencyCode currencyCode, const std::string& cashString ) override;

	//! @brief Create and return an account with non duplicated GUID.
	//! @param [in] account New user's account
	//! @param [in] password New user's password
	//! @return Personal data of registered user.
	TBankAccount CreateAccount( const std::string& account, const std::string& password );
		
	//! @brief Check if numerical value fit the smallest denomination of corresponding currency.
	//! @param currency 
	//! @param value 
	//! @return 
	BOOl LegalDenomination( ECurrencyCode currency, DOUBLE value ) const;

	//! @brief Check if input numerical value is fit with corresponding currency's denomination.
	//! For example, if denomination is 0.01, input shouldn't be 0.013.
	//! @param currency 
	//! @param cashValue 
	//! @return 
	BOOl LegalDenomination( ECurrencyCode currency, const std::string& cashValue );

	//! @brief Detect if input numercial amount is fit in legal region
	BOOl LegalNumercialRegion( const std::string& cashValue ) const;

	//! @brief Convert input numerical to correct denomination cash.
	int ConvertInputToDenomiationCash( ECurrencyCode currency, const std::string& cashValue );

	//! {@ Protocal decryption related

	//! @brief Decrypt the context word receieved from client.
	//! @param context 
	//! @param result Return result context.
	//! @return 
	BOOl DecryptContext( std::string&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptRegistContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptLoginContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptLogoutContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptInquireContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptWithdrawContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptDepositContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptExchangeContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptTransferContext( std::vector<std::string>&& context, std::string& result );

	//! @brief 
	//! 
	//! @param context 
	//! @param result 
	//! @return BOOl 
	BOOl DecryptTransferInContext( std::vector<std::string>&& context, std::string& result );

	//! end Protocal decryption related @}

	//! @brief 
	//! 
	//! @param context 
	//! @param pattern 
	//! @return std::vector<std::string> 
	std::vector<std::string> SplitContext( std::string&& context, const std::string& pattern );

private:
 
	//! @brief Record of currently borrowed token
	std::map<std::string, std::string> m_AccessTokens;

	//! @brief Recognized code of this bank
	const EBankCode m_BankCode;

	//! @brief All of registered personal data.
	AccountBook m_BankAccounts;

	//! @brief Communication service protocal instance.
	std::unique_ptr<IServerProtocal> m_pServerProtocal;

	//! @brief Communication client protocal instance.
	std::unique_ptr<IClientProtocal> m_pClientProtocal;

	//! @brief PlugedIn serialized object.
	std::unique_ptr<IDataSerializer> m_pSerializer;
};
