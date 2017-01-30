#ifndef _WON_BFSYMMETRICKEY_H
#define _WON_BFSYMMETRICKEY_H

// WON_BFSymmetricKey

// Interface class for the Blowfish implementation of a symmetric key.  Length of
// key in bytes can be defined on creation.  Key can be used to encrypt and decrypt
// binary blocks of arbitrary size.

// This class simply makes use of the BFSymmectricKey class.  It eliminates the STL
// and exceptions from the interface.

// *NOTE*
// This header is compatible with VC4.  Ergo, no using bool, string, etc.


#include "WON_CryptKeyBase.h"


class DLL_OPER WON_BFSymmetricKey : public WON_CryptKeyBase
{
public:
	// Assignment Operator
	WON_BFSymmetricKey& operator=(const WON_BFSymmetricKey& theKeyR);

	// Create key - from existing raw key.  Returns true on success, false on failure.
	// NOTE:  This method resets lastError from the base class.
	int Create(unsigned short theLen, const unsigned char* theKeyP);

	// Create key - new key of specified length.  Specifying theLen as 0 will use
	// the default length.  Returns true on success, false on failure.
	// NOTE:  This method resets lastError from the base class.
	int Create(unsigned short theLen=0);

	// Encrypt a block.  Returns (encrypted block, length of block) on success,
	// (NULL, 0) on failure.
	// NOTE:  This method resets lastError from the base class.
	CryptReturn Encrypt(const void* theMsgP, unsigned long theLen);

	// Decrypt a block.  Returns (decrypted block, length of block) on success,
	// (NULL, 0) on failure.
	// NOTE:  This method resets lastError from the base class.
	CryptReturn Decrypt(const unsigned char* theMsgP, unsigned long theLen);

private:
	// Time to trick the compiler.  Need mBFKeyP to point to a class in the WONCrypt
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mBFKeyP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mBFKeyP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	WONCrypt::BFSymmetricKey* mBFKeyP;  // Real Blowfish key object
#else
	void* mBFKeyP;
#endif

	// Private Constructor - construct new key of specified length if theKeyP is NULL.
	// Otherwise copy existing key from buffer.  If theLen is 0, a new key of
	// default length will be constructed.
	WON_BFSymmetricKey(unsigned short theLen=0, const unsigned char* theKeyP=NULL);

	// Private Copy constructor
	WON_BFSymmetricKey(const WON_BFSymmetricKey& theKeyR);

	// Private Destructor
	~WON_BFSymmetricKey(void);

	// Private Methods
	void AllocateKey();

	// Factory is a friend
	friend class WON_CryptFactory;
};


#endif