#pragma once

// standard header
#include "map"
#include "string"
#include "functional"
#include "memory"

// project header
#include "MenuCode.h"
#include "IBankClient.h"

class Console
{

public:

	Console();

	//! @brief Output string context to IO manipulation. Default is console.
	//! @param [in] str Context to output. 
	void Output( std::string&& str );

private:

	//! @brief Integration method that is needed to launch the console program .
	void Launch();

	//! @brief Refresh current content in the console. Clear first and show the output.
	//! @param str 
	void Refresh( std::string&& str );

	//! @brief Return illustration word in each mode.  
	std::string ModeTitle( EMenu mode ) const;

	//! @brief Operation handle of welcome menu.
	void WelcomeMenu();

	//! @brief Operation handle of login menu.
	void LoginMenu();

	//! @brief Operation handle of regist menu.
	void RegistMenu();

	//! @brief Operation handle of function main menu.
	void MainMenu();

	//! @brief Operation handle of inquire menu.
	void InquireMenu();

	//! @brief Operation handle of inquire menu.
	void WithdrawMenu();

	//! @brief Operation handle of inquire menu.
	void DepositMenu();

	//! @brief Operation handle of inquire menu.
	void ExchangeMenu();

	//! @brief Operation handle of inquire menu.
	void TransferMenu();

	//! @brief 
	BOOl m_bShutDown;

	//! @brief Current page that user is browsing.
	EMenu m_Menu;

	//! @brief Prompt word in mode title.
	std::string m_Prompt;

	//! @brief User specify connected bank
	std::unique_ptr<IBankClient> m_pConnect;

	//! @brief 
	std::function<void( void )> m_MenuHandle;
};
