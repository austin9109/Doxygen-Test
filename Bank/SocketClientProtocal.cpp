// standard library
#include "iostream"
#include "map"

// windows library
#include "winsock2.h"
#include "ws2tcpip.h"

// project library
#include "SocketClientProtocal.h"

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const std::map<EBankCode, u_short> m_portMap = {
    { EBankCode::EBC_Alishan, 8885 },
    { EBankCode::EBC_XueMountain, 8886 },
    { EBankCode::EBC_GreenIsland, 8887 },
    { EBankCode::EBC_XiaoLiuqiu, 8888 },
};

CSocketClientProtocal::CSocketClientProtocal()
{
    // init winsock. This is needed to use socekt function under windows.
    WSADATA wsa = { 0 };
    if( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != NO_ERROR ) {
        printf( "Error: init winsock\n" );
    }
}

CSocketClientProtocal::~CSocketClientProtocal()
{
    WSACleanup();
}

BOOl CSocketClientProtocal::Launch( EBankCode bankCode )
{
    // if have already launch, return
    if( m_bLaunch == true ) {
        return false;
    }

    const char* host = "127.0.0.1"; // local host
    struct sockaddr_in serv_name;
    
    // create a socket
    m_nClientSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if( m_nClientSocket == -1 ) {
        perror( "Socket creation error" );
        return false;
    }

    // defined server address
    ZeroMemory( &serv_name, sizeof( serv_name ) );
    serv_name.sin_family = AF_INET;
    inet_pton( AF_INET, host, &serv_name.sin_addr );
    serv_name.sin_port = htons( m_portMap.at( bankCode ) );

    // connect with server
    if( connect( m_nClientSocket, ( struct sockaddr* )&serv_name, sizeof( serv_name ) ) == SOCKET_ERROR ) {
        perror( "Connection error" );
        closesocket( m_nClientSocket );
        return false;
    }

    // everything is fine, mark as launched
    m_bLaunch = true;
    return true;
}

BOOl CSocketClientProtocal::Send( string&& context, int timeout, string& result )
{
    // if not launch, directly return
    if( m_bLaunch == false ) {
        return false;
    }

    char indata[ 1024 ] = { 0 }, outdata[ 1024 ] = { 0 };

    // set socket option: timeout for receive response from the server
    if( setsockopt( m_nClientSocket, SOL_SOCKET, SO_RCVTIMEO, ( char* )&timeout, sizeof( timeout ) ) == SOCKET_ERROR ) {
        perror( "setsockopt failed." );
        return false;
    }

    // connect success, send context to another bank and attain it's result
    if( send( m_nClientSocket, context.c_str(), strlen( context.c_str() ), 0 ) == SOCKET_ERROR ) {
        perror( "Communicate with server failed." );
        return false;
    }

    // resolve return words.
    int nbytes = recv( m_nClientSocket, indata, sizeof( indata ), 0 );
    if( nbytes <= 0 ) {
        printf( "server closed connection.\n" );
        return false;
    }

    // set null byte for indata
    indata[ nbytes ] = '\0';

    // set indata content to string result
    result = string( indata, strlen( indata ) );

    return true;
}

BOOl CSocketClientProtocal::Close()
{
    // if not launched, return false
    if( m_bLaunch == false ) {
        return false;
    }

    // mark as non launched
    m_bLaunch = false;

    // close socket descriptor and return result
    return closesocket( m_nClientSocket ) == SOCKET_ERROR;
}