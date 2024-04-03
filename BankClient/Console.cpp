// standard header
#include "iostream"
#include "sstream"
#include "conio.h"
#include "thread"
#include "format"
#include "iomanip"

// third party header
#include "magic_enum/magic_enum_all.hpp"

// project header
#include "Bank.h"
#include "BankClient.h"
#include "JsonSerializer.h"
#include "SocketClientProtocal.h"
#include "Console.h"

using namespace std;
using namespace magic_enum;

Console::Console()
	: m_Menu( EMenu::EM_WELCOME )
	, m_MenuHandle( std::bind( &Console::WelcomeMenu, this ) )
	, m_bShutDown( false )
{
	// TODO: fix this.
	std::cout << setprecision( 3 );

	Launch();
}

void Console::Output( string&& str )
{
	Refresh( move( str ) );
}

void Console::Launch()
{
	while( m_bShutDown == false ) {

		// show mode illustraion
		Output( move( ModeTitle( m_Menu ) ) );

		// calling currently page handle to deal with input
		m_MenuHandle();
	}
}

void Console::Refresh( string&& str  )
{

#ifdef NDEBUG
	// CLEAR console first
	system( "CLS" );
#endif

	// output
	cout << move( str );
}

string Console::ModeTitle( EMenu mode ) const
{
	stringstream ss;

	ss << "==================================================" << endl;
	ss << endl;

	// mainly illustration logic..
	{

		switch( mode )
		{

		case EMenu::EM_WELCOME:
		{
			ss << "Welcome to bank system!" << endl;

			INT modeKey = 1;
			enum_for_each<EBankCode>( [&]( auto val ) {
				constexpr EBankCode bankCode = val;
				ss << "Press " << modeKey << " to select bank " << enum_name( bankCode ) << endl;
				modeKey++;
				} );

			ss << "Press exit() to Exit program." << endl;

			break;
		}
		case EMenu::EM_LOGIN:
		{
			ss << "You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Please type your Account and Password to login" << endl;
			ss << "Type \"exit()\" to back to welcome menu." << endl;
			ss << "Type \"regist()\" to switch to regist menu." << endl;

			break;
		}
		case EMenu::EM_REGIST:
		{
			ss << "You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Please type your \"Account\" and \"Password\" to regist" << endl;
			ss << "Type \"exit()\" to back to Welcome Menu." << endl;

			break;
		}
		case EMenu::EM_MAIN:
		{
			ss << "Hello user! You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Type inquire() to Go to Inquire menu." << endl;
			ss << "Type withdraw() to Go to Withdraw menu." << endl;
			ss << "Type deposit() to Go to Deposit menu." << endl;
			ss << "Type exchange() to Go to Exchange menu." << endl;
			ss << "Type transfer() to Go to Transfer menu." << endl;
			ss << "Type logout() to log out and go back to welcome menu." << endl;

			break;
		}
		case EMenu::EM_INQUIRE:
		{
			ss << "Hello user! You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Following is your current balance in acount. " << endl;

			break;
		}
		case EMenu::EM_WITHDRAW:
		{
			ss << "Hello user! You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Please select currency and cash you want to withdraw:" << endl;
			ss << endl;
			ss << "Currency: NTD 1, USD 2, CNY 3" << endl;
			ss << "Denomination: ";
			enum_for_each<ECurrencyCode>( [&]( auto val ) {
				constexpr ECurrencyCode currency = val;
				if( currency != ECurrencyCode::ECC_NODEFINE ) {
					ss << enum_name( currency ) << " " << CBank::GetDenomination( currency ) << " ";
				}
			} );
			ss << endl;
			ss << endl;
			ss << "Type show() to show current balance in account." << endl;
			ss << "Type exit() to back to Main Menu." << endl;

			break;
		}
		case EMenu::EM_DEPOSIT:
		{
			ss << "Hello user! You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Please select currency and cash you want to deposit: " << endl;
			ss << endl;
			ss << "Currency: NTD 1, USD 2, CNY 3" << endl;
			ss << "Denomination: ";
			enum_for_each<ECurrencyCode>( [&]( auto val ) {
				constexpr ECurrencyCode currency = val;
				if( currency != ECurrencyCode::ECC_NODEFINE ) {
					ss << enum_name( currency ) << " " << CBank::GetDenomination( currency ) << " ";
				}
			} );
			ss << endl;
			ss << endl;
			ss << "Type exit() to back to Main Menu." << endl;

			break;
		}
		case EMenu::EM_EXCHANGE:
		{
			ss << "Hello user! You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Please select currency and cash you want to Exchange: " << endl;
			ss << endl;
			ss << "Currency: NTD 1, USD 2, CNY 3" << endl;
			ss << "Exchange rate (based on USD): ";
			enum_for_each<ECurrencyCode>( [&]( auto val ) {
				constexpr ECurrencyCode currency = val;
				if( currency != ECurrencyCode::ECC_NODEFINE ) {
					ss << enum_name( currency ) << " " << CBank::GetExchangeRate( currency ) << " ";	
				}
			} );
			ss << endl;
			ss << "Denomination: ";
			enum_for_each<ECurrencyCode>( [&]( auto val ) {
				constexpr ECurrencyCode currency = val;
				if( currency != ECurrencyCode::ECC_NODEFINE ) {
					ss << enum_name( currency ) << " " << CBank::GetDenomination( currency ) << " ";
				}
				} );
			ss << endl;
			ss << endl;
			ss << "Type show() to show current balance in account." << endl;
			ss << "Type exit() to back to Main Menu." << endl;

			break;
		}
		case EMenu::EM_TRANSFER:
		{
			ss << "Hello user! You are currently connect with Bank " << m_pConnect->GetBankName() << endl;
			ss << "Please select currency and cash you want to Exchange: " << endl;
			ss << endl;
			ss << "Currency: NTD 1, USD 2, CNY 3" << endl;
			ss << "Bank: Alishan 1, XueMountain 2, GreenIsland 3, XiaoLiuqiu 4" << endl;
			ss << "Exchange rate (based on USD): ";
			enum_for_each<ECurrencyCode>( [&]( auto val ) {
				constexpr ECurrencyCode currency = val;
				if( currency != ECurrencyCode::ECC_NODEFINE ) {
					ss << enum_name( currency ) << " " << CBank::GetExchangeRate( currency ) << " ";
				}
			} );
			ss << endl;
			ss << "Transfer fee: ";
			enum_for_each<EBankCode>( [&]( auto val ) {
				constexpr EBankCode bank = val;
				if( bank != EBankCode::EBC_NoneExist ) {
					ss << enum_name( bank ) << " " << CBank::GetChargeFee( bank ) << " ";
				}
			} );
			ss << endl;
			ss << "Denomination: ";
			enum_for_each<ECurrencyCode>( [&]( auto val ) {
				constexpr ECurrencyCode currency = val;
				if( currency != ECurrencyCode::ECC_NODEFINE ) {
					ss << enum_name( currency ) << " " << CBank::GetDenomination( currency ) << " ";
				}
				} );
			ss << endl;
			ss << endl;
			ss << "Type show() to show current balance in account." << endl;
			ss << "Type exit() to back to Main Menu." << endl;

			break;
		}
		default:
			break;
		}
	}

	// prompt pin on the top
	ss << endl;
	ss << m_Prompt;
	ss << endl;

	// bottom of the title
	ss << endl;
	ss << "==================================================" << endl;

	return move( ss.str() );
}

void Console::WelcomeMenu()
{
	// waiting for user to input
	string input;
	getline( cin, input );

	// Login with Bank Allishan
	if( input == "1" ) {
		
		// switch to login page
		m_Menu = EMenu::EM_LOGIN;
		m_MenuHandle = std::bind( &Console::LoginMenu, this );

		// create corresponding bank client
		m_pConnect = make_unique<CBankClient>( EBankCode::EBC_Alishan, make_unique<CSocketClientProtocal>() );

	}
	// Login with Xue Mountain
	else if( input == "2" ) {

		// switch to login page
		m_Menu = EMenu::EM_LOGIN;
		m_MenuHandle = std::bind( &Console::LoginMenu, this );

		// connect with bank client
		m_pConnect = make_unique<CBankClient>( EBankCode::EBC_XueMountain, make_unique<CSocketClientProtocal>() );

	}
	// Login with Green Island
	else if( input == "3" ) {

		// switch to login page
		m_Menu = EMenu::EM_LOGIN;
		m_MenuHandle = std::bind( &Console::LoginMenu, this );

		// connect with bank client
		m_pConnect = make_unique<CBankClient>( EBankCode::EBC_GreenIsland, make_unique<CSocketClientProtocal>() );

	}
	// Login with Xiao Liuqiu
	if( input == "4" ) {

		// switch to login page
		m_Menu = EMenu::EM_LOGIN;
		m_MenuHandle = std::bind( &Console::LoginMenu, this );

		// connect with bank client
		m_pConnect = make_unique<CBankClient>( EBankCode::EBC_XiaoLiuqiu, make_unique<CSocketClientProtocal>() );

	}
	// Exit program
	else if( input == "exit()" ) {
		m_bShutDown = true;
	}
}

void Console::LoginMenu()
{
	// record user's input: 
	// if user type exit(), return to welcome menu.
	vector<string> strInput( 2 ); // account & password
	for( INT ni = 0; ni < strInput.size(); ni++ ) {
		
		if( ni == 0 ) {
			cout << "Account: " << endl;
		}
		else {
			cout << "Password: " << endl;
		}

		// attain user's input in console
		getline( cin, strInput[ni] );

		// detect if input is special keyword
		// user want to go back to main menu
		if( strInput[ ni ] == "exit()" ) {

			// clean prompt word
			m_Prompt = "";

			m_Menu = EMenu::EM_WELCOME;
			m_MenuHandle = std::bind( &Console::WelcomeMenu, this );

			// disconnect with bank client
			m_pConnect.reset();
			return;
		}
		// user want to regist a new account, switch to regist menu
		else if( strInput[ ni ] == "regist()" ) {

			// clean prompt word
			m_Prompt = "";

			m_Menu = EMenu::EM_REGIST;
			m_MenuHandle = std::bind( &Console::RegistMenu, this );

			return;		
		}
	}

	// try to Login
	ERetLogin ret = m_pConnect->Login( strInput[0], strInput[1] );
	if( ret == ERetLogin::ERL_Valid ) {
		// clean prompt word
		m_Prompt = "";
			
		m_Menu = EMenu::EM_MAIN;
		m_MenuHandle = std::bind( &Console::MainMenu, this );
	}
	else if( ret == ERetLogin::ERL_WrongFormat ) {
		// revise prompt word in mode title
		m_Prompt = "Account or password has special character. Please try again.";
	}
	else if( ret == ERetLogin::ERL_NoAccount ) {
		// revise prompt word in mode title
		m_Prompt = "Account not exist. Please try again or switch a new account.";
	}
	else if( ret == ERetLogin::ERL_WrongPassword ) {
		// revise prompt word in mode title
		m_Prompt = "Password is wrong. Please try again.";
	}
	else if( ret == ERetLogin::ERL_AlreadyLogin ) {
		// revise prompt word in mode title
		m_Prompt = "There is user currently login. Please try again.";
	}
	else {
		// revise prompt word in mode title
		m_Prompt = "Something else failed. Please try again.";
	}
}

void Console::RegistMenu()
{
	// record user's input: 
	// if user type exit(), return to welcome menu.
	vector<string> strInput( 3 ); // account, password and deposit
	for( INT ni = 0; ni < strInput.size(); ni++ ) {

		if( ni == 0 ) {
			cout << "Regist account: " << endl;
		}
		else if ( ni == 1){
			cout << "Regist password: " << endl;
		}
		else {
			cout << "Pre-deposit: " << endl;
		}

		// attain user's input in console
		getline( cin, strInput[ ni ] );

		// detect if input is special keyword
		// user want to go back to main menu
		if( strInput[ ni ] == "exit()" ) {

			// clean prompt word
			m_Prompt = "";

			// switch menu state
			m_Menu = EMenu::EM_WELCOME;
			m_MenuHandle = std::bind( &Console::WelcomeMenu, this );

			// disconnect with bank client
			m_pConnect.reset();
			return;
		}
	}

	// try to Login
	if( m_pConnect->Regist( strInput[ 0 ], strInput[ 1 ], strInput[ 2 ] ) == false ) {
			
		// revise prompt word in mode title
		m_Prompt = "Regist failed. Please try again.";
	}
	// successfully regist, go back to login menu
	else {

		// revise prompt word in mode title
		m_Prompt = "Regist successful!";
			
		m_Menu = EMenu::EM_LOGIN;
		m_MenuHandle = std::bind( &Console::LoginMenu, this );
	}
}

void Console::MainMenu()
{
	// record user's input:
	string input;
	getline( cin, input );

	// clean prompt
	m_Prompt = "";

	// switch to inquire menu
	if( input == "inquire()" ) {

		// switch menu state
		m_Menu = EMenu::EM_INQUIRE;
		m_MenuHandle = std::bind( &Console::InquireMenu, this );
		return;

	}
	// switch to withdraw menu
	else if( input == "withdraw()" ) {

		// switch menu state
		m_Menu = EMenu::EM_WITHDRAW;
		m_MenuHandle = std::bind( &Console::WithdrawMenu, this );
		return;

	}
	// switch to deposit menu
	else if( input == "deposit()" ) {

		// switch menu state
		m_Menu = EMenu::EM_DEPOSIT;
		m_MenuHandle = std::bind( &Console::DepositMenu, this );
		return;

	}
	// switch to exchange menu
	else if( input == "exchange()" ) {

		// switch menu state
		m_Menu = EMenu::EM_EXCHANGE;
		m_MenuHandle = std::bind( &Console::ExchangeMenu, this );
		return;

	}
	// switch to transfer menu
	else if( input == "transfer()" ) {

		// switch menu state
		m_Menu = EMenu::EM_TRANSFER;
		m_MenuHandle = std::bind( &Console::TransferMenu, this );
		return;

	}
	// exit and log out
	else if( input == "logout()" ) {

		// clean prompt word
		m_Prompt = "Logout success!";

		// logout and disconnect with bank client
		m_pConnect->Logout();
		m_pConnect.reset();

		// switch to welcome menu
		m_Menu = EMenu::EM_WELCOME;
		m_MenuHandle = std::bind( &Console::WelcomeMenu, this );
	}
	// non-recognized command, just refresh
	else {
		m_Prompt = "";
	}
}

void Console::InquireMenu()
{
	cout << endl;

	// iterate through each currency in account
	enum_for_each<ECurrencyCode>( [&]( auto val ) {

		constexpr ECurrencyCode currencyCode = val;
		if( currencyCode != ECurrencyCode::ECC_NODEFINE ) {

			// attain balance to output
			DOUBLE balance = 0;
			m_pConnect->Inquire( val, balance );
			cout << enum_name( currencyCode ) << ": "  << format( "{}", balance ) << endl;

		}

	} );

	// switch menu state
	m_Menu = EMenu::EM_MAIN;
	m_MenuHandle = std::bind( &Console::MainMenu, this );

	cout << endl;
	cout << "Press any key to return to function Menu... " << endl;
	cout << endl;

	// waiting for use press key
	_getch();
}

void Console::WithdrawMenu()
{
	// record user's input: 
	// if user type show(), show current currency
	// if user type exit(), return to main menu.
	vector<string> strInput( 2 ); // currency and withdraw cash
	for( INT ni = 0; ni < strInput.size(); ni++ ) {

		if( ni == 0 ) {
			cout << "Currency: " << endl;
		}
		else {
			cout << "Withdraw how much: " << endl;
		}

		// attain user's input in console
		getline( cin, strInput[ ni ] );

		// detect if input is special keyword
		// user want to go back to main menu
		if( strInput[ ni ] == "exit()" ) {

			// clean prompt word
			m_Prompt = "";

			// switch menu state
			m_Menu = EMenu::EM_MAIN;
			m_MenuHandle = std::bind( &Console::MainMenu, this );

			return;
		}
		else if ( strInput[ ni ] == "show()" ) {

			// clean prompt word
			m_Prompt = "";

			// iterate through each currency in account
			stringstream ss;
			enum_for_each<ECurrencyCode>( [&]( auto val ) {

				constexpr ECurrencyCode currencyCode = val;
				if( currencyCode != ECurrencyCode::ECC_NODEFINE ) {

					// attain balance to output
					DOUBLE balance = 0;
					m_pConnect->Inquire( val, balance );
					ss << enum_name( currencyCode ) << ": " << format( "{}", balance ) << endl;

				}

			} );

			// push balance result into prompt string
			m_Prompt = move( ss.str() );

			return;
		}
	}

	// analysis currency input
	ECurrencyCode currency = ECurrencyCode::ECC_NODEFINE;

	if( strInput[ 0 ] == "1" ) {
		currency = ECurrencyCode::ECC_NTD;
	}
	else if( strInput[ 0 ] == "2" ) {
		currency = ECurrencyCode::ECC_USD;
	}
	else if( strInput[ 0 ] == "3" ) {
		currency = ECurrencyCode::ECC_CNY;
	}

	// try to withdraw and record result
	DOUBLE remainBalance = 0;
	BOOl result = m_pConnect->Withdraw( currency, strInput[1], remainBalance );

	// no matter success or not, switch back to main menu
	// if success, additionally show current balance w.r.t withdraw currency.
	if( result == true ) {

		stringstream ss;
		ss << "Withdraw success! Current balance: " << enum_name( currency ) << " " << format( "{}", remainBalance );
		
		// sucess prompt
		m_Prompt = move( ss.str() );
	}
	else {
		// fail prompt
		m_Prompt = "Withdraw failed!";
	}

	// switch back to main menu
	m_Menu = EMenu::EM_MAIN;
	m_MenuHandle = std::bind( &Console::MainMenu, this );

	return;
}

void Console::DepositMenu()
{
	// record user's input: 
	// if user type exit(), return to main menu.
	vector<string> strInput( 2 ); // currency and deposit cash
	for( INT ni = 0; ni < strInput.size(); ni++ ) {

		if( ni == 0 ) {
			cout << "Currency: " << endl;
		}
		else {
			cout << "Deposit how much: " << endl;
		}

		// attain user's input in console
		getline( cin, strInput[ ni ] );

		// detect if input is special keyword
		// user want to go back to main menu
		if( strInput[ ni ] == "exit()" ) {

			// clean prompt word
			m_Prompt = "";

			// switch menu state
			m_Menu = EMenu::EM_MAIN;
			m_MenuHandle = std::bind( &Console::MainMenu, this );

			return;
		}		
	}

	// analysis currency input
	ECurrencyCode currency = ECurrencyCode::ECC_NODEFINE;

	if( strInput[ 0 ] == "1" ) {
		currency = ECurrencyCode::ECC_NTD;
	}
	else if( strInput[ 0 ] == "2" ) {
		currency = ECurrencyCode::ECC_USD;
	}
	else if( strInput[ 0 ] == "3" ) {
		currency = ECurrencyCode::ECC_CNY;
	}

	// try to deposit and record result
	BOOl result = m_pConnect->Deposit( currency, strInput[ 1 ] );

	// no matter success or not, switch back to main menu
	// if success, additionally show current balance w.r.t withdraw currency.
	if( result == true ) {
		// sucess prompt
		m_Prompt = "Deposit success!";
	}
	else {
		// fail prompt
		m_Prompt = "Deposit failed!";
	}

	// switch back to main menu
	m_Menu = EMenu::EM_MAIN;
	m_MenuHandle = std::bind( &Console::MainMenu, this );

	return;
}

void Console::ExchangeMenu()
{
	// record user's input: 
	// if user type show(), show current currency
	// if user type exit(), return to main menu.
	vector<string> strInput( 3 ); // out currency, in currency, cash
	for( INT ni = 0; ni < strInput.size(); ni++ ) {

		if( ni == 0 ) {
			cout << "Exchange from which currency: " << endl;
		}
		else if( ni == 1){
			cout << "Exchange to which currency: " << endl;
		}
		else {
			cout << "Exchange how much: " << endl;
		}

		// attain user's input in console
		getline( cin, strInput[ ni ] );

		// detect if input is special keyword
		// user want to go back to main menu
		if( strInput[ ni ] == "exit()" ) {

			// clean prompt word
			m_Prompt = "";

			// switch menu state
			m_Menu = EMenu::EM_MAIN;
			m_MenuHandle = std::bind( &Console::MainMenu, this );

			return;
		}
		else if( strInput[ ni ] == "show()" ) {

			// clean prompt word
			m_Prompt = "";

			// iterate through each currency in account
			stringstream ss;
			enum_for_each<ECurrencyCode>( [&]( auto val ) {

				constexpr ECurrencyCode currencyCode = val;
				if( currencyCode != ECurrencyCode::ECC_NODEFINE ) {

					// attain balance to output
					DOUBLE balance = 0;
					m_pConnect->Inquire( val, balance );
					ss << enum_name( currencyCode ) << ": " << format( "{}", balance ) << endl;

				}

				} );

			// push balance result into prompt string
			m_Prompt = move( ss.str() );

			return;
		}
	}

	// analysis currency input
	ECurrencyCode out = ECurrencyCode::ECC_NODEFINE;
	ECurrencyCode in = ECurrencyCode::ECC_NODEFINE;

	if( strInput[ 0 ] == "1" ) {
		out = ECurrencyCode::ECC_NTD;
	}
	else if( strInput[ 0 ] == "2" ) {
		out = ECurrencyCode::ECC_USD;
	}
	else if( strInput[ 0 ] == "3" ) {
		out = ECurrencyCode::ECC_CNY;
	}

	if( strInput[ 1 ] == "1" ) {
		in = ECurrencyCode::ECC_NTD;
	}
	else if( strInput[ 1 ] == "2" ) {
		in = ECurrencyCode::ECC_USD;
	}
	else if( strInput[ 1 ] == "3" ) {
		in = ECurrencyCode::ECC_CNY;
	}

	// try to exchange and record result
	BOOl result = m_pConnect->Exchange( out, in, strInput[ 2 ] );

	// no matter success or not, switch back to main menu
	// if success, additionally show current balance w.r.t exchange currency.
	if( result == true ) {

		DOUBLE outBalance = 0;
		DOUBLE inBalance = 0;

		m_pConnect->Inquire( out, outBalance );
		m_pConnect->Inquire( in, inBalance );

		stringstream ss;
		ss << "Exchange success! Current balance: " << enum_name( out ) << " " << format( "{}", outBalance ) << " " << enum_name( in ) << " " << format( "{}", inBalance ) << endl;

		// sucess prompt
		m_Prompt = move( ss.str() );
	}
	else {
		// fail prompt
		m_Prompt = "Exchange failed!";
	}

	// switch back to main menu
	m_Menu = EMenu::EM_MAIN;
	m_MenuHandle = std::bind( &Console::MainMenu, this );

	return;
}

void Console::TransferMenu()
{
	// record user's input: 
	// if user type show(), show current currency
	// if user type exit(), return to main menu.
	vector<string> strInput( 5 ); // target bank, target account, out currency, in currency, cash value
	for( INT ni = 0; ni < strInput.size(); ni++ ) {

		if( ni == 0 ) {
			cout << "Which bank: " << endl;
		}
		else if( ni == 1 ) {
			cout << "Which account: " << endl;
		}
		else if( ni == 2 ) {
			cout << "Transfer from which currency: " << endl;
		}
		else if( ni == 3 ) {
			cout << "Transfer to which currency: " << endl;
		}
		else {
			cout << "How much: " << endl;
		}

		// attain user's input in console
		getline( cin, strInput[ ni ] );

		// detect if input is special keyword
		// user want to go back to main menu
		if( strInput[ ni ] == "exit()" ) {

			// clean prompt word
			m_Prompt = "";

			// switch menu state
			m_Menu = EMenu::EM_MAIN;
			m_MenuHandle = std::bind( &Console::MainMenu, this );

			return;
		}
		else if( strInput[ ni ] == "show()" ) {

			// clean prompt word
			m_Prompt = "";

			// iterate through each currency in account
			stringstream ss;
			enum_for_each<ECurrencyCode>( [&]( auto val ) {

				constexpr ECurrencyCode currencyCode = val;
				if( currencyCode != ECurrencyCode::ECC_NODEFINE ) {

					// attain balance to output
					DOUBLE balance = 0;
					m_pConnect->Inquire( val, balance );
					ss << enum_name( currencyCode ) << ": " << format( "{}", balance ) << endl;

				}

				} );

			// push balance result into prompt string
			m_Prompt = move( ss.str() );

			return;
		}
	}

	// analysis currency input
	ECurrencyCode out = ECurrencyCode::ECC_NODEFINE;
	ECurrencyCode in = ECurrencyCode::ECC_NODEFINE;

	if( strInput[ 2 ] == "1" ) {
		out = ECurrencyCode::ECC_NTD;
	}
	else if( strInput[ 2 ] == "2" ) {
		out = ECurrencyCode::ECC_USD;
	}
	else if( strInput[ 2 ] == "3" ) {
		out = ECurrencyCode::ECC_CNY;
	}

	if( strInput[ 3 ] == "1" ) {
		in = ECurrencyCode::ECC_NTD;
	}
	else if( strInput[ 3 ] == "2" ) {
		in = ECurrencyCode::ECC_USD;
	}
	else if( strInput[ 3 ] == "3" ) {
		in = ECurrencyCode::ECC_CNY;
	}

	// analysis bank code
	EBankCode bank = EBankCode::EBC_NoneExist;

	if( strInput[ 0 ] == "1" ) {
		bank = EBankCode::EBC_Alishan;
	}
	else if( strInput[ 0 ] == "2" ) {
		bank = EBankCode::EBC_XueMountain;
	}
	else if( strInput[ 0 ] == "3" ) {
		bank = EBankCode::EBC_GreenIsland;
	}
	else if( strInput[ 0 ] == "4" ) {
		bank = EBankCode::EBC_XiaoLiuqiu;
	}

	// try to transfer and record result
	BOOl result = m_pConnect->Transfer( out, in, bank, strInput[ 1 ], strInput[ 4 ] );

	// no matter success or not, switch back to main menu
	// if success, additionally show current balance w.r.t transfer currency.
	if( result == true ) {

		DOUBLE balance = 0;
		m_pConnect->Inquire( out, balance );

		stringstream ss;
		ss << "Transfer success! Current balance: " << enum_name( out ) << " " << format( "{}", balance ) << endl;

		// sucess prompt
		m_Prompt = move( ss.str() );
	}
	else {
		// fail prompt
		m_Prompt = "Transfer failed!";
	}

	// switch back to main menu
	m_Menu = EMenu::EM_MAIN;
	m_MenuHandle = std::bind( &Console::MainMenu, this );

	return;
}