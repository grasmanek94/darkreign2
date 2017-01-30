// Auth1PrivateKeyBlock

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
#include "crypt/CryptException.h"
//#include "EGPublicKey.h"
#include "crypt/EGPrivateKey.h"
#include "AuthFamilies.h"
#include "Auth1PrivateKeyBlock.h"
#include "common/WONEndian.h"

// Private namespace for using and constants
namespace {
	using WONCommon::RawBuffer;
	using WONAuth::AuthFamilyBuffer;
	using WONAuth::AuthPublicKeyBlockBase;
	using WONCrypt::EGPrivateKey;
	using WONAuth::Auth1PrivateKeyBlock;

	// Constants
	// Min length of a block.  (Used in UnpackData)
	const unsigned short BLOCK_MINLEN = sizeof(unsigned short);
};


// ** Constructors / Destructor

// Default constructor
Auth1PrivateKeyBlock::Auth1PrivateKeyBlock(unsigned short theBlockId) :
	AuthPublicKeyBlockBase(theBlockId),
	mKeyList()
{}


// Construct from raw representation
Auth1PrivateKeyBlock::Auth1PrivateKeyBlock(const unsigned char* theRawP, unsigned short theLen) :
	AuthPublicKeyBlockBase(),
	mKeyList(),
	mKeyMap()
{
	Unpack(theRawP, theLen);
}


// Copy Constructor
// Auth1PrivateKeyBlock::Auth1PrivateKeyBlock(const Auth1PrivateKeyBlock& theBlockR) :
//	AuthPublicKeyBlockBase(theBlockR),
//	mKeyList(theBlockR.mKeyList),
//	mKeyMap(theBlockR.mKeyList)
// {}

	
// Destructor
inline void del( WONCrypt::EGPrivateKey* theKey ) { delete theKey; }
Auth1PrivateKeyBlock::~Auth1PrivateKeyBlock(void)
{
    std::for_each( mKeyList.begin(), mKeyList.end(), &del );
}


// ** Private Methods **

// Auth1PrivateKeyBlock::ComputeBufSize
// Determines number of bytes required in buffer for PACK and UNPACK operations.
// For PACK, returns base class total + fixed size + length of public keys.  For
// UNPACK, returns base class total + fixed size.
RawBuffer::size_type
Auth1PrivateKeyBlock::ComputeBufSize(SizeComputeMode theMode) const
{
	WTRACE("Auth1PrivateKeyBlock::ComputeBufSize");
	RawBuffer::size_type aRet = AuthPublicKeyBlockBase::ComputeBufSize(theMode) + BLOCK_MINLEN;
	if (theMode == PACK)
	{
		WDBG_LL("Auth1PrivateKeyBlock::ComputeBufSize Add size of each key (" << mKeyList.size() << ')');
		PrivateKeyList::const_iterator anItr(mKeyList.begin());
		for (; anItr != mKeyList.end(); anItr++)
			aRet += sizeof(unsigned short) + (*anItr)->GetKeyLen();

        aRet += sizeof(unsigned short);
        aRet += sizeof(unsigned short) * mKeyMap.size();
	}

	return aRet;
}


// Auth1PrivateKeyBlock::PackData
// Packs member data into raw buffer in base class.  Returns true on success and
// false on failure.  Makes sure keyList is not empty.  Appends number of keys
// in key list.  For each key, appends key length and the key.
bool
Auth1PrivateKeyBlock::PackData()
{
	WTRACE("Auth1PrivateKeyBlock::PackData");
	if (! AuthPublicKeyBlockBase::PackData()) return false;

	// KeyList cannot be empty.
	WDBG_LL("Auth1PrivateKeyBlock::PackData Validating...");
	if (mKeyList.empty())
	{
		WDBG_LH("Auth1PrivateKeyBlock::PackData KeySet is empty, pack fails.");
		return false;
	}

	// Append fixed length data
	WDBG_LL("Auth1PrivateKeyBlock::PackData Packing...");
	unsigned short aNumKeys = mKeyList.size();
	unsigned short tmpNumKeys = getLittleEndian(aNumKeys);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpNumKeys), sizeof(tmpNumKeys));

	// Append each key (length and binary data)
	WDBG_LL("Auth1PrivateKeyBlock::PackData Packing keys (" << mKeyList.size() << ')');
	PrivateKeyList::iterator anItr(mKeyList.begin());
	for (; anItr != mKeyList.end(); anItr++)
	{
		unsigned short aKeyLen = (*anItr)->GetKeyLen();
		unsigned short tmpKeyLen = getLittleEndian(tmpKeyLen);
		mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpKeyLen), sizeof(tmpKeyLen));
		mRawBuf.append((*anItr)->GetKey(), aKeyLen);

        // count and record which BlockId's map to this key.
	    PrivateKeyMap::iterator aMapIter(mKeyMap.begin());
		unsigned short aBlockIDCount=0;
        std::list<unsigned short> aBlockIdList;
        for(; aMapIter != mKeyMap.end(); aMapIter++)
        {
            if( aMapIter->second == *anItr )
            {
                aBlockIDCount++;
                aBlockIdList.push_back( aMapIter->first );
            }
        }

		unsigned short tmpBlockIDCount = getLittleEndian(aBlockIDCount);
		
        // pack BlockId count and BlockIds
		mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpBlockIDCount), sizeof(tmpBlockIDCount));
        while( aBlockIdList.size() )
        {
            unsigned short aBlockId = aBlockIdList.front();
            makeLittleEndian(aBlockId);
		    mRawBuf.append(reinterpret_cast<unsigned char*>(& aBlockId), sizeof(unsigned short));
            aBlockIdList.pop_front();
        }
	}

	return true;
}


// Auth1PrivateKeyBlock::UnpackData
// Unpacks member data from raw buffer in base class.  Returns true on success and
// false on failure.  Reads number of keys and then reads each key in turn.  Will
// abort if buffer size is exceeded.
bool
Auth1PrivateKeyBlock::UnpackData()
{
	WTRACE("AuthPublicKeyBlock1::UnpackData");
	if (! AuthPublicKeyBlockBase::UnpackData()) return false;

	// Get data pointer (skip header data)
	WDBG_LL("Auth1PrivateKeyBlock::UnpackData Unpack fixed fields.");
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
		WDBG_LL("Auth1PrivateKeyBlock::UnpackData Read key length.");
		unsigned short aKeyLen = *(reinterpret_cast<const unsigned short*>(aDataP));
		makeLittleEndian(aKeyLen);
		aDataP += sizeof(aKeyLen);

		// Make sure key can be read
		mDataLen += aKeyLen;
		if (mRawBuf.size() < mDataLen) break;

		// Read key and add to key set
		WDBG_LL("Auth1PrivateKeyBlock::UnpackData Read key.");
		EGPrivateKey *aKeyP = new EGPrivateKey(aKeyLen, aDataP);
		aDataP += aKeyLen;
		mKeyList.push_back(aKeyP);

        // Read BlockIds and add to KeyMap
        mDataLen += sizeof(unsigned short);
		if (mRawBuf.size() < mDataLen) break;

		WDBG_LL("Auth1PrivateKeyBlock::UnpackData BlockId count.");
		unsigned short aBlockIdCount = *(reinterpret_cast<const unsigned short*>(aDataP));
		makeLittleEndian(aBlockIdCount);
		aDataP += sizeof(aBlockIdCount);
		int j;
        for (j=0; j < aBlockIdCount; j++ )
        {
            mDataLen += sizeof(unsigned short);
		    if (mRawBuf.size() < mDataLen) break;
		    unsigned short aBlockId = *(reinterpret_cast<const unsigned short*>(aDataP));
		    makeLittleEndian(aBlockId);
		    aDataP += sizeof(aBlockId);

            mKeyMap[ aBlockId ] = mKeyList.back();
        }
        if( j != aBlockIdCount ) break;
	}

	// Return true if all keys were read
	return (i == aNumKeys);
}


// ** Public Methods **

// Assignment operator
Auth1PrivateKeyBlock&
Auth1PrivateKeyBlock::operator=(const Auth1PrivateKeyBlock& theBlockR)
{
	if (this != &theBlockR)  // protect vs a = a
	{
		AuthPublicKeyBlockBase::operator=(theBlockR);
		mKeyList = theBlockR.mKeyList;
	}
	return *this;
}


// Compare method

// Auth1PrivateKeyBlock::Compare
// Compares two AuthPublicKeyBlock1 objects.  Performs base class compare.  If equal,
// compares list of keys.
int
Auth1PrivateKeyBlock::Compare(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("Auth1PrivateKeyBlock::Compare");
	int aRet = AuthPublicKeyBlockBase::Compare(theBufR);
	if (aRet != 0) return aRet;

	// If not of same type, have to PUNT
	const Auth1PrivateKeyBlock* aCmpP = dynamic_cast<const Auth1PrivateKeyBlock*>(&theBufR);
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


// Auth1PrivateKeyBlock::GetFamily
// Returns certificate family.  Uses AuthFamily1 which is defined in AuthFamilies.h.
unsigned short
Auth1PrivateKeyBlock::GetFamily() const
{
	return WONAuth::AuthFamily1;  // From AuthFamilies.h
}

/*
bool
Auth1PrivateKeyBlock::VerifyFamilyBuffer(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("Auth1PrivateKeyBlock::VerifyFamilyBuffer");
	WDBG_LM("Auth1PrivateKeyBlock::VerifyFamilyBuffer Verify Buffer: " << theBufR);
	if ((! IsValid()) || (! theBufR.IsValid())) return false;

	PublicKeyList::const_iterator anItr(mKeyList.begin());
	for (; anItr != mKeyList.end(); anItr++)
	{
		WDBG_LL("Auth1PrivateKeyBlock::VerifyFamilyBuffer Verify with key=" << *anItr);
		if (theBufR.Verify(*anItr)) break;
	}

	WDBG_LM("Auth1PrivateKeyBlock::VerifyFamilyBuffer Verify Result=" << (anItr != mKeyList.end()));
	return (anItr != mKeyList.end());
}
*/
/*
bool
Auth1PrivateKeyBlock::VerifyRawBuffer(const unsigned char* theSigP, unsigned long theSigLen,
                                     const unsigned char* theMsgP, unsigned long theMsgLen) const
{
	WTRACE("Auth1PrivateKeyBlock::VerifyRawBuffer");
	WDBG_LM("Auth1PrivateKeyBlock::VerifyRawBuffer Verify Buffer, SigLen=" << theSigLen << " Msglen=" << theMsgLen);
	if (! IsValid()) return false;

	PublicKeyList::const_iterator anItr(mKeyList.begin());
	for (; anItr != mKeyList.end(); anItr++)
	{
		try
		{
			WDBG_LL("Auth1PrivateKeyBlock::VerifyFamilyBuffer Verify with key=" << *anItr);
			if (anItr->Verify(theSigP, theSigLen, theMsgP, theMsgLen)) break;
		}
		catch (WONCrypt::CryptException& theExR)
		{
			WDBG_LM("Auth1PrivateKeyBlock::VerifyRawBuffer Exception verifying with key=" << *anItr);
			theExR.SetLog(false);
		}
	}

	WDBG_LM("Auth1PrivateKeyBlock::VerifyRawBuffer Verify Result=" << (anItr != mKeyList.end()));
	return (anItr != mKeyList.end());
}
*/
/*
EGPublicKey::CryptReturn
Auth1PrivateKeyBlock::EncryptRawBuffer(const unsigned char* theBufP, unsigned long theLen) const
{
	WTRACE("Auth1PrivateKeyBlock::EncryptRawBuffer");
	WDBG_LM("Auth1PrivateKeyBlock::EncryptRawBuffer Encrypt Buffer, len=" << theLen);
	try
	{
		return mKeyList.front().Encrypt(theBufP, theLen);
	}
	catch (WONCrypt::CryptException& theExR)
	{
		WDBG_LM("Auth1PrivateKeyBlock::VerifyRawBuffer Exception encrypting with key=" << mKeyList.front());
		theExR.SetLog(false);
		return EGPublicKey::CryptReturn(NULL,0);
	}
}
*/

// Auth1PrivateKeyBlock::Dump
// Streaming method.  Outputs base class info and all keys.
void
Auth1PrivateKeyBlock::Dump(std::ostream& os) const
{
	// Output buf and data length
	os << "(AuthPrivKeyBlk1 RawLen=" << mRawBuf.size() << " DataLen=" << mDataLen << endl;
	os << "  Family=" << GetFamily() << endl;
	os << "  IssueTime=" << ctime(&mIssueTime);
	os << "  ExpireTime=" << ctime(&mExpireTime);
	os << "  BlockId=" << mBlockId << " NumKeys=" << mKeyList.size() << endl;

	PrivateKeyList::const_iterator anItr(mKeyList.begin());
	for (int i=1; anItr != mKeyList.end(); i++,anItr++)
		os << "    Key" << i << '=' << *anItr << endl;
}
