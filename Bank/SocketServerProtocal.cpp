// standard header
#include "iostream"
#include "map"

// windows header
#include "ws2tcpip.h"

// third party header
#include "magic_enum/magic_enum_all.hpp"

// project header
#include "SocketServerProtocal.h"

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

using namespace magic_enum;

const std::map<EBankCode, u_short> m_portMap = {
    { EBankCode::EBC_Alishan, 8885 },
    { EBankCode::EBC_XueMountain, 8886 },
    { EBankCode::EBC_GreenIsland, 8887 },
    { EBankCode::EBC_XiaoLiuqiu, 8888 },
};

CSocketServerProtocal::CSocketServerProtocal()
{
    // init winsock. This is needed to use socekt function under windows.
    WSADATA wsa = { 0 };
    if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != NO_ERROR ) {
        printf( "Error: init winsock\n" );
    }
}

CSocketServerProtocal::~CSocketServerProtocal()
{
    WSACleanup();
}

BOOl CSocketServerProtocal::Launch( EBankCode bankCode )
{
    // if already launched, return false
    if( m_launch == true ) {
        return false;
    }

    // record bankcode
    this->m_bankCode = bankCode;

    // Create a socket host:
    // AF_INET: ipv4
    // SOCK_STREAM: TCP usuge
    // return: a socket description
    // 
    // Here we can direct convert from type "SOCKET" to int, since
    // although the Microsoft documentation claims that the upper limit is INVALID_SOCKET-1 (2^64 - 2), 
    // in practice the current socket() implementation returns an index into the kernel handle table,
    //  the size of which is limited to 2^24.
    // See https://stackoverflow.com/questions/1953639/is-it-safe-to-cast-socket-to-int-under-win64 for more details
    INT master_socket = ( INT )socket( AF_INET, SOCK_STREAM, IPPROTO_HOPOPTS );
    if( master_socket == INVALID_SOCKET ) {
        printf( "[%s] socket failed with error: %ld\n", enum_name( bankCode ).data(), WSAGetLastError() );
        WSACleanup();
        return false;
    }

    // set master socket to allow multiple connections ,  
    // this is just a good habit, it will work without this  
    INT opt = TRUE;
    if( setsockopt( master_socket, SOL_SOCKET, SO_REUSEADDR, ( CHAR* )&opt, sizeof( opt ) ) < 0 ) {
        printf( "[%s] setsockopt option failed with error: %ld\n", enum_name( bankCode ).data(), WSAGetLastError() );
        WSACleanup();
        return false;
    }

    // type of socket created:
    // setting socket listen to which protocal and which port.
    struct sockaddr_in address;
    ZeroMemory( &address, sizeof( address ) ); // clear address content
    address.sin_family = AF_INET; // Ipv4. For Ipv6, use AF_INET6
    address.sin_addr.s_addr = INADDR_ANY; // listen on ip 0.0.0.0 (or any ip)
    //if( inet_pton( AF_INET, "10.10.203.35", &address.sin_addr ) == SOCKET_ERROR ) { // listen on ip 10.10.203.35
    //    perror( "configure ip address failed." );
    //    closesocket( master_socket );
    //    WSACleanup();
    //    return false;
    //}
    address.sin_port = htons( m_portMap.at( bankCode ) ); // convert from host word "8888" to network.

    printf( "[%s] Launch server at port: %d\n", enum_name( bankCode ).data(), m_portMap.at( bankCode ) );

    // bind the socket w.r.t previous configuration
    if( bind( master_socket, ( struct sockaddr* )&address, sizeof( address ) ) == SOCKET_ERROR ) {
        printf( "[%s] bind failed with error: %ld\n", enum_name( bankCode ).data(), WSAGetLastError() );
        closesocket( master_socket );
        WSACleanup();
        return false;
    }

    // try to specify maximum of pending connections for the master socket  
    if( listen( master_socket, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "[%s] Listen failed with error: %ld\n", enum_name( bankCode ).data(), WSAGetLastError() );
        closesocket( master_socket );
        WSACleanup();
        return 1;
    }

    // everything is fine, mark as launched
    printf( "[%s] Waiting for connections ...\n", enum_name( bankCode ).data() );
    m_launch = true;

    INT max_sd = 0; // maximum observer socket descriptor amount
    INT sd = 0, activity = 0, new_socket = 0, valread = 0, addrlen = sizeof( address ), client_socket[ 30 ], max_clients = 30;
   
#ifdef DEBUG
    const timeval timeout{ 5, 0 }; // 5 s
#else
    const timeval timeout{ 0, 200 };; // 200 ms
#endif
    
    CHAR buffer[ 1025 ];  // data buffer of 1K 

    // initialise all client_socket[] to 0 so not checked  
    for( INT i = 0; i < max_clients; i++ ) {
        client_socket[ i ] = 0;
    }

    while( m_launch )
    {
        // clear the socket set  
        FD_ZERO( &m_readfds );

        // add master socket to set  
        FD_SET( master_socket, &m_readfds );
        max_sd = master_socket; // record master as max_sd

        // find the maximum socket descriptor here..
        // max_sd is needed later on when calling `Select()`
        for( INT i = 0; i < max_clients; i++ )
        {
            // socket descriptor  
            sd = client_socket[ i ];

            // if valid socket descriptor then add to read list  
            if( sd > 0 ) {
                FD_SET( sd, &m_readfds );
            }

            // highest file descriptor number, need it for the select function  
            if( sd > max_sd ) {
                max_sd = sd;
            }
        }

        // wait for an activity on one of the sockets , timeout is specific by the user  
        activity = select( max_sd + 1 /*needs to be max sd + 1 (API Spec)*/, &m_readfds, NULL, NULL, &timeout );
        if( ( ( activity < 0 ) && ( errno != EINTR ) ) ) {
            printf( "[%s] Socket select failed with error: %ld\n", enum_name( bankCode ).data(), WSAGetLastError() );
            continue;
        }

        // If something happened on the master socket ,  
        // then its an incoming connection  
        if( FD_ISSET( master_socket, &m_readfds ) )
        {
            // since FD_ISSET have been triggered, this "accept" will not block the thread.
            if( ( new_socket = ( INT )accept( master_socket, ( struct sockaddr* )&address, &addrlen ) ) == SOCKET_ERROR ) {
                printf( "[%s] Socket accept failed with error: %ld\n", enum_name( bankCode ).data(), WSAGetLastError() );
                closesocket( master_socket );
                WSACleanup();
            }

            // inform user of socket number - used in send and receive commands  
            // inet_ntop: convert from binary numerical to ipv4 or ipv6 address.
            // ntohs: network word byte to host in short
            CHAR buff[ 17 ];
            printf( "[%s] New connection , socket fd is %d , ip is : %s , port : %d  \n", enum_name( bankCode ).data(), new_socket, inet_ntop( AF_INET, ( const void* )&address.sin_addr, buff, sizeof( buff ) ), ntohs( address.sin_port ) );

            // add new socket to array of sockets  
            for( INT i = 0; i < max_clients; i++ ) {
                //if position is empty  
                if( client_socket[ i ] == 0 )
                {
                    client_socket[ i ] = new_socket;
                    printf( "[%s] Adding to list of sockets as %d\n", enum_name( bankCode ).data(), i );
                    break;
                }
            }
        }

        // else its some IO operation on some other socket 
        for( INT i = 0; i < max_clients; i++ )
        {
            sd = client_socket[ i ];

            if( FD_ISSET( sd, &m_readfds ) )
            {
                // Check if it was for closing , and also read the incoming message  
                if( ( valread = recv( sd, buffer, sizeof( buffer ), 0 ) ) <= 0 )
                {
                    // Somebody disconnected , get his details and print  
                    getpeername( sd, ( struct sockaddr* )&address, &addrlen );
                    CHAR buff[ 17 ];
                    printf( "[%s] Client disconnected , ip %s , port %d \n", enum_name( bankCode ).data(), inet_ntop( AF_INET, ( const void* )&address.sin_addr, buff, sizeof( buff ) ), ntohs( address.sin_port ) );

                    // Close the socket and mark as 0 in list for reuse  
                    closesocket( sd );
                    client_socket[ i ] = 0;
                }
                // Echo back the message that came in  
                else
                {
                    // set the string terminating NULL byte on the end  
                    // of the data read                      
                    buffer[ valread ] = '\0';
                    printf( "[%s] Sock recv with %d words: , content: %s\n", enum_name( bankCode ).data(), valread, buffer );

                    // resolve the word
                    std::string context( buffer, strlen( buffer ) );
                    std::string result;
                    m_eventHandle( std::move( context ), result );

                    // send result word!
                    send( sd, result.c_str(), ( INT )result.size(), 0 );
                }
            }
        }
    }

    printf( "[%s] Close socket success.\n", enum_name( bankCode ).data() );
    closesocket( master_socket );

    // reset the parameter
    bankCode = EBankCode::EBC_NoneExist;

    return true;
}

BOOl CSocketServerProtocal::Close()
{
    if( m_launch == true ) {
        m_launch = false;
        return true;
    }
    else {
        return false;
    }
}

BOOl CSocketServerProtocal::RegistCB( std::function<BOOl( std::string&&, std::string& )>&& callback )
{
    m_eventHandle = std::move( callback );
    return true;
}