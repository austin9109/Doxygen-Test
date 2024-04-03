#pragma once

#include "Bank.h"

/// <summary>
/// Offering default created bank within this server.
/// </summary>
class CGlobalServer
{

public:

	CGlobalServer();

private:

	void Launch();
	std::map<BankCode, std::shared_ptr<CBank>> m_banks;

};
