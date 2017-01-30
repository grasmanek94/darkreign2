#ifndef _WON_EGPRIVATEKEY_H
#define _WON_EGPRIVATEKEY_H

// WON_EGPrivateKey

// Interface class for the ElGamal implementation of a private key.  Length of each
// portion of the key in bytes can be defined on creation.  Key can be used to decrypt
// binary blocks of arbitrary size and sign binary blocks of arbitrary size.

// This class simply makes use of the EGPrivateKey class.  It eliminates the STL
// and exceptions from the interface.

// *NOTE*
// This header is compatible with VC4.  Ergo, no using bool, string, etc.


#include "WON_CryptKeyBase.h"

// Forwards
class WON_EGPublicKey;

class DLL_OPER WON_EGPrivateKey : public WON_CryptKeyBase
{
public:
	// Assignment Operators
	WON_EGPrivateKey& operator=(const WON_EGPrivateKey& theKeyR);

	// Create key - from existing raw key.  Returns true on success, false on failure.
	// NOTE:  This method resets lastError from the base class.
	int Create(unsigned short theLen, const unsigned char* theKeyP);

	// Create key - new key of specified length.  Specifying theLen as 0 will use
	// the default length.  Returns true on success, false on failure.
	// NOTE:  This method resets lastError from the base class.
	int Create(unsigned short theLen=0);

	// Fetch the WON_EGPublicKey corresponding to this private key.  Returns true
	// on success and false on failure.  TheKeyR is set to the [public key on success
	// and is indeterminate on failure.
	int GetPublicKey(WON_EGPublicKey& theKeyR) const;

	// Decrypt a block.  Returns (decrypted block, length of block) on success,
	// (NULL, 0) on failure.
	// NOTE:  This method resets lastError from the base class.
	CryptReturn Decrypt(const unsigned char* theMsgP, unsigned long theLen);

	// Sign a block.  Returns (signature block, length of block) on success,
	// (NULL, 0) on failure.
	// NOTE:  This method resets lastError from the base class.
	CryptReturn Sign(const void* theMsgP, unsigned long theLen);

private:
	// Time to trick the compiler.  Need mEGKeyP to point to a class in the WONCrypt
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mEGKeyP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mEGKeyP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	WONCrypt::EGPrivateKey* mEGKeyP;  // Real ElGamal Private key object
#else
	void* mEGKeyP;
#endif

	// Private Constructor - construct new key of specified length if theKeyP is NULL.
	// Otherwise copy existing key from buffer.  If theLen is 0, a new key of
	// default length will be constructed.
	WON_EGPrivateKey(unsigned short theLen=0, const unsigned char* theKeyP=NULL);

	// Private Copy constructor
	WON_EGPrivateKey(const WON_EGPrivateKey& theKeyR);

	// Private Destructor
	~WON_EGPrivateKey(void);

	// Private Methods
	void AllocateKey();

	// Friends
	friend class WON_CryptFactory;
};


#endif