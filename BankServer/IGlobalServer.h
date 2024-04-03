#pragma once

// standard library
#include "memory"
#include "vector"
#include "map"

// project library
#include "BankCode.h"
#include "IBankService.h"
#include "ICrossBankService.h"

/// <summary>
/// Interface the global server. We design this interface to let implement class
/// to offer find bank service & cross bank service method.
/// Note that since user might access bank with erro bank code, we return pointer as Null to represent non-exist bank instead of reference here.
/// </summary>
class IGlobalServer
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~IGlobalServer() {}
};