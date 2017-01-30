// Auth1PublicKeyBlock

// WON AuthServer Public Key Block for Auth Family 1.  Encapsulates a list of one
// or more ElGamal public keys.  

// The first key in the key block is the most recent and should be considered the
// only valid key.  Other keys in the block are still valid but are being
// obsoleted.  Always use the first key for encryption.  Singature verification
// should be performed first with the first key and, if the verify fails, any
// additional keys in the block.


#include "common/won.h"
#include <list>
#include <algorithm>
#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthFamilies.h"
#include "Auth1PublicKeyBlock.h"
#include "common/WONEndian.h"

// Private namespace for using and constants
namespace {
	using WONCommon::RawBuffer;
	using WONAuth::AuthFamilyBuffer;
	using WONAuth::AuthPublicKeyBlockBase;
	using WONCrypt::EGPublicKey;
	using WONAuth::Auth1PublicKeyBlock;

	// Constants
	// Min length of a block.  (Used in UnpackData)
	const unsigned short BLOCK_MINLEN = sizeof(unsigned short);
};


// ** Constructors / Destructor

// Default constructor
Auth1PublicKeyBlock::Auth1PublicKeyBlock(unsigned short theBlockId) :
	AuthPublicKeyBlockBase(theBlockId),
	mKeyList()
{}


// Construct from raw representation
Auth1PublicKeyBlock::Auth1PublicKeyBlock(const unsigned char* theRawP, unsigned short theLen) :
	AuthPublicKeyBlockBase(),
	mKeyList()
{
	Unpack(theRawP, theLen);
}


// Copy Constructor
Auth1PublicKeyBlock::Auth1PublicKeyBlock(const Auth1PublicKeyBlock& theBlockR) :
	AuthPublicKeyBlockBase(theBlockR),
	mKeyList(theBlockR.mKeyList)
{}

	
// Destructor
Auth1PublicKeyBlock::~Auth1PublicKeyBlock(void)
{}


// ** Private Methods **

// Auth1PublicKeyBlock::ComputeBufSize
// Determines number of bytes required in buffer for PACK and UNPACK operations.
// For PACK, returns base class total + fixed size + length of public keys.  For
// UNPACK, returns base class total + fixed size.
RawBuffer::size_type
Auth1PublicKeyBlock::ComputeBufSize(SizeComputeMode theMode) const
{
	WTRACE("Auth1PublicKeyBlock::ComputeBufSize");
	RawBuffer::size_type aRet = AuthPublicKeyBlockBase::ComputeBufSize(theMode) + BLOCK_MINLEN;
	if (theMode == PACK)
	{
		WDBG_LL("Auth1PublicKeyBlock::ComputeBufSize Add size of each key (" << mKeyList.size() << ')');
		PublicKeyList::const_iterator anItr(mKeyList.begin());
		for (; anItr != mKeyList.end(); anItr++)
			aRet += sizeof(unsigned short) + anItr->GetKeyLen();
	}

	return aRet;
}


// Auth1PublicKeyBlock::PackData
// Packs member data into raw buffer in base class.  Returns true on success and
// false on failure.  Makes sure keyList is not empty.  Appends number of keys
// in key list.  For each key, appends key length and the key.
bool
Auth1PublicKeyBlock::PackData()
{
	WTRACE("Auth1PublicKeyBlock::PackData");
	if (! AuthPublicKeyBlockBase::PackData()) return false;

	// KeyList cannot be empty.
	WDBG_LL("Auth1PublicKeyBlock::PackData Validating...");
	if (mKeyList.empty())
	{
		WDBG_LH("Auth1PublicKeyBlock::PackData KeySet is empty, pack fails.");
		return false;
	}

	// Append fixed length data
	WDBG_LL("Auth1PublicKeyBlock::PackData Packing...");
	unsigned short aNumKeys = mKeyList.size();
	makeLittleEndian(aNumKeys);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&aNumKeys), sizeof(aNumKeys));

	// Append each key (length and binary data)
	WDBG_LL("Auth1PublicKeyBlock::PackData Packing keys (" << mKeyList.size() << ')');
	PublicKeyList::iterator anItr(mKeyList.begin());
	for (; anItr != mKeyList.end(); anItr++)
	{
		unsigned short aKeyLen = anItr->GetKeyLen();
		unsigned short tmpKeyLen = getLittleEndian(aKeyLen);
		mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpKeyLen), sizeof(tmpKeyLen));
		mRawBuf.append(anItr->GetKey(), aKeyLen);
	}

	return true;
}


// Auth1PublicKeyBlock::UnpackData
// Unpacks member data from raw buffer in base class.  Returns true on success and
// false on failure.  Reads number of keys and then reads each key in turn.  Will
// abort if buffer size is exceeded.
bool
Auth1PublicKeyBlock::UnpackData()
{
	WTRACE("Auth1PublicKeyBlock::UnpackData");
	if (! AuthPublicKeyBlockBase::UnpackData()) return false;

	// Get data pointer (skip header data)
	WDBG_LL("Auth1PublicKeyBlock::UnpackData Unpack fixed fields.");
	const unsigned char* aDataP = mRawBuf.data() + mDataLen;

	// Read number of keys
	unsigned short aNumKeys = *(reinterpret_cast<const unsigned short*>(aDataP));
	makeLittleEndian(aNumKeys);
	aDataP   += sizeof(aNumKeys);
	mDataLen += BLOCK_MINLEN;

	// Read each key
	int i;
	for (i=0; i < aNumKeys; i++)
	{
		// Make sure key len can be read
		mDataLen += sizeof(unsigned short);
		if (mRawBuf.size() < mDataLen) break;

		// Read key len
		WDBG_LL("Auth1PublicKeyBlock::UnpackData Read key length.");
		unsigned short aKeyLen = *(reinterpret_cast<const unsigned short*>(aDataP));
		makeLittleEndian(aKeyLen);
		aDataP += sizeof(aKeyLen);

		// Make sure key can be read
		mDataLen += aKeyLen;
		if (mRawBuf.size() < mDataLen) break;

		// Read key and add to key set
		WDBG_LL("Auth1PublicKeyBlock::UnpackData Read key.");
		EGPublicKey aKey(aKeyLen, aDataP);
		aDataP += aKeyLen;
		mKeyList.push_back(aKey);
	}

	// Return true if all keys were read
	return (i == aNumKeys);
}


// ** Public Methods **

// Assignment operator
Auth1PublicKeyBlock&
Auth1PublicKeyBlock::operator=(const Auth1PublicKeyBlock& theBlockR)
{
	if (this != &theBlockR)  // protect vs a = a
	{
		AuthPublicKeyBlockBase::operator=(theBlockR);
		mKeyList = theBlockR.mKeyList;
	}
	return *this;
}


// Compare method

// Auth1PublicKeyBlock::Compare
// Compares two Auth1PublicKeyBlock objects.  Performs base class compare.  If equal,
// compares list of keys.
int
Auth1PublicKeyBlock::Compare(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("Auth1PublicKeyBlock::Compare");
	int aRet = AuthPublicKeyBlockBase::Compare(theBufR);
	if (aRet != 0) return aRet;

	// If not of same type, have to PUNT
	const Auth1PublicKeyBlock* aCmpP = dynamic_cast<const Auth1PublicKeyBlock*>(&theBufR);
	if (! aCmpP) return aRet;

	// See if mKeyList < theBlockR.mKeyList
	if (std::lexicographical_compare(mKeyList.begin(), mKeyList.end(), aCmpP->mKeyList.begin(), aCmpP->mKeyList.end()))
		return -1;

	// See if mKeyList > theBlockR.mKeyList
	if (std::lexicographical_compare(aCmpP->mKeyList.begin(), aCmpP->mKeyList.end(), mKeyList.begin(), mKeyList.end()))
		return 1;

	// They're equal.
	return 0;
}


// Auth1PublicKeyBlock::GetFamily
// Returns certificate family.  Uses AuthFamily1 which is defined in AuthFamilies.h.
unsigned short
Auth1PublicKeyBlock::GetFamily() const
{
	return WONAuth::AuthFamily1;  // From AuthFamilies.h
}


bool
Auth1PublicKeyBlock::VerifyFamilyBuffer(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("Auth1PublicKeyBlock::VerifyFamilyBuffer");
	WDBG_LM("Auth1PublicKeyBlock::VerifyFamilyBuffer Verify Buffer: " << theBufR);
	if ((! IsValid()) || (! theBufR.IsValid())) return false;

	PublicKeyList::const_iterator anItr(mKeyList.begin());
	for (; anItr != mKeyList.end(); anItr++)
	{
		WDBG_LL("Auth1PublicKeyBlock::VerifyFamilyBuffer Verify with key=" << *anItr);
		if (theBufR.Verify(*anItr)) break;
	}

	WDBG_LM("Auth1PublicKeyBlock::VerifyFamilyBuffer Verify Result=" << (anItr != mKeyList.end()));
	return (anItr != mKeyList.end());
}


bool
Auth1PublicKeyBlock::VerifyRawBuffer(const unsigned char* theSigP, unsigned long theSigLen,
                                     const unsigned char* theMsgP, unsigned long theMsgLen) const
{
	WTRACE("Auth1PublicKeyBlock::VerifyRawBuffer");
	WDBG_LM("Auth1PublicKeyBlock::VerifyRawBuffer Verify Buffer, SigLen=" << theSigLen << " Msglen=" << theMsgLen);
	if (! IsValid()) return false;

	PublicKeyList::const_iterator anItr(mKeyList.begin());
	for (; anItr != mKeyList.end(); anItr++)
	{
		WDBG_LL("Auth1PublicKeyBlock::VerifyFamilyBuffer Verify with key=" << *anItr);
		if (anItr->Verify(theSigP, theSigLen, theMsgP, theMsgLen)) break;
	}

	WDBG_LM("Auth1PublicKeyBlock::VerifyRawBuffer Verify Result=" << (anItr != mKeyList.end()));
	return (anItr != mKeyList.end());
}


EGPublicKey::CryptReturn
Auth1PublicKeyBlock::EncryptRawBuffer(const unsigned char* theBufP, unsigned long theLen) const
{
	WTRACE("Auth1PublicKeyBlock::EncryptRawBuffer");
	WDBG_LM("Auth1PublicKeyBlock::EncryptRawBuffer Encrypt Buffer, len=" << theLen);
	return mKeyList.front().Encrypt(theBufP, theLen);
}


// Auth1PublicKeyBlock::Dump
// Streaming method.  Outputs base class info and all keys.
void
Auth1PublicKeyBlock::Dump(std::ostream& os) const
{
	// Output buf and data length
	os << "(AuthPubKeyBlk1 RawLen=" << mRawBuf.size() << " DataLen=" << mDataLen << endl;
	os << "  Family=" << GetFamily() << endl;
	os << "  IssueTime=" << ctime(&mIssueTime);
	os << "  ExpireTime=" << ctime(&mExpireTime);
	os << "  BlockId=" << mBlockId << " NumKeys=" << mKeyList.size() << endl;

	PublicKeyList::const_iterator anItr(mKeyList.begin());
	for (int i=1; anItr != mKeyList.end(); i++,anItr++)
		os << "    Key" << i << '=' << *anItr << endl;
}
