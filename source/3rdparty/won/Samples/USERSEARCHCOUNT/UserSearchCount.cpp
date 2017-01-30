#pragma warning (disable : 4786)
#include <iostream>
#include "UserSearch/UserSearchCounts.h"

using namespace std;
using namespace WONAPI;
using namespace WONMsg;

void Usage()
	{
	cout << "UserSearchCount.exe <server address> <server port> <category>" << endl;
	}

void main(int argc, char** argv)
	{
	// Need at least three parameters

	if (argc < 4)
		{
		Usage();

		return;
		}

	const char* server_address = argv[1];

	unsigned long server_port = atoi(argv[2]);

	string category_name = argv[3];

	IPSocket::Address server_address_object(server_address, 
											server_port);

	IPSocket::Address auth_server_address_object("master1.west.won.net", 
											     15200);

	CompletionContainer<const UserSearchServerCountResult&> result_object;

	Identity identity = Identity("neeraj", 
								 "Admin",
								 PW, 
								 "", 
								 auth_server_address_object);

	Error aError = identity.Authenticate(false , false, 20000);

	if (aError == StatusCommon_Success)
		{
		cout << "\n\n\nResult is: " 
			 << UserSearchGetNumberOfUsers(/*&identity*/ NULL,
										   &server_address_object,
										   1,
										   WONCommon::StringToWString(category_name),
										   20000,
										   false,
										   result_object).m_result
			 << ".\n\n";
		}
	}