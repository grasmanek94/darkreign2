#ifndef _WON_AUTHFACTORY_H
#define _WON_AUTHFACTORY_H

// WON_AuthFactory

// Static class resposible for allocating and deleting class instances within the
// Auth DLL.  Other WONAuth DLL classes have private constructors/destructor that
// are accessible to the factory (it's a friend).

// *NOTE*
// This class is static and cannot be instanciated.


// Export or import to/from DLL
#ifdef _WON_SOURCE_
#define DLL_OPER __declspec(dllexport)
#else
#define DLL_OPER __declspec(dllimport)
#endif

// Forwards
class WON_AuthCertificate1;
class WON_AuthPublicKeyBlock1;


class DLL_OPER WON_AuthFactory
{
public:
	// WON_AuthCertificate1 default construction.  May specify optional User info
	static WON_AuthCertificate1* NewAuthCertificate1(
		unsigned long theUserId=0, unsigned long theCommunityId=0,
		unsigned short theTrustLevel=0
	);

	// WON_AuthCertificate1 construction from raw representation.
	// Calls Unpack()
	static WON_AuthCertificate1* NewAuthCertificate1(
		const unsigned char* theRawP, unsigned short theLen
	);

	// WON_AuthCertificate1 copy construction
	static WON_AuthCertificate1* NewAuthCertificate1(
		const WON_AuthCertificate1* theCertP
	);

	// Dealloc WON_AuthCertificate1
	static void DeleteAuthCertificate1(WON_AuthCertificate1* theCertP);


	// WON_AuthPublicKeyBlock1 default construction.  Created object is not valid.
	// Must call Unpack() to make valid instance.
	static WON_AuthPublicKeyBlock1* NewAuthPublicKeyBlock1();

	// WON_AuthPublicKeyBlock1 construction from raw representation.
	// Calls Unpack()
	static WON_AuthPublicKeyBlock1* NewAuthPublicKeyBlock1(
		const unsigned char* theRawP, unsigned short theLen
	);

	// WON_AuthPublicKeyBlock1 copy construction
	static WON_AuthPublicKeyBlock1* NewAuthPublicKeyBlock1(
		const WON_AuthPublicKeyBlock1* theBlockP
	);

	// Dealloc WON_AuthPublicKeyBlock1
	static void DeleteAuthPublicKeyBlock1(WON_AuthPublicKeyBlock1* theBlockP);


private:
	// Static class, disable construction
	WON_AuthFactory();
};


#endif