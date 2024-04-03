#pragma once

#include "gmock/gmock.h"
#include "TypeDefine.h"
#include "IServerProtocal.h"

class CMockServerProtocal : public IServerProtocal
{

public:

	MOCK_METHOD( BOOl, Launch, ( EBankCode bankCode ), ( override ) );
	MOCK_METHOD( BOOl, RegistCB, ( std::function< BOOl( std::string&&, std::string& ) >&& callback ), ( override ) );
	MOCK_METHOD( BOOl, Close, ( ), ( override ) );
};