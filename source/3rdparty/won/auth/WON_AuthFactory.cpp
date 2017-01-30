// WON_AuthFactory

// Static class resposible for allocating and deleting class instances within the
// Auth DLL.  Other WONAuth DLL classes have private constructors/destructor that
// are accessible to the factory (it's a friend).

// *NOTE*
// This class is static and cannot be instanciated.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include "AuthFamilyBuffer.h"
#include "Auth1Certificate.h"
#include "Auth1PublicKeyBlock.h"
#include "WON_AuthCertificate1.h"
#include "WON_AuthPublicKeyBlock1.h"
#include "WON_AuthFactory.h"


// ** WON_AuthCertificate1 Methods **

// Default construct with option user info.
WON_AuthCertificate1*
WON_AuthFactory::NewAuthCertificate1(unsigned long theUserId, unsigned long theCommunityId,
                                     unsigned short theTrustLevel)
{
	return new WON_AuthCertificate1(theUserId, theCommunityId, theTrustLevel);
}


// Construct from raw form
WON_AuthCertificate1*
WON_AuthFactory::NewAuthCertificate1(const unsigned char* theRawP, unsigned short theLen)
{
	return new WON_AuthCertificate1(theRawP, theLen);
}


// Copy construct
WON_AuthCertificate1*
WON_AuthFactory::NewAuthCertificate1(const WON_AuthCertificate1* theCertP)
{
	return (theCertP ? new WON_AuthCertificate1(*theCertP) : NULL);
}


// Destruct
void
WON_AuthFactory::DeleteAuthCertificate1(WON_AuthCertificate1* theCertP)
{
	delete theCertP;
}


// ** WON_AuthPublicKeyBlock1 Methods **

// Default construct
WON_AuthPublicKeyBlock1*
WON_AuthFactory::NewAuthPublicKeyBlock1()
{
	return new WON_AuthPublicKeyBlock1;
}


// Construct from raw form
WON_AuthPublicKeyBlock1*
WON_AuthFactory::NewAuthPublicKeyBlock1(const unsigned char* theRawP, unsigned short theLen)
{
	return new WON_AuthPublicKeyBlock1(theRawP, theLen);
}


// Copy construct
WON_AuthPublicKeyBlock1*
WON_AuthFactory::NewAuthPublicKeyBlock1(const WON_AuthPublicKeyBlock1* theBlockP)
{
	return (theBlockP ? new WON_AuthPublicKeyBlock1(*theBlockP) : NULL);
}


// Destruct
void
WON_AuthFactory::DeleteAuthPublicKeyBlock1(WON_AuthPublicKeyBlock1* theBlockP)
{
	delete theBlockP;
}
