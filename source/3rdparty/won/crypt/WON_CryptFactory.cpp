// WON_CryptFactory

// Static class resposible for allocating and deleting class instances within the
// Crypt DLL.  Other WONCrypt DLL classes have private constructors/destructor that
// are accessible to the factory (it's a friend).

// *NOTE*
// This class is static and cannot be instanciated.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include "CryptKeyBase.h"
#include "BFSymmetricKey.h"
#include "EGPublicKey.h"
#include "EGPrivateKey.h"
#include "WON_BFSymmetricKey.h"
#include "WON_EGPublicKey.h"
#include "WON_EGPrivateKey.h"
#include "WON_CryptFactory.h"


// ** WON_BFSymmectricKey Methods **

// Default Constructor, optionally load from raw form
WON_BFSymmetricKey*
WON_CryptFactory::NewBFSymmetricKey(unsigned short theLen, const unsigned char* theRawP)
{
	return new WON_BFSymmetricKey(theLen, theRawP);
}


// Copy construct
WON_BFSymmetricKey*
WON_CryptFactory::NewBFSymmetricKey(const WON_BFSymmetricKey* theKeyP)
{
	return (theKeyP ? new WON_BFSymmetricKey(*theKeyP) : NULL);
}


// Destruct
void
WON_CryptFactory::DeleteBFSymmetricKey(WON_BFSymmetricKey* theKeyP)
{
	delete theKeyP;
}


// ** WON_EGPublicKey Methods **

// Default Constructor, optionally load from raw form
WON_EGPublicKey*
WON_CryptFactory::NewEGPublicKey(unsigned short theLen, const unsigned char* theRawP)
{
	return new WON_EGPublicKey(theLen, theRawP);
}


// Construct from private key
WON_EGPublicKey*
WON_CryptFactory::NewEGPublicKey(const WON_EGPrivateKey* theKeyP)
{
	return (theKeyP ? new WON_EGPublicKey(*theKeyP) : NULL);
}


// Copy construct
WON_EGPublicKey*
WON_CryptFactory::NewEGPublicKey(const WON_EGPublicKey* theKeyP)
{
	return (theKeyP ? new WON_EGPublicKey(*theKeyP) : NULL);
}


// Destruct
void
WON_CryptFactory::DeleteEGPublicKey(WON_EGPublicKey* theKeyP)
{
	delete theKeyP;
}


// ** WON_EGPrivateKey Methods **

// Default Constructor, optionally load from raw form
WON_EGPrivateKey*
WON_CryptFactory::NewEGPrivateKey(unsigned short theLen, const unsigned char* theRawP)
{
	return new WON_EGPrivateKey(theLen, theRawP);
}


// Copy construct
WON_EGPrivateKey*
WON_CryptFactory::NewEGPrivateKey(const WON_EGPrivateKey* theKeyP)
{
	return (theKeyP ? new WON_EGPrivateKey(*theKeyP) : NULL);
}


// Destruct
void
WON_CryptFactory::DeleteEGPrivateKey(WON_EGPrivateKey* theKeyP)
{
	delete theKeyP;
}
