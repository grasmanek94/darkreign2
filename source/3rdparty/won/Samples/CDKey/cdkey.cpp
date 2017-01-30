#pragma warning (disable : 4786)
#include <iostream>
#include "Authentication/Identity.h"

using namespace std;
using namespace WONAPI;

void Usage()
{
	cout << "cdkey.exe <product-name> <cd-key-string> [<community-name>]" << endl;
}

int main(int argc, char** argv)
{
	// Need at least two parameters
	if (argc < 3)
	{
		Usage();
		return 1;
	}
	
	//
	// 
	// PART 1: To be done during install or on first run of the game.
	//
	//	
	const char* aProductName = argv[1];

	// Obtain the cd key string (CVC#-CVC#-CVC#-CVC#-####) from the user
	// In this example, it comes in as argv[2].
	const char* aCDKeyString = argv[2];

	// Validate the string.  For beta builds, allow beta keys (IsBeta() == true).
	// For retail builds, don't.
	WONCDKey::ClientCDKey aCDKeyFromUser(aProductName);
	aCDKeyFromUser.Init(aCDKeyString);
	if (/*aCDKeyFromUser.IsBeta() ||*/ !aCDKeyFromUser.IsValid())
	{
		cout << "CD key is invalid" << endl;
		return 1;
	}

	// Store the valid CD key in the registry (encrypted)
	aCDKeyFromUser.Save();

	//
	//
	// PART 2: To be done once per session if the user will login.
	//
	//

	// Get the CD key from the registry
	WONCDKey::ClientCDKey aCDKeyFromRegistry(aProductName);
	aCDKeyFromRegistry.Load();

	// Set up the Authentication API for CD key usage
	Identity::SetCDKey(aCDKeyFromRegistry);
	Identity::SetLoginKeyFile("login.ks");
	Identity::LoadVerifierKeyFromFile("kver.pub");

	//
	//
	// PART 3: Test it out...
	//
	//

	// Get address of Auth Servers.
	// **** Note: Please, please, please, do not use this code in your game.  Instead, 
	// **** get the Auth Servers from a Directory Server as shown in the matchmaker 
	// **** sample.  I'm only doing this here for sample code simplicity.
	IPSocket::Address aAuthAddr("auth.west.won.net", 15200);

	// Login
	const char* aCommunityName = (argc == 3) ? aProductName : argv[3];
	Identity aIdentity("SampleUser", aCommunityName, "eriksample", "", &aAuthAddr, 1);
	Error aError = aIdentity.Authenticate(false, false, -1);
	cout << "Result: " << WONErrorToString(aError) << endl;

	return (aError == Error_Success ? 0 : 1);
}