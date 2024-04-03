#pragma once

#include "gmock/gmock.h"
#include "TypeDefine.h"
#include "IDataSerializer.h"

class CMockDataSerializer : public IDataSerializer
{

public:

	MOCK_METHOD( BOOl, Serialize, ( const std::string& filePath, const std::vector<TBankAccount>& bankAccounts ), ( override ) );
	MOCK_METHOD( BOOl, Deserialize, ( const std::string& filePath, std::vector<TBankAccount>& bankAccounts ), ( override ) );
};