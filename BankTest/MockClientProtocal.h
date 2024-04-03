#pragma once

#include "gmock/gmock.h"
#include "TypeDefine.h"
#include "IClientProtocal.h"

class CMockClientProtocal : public IClientProtocal
{

public:

	MOCK_METHOD( BOOl, Launch, ( EBankCode bankCode ), ( override ) );
	MOCK_METHOD( BOOl, Send, ( std::string&& context, INT timeout, std::string& result ), ( override ) );
	MOCK_METHOD( BOOl, Close, ( ), ( override ) );
};