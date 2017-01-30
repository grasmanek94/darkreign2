#ifndef _WON_CRYPTFACTORY_H
#define _WON_CRYPTFACTORY_H

// WON_CryptFactory

// Static class resposible for allocating and deleting class instances within the
// Crypt DLL.  Other WONCrypt DLL classes have private constructors/destructor that
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
class WON_BFSymmetricKey;
class WON_EGPublicKey;
class WON_EGPrivateKey;


class DLL_OPER WON_CryptFactory
{
public:
	// WON_BFSymmectricKey default construction.  May specify raw form to init with.
	// If key is NULL, new key of specified length in bytes is generated.  If len is
	// 0, new key of default length is generated.
	static WON_BFSymmetricKey* NewBFSymmetricKey(
		unsigned short theLen=0, const unsigned char* theKeyP=NULL
	);

	// WON_BFSymmectricKey copy construction
	static WON_BFSymmetricKey* NewBFSymmetricKey(
		const WON_BFSymmetricKey* theKeyP
	);

	// Dealloc WON_BFSymmectricKey
	static void DeleteBFSymmetricKey(WON_BFSymmetricKey* theKeyP);


	// WON_EGPublicKey default construction.  May specify raw form to init with
	// If not init from raw form, then must call Create() or assign from
	// WON_EGPrivateKey before key will be valid.
	static WON_EGPublicKey* NewEGPublicKey(
		unsigned short theLen=0, const unsigned char* theKeyP=NULL
	);

	// WON_EGPublicKey construct from WON_EGPrivateKey
	static WON_EGPublicKey* NewEGPublicKey(
		const WON_EGPrivateKey* theKeyP
	);

	// WON_EGPublicKey copy construction
	static WON_EGPublicKey* NewEGPublicKey(
		const WON_EGPublicKey* theKeyP
	);

	// Dealloc WON_EGPublicKey
	static void DeleteEGPublicKey(WON_EGPublicKey* theKeyP);


	// WON_EGPrivateKey default construction.  May specify raw form to init with.
	// If key is NULL, new key of using length in bytes as seed is generated.  If len
	// is 0, new key using default length as seed is generated.
	static WON_EGPrivateKey* NewEGPrivateKey(
		unsigned short theLen=0, const unsigned char* theKeyP=NULL
	);

	// WON_EGPublicKey copy construction
	static WON_EGPrivateKey* NewEGPrivateKey(
		const WON_EGPrivateKey* theKeyP
	);

	// Dealloc WON_EGPublicKey
	static void DeleteEGPrivateKey(WON_EGPrivateKey* theKeyP);


private:
	// Static class, disable construction
	WON_CryptFactory();
};


#endif