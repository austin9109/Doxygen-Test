// standard header
#include "sstream"

// third party header
#include "magic_enum/magic_enum_all.hpp"

// project header
#include "OutDebug.h"
#include "ValidInputChecker.h"
#include "StringCoverter.h"
#include "BankClient.h"

#ifdef DEBUG
	const INT TIMEOUT = 5000; // 5 s
#else
	const INT TIMEOUT = 200; // 200 ms
#endif

using namespace std;
using namespace magic_enum;

CBankClient::CBankClient( EBankCode bankCode, unique_ptr<IClientProtocal> cprotocal )
	: m_bankCode( bankCode )
	, m_accessToken()
	, m_pClientProtocal( cprotocal.release() )
{
	m_pClientProtocal->Launch( this->m_bankCode );
}

CBankClient::~CBankClient()
{
	m_pClientProtocal->Close();
}

BOOl CBankClient::Regist( const std::string& account, const std::string& password, const std::string& preDepositCash )
{
	// convert the argument into protocal context
	stringstream ss;
	ss << "Regist," << account << "," << password << "," << preDepositCash;

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Regist recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Regist, state
	if( subStrs.size() != 2 ) {
		return false;
	}

	return subStrs[ 1 ] == "success" ? true : false;
}

ERetLogin CBankClient::Login( const std::string& account, const std::string& password )
{
	// if already login, reject the request
	if( m_accessToken.empty() == false ) {
		return ERetLogin::ERL_AlreadyLogin;
	}

	// convert the argument into protocal context
	stringstream ss;
	ss << "Login," << account << "," << password;

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return ERetLogin::ERL_ConnectionFailed;
	}

	OUTDEBUG( "Login recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Login, state. If success, plus token
	if( subStrs.size() <= 2 ) {
		return ERetLogin::ERL_ConnectionFailed;
	}

	// according to context, analysis each result
	if( subStrs[ 1 ] == "valid" ) {
		// should be Login, success, token
		// also examine if token is with correct format
		if( subStrs.size() != 3 || subStrs[ 2 ].size() == 0 ) {
			return ERetLogin::ERL_ConnectionFailed;
		}
		// retrieve the token
		m_accessToken = subStrs[ 2 ];
		return ERetLogin::ERL_Valid;
	}
	else if( subStrs[ 1 ] == "noaccount" ) {
		return ERetLogin::ERL_NoAccount;
	}
	else if( subStrs[ 1 ] == "wrongpassword" ) {
		return ERetLogin::ERL_WrongPassword;
	}
	else if( subStrs[ 1 ] == "wrongformat" ) {
		return ERetLogin::ERL_WrongFormat;
	}
	else if( subStrs[ 1 ] == "alreadylogin" ) {
		return ERetLogin::ERL_AlreadyLogin;
	}
	else {
		return ERetLogin::ERL_ConnectionFailed;
	}
}

BOOl CBankClient::Logout()
{
	// convert the argument into protocal context
	stringstream ss;
	ss << "Logout," << m_accessToken;

	// after record the token, simply clean the token
	m_accessToken.clear();

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Logout recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Logout, state.
	if( subStrs.size() != 2 ) {
		return false;
	}

	return subStrs[ 1 ] == "success";
}

BOOl CBankClient::Inquire( ECurrencyCode currencyCode, double& cashValue ) const
{
	// pre-reset the request value
	cashValue = -1;

	// if not login, return false
	if( m_accessToken.empty() ) {
		return false;
	}

	// convert the argument into protocal context
	stringstream ss;
	ss << "Inquire," << m_accessToken << "," << enum_name( currencyCode );

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Inquire recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Inquire, state, cash.
	if( subStrs.size() != 3 ) {
		return false;
	}

	// according to context, analysis each result
	if( subStrs[ 1 ] == "success" ) {
		cashValue = CStringCoverter::String2Double( subStrs[ 2 ] );
		return true;
	}
	else {
		return false;
	}
}

BOOl CBankClient::Deposit( ECurrencyCode currencyCode, const std::string& cashString )
{
	// if not login, return false
	if( m_accessToken.empty() ) {
		return false;
	}

	// convert the argument into protocal context
	stringstream ss;
	ss << "Deposit," << m_accessToken << "," << enum_name( currencyCode ) << "," << cashString;

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Deposit recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Deposit, state.
	if( subStrs.size() != 2 ) {
		return false;
	}

	// according to context, analysis each result
	return subStrs[ 1 ] == "success" ? true : false;
}

BOOl CBankClient::Withdraw( ECurrencyCode currencyCode, const std::string& cashString, double& withdrawValue )
{
	// pre-reset the request value
	withdrawValue = -1;

	// if not login, return false
	if( m_accessToken.empty() ) {
		return false;
	}

	// convert the argument into protocal context
	stringstream ss;
	ss << "Withdraw," << m_accessToken << "," << enum_name( currencyCode ) << "," << cashString;

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Withdraw recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Withdraw, state, remain cash.
	if( subStrs.size() != 3 ) {
		return false;
	}

	// according to context, analysis each result
	if( subStrs[ 1 ] == "success" ) {
		withdrawValue = CStringCoverter::String2Double( subStrs[ 2 ] );
		return true;
	}
	else {
		return false;
	}
}

BOOl CBankClient::Exchange( ECurrencyCode exportCode, ECurrencyCode importCode, const std::string& cashString )
{
	// if not login, return false
	if( m_accessToken.empty() ) {
		return false;
	}

	// convert the argument into protocal context
	stringstream ss;
	ss << "Exchange," << m_accessToken << "," << enum_name( exportCode ) << "," << enum_name( importCode ) << "," << cashString;

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Exchange recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Exchange, state.
	if( subStrs.size() != 2 ) {
		return false;
	}

	// according to context, analysis each result
	return subStrs[ 1 ] == "success" ? true : false;
}

BOOl CBankClient::Transfer( ECurrencyCode exportCode, ECurrencyCode importCode, EBankCode bankCode, const std::string& targetId, const std::string& cashString )
{
	// if not login, return false
	if( m_accessToken.empty() ) {
		return false;
	}

	// convert the argument into protocal context
	stringstream ss;
	ss << "Transfer" << ","
		<< m_accessToken << ","
		<< enum_name( exportCode ) << ","
		<< enum_name( importCode ) << ","
		<< enum_name( bankCode ) << ","
		<< targetId << ","
		<< cashString;

	// send the request with protocal and receive comeback result
	string result;
	if( m_pClientProtocal->Send( std::move( ss.str() ), TIMEOUT, result ) == false ) {
		return false;
	}

	OUTDEBUG( "Transfer recieved context feedback: " << result.c_str() );

	// first split context
	vector<string> subStrs = SplitContext( move( result ), "," );

	// check if word is enough, the content should be:
	// Transfer, state.
	if( subStrs.size() != 2 ) {
		return false;
	}

	// according to context, analysis each result
	return subStrs[ 1 ] == "success" ? true : false;
}

string CBankClient::GetBankName() const
{
	switch( m_bankCode )
	{
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

vector<string> CBankClient::SplitContext( string&& context, const string& pattern ) const
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
