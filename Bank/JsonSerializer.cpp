#include "fstream"
#include "nlohmann/json.hpp"
#include "JsonSerializer.h"

using namespace std;
using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE( TBankAccount, m_account, m_password, m_balance );

BOOl CJsonSerializer::Serialize( const std::string& filePath, const std::vector<TBankAccount>& bankAccounts )
{
    ofstream out;
    out.open( filePath, ios::out );
    if( !out ) {
        return false;
    }

    json j = bankAccounts;
    out << j;

    out.close();
	return true;
}

BOOl CJsonSerializer::Deserialize( const std::string& filePath, std::vector<TBankAccount>& bankAccounts )
{
    ifstream in;
    in.open( filePath, ios::in );
    if( !in ) {
        return false;
    }

    json j = json::parse( in );
    from_json( j , bankAccounts );

    in.close();
    return true;
}