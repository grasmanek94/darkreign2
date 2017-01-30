#ifndef _WON_AuthCertificate1_H
#define _WON_AuthCertificate1_H

// WON_AuthCertificate1

// Interface class for the Auth Certificate1 for WON.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//	User Info - UserId, CommunityId, TrustLevel
//	Client's Public Key (ElGamal)

// Used as system ticket for various authentication protocols within the WON
// system.

// Note that the mCertP member is simply a pointer to the allocated certificate stored
// in the base class.  Ergo, mCertP does not need to be deleted.

// *IMPORTANT**
// Implementation relies on the fact that mCertP will NEVER be NULL after construction.

#include "WON_AuthFamilyBuffer.h"

// Forwards
class WON_AuthPublicKeyBlock1;


class DLL_OPER WON_AuthCertificate1 : public WON_AuthFamilyBuffer
{
public:
	// Operators
	WON_AuthCertificate1& operator=(const WON_AuthCertificate1& theCertR);

	// User information access
	unsigned long  GetUserId() const;
	unsigned long  GetCommunityId() const;
	unsigned short GetTrustLevel() const;

	// Public Key access
	const unsigned char* GetPubKey() const;
	unsigned short       GetPubKeyLen() const;

	// Member update - will invalidate certificate until pack is called again
	void SetUserId(unsigned long theId);
	void SetCommunityId(unsigned long theId);
	void SetTrustLevel(unsigned short theLevel);
	void SetPublicKey(const unsigned char* theKeyP, unsigned short theLen);

private:
	// Time to trick the compiler.  Need mCertP to point to a class in the WONAuth
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mCertP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mCertP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	WONAuth::Auth1Certificate* mCertP;  // Real AuthCertificate1 object
#else
	void* mCertP;
#endif

	// Default constructor - may provide user info
	WON_AuthCertificate1(unsigned long theUserId=0, unsigned long theCommunityId=0,
	                     unsigned short theTrustLevel=0);

	// Construct from raw representation (calls Unpack())
	WON_AuthCertificate1(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	WON_AuthCertificate1(const WON_AuthCertificate1& theCertR);

	// Destructor
	~WON_AuthCertificate1();

	// Factory and keyblock classes are friends
	friend class WON_AuthFactory;
	friend class WON_AuthPublicKeyBlock1;
};


#endif