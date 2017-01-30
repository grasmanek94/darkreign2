#ifndef _WON_EGPUBLICKEY_H
#define _WON_EGPUBLICKEY_H

// WON_EGPublicKey

// Interface class for the ElGamal implementation of a public key.  ElGamal public keys
// cannot be created by this class.  Use the ElGamal private key class to generate the
// corresponding public key..  Key can be used to encrypt binary blocks of arbitrary
// size and verify signatures on binary blocks of arbitrary size.

// This class simply makes use of the EGPublicKey class.  It eliminates the STL
// and exceptions from the interface.

// *NOTE*
// This header is compatible with VC4.  Ergo, no using bool, string, etc.


#include "WON_CryptKeyBase.h"

// Forwards
class WON_EGPrivateKey;

class DLL_OPER WON_EGPublicKey : public WON_CryptKeyBase
{
public:
	// Assignment Operators
	// Note: Assignment from private key resets lastError from base class
	WON_EGPublicKey& operator=(const WON_EGPublicKey& theKeyR);
	WON_EGPublicKey& operator=(const WON_EGPrivateKey& theKeyR);

	// Create key - from existing raw key.  Returns true on success, false on failure.
	// NOTE:  This method resets lastError from the base class.
	int Create(unsigned short theLen, const unsigned char* theKeyP);

	// Encrypt a block.  Returns (encrypted block, length of block) on success,
	// (NULL, 0) on failure.
	// NOTE:  This method resets lastError from the base class.
	CryptReturn Encrypt(const void* theMsgP, unsigned long theLen);

	// Verify a signature block.  Returns true if signature is valid, false if not.
	// NOTE:  This method resets lastError from the base class.
	int Verify(const unsigned char* theSigP, unsigned long theSigLen,
	           const void* theMsgP, unsigned long theMsgLen);

private:
	// Time to trick the compiler.  Need mEGKeyP to point to a class in the WONCrypt
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mEGKeyP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mEGKeyP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	WONCrypt::EGPublicKey* mEGKeyP;  // Real ElGamal Public key object
#else
	void* mEGKeyP;
#endif

	// Private Default Constructor - may init from raw form.  If no raw form, key
	// will be invalid until created from raw form or assigned from a private key.
	WON_EGPublicKey(unsigned short theLen=0, const unsigned char* theKey=NULL);

	// Private Constructor - from private key
	WON_EGPublicKey(const WON_EGPrivateKey& theKeyR);

	// Private Copy constructor
	WON_EGPublicKey(const WON_EGPublicKey& theKeyR);

	// Private Destructor
	~WON_EGPublicKey(void);

	// Private Methods
	void AllocateKey();

	// Friends
	friend class WON_EGPrivateKey;
	friend class WON_CryptFactory;
};


#endif