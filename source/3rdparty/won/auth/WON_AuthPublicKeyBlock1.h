#ifndef _WON_AuthPublicKeyBlock1_H
#define _WON_AuthPublicKeyBlock1_H

// WON_AuthPublicKeyBlock1

// Interface class for the Auth PublicKeyBlock for WON.  Encapsulates a list of one
// or more ElGamal public keys.  

// The first key in the key block is the most recent and should be considered the
// only valid key.  Other keys in the block are still valid but are being
// obsoleted.  Always use the first key for encryption.  Singature verification
// should be performed first with the first key and, if the verify fails, any
// additional keys in the block.

// **IMPORTANT NOTE**
// The Auth keyBlock is meant ot only be gnerated by the WON AuthServer.  Thus there
// is not interface to generate a new key block in this class.  A key block may be
// constructed from raw data and its members read.  No setting or buuilding.

// Note that the mBlockP member is simply a pointer to the allocated block stored
// in the base class.  Ergo, mBlockP does not need to be deleted.

// *IMPORTANT**
// Implementation relies on the fact that mBlockP will NEVER be NULL after construction.


#include "WON_AuthFamilyBuffer.h"

// Forwards
class WON_AuthCertificate1;


class DLL_OPER WON_AuthPublicKeyBlock1 : public WON_AuthFamilyBuffer
{
public:
	// Types
	struct PubKeyReturn {
		const unsigned char* mKeyP;
		unsigned short       mKeyLen;
		PubKeyReturn() : mKeyP(NULL), mKeyLen(0) {}
	};

	// Operators
	WON_AuthPublicKeyBlock1& operator=(const WON_AuthPublicKeyBlock1& theBlockR);

	// Block ID access
	unsigned short GetBlockId() const;

	// Veify an Auth certificate using each key in turn.  Returns true if any
	// key verifies certificate, false if no key verfies.
	int VerifyCertificate(const WON_AuthCertificate1* theCertP) const;

	// PublicKey access.  First returns (key,len) of first key.  Next returns
	// (key,len) of next key or (NULL,0) if no more keys.
	int GetNumKeys() const;
	PubKeyReturn GetFirstKey() const;
	PubKeyReturn GetNextKey() const;

private:
	// Time to trick the compiler.  Need mBlockP to point to a class in the WONAuth
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mBlockP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mCertP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	WONAuth::Auth1PublicKeyBlock* mBlockP;  // Real PubKeyBlock object
	WONAuth::Auth1PublicKeyBlock::PublicKeyList::iterator* mItrP;  // Real Iterator
#else
	void* mBlockP;
	void* mItrP;
#endif

	// Default constructor.  Resulting object is not valid.  Must call Unpack()
	WON_AuthPublicKeyBlock1();

	// Construct from raw representation (calls Unpack)
	WON_AuthPublicKeyBlock1(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	WON_AuthPublicKeyBlock1(const WON_AuthPublicKeyBlock1& theBlockR);

	// Destructor
	~WON_AuthPublicKeyBlock1();

	// Factory class is a friend
	friend class WON_AuthFactory;
};


#endif