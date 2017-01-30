#ifndef _Auth1PrivateKeyBlock_H
#define _Auth1PrivateKeyBlock_H

// AuthPublicKeyBlock1

// WON AuthServer Private Key Block for Auth Family 1.  Encapsulates a list of one
// or more ElGamal private keys.  

// The first key in the key block is the most recent and should be considered the
// only valid key.  Other keys in the block are still valid but are being
// obsoleted.  Always use the first key for encryption.  Singature verification
// should be performed first with the first key and, if the verify fails, any
// additional keys in the block.


#include <list>
#include <map>
#include <time.h>
#include "crypt/EGPrivateKey.h"
#include "AuthPublicKeyBlockBase.h"

// In the WONAuth namespace
namespace WONAuth {


class Auth1PrivateKeyBlock : public AuthPublicKeyBlockBase
{
public:
	// Types
	typedef std::list<WONCrypt::EGPrivateKey*> PrivateKeyList;
    typedef std::list<WONCrypt::EGPrivateKey*>::iterator PrivateKeyListIter;
    typedef std::list<WONCrypt::EGPrivateKey*>::const_iterator PrivateKeyListCIter;

	typedef std::map<unsigned short,WONCrypt::EGPrivateKey*> PrivateKeyMap;
    typedef std::map<unsigned short,WONCrypt::EGPrivateKey*>::iterator PrivateKeyMapIter;
    typedef std::map<unsigned short,WONCrypt::EGPrivateKey*>::const_iterator PrivateKeyMapCIter;

	// Default constructor - may provide block ID (for base class)
	explicit Auth1PrivateKeyBlock(unsigned short theBlockId=0);

	// Construct from raw representation (calls Unpack())
	Auth1PrivateKeyBlock(const unsigned char* theRawP, unsigned short theLen);

	// Destructor
	~Auth1PrivateKeyBlock();

private:
	// Copy Constructor
	Auth1PrivateKeyBlock(const Auth1PrivateKeyBlock& theBlockR);

	// Operators
	Auth1PrivateKeyBlock& operator=(const Auth1PrivateKeyBlock& theBlockR);

public:
	// Compare (overridden from base class)
	int Compare(const AuthFamilyBuffer& theBufR) const;

	// Fetch key block family
	unsigned short GetFamily() const;

	// Private Key set access
	const PrivateKeyList& KeyList() const;
	PrivateKeyList&       KeyList();

	// Private Key map access
	const PrivateKeyMap& KeyMap() const;
	PrivateKeyMap&       KeyMap();

    const WONCrypt::EGPrivateKey* GetPrivateKey( unsigned short theBlockId );
    

	// Encrypts a raw buffer with first key from the key block.  Returns encrypted block
//	WONCrypt::EGPublicKey::CryptReturn EncryptRawBuffer(
//		const unsigned char* theBufP, unsigned long theLen
//	) const;

	// Dump to stream
	void Dump(std::ostream& os) const;

private:
	PrivateKeyList mKeyList;
	PrivateKeyMap  mKeyMap;  // maps block id's to private keys.

	// Compute size of buffer needed form pack and unpack operations.
	WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

	// Pack local members into base raw buffer
	bool PackData();

	// Unpack local members from base raw buffer
	bool UnpackData();
};


// Inlines
inline const Auth1PrivateKeyBlock::PrivateKeyList&
Auth1PrivateKeyBlock::KeyList() const
{ return mKeyList; }

inline Auth1PrivateKeyBlock::PrivateKeyList&
Auth1PrivateKeyBlock::KeyList()
{ return mKeyList; }

inline const Auth1PrivateKeyBlock::PrivateKeyMap&
Auth1PrivateKeyBlock::KeyMap() const
{ return mKeyMap; }

inline Auth1PrivateKeyBlock::PrivateKeyMap&
Auth1PrivateKeyBlock::KeyMap()
{ return mKeyMap; }


};  // Namespace WONAuth

#endif