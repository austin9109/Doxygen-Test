#pragma once

// project header
#include "IDataSerializer.h"

//! @brief Bank data m_pSerializer that implemented with Json format.
class CJsonSerializer : public IDataSerializer
{

public:

	BOOl Serialize( const std::string& filePath, const std::vector<TBankAccount>& bankAccounts ) override;
	BOOl Deserialize( const std::string& filePath, std::vector<TBankAccount>& bankAccounts ) override;

};
