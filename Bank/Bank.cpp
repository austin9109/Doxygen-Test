#define NOMINMAX

// standard library
#include "ranges"
#include "sstream"
#include "objbase.h"
#include "iostream"

// third library
#include "magic_enum/magic_enum_all.hpp"

// project library
#include "ValidInputChecker.h"
#include "StringCoverter.h"
#include "ExchangeRate.h"
#include "Bank.h"

#define TRANSFER_LEGAL_STRING_SIZE	( 2 )

using namespace std;
using namespace magic_enum;

const DOUBLE ChargeFee::Alishan = 0.05;
const DOUBLE ChargeFee::XueMountain = 0.01;
const DOUBLE ChargeFee::GreenIsland = 0.02;
const DOUBLE ChargeFee::XiaoLiuqiu = 0.07;

//! @brief Conversion rate from dollars to smallest unit.
const map<ECurrencyCode, DOUBLE> g_Denomination = { 
	{ ECurrencyCode::ECC_NTD, 1.0 }, 
	{ ECurrencyCode::ECC_USD, 0.01 }, 
	{ ECurrencyCode::ECC_CNY, 0.1 },
};

const map<ECurrencyCode, INT> g_DenominationPlace = {
	{ ECurrencyCode::ECC_NTD, 0 },
	{ ECurrencyCode::ECC_USD, 2 },
	{ ECurrencyCode::ECC_CNY, 1 },
};

string GuidToString( const GUID& guid )
{
	char szBuf[ 64 ] = { 0 };
	sprintf_s( szBuf, sizeof( szBuf ),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[ 0 ], guid.Data4[ 1 ], guid.Data4[ 2 ], guid.Data4[ 3 ],
		guid.Data4[ 4 ], guid.Data4[ 5 ], guid.Data4[ 6 ], guid.Data4[ 7 ] );
	return std::string( szBuf );
}

CBank::CBank( EBankCode bankCode, unique_ptr<IDataSerializer> serializer, unique_ptr<IServerProtocal> sprotocal, unique_ptr<IClientProtocal> cprotocal )
	: m_BankCode( bankCode )
	, m_pServerProtocal( sprotocal.release() )
	, m_pClientProtocal( cprotocal.release() )
	, m_pSerializer( serializer.release() )
{
	if( this->m_pSerializer != nullptr ) {

		// convert dictionary's value into vector
		auto DcitionaryValues = std::views::values( m_BankAccounts );
		std::vector<TBankAccount> BankAccounts{ DcitionaryValues.begin(), DcitionaryValues.end() };

		// load bank accounts through m_pSerializer
		this->m_pSerializer->Deserialize( this->GetBankName( m_BankCode ) + ".txt", BankAccounts );

		// convert loaded values into dictionary
		for( auto& account : BankAccounts ) {
			m_BankAccounts[ account.m_account ] = std::move( account );
		}
	}
}

CBank::~CBank()
{
	if( m_pSerializer == NULL ) {
		return;
	}

	auto DcitionaryValues = std::views::values( m_BankAccounts );
	std::vector<TBankAccount> BankAccounts{ DcitionaryValues.begin(), DcitionaryValues.end() };

	m_pSerializer->Serialize( this->GetBankName( m_BankCode ) + ".txt", BankAccounts );
}

BOOl CBank::Regist( const string& account, const string& password, const std::string& preDepositCash )
//! @brief 
//! @param bankCode 
//! @return 
{
	// check account
	if( CValidInputChecker::IsLegacyString( account ) == false ) {
		return false;
	}

	// check password
	if( CValidInputChecker::IsLegacyString( password ) == false ) {
		return false;
	}

	// check if has duplicate account
	for( auto const& x : m_BankAccounts ) {
		if( account == x.first ) {
			return false;
		}
	}

	// check if corect numercial string
	if( CValidInputChecker::IsNumercialString( preDepositCash ) == false ) {
		return false;
	}

	// check if decimal place is correct
	if( LegalDenomination( ECurrencyCode::ECC_NTD, preDepositCash ) == false ) {
		return false;
	}

	// user need to deposit more than 1 dolloar when regist, and not exist max money
	INT nDepositCash = ( INT )( CStringCoverter::String2Double( preDepositCash ) );
	if( nDepositCash <= 0 || nDepositCash > std::numeric_limits<INT>::max() ) {
		return false;
	}

	// first time regist, archiving the information, and create a default account of this bank under newbie
	TBankAccount BankAccount = CreateAccount( account, password ); // default account of this bank
	BankAccount.m_balance.at( ECurrencyCode::ECC_NTD ) = nDepositCash;
	m_BankAccounts[ account ] = std::move( BankAccount );

	return true;
}

BOOl CBank::Inquire( const std::string& token, ECurrencyCode currencyCode, DOUBLE& cashValue ) const
{
	// try to find if user exist
	const TBankAccount* pUser = NULL;

	// first check if access token is valid
	for( auto const& x : m_AccessTokens ) {
		if( token == x.first ) {
			pUser = &m_BankAccounts.at( x.second );
			break;
		}
	}

	// if not found pUser
	if( pUser == NULL ) {
		return false;
	}

	// check if has corresponding currency-sub account
	if( pUser->m_balance.find( currencyCode ) == pUser->m_balance.end() ) {
		return false;
	}

	// since we save the smallest cash amount in bank, we change it to dollars.
	cashValue = ( DOUBLE )pUser->m_balance.at( currencyCode ) * g_Denomination.at( currencyCode );
	return true;
}

BOOl CBank::Deposit( const std::string& token, ECurrencyCode currencyCode, const std::string& cashString )
{
	// try to find if user exist
	TBankAccount* pUser = NULL;

	// first check if access token is valid
	for( auto const& x : m_AccessTokens ) {
		if( token == x.first ) {
			pUser = &m_BankAccounts.at( x.second );
			break;
		}
	}

	// if not found user
	if( pUser == NULL ) {
		return false;
	}

	// check if has corresponding currency-sub account
	if( pUser->m_balance.find( currencyCode ) == pUser->m_balance.end() ) {
		return false;
	}

	// check if correct cash string
	if( CValidInputChecker::IsNumercialString( cashString ) == false ) {
		return false;
	}

	// check if numerical region is correct
	if( LegalNumercialRegion( cashString ) == false ) {
		return false;
	}

	// check if decimal place is correct
	if( LegalDenomination( currencyCode, cashString ) == false ) {
		return false;
	}

	// convert from input double cash to smallest denomination int cash!
	INT nCashValue = ConvertInputToDenomiationCash( currencyCode, cashString );

	// check if value is larger than 0
	if( nCashValue <= 0 ) {
		return false;
	}

	// check if balance will overflow
	if( ( pUser->m_balance.at( currencyCode ) < 0.0 ) == ( nCashValue < 0.0 )
		&& std::abs( nCashValue ) > std::numeric_limits<INT>::max() - std::abs( pUser->m_balance.at( currencyCode ) ) ) {
		//  Addition would overflow...
		return false;
	}

	// everything is ok, revise the balance value
	pUser->m_balance.at( currencyCode ) += nCashValue;

	return true;
}

BOOl CBank::Withdraw( const std::string& token, ECurrencyCode currencyCode, const std::string& cashString, DOUBLE& remainBalance )
{
	remainBalance = -1;

	// try to find if user exist
	TBankAccount* pUser = NULL;

	// first check if access token is valid
	for( auto const& x : m_AccessTokens ) {
		if( token == x.first ) {
			pUser = &m_BankAccounts.at( x.second );
			break;
		}
	}

	// if not found pUser
	if( pUser == NULL ) {
		return false;
	}

	// check if has corresponding currency-sub account
	if( pUser->m_balance.find( currencyCode ) == pUser->m_balance.end() ) {
		return false;
	}

	// check if correct cash string
	if( CValidInputChecker::IsNumercialString( cashString ) == false ) {
		return false;
	}

	// check if numerical region is correct
	if( LegalNumercialRegion( cashString ) == false ) {
		return false;
	}

	// check if decimal place is correct
	if( LegalDenomination( currencyCode, cashString ) == false ) {
		return false;
	}

	// convert from input double cash to smallest denomination int cash!
	INT nCashValue = ConvertInputToDenomiationCash( currencyCode, cashString );

	// check if value is larger than 0
	if( nCashValue <= 0 ) {
		return false;
	}

	// check if balance has enough money
	if( pUser->m_balance.at( currencyCode ) - nCashValue < 0 ) {
		return false;
	}

	// everything is ok, revise the balance value and return correct withdraw value
	pUser->m_balance.at( currencyCode ) -= nCashValue;
	remainBalance = ( DOUBLE )pUser->m_balance.at( currencyCode ) * g_Denomination.at( currencyCode ); // convert from smallest denomination to dollars

	return true;
}

BOOl CBank::Exchange( const std::string& token, ECurrencyCode exportCode, ECurrencyCode importCode, const std::string & cashString )
{
	// try to find if user exist
	TBankAccount* pUser = NULL;

	// first check if access token is valid
	for( auto const& x : m_AccessTokens ) {
		if( token == x.first ) {
			pUser = &m_BankAccounts.at( x.second );
			break;
		}
	}

	// if not found pUser
	if( pUser == NULL ) {
		return false;
	}

	// check if has corresponding currency-sub account
	if( pUser->m_balance.find( exportCode ) == pUser->m_balance.end() ||
		pUser->m_balance.find( importCode ) == pUser->m_balance.end() ) {
		return false;
	}

	// check if import currency is same as export currency
	if( exportCode == importCode ) {
		return false;
	}

	// check if correct cash string
	// TODO: this might combine with exchangeCashValue examine
	if( CValidInputChecker::IsNumercialString( cashString ) == false ) {
		return false;
	}

	// check if numerical region is correct
	if( LegalNumercialRegion( cashString ) == false ) {
		return false;
	}

	// check if decimal place is correct
	if( LegalDenomination( exportCode, cashString ) == false ) {
		return false;
	}

	// convert from input double cash to smallest denomination int cash!
	INT nCashValue = ConvertInputToDenomiationCash( exportCode, cashString );

	// check if balance has enough money
	if( pUser->m_balance.at( exportCode ) - nCashValue < 0 ) {
		return false;
	}

	// add currency exchange rate multipiler, caculate cash value after exchange
	// this also check if nCashValue is larger than 0
	INT nExchangeCashValue = ( INT )( ( DOUBLE )ConvertInputToDenomiationCash( exportCode, cashString ) / GetExchangeRate( exportCode ) * GetExchangeRate( importCode ) * g_Denomination.at( exportCode ) / g_Denomination.at( importCode ) );
	if( nExchangeCashValue <= 0 ) {
		return false;
	}

	// check if balance will overflow
	if( ( pUser->m_balance.at( importCode ) < 0.0 ) == ( nExchangeCashValue < 0.0 )
		&& std::abs( nExchangeCashValue ) > std::numeric_limits<INT>::max() - std::abs( pUser->m_balance.at( importCode ) ) ) {
		//  Addition would overflow...
		return false;
	}

	// everything is ok, revise the balance amount
	pUser->m_balance.at( exportCode ) -= nCashValue;
	pUser->m_balance.at( importCode ) += nExchangeCashValue;

	return true;
}

BOOl CBank::Transfer( const std::string& token, ECurrencyCode exportCode, ECurrencyCode importCode, EBankCode exportBank, const std::string& targetId, const std::string& cashString )
{
	// try to find if user exist
	TBankAccount* pUser = NULL;

	// first check if access token is valid
	for( auto const& x : m_AccessTokens ) {
		if( token == x.first ) {
			pUser = &m_BankAccounts.at( x.second );
			break;
		}
	}

	// if not found pUser
	if( pUser == NULL ) {
		return false;
	}

	// check if has corresponding currency-sub account
	if( pUser->m_balance.find( importCode ) == pUser->m_balance.end() ||
		pUser->m_balance.find( exportCode ) == pUser->m_balance.end() ) {
		return false;
	}
	
	// check if bankcode is legal
	if( exportBank == EBankCode::EBC_NoneExist ) {
		return false;
	}

	// check if try to transfer to self
	if( m_BankCode == exportBank && m_AccessTokens[ token ] == targetId ) {
		return false;
	}

	// check if correct cash string
	if( CValidInputChecker::IsNumercialString( cashString ) == false ) {
		return false;
	}

	// check if numerical region is correct
	if( LegalNumercialRegion( cashString ) == false ) {
		return false;
	}

	// check if decimal place is correct
	if( LegalDenomination( exportCode, cashString ) == false ) {
		return false;
	}

	// caculate chargeFee
	DOUBLE chargeFee = 0;
	if( m_BankCode != exportBank ) {
		chargeFee = GetChargeFee( m_BankCode );
	}

	// convert from input double cash to smallest denomination int cash!
	INT nCashValue = ( INT )( ( DOUBLE )ConvertInputToDenomiationCash( exportCode, cashString ) * ( 1. + chargeFee ) );

	// check if balance has enough money
	if( pUser->m_balance.at( exportCode ) - ( nCashValue ) < 0 ) {
		return false;
	}

	// add currency exchange rate multipiler, caculate cash value after exchange
	// this also check if nCashValue is larger than 0
	INT nExchangeCashValue = ( INT )( ( DOUBLE )ConvertInputToDenomiationCash( exportCode, cashString ) / GetExchangeRate( exportCode ) * GetExchangeRate( importCode ) * g_Denomination.at( exportCode ) / g_Denomination.at( importCode ) );
	if( nExchangeCashValue <= 0 ) {
		return false;
	}

	// if export to same bank, directly deposit.
	BOOl bTransferResult = false;
	if( exportBank == this->m_BankCode ) {
		bTransferResult = TransferIn( targetId, importCode, to_string( nExchangeCashValue ) );
	}
	// if export to another bank, try invoke protocal to send message.
	else
	{
		// launch client protocal to transmit message
		if( m_pClientProtocal->Launch( exportBank ) == false ) {
			return false;
		}

		// convert the argument into protocal context
		stringstream ss;
		ss << "TransferIn," << targetId << "," << enum_name( importCode ) << "," << to_string( nExchangeCashValue );

		// send the request with protocal and receive comeback result
		//crossBankService->TransferIn( targetId, exportCode, std::to_string( exchangeCashValue )
		string result;
		if( m_pClientProtocal->Send( move( ss.str() ), 2000, result ) == false ) {
			m_pClientProtocal->Close();
			return false;
		}

		// after succesfully received message through protocal, close the protocal
		m_pClientProtocal->Close();

		// first split context
		vector<string> subStrs = SplitContext( move( result ), "," );

		// check if word is enough, the content should be:
		// TransferIn, state.
		if( subStrs.size() != TRANSFER_LEGAL_STRING_SIZE ) {
			return false;
		}

		// according to context, analysis each result
		bTransferResult = subStrs[ 1 ] == "success" ? true : false; 
	}

	// if transfer failed, directly return
	if( bTransferResult == false ) {
		return false;
	}

	// if target bank deposit these money success, withdraw these money + charge fee from pUser
	pUser->m_balance.at( exportCode ) -= ( nCashValue );
	return true;
}

BOOl CBank::TransferIn( const std::string& account, ECurrencyCode currencyCode, const std::string& cashString )
{
	// try to find if user exist
	TBankAccount* pUser = NULL;

	// first check if access token is valid
	for( auto const& x : m_BankAccounts ) {
		if( account == x.first ) {
			pUser = &m_BankAccounts.at( account );
			break;
		}
	}

	// if not found pUser
	if( pUser == NULL ) {
		return false;
	}

	// check if has corresponding currency-sub account
	if( pUser->m_balance.find( currencyCode ) == pUser->m_balance.end() ) {
		return false;
	}

	// check if correct cash string
	if( CValidInputChecker::IsNumercialString( cashString ) == false ) {
		return false;
	}

	// check if numerical region is correct
	if( LegalNumercialRegion( cashString ) == false ) {
		return false;
	}

	// check if decimal place is correct
	if( LegalDenomination( currencyCode, cashString ) == false ) {
		return false;
	}

	// convert from input double cash to smallest denomination int cash!
	// here the cash string should already being smallest denomination, thus we don't do any convert
	INT nCashValue = ( INT )stod( cashString );

	// check if value is larger than 0
	if( nCashValue <= 0 ) {
		return false;
	}

	// check if balance will overflow
	if( ( pUser->m_balance.at( currencyCode ) < 0.0 ) == ( nCashValue < 0.0 )
		&& std::abs( nCashValue ) > std::numeric_limits<INT>::max() - std::abs( pUser->m_balance.at( currencyCode ) ) ) {
		//  Addition would overflow...
		return false;
	}

	// everything is ok, revise the balance value
	pUser->m_balance.at( currencyCode ) += nCashValue;

	return true;
}

void CBank::LaunchServer()
{
	m_pServerProtocal->RegistCB( std::move( std::bind( &CBank::DecryptContext, this, std::placeholders::_1, std::placeholders::_2 ) ) );

	if( m_pServerProtocal != NULL ) {
		m_pServerProtocal->Launch( this->m_BankCode );
	}
}

void CBank::CloseServer()
{
	if( m_pServerProtocal != NULL ) {
		m_pServerProtocal->Close();
	}
}

ERetLogin CBank::GetAccessToken( const std::string& account, const std::string& password, std::string& token )
{
	// first set token to empty
	token = "";

	// check account
	if( CValidInputChecker::IsLegacyString( account ) == false ) {
		return ERetLogin::ERL_WrongFormat;
	}

	// check password
	if( CValidInputChecker::IsLegacyString( password ) == false ) {
		return ERetLogin::ERL_WrongFormat;
	}

	// check if has this account
	BOOl bRegisted = false;
	for( auto const& x : m_BankAccounts ) {
		if( account == x.first ) {
			bRegisted = true;
			break;
		}
	}

	if( bRegisted == false ) {
		return ERetLogin::ERL_NoAccount;
	}

	// check if password is same
	TBankAccount BankAccount = m_BankAccounts.at( account );
	if( BankAccount.m_password != password ) {
		return ERetLogin::ERL_WrongPassword;
	}

	// check if the token have already been borrowed
	for( auto const& x : m_AccessTokens ) {
		if( account == x.second ) {
			return ERetLogin::ERL_AlreadyLogin;
		}
	}

	// everything is fine, generate an access token for the login user
	// user will take this token as a pass
	// create guid code
	GUID Guid;
	string GeneratedCode;

	// keep generating until hash code is correct and not repeat
	do
	{
		// if corrupt hash code, continue generating
		HRESULT h = CoCreateGuid( &Guid );
		if( h != S_OK ) {
			continue;
		}

		// detect if duplicate account in bank system
		GeneratedCode = std::move( GuidToString( Guid ) );

		for( auto const& x : m_AccessTokens ) {
			if( GeneratedCode == x.first ) { // duplicate id, false case
				continue;
			}
		}

		// legal account, break 
		break;

	} while( true );

	// record the access token bind to which account, and return it to the user
	m_AccessTokens[ GeneratedCode ] = account;
	token = GeneratedCode;

	return ERetLogin::ERL_Valid;
}

ERetLogin CBank::ReturnAccessToken( const std::string& token )
{
	// check if the token have already been borrowed
	for( auto const& x : m_AccessTokens ) {
		if( token == x.first ) {

			// remove element from map and return as success.
			m_AccessTokens.erase( token );
			return ERetLogin::ERL_Valid;
		}
	}

	// if not found in borrowed tokens, return failed
	return ERetLogin::ERL_Failed;
}

EBankCode CBank::GetBankCode() const
{
	return m_BankCode;
}

DOUBLE CBank::GetExchangeRate( ECurrencyCode currencyCode )
{
	switch( currencyCode ) {

	case ECurrencyCode::ECC_NTD:
		return Currency::Value::NTD;

	case ECurrencyCode::ECC_USD:
		return Currency::Value::USD;

	case ECurrencyCode::ECC_CNY:
		return Currency::Value::CNY;

	default:
		return -1;
	}
}

DOUBLE CBank::GetChargeFee( EBankCode bankCode )
{
	switch( bankCode ) {

	case EBankCode::EBC_Alishan:
		return ChargeFee::Alishan;

	case EBankCode::EBC_XueMountain:
		return ChargeFee::XueMountain;

	case EBankCode::EBC_GreenIsland:
		return ChargeFee::GreenIsland;

	case EBankCode::EBC_XiaoLiuqiu:
		return ChargeFee::XiaoLiuqiu;

	default:
		return std::numeric_limits<DOUBLE>::max();
	}
}

DOUBLE CBank::GetDenomination( ECurrencyCode currency )
{
	if( g_Denomination.find( currency ) == g_Denomination.end() ) {
		return -1.0;
	}
	else {
		return g_Denomination.at( currency );
	}
}

string CBank::GetBankName( EBankCode bankCode )
{
	switch( bankCode ) {
	
	case EBankCode::EBC_Alishan:
		return "Alishan";

	case EBankCode::EBC_XueMountain:

		return "XueMountain";
	case EBankCode::EBC_GreenIsland:

		return "GreenIsland";
	
	case EBankCode::EBC_XiaoLiuqiu:
		return "XiaoLiuqiu";
	
	default:
		return "";
	}
}

TBankAccount CBank::CreateAccount( const string& account, const string& password )
{
	// regist a new account into bank
	TBankAccount BrandNew;
	BrandNew.m_account = account;
	BrandNew.m_password = password;

	return BrandNew;
}

BOOl CBank::LegalDenomination( ECurrencyCode currency, DOUBLE value ) const
{
	// This method is wrong >>>
	// attain decimal place of each 2.
	//return ( ( -1 * static_cast< int >( floor( log10( fabs( denomination.at( currency ) ) ) ) ) ) == ( -1 * static_cast< int >( floor( log10( fabs( value ) ) ) ) ) );

	// This method still wrong if input string is "0.3" >>>
	DOUBLE Truncated = value;
	DOUBLE ExtraTruncated = value;

	// convert to Truncated place
	// here we do not directly use (int) but use "trunc" to covert,
	// since when value is maxmium int value, direclty to convert to int
	// might cause numerical overflow
	if( g_Denomination.at( currency ) == 0 ) {
		// assert( false );
	}

	Truncated = trunc( Truncated / g_Denomination.at( currency ) );
	// covert back
	Truncated = Truncated * g_Denomination.at( currency );

	// convert to Truncated place, plus 1 extra place
	// here we do not directly use (int) but use "trunc" to covert,
	// since when value is maxmium int value, direclty to convert to int
	// might cause numerical overflow
	ExtraTruncated = trunc( ExtraTruncated / g_Denomination.at( currency ) * 10 );
	
	// covert back
	ExtraTruncated = ExtraTruncated * g_Denomination.at( currency ) / 10;

	return abs( Truncated - ExtraTruncated ) <= numeric_limits<DOUBLE>::min();
}

BOOl CBank::LegalDenomination( ECurrencyCode currency, const string& cashValue )
{
	INT currencyPlace = 0, examinePlace = 0;
	BOOl hasDecimal = false;

	string denominationStr = to_string( g_Denomination.at( currency ) );

	// first caculate currency denomination
	currencyPlace = g_DenominationPlace.at( currency );

	// next check cash value's denomination
	for( const char& c : cashValue ) {
		if( c == '.' ) {
			hasDecimal = true;
		}
		else if( hasDecimal ) {
			examinePlace++;
		}
	}

	// for legal denomination, cashvalue's decimal place should not larger than currency's denomination
	return currencyPlace >= examinePlace;
}

BOOl CBank::LegalNumercialRegion( const string& cashValue ) const
{
	// since we record money in "int type", and input can be float number,
	// hence we convert input string to double first, than compare to int.
	// this can avoid the result of numerical overflow.
	try {
		DOUBLE examine = stod( cashValue );
		return 0 <= examine && examine <= numeric_limits<INT>::max();
	}
	catch( ... ) {
		return false;
	}
}

INT CBank::ConvertInputToDenomiationCash( ECurrencyCode currency, const string& cashValue )
{
	INT currencyPlace = 0, intputPlace = 0;
	BOOl hasDecimal = false;

	string denominationStr = to_string( g_Denomination.at( currency ) );

	// first caculate currency denomination
	currencyPlace = g_DenominationPlace.at( currency );

	// next check cash value's denomination
	for( const char& c : cashValue ) {
		if( c == '.' ) {
			hasDecimal = true;
		}
		else if( hasDecimal ) {
			intputPlace++;
		}
	}

	INT differentPlace = currencyPlace - intputPlace;

	// caculate result
	string result;

	for( auto const& c : cashValue ) {
		if( c == '.' ) {
			continue;
		}
		result.push_back( c );
	}

	// compensate different place
	for( INT i = 0; i < differentPlace; i++ ) {
		result.push_back( '0' );
	}

	return stoi( result );
}

BOOl CBank::DecryptContext( string&& context, string& result )
{
	printf( "Start to decrypt context\n" );

	// first split context
	vector<string> subStrs = SplitContext( move( context ), "," );

	// if non-legal context, return as false
	if( subStrs.size() == 0 ) {
		return false;
	}

	// divide into seperate case: first word represent the mode 
	if( subStrs[ 0 ] == "Regist" ) {
		return DecryptRegistContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Login" ) {
		return DecryptLoginContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Logout" ) {
		return DecryptLogoutContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Inquire" ) {
		return DecryptInquireContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Withdraw" ) {
		return DecryptWithdrawContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Deposit" ) {
		return DecryptDepositContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Exchange" ) {
		return DecryptExchangeContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "Transfer" ) {
		return DecryptTransferContext( move( subStrs ), result );
	}
	else if( subStrs[ 0 ] == "TransferIn" ) {
		return DecryptTransferInContext( move( subStrs ), result );
	}
	else {
		return false;
	}
}

BOOl CBank::DecryptRegistContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Regist, account, password, predeposit
	if( context.size() != 4 ) {
		printf( "Incorrect format when decrypt regist context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Regist,";

	// try to access create an account and return the result.
	ss << ( Regist( context[ 1 ], context[ 2 ], context[ 3 ] ) == true ? "success" : "failed" );

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptLoginContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Login, account, password
	if( context.size() != 3 ) {
		printf( "Incorrect format when decrypt login context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Login,";

	// try to access bank account and return the result.
	string token;
	ERetLogin ret = GetAccessToken( context[ 1 ], context[ 2 ], token );

	// according to the result, assign different result context
	if( ret == ERetLogin::ERL_Valid ) {
		ss << "valid," << token;
	}
	else if( ret == ERetLogin::ERL_NoAccount ){
		ss << "noaccount";
	}
	else if( ret == ERetLogin::ERL_WrongPassword ) {
		ss << "wrongpassword";
	}
	else if( ret == ERetLogin::ERL_WrongFormat ) {
		ss << "wrongformat";
	}
	else if( ret == ERetLogin::ERL_AlreadyLogin ) {
		ss << "alreadylogin";
	}
	else {
		ss << "failed";
	}

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptLogoutContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Logout, token
	if( context.size() != 2 ) {
		printf( "Incorrect format when decrypt logout context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Logout,";

	// try to logout
	ERetLogin ret = ReturnAccessToken( context[ 1 ] );

	// save the context into result
	result = ret == ERetLogin::ERL_Valid ? "success" : "failed";
	return true;
}

BOOl CBank::DecryptInquireContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Inquire, token, currency
	if( context.size() != 3 ) {
		printf( "Incorrect format when decrypt inquire context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Inquire,";

	// analysis currency input
	ECurrencyCode currency = ECurrencyCode::ECC_NODEFINE;

	if( context[ 2 ] == "ECC_NTD" ) {
		currency = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 2 ] == "ECC_USD" ) {
		currency = ECurrencyCode::ECC_USD;
	}
	else if( context[ 2 ] == "ECC_CNY" ) {
		currency = ECurrencyCode::ECC_CNY;
	}

	// try to inquire and return the result.
	DOUBLE cash = 0;
	BOOl ret = Inquire( context[ 1 ], currency, cash );

	// according to the result, assign different result context
	ret ? ( ss << "success," << cash ) : ( ss << "failed, " << -1 );

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptWithdrawContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Withdraw, token, currency, withdraw value
	if( context.size() != 4 ) {
		printf( "Incorrect format when decrypt withdraw context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Withdraw,";

	// analysis currency input
	ECurrencyCode currency = ECurrencyCode::ECC_NODEFINE;

	if( context[ 2 ] == "ECC_NTD" ) {
		currency = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 2 ] == "ECC_USD" ) {
		currency = ECurrencyCode::ECC_USD;
	}
	else if( context[ 2 ] == "ECC_CNY" ) {
		currency = ECurrencyCode::ECC_CNY;
	}

	// try to withdraw and return the result.
	DOUBLE cash = 0;
	BOOl ret = Withdraw( context[ 1 ], currency, context[ 3 ], cash );

	// according to the result, assign different result context
	ret ? ( ss << "success" << "," << cash ) : ( ss << "failed" << "," << -1 );

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptDepositContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Deposit, token, currency, deposit value
	if( context.size() != 4 ) {
		printf( "Incorrect format when decrypt deposit context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Deposit,";

	// analysis currency input
	ECurrencyCode currency = ECurrencyCode::ECC_NODEFINE;

	if( context[ 2 ] == "ECC_NTD" ) {
		currency = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 2 ] == "ECC_USD" ) {
		currency = ECurrencyCode::ECC_USD;
	}
	else if( context[ 2 ] == "ECC_CNY" ) {
		currency = ECurrencyCode::ECC_CNY;
	}

	// try to deposit and attain the result.
	BOOl ret = Deposit( context[ 1 ], currency, context[ 3 ] );

	// according to the result, assign different result context
	ret ? ( ss << "success" ) : ( ss << "failed" );

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptExchangeContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Exchange, token, currency, currency, exchange value
	if( context.size() != 5 ) {
		printf( "Incorrect format when decrypt exchange context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Exchange,";

	// analysis currency input
	ECurrencyCode exportCode = ECurrencyCode::ECC_NODEFINE;
	if( context[ 2 ] == "ECC_NTD" ) {
		exportCode = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 2 ] == "ECC_USD" ) {
		exportCode = ECurrencyCode::ECC_USD;
	}
	else if( context[ 2 ] == "ECC_CNY" ) {
		exportCode = ECurrencyCode::ECC_CNY;
	}

	ECurrencyCode importCode = ECurrencyCode::ECC_NODEFINE;
	if( context[ 3 ] == "ECC_NTD" ) {
		importCode = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 3 ] == "ECC_USD" ) {
		importCode = ECurrencyCode::ECC_USD;
	}
	else if( context[ 3 ] == "ECC_CNY" ) {
		importCode = ECurrencyCode::ECC_CNY;
	}

	// try to exchange and return the result.
	BOOl ret = Exchange( context[ 1 ], exportCode, importCode, context[ 4 ] );

	// according to the result, assign different result context
	ret ? ( ss << "success" ) : ( ss << "failed" );

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptTransferContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// Transfer, token, currency, currency, bankCode, id, transfer value
	if( context.size() != 7 ) {
		printf( "Incorrect format when decrypt transfer context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "Transfer,";

	// analysis currency input
	ECurrencyCode exportCode = ECurrencyCode::ECC_NODEFINE;
	if( context[ 2 ] == "ECC_NTD" ) {
		exportCode = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 2 ] == "ECC_USD" ) {
		exportCode = ECurrencyCode::ECC_USD;
	}
	else if( context[ 2 ] == "ECC_CNY" ) {
		exportCode = ECurrencyCode::ECC_CNY;
	}

	ECurrencyCode importCode = ECurrencyCode::ECC_NODEFINE;
	if( context[ 3 ] == "ECC_NTD" ) {
		importCode = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 3 ] == "ECC_USD" ) {
		importCode = ECurrencyCode::ECC_USD;
	}
	else if( context[ 3 ] == "ECC_CNY" ) {
		importCode = ECurrencyCode::ECC_CNY;
	}

	EBankCode bankcode = EBankCode::EBC_NoneExist;
	if( context[ 4 ] == "EBC_Alishan" ) {
		bankcode = EBankCode::EBC_Alishan;
	}
	else if( context[ 4 ] == "EBC_XueMountain" ) {
		bankcode = EBankCode::EBC_XueMountain;
	}
	else if( context[ 4 ] == "EBC_GreenIsland" ) {
		bankcode = EBankCode::EBC_GreenIsland;
	}
	else if( context[ 4 ] == "EBC_XiaoLiuqiu" ) {
		bankcode = EBankCode::EBC_XiaoLiuqiu;
	}

	// try to transfer and return the result.
	BOOl ret = Transfer( context[ 1 ], exportCode, importCode, bankcode, context[5], context[ 6 ] );

	// according to the result, assign different result context
	ret ? ( ss << "success" ) : ( ss << "failed" );

	// save the context into result
	result = move( ss.str() );
	return true;
}

BOOl CBank::DecryptTransferInContext( vector<string>&& context, string& result )
{
	// check if word is enough, should be:
	// TransferIn, account id, currency, transfer value
	if( context.size() != 4 ) {
		printf( "Incorrect format when decrypt transferIn context.\n" );
		return false;
	}

	// construct string stream and set current mode
	stringstream ss;
	ss << "TransferIn,";

	// analysis currency input
	ECurrencyCode currency = ECurrencyCode::ECC_NODEFINE;
	if( context[ 2 ] == "ECC_NTD" ) {
		currency = ECurrencyCode::ECC_NTD;
	}
	else if( context[ 2 ] == "ECC_USD" ) {
		currency = ECurrencyCode::ECC_USD;
	}
	else if( context[ 2 ] == "ECC_CNY" ) {
		currency = ECurrencyCode::ECC_CNY;
	}

	// try to transfer and return the result.
	BOOl ret = TransferIn( context[ 1 ], currency, context[ 3 ] );

	// according to the result, assign different result context
	ret ? ( ss << "success" ) : ( ss << "failed" );

	// save the context into result
	result = move( ss.str() );
	return true;
}

vector<string> CBank::SplitContext( string&& context, const string& pattern )
{
	vector<string> result;
	string::size_type begin, end;

	end = context.find( pattern );
	begin = 0;

	while( end != string::npos ) {

		if( end - begin != 0 ) {
			result.push_back( context.substr( begin, end - begin ) );
		}

		begin = end + pattern.size();
		end = context.find( pattern, begin );
	}

	if( begin != context.length() ) {
		result.push_back( context.substr( begin ) );
	}
	return result;

}
