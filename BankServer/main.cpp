// standard header
#include "memory"
#include "thread"

// windows header
#include "conio.h"

// project header
#include "Bank.h"
#include "JsonSerializer.h"
#include "SocketServerProtocal.h"
#include "SocketClientProtocal.h"

using namespace std;

INT main()
{
	// create bank instance of each kind
	map<EBankCode, shared_ptr<CBank>> m_banks;
	m_banks[ EBankCode::EBC_Alishan ] = make_shared<CBank>( EBankCode::EBC_Alishan, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );
	m_banks[ EBankCode::EBC_GreenIsland ] = make_shared<CBank>( EBankCode::EBC_GreenIsland, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );
	m_banks[ EBankCode::EBC_XiaoLiuqiu ] = make_shared<CBank>( EBankCode::EBC_XiaoLiuqiu, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );
	m_banks[ EBankCode::EBC_XueMountain ] = make_shared<CBank>( EBankCode::EBC_XueMountain, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );

	// push each bank into independent thread
	std::vector<std::thread> workers;
	for( auto& bank : m_banks ) {
	
		// create a thread and throw the bank server into it
		workers.push_back( std::thread( [ & ] {
			bank.second->LaunchServer();
		} ) );
	}

	// keyboard event thread
	while( TRUE ) {

		if( _kbhit() ) {

			INT key = _getch();

			if( key == 'q' || key == 'Q' ) {

				// stop all thread of m_banks
				for( auto& bank : m_banks ) {
					bank.second->CloseServer();
				}

				break;
			}
		}
	}

	// join all thread.
	for( auto& worker : workers ) {
		worker.join();
	}
}