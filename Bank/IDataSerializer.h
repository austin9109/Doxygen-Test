#pragma once

// standard library
#include "string"
#include "vector"

// project header
#include "TypeDefine.h"
#include "BankAccount.h"

//! @brief Collection of methods of data m_pSerializer that
//! stands for read/write bank's data.
class IDataSerializer
{

public:

	//! @brief This must be declare, otherwise runtime error will occured if 
	//! delete through base pointer.
	virtual ~IDataSerializer() {}

	//! @brief Write bank's data into disk.
	//! @param filePath 
	//! @param bankAccounts 
	//! @return 
	virtual BOOl Serialize( const std::string& filePath, const std::vector<TBankAccount>& bankAccounts ) = 0;
	
	//! @brief Read bank's data from the disk.
	//! @param filePath 
	//! @param bankAccounts 
	//! @return 
	virtual BOOl Deserialize( const std::string& filePath, std::vector<TBankAccount>& bankAccounts ) = 0;

};
