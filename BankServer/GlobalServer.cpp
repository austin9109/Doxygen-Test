#include "thread"
#include "conio.h"

#include "SocketServerProtocal.h"
#include "SocketClientProtocal.h"
#include "JsonSerializer.h"
#include "GlobalServer.h"

using namespace std;

CGlobalServer::CGlobalServer()
{
	m_banks[ BankCode::Alishan ] = make_shared<CBank>( BankCode::Alishan, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );
	m_banks[ BankCode::GreenIsland ] = make_shared<CBank>( BankCode::GreenIsland, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );
	m_banks[ BankCode::XiaoLiuqiu ] = make_shared<CBank>( BankCode::XiaoLiuqiu, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );
	m_banks[ BankCode::XueMountain ] = make_shared<CBank>( BankCode::XueMountain, make_unique<CJsonSerializer>(), make_unique<CSocketServerProtocal>(), make_unique<CSocketClientProtocal>() );

	Launch();
}

void CGlobalServer::Launch()
{
	std::vector<std::thread> workers;

	// bank thread: Alishan
	workers.push_back( std::thread( [ this ] {

		m_banks[ BankCode::Alishan ]->LaunchServer();

	} ) );

	// bank thread: Green Island
	workers.push_back( std::thread( [this] {

		m_banks[ BankCode::GreenIsland ]->LaunchServer();

	} ) );


	// keyboard event thread
	while( TRUE ) {

		if( _kbhit() ) {

			int key = _getch();

			if( key == 'q' ) {

				// stop all thread of m_banks
				m_banks[ BankCode::Alishan ]->CloseServer();
				m_banks[ BankCode::GreenIsland ]->CloseServer();
				break;
			}
		}
	}

	// join all thread.
	for( auto& worker : workers ) {
		worker.join();
	}
}
