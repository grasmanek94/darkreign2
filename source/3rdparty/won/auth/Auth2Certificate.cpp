// Auth2Certificate

// WON Authentication Certificate.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//  UserId
//	List of CommunityAccess structs - CommunityId and TrustLevel pairs
//	Client's Public Key (ElGamal)

// Used for various authentication protocols within the WON system.

#include "common/won.h"
#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthFamilies.h"
#include "Auth2Certificate.h"

// Private namespace for using and constants
namespace {
	using WONCommon::RawBuffer;
	using WONAuth::AuthFamilyBuffer;
	using WONAuth::AuthCertificateBase;
	using WONAuth::Auth2Certificate;
	using WONAuth::CertificateDataBlock;
	using WONAuth::AuthDataBlk;

	// Constants
	// Min length of a cert.  (Used in UnpackData)
	const unsigned short CERT_MINLEN = sizeof(unsigned long)  + // user id
                                       sizeof(unsigned short) + // community count
                                       sizeof(unsigned long)  + // a single community id
                                       sizeof(unsigned short) + // a single trustlevel
                                       sizeof(unsigned short);  // family
};



// CertificateDataBlock functions
// ** Constructors / Destructor

// static factory function.
// Unpacks datablock into a new DataBlock object.
// Updates aBuf to point past this data.
// Updates aLength to contain the remaining length of the buffer.
// Returns a NULL in case of error or unknown content type.

// ( Need to distinguish between error and unknown content type. )

CertificateDataBlock *
CertificateDataBlock::UnpackDataBlock(
				const unsigned char * &theBuf,
				int                   &theLength )
{

	// get content type
	if( theLength < sizeof( unsigned short ) ) return NULL;
	const unsigned short aContentType = *(reinterpret_cast<const unsigned short*>(theBuf));
	theBuf    += sizeof(aContentType);
	theLength -= sizeof(aContentType);

	// get length
	if( theLength < sizeof( unsigned short ) ) return NULL;
	const unsigned short aSize = *(reinterpret_cast<const unsigned short*>(theBuf));
	theBuf    += sizeof(aSize);
	theLength -= sizeof(aSize);

	if( theLength < aSize ) return NULL; // length points past end of data.

	auto_ptr<CertificateDataBlock> aBlkAP;
	switch( aContentType )
	{
	case CTAuthData:
		aBlkAP = auto_ptr<CertificateDataBlock>(new AuthDataBlk);
		break;
	}

	if( aBlkAP.get() )
	{
	    if( ! aBlkAP->UnpackData( theBuf, aSize ) ) return NULL;
	}

	theBuf    += aSize;
	theLength -= aSize;

	return( aBlkAP.release() );
}


bool CertificateDataBlock::Serialize( WONCommon::RawBuffer &theRawBuf ) const
{
	unsigned short aDataTag = GetContentType();
	theRawBuf.append(reinterpret_cast<const unsigned char*>(&aDataTag), sizeof(aDataTag));

	unsigned short aDataLength = ComputeSize();
	theRawBuf.append(reinterpret_cast<const unsigned char*>(&aDataLength), sizeof(aDataLength));

	return true;
}

// CertificateDataBlock::Compare
// Compares two CertificateDataBlock objects.
// This function should be called from the base class Compare.
//
int CertificateDataBlock::Compare(const CertificateDataBlock& theDataR) const
{
	return ( GetContentType() - theDataR.GetContentType() );
}


// AuthDataBlk functions
// ** Constructors / Destructor

// copy constructor
AuthDataBlk::AuthDataBlk( const AuthDataBlk &theDataBlk )
	: CertificateDataBlock( theDataBlk ),
	  mUserId( theDataBlk.mUserId ),
	  mPubKey( theDataBlk.mPubKey ),
	  mUserName( theDataBlk.mUserName ),
	  mAccessList( theDataBlk.mAccessList )
{}

CertificateDataBlock *
AuthDataBlk::Duplicate() const
{
	AuthDataBlk *aDataBlk = new AuthDataBlk( *this );
	return( aDataBlk );
}


unsigned long AuthDataBlk::ComputeSize() const
{
	unsigned long aLen = 0;
    aLen += 4;  // UserSeq      = ulong   = 4
	aLen += 2;  // PubKeyLength = ushort  = 2
	aLen += mPubKey.GetKeyLen(); // PubKey
	aLen += 2;  // User name length 
	aLen += (mUserName.size()   * sizeof(wchar_t) ); // user name
	aLen += 1;  // Number of community trust pairs
	aLen += (mAccessList.size() * ( 4 + 2 ));        // community / trust-levels
	return aLen;
}


bool AuthDataBlk::Serialize( WONCommon::RawBuffer &theRawBuf ) const
{
	WTRACE("AuthDataBlk::Serialize");
	if (! CertificateDataBlock::Serialize( theRawBuf )) return false;
	// UserId and be non-zero and at least one community's info must be known.

	WDBG_LL("AuthDataBlk::Serialize Validating...");
	if ((mUserId == 0) || (mAccessList.size() == 0))
	{
		WDBG_LH("AuthDataBlk::Serialize UserId is 0 or AccessList is empty, pack fails.");
		return false;
	}

	// PubKey must be valid
	if (mPubKey.GetKeyLen() <= 1)
	{
		WDBG_LH("Auth2Certificate::PackData PubKey not valid, pack fails.");
		return false;
	}

	// Append fixed length data
	WDBG_LL("AuthDataBlk::Serialize Packing...");

	theRawBuf.append(reinterpret_cast<const unsigned char*>(&mUserId), sizeof(mUserId));

	// Append PubKey length and PubKey
	WDBG_LL("AuthDataBlk::Serialize Packing PublicKey.");
	unsigned short aKeyLen = mPubKey.GetKeyLen();
	theRawBuf.append(reinterpret_cast<unsigned char*>(&aKeyLen), sizeof(aKeyLen));
	theRawBuf.append(mPubKey.GetKey(), aKeyLen);

	// Append user name
	unsigned short aNameLen = mUserName.size();
	theRawBuf.append(reinterpret_cast<unsigned char*>(&aNameLen), sizeof(aNameLen));
	theRawBuf.append(reinterpret_cast<const unsigned char*>( mUserName.data() ), (aNameLen*sizeof(wchar_t)) );
	
	// Append community data
	unsigned char aCommunityCount = mAccessList.size();
    theRawBuf.append(reinterpret_cast<unsigned char*>(&aCommunityCount),       sizeof(aCommunityCount));
    for (AuthCertificateBase::AccessList::const_iterator itr = mAccessList.begin(); itr != mAccessList.end(); ++itr)
    {
	    theRawBuf.append(reinterpret_cast<const unsigned char*>(&itr->mCommunityId), sizeof(itr->mCommunityId));
	    theRawBuf.append(reinterpret_cast<const unsigned char*>(&itr->mTrustLevel),  sizeof(itr->mTrustLevel));
    }

	return true;
}

// AuthDataBlk::UnpackData
// Unpacks member data from raw buffer.  Returns true on success and
// false on failure.  Verifies raw data length, reads memeber data.
bool
AuthDataBlk::UnpackData( const unsigned char *theBuf, int theLength )
{
	WTRACE("AuthDataBlk::UnpackData");

	// unpack UserSeq
	if( theLength < sizeof(mUserId) ) return false;
	mUserId = *(reinterpret_cast<const unsigned long*>(theBuf));
	theBuf     += sizeof(mUserId);
	theLength  -= sizeof(mUserId);

	// unpack PubKey
	if( theLength < sizeof(unsigned short) ) return false;
	unsigned short aKeyLen = *(reinterpret_cast<const unsigned short*>(theBuf));
	theBuf     += sizeof(aKeyLen);
	theLength  -= sizeof(aKeyLen);

	if( theLength < aKeyLen ) return false;
	mPubKey.Create(aKeyLen, theBuf);
	theBuf     += aKeyLen;
	theLength  -= aKeyLen;

	// unpack UserName
	if( theLength < sizeof(unsigned short) ) return false;
	unsigned short aNameLen = *(reinterpret_cast<const unsigned short*>(theBuf));
	theBuf     += sizeof(aNameLen);
	theLength  -= sizeof(aNameLen);

    if( theLength < aNameLen*sizeof(wchar_t) ) return false;
    mUserName.assign( (wchar_t *)theBuf, aNameLen );
    theBuf    += aNameLen*sizeof(wchar_t);
    theLength -= aNameLen*sizeof(wchar_t);

	// unpack community data
	if( theLength < sizeof(unsigned char) ) return false;
	unsigned char aCommunityCount = *(reinterpret_cast<const unsigned char*>(theBuf));
	theBuf     += sizeof(aCommunityCount);
	theLength  -= sizeof(aCommunityCount);

	while( aCommunityCount-- )
	{
	    // Read CommunityId
	    if( theLength < sizeof(unsigned long) ) return false;
	    unsigned long aCommunityId = *(reinterpret_cast<const unsigned long*>(theBuf));
		theBuf     += sizeof(aCommunityId);
		theLength  -= sizeof(aCommunityId);

	    // Read TrustLevel
	    if( theLength < sizeof(unsigned short) ) return false;
	    unsigned short aTrustLevel = *(reinterpret_cast<const unsigned short*>(theBuf));
		theBuf     += sizeof(aTrustLevel);
		theLength  -= sizeof(aTrustLevel);

        mAccessList.push_back(AuthCertificateBase::CommunityAccess(aCommunityId, aTrustLevel));
	}

	return true;
}

// AuthDataBlk::Compare
// Compares two AuthDataBlk objects.  Performs base class compare.  If equal,
// compares (in this order) userId and PublicKey.  Note that UserName and community-trustlevel
// info is ignored for purposes of compare.
int AuthDataBlk::Compare(const CertificateDataBlock& theBufR) const
{
	WTRACE("AuthDataBlk::Compare");
	int aRet = CertificateDataBlock::Compare(theBufR);
	if (aRet != 0) return aRet;

	// If not of same type, have to PUNT
	const AuthDataBlk* aCmpP = dynamic_cast<const AuthDataBlk*>(&theBufR);
	if (! aCmpP) return aRet;

	// Compare userId
	aRet = (mUserId < aCmpP->mUserId ? -1 : mUserId - aCmpP->mUserId);
	if (aRet != 0) return aRet;

	// Compare publicKey
	return (mPubKey <  aCmpP->mPubKey ? -1 : (mPubKey == aCmpP->mPubKey ? 0 : 1));

	/*
	// user name
	aRet = wstrcmp( mUserName.c_str(), aCmpP->mUserName.c_str() );
	if (! aCmpP) return aRet;

    // community - trust-levels
	aRet = mAccessList.size() - aCmpP->mAccessList.size();
	if (! aCmpP) return aRet;

	// compare data in the certificate
	Auth2Certificate::AccessListCIter anIter1 = mAccessList.begin();
	Auth2Certificate::AccessListCIter anIter2 = aCmpP->mAccessList.begin();
	while( anIter1 != mAccessList.end() )
	{
		aRet = (*anIter1)->Compare( *(*anIter2) );
		if( aRet != 0 ) return aRet;
		++anIter1;
		++anIter2;
	}
	*/

	return 0;
}

void AuthDataBlk::Dump( std::ostream &os ) const
{
	// Output buf and data length
	os << "(AuthDataBlock " << endl;
	os << " ContentType=" << (long)GetContentType();
	os << "  UserId="    << mUserId << endl;
    for (AuthCertificateBase::AccessList::const_iterator itr = mAccessList.begin(); itr != mAccessList.end(); ++itr)
        os << " CommunityId=" << itr->mCommunityId << " TrustLevel=" << itr->mTrustLevel << endl;
	os << "  PublicKey=" << mPubKey << endl << ')';
}


// Auth2Certificate functions
// ** Constructors / Destructor

// Default constructor
Auth2Certificate::Auth2Certificate() 
{}

// Construct from raw representation
Auth2Certificate::Auth2Certificate(const unsigned char* theRawP, unsigned short theLen)
{
	Unpack(theRawP, theLen);
}

// Copy Constructor
Auth2Certificate::Auth2Certificate(const Auth2Certificate& theCertR) :
	AuthCertificateBase(theCertR),
	mUserName(theCertR.mUserName)
{
	// deep copy of mDataList
	AppendDataList( theCertR.mDataList );
}

// Default constructor
Auth2Certificate::~Auth2Certificate()
{
	FreeDataList();
}

// Assignment operator
Auth2Certificate& Auth2Certificate::operator=(const Auth2Certificate& theCertR)
{
	if (this != &theCertR)  // protect vs a = a
	{
		AuthCertificateBase::operator=(theCertR);
		mUserName = theCertR.mUserName;
		FreeDataList();
		AppendDataList( theCertR.mDataList ); // deep copy
	}
	return *this;
}

// ** Private Methods **
void Auth2Certificate::FreeDataList()
{
    while( mDataList.size() )
    {
        auto_ptr<CertificateDataBlock> aDataBlock( mDataList.front() );
        mDataList.pop_front();
    }
}

void Auth2Certificate::AppendDataList( const DataList &theDataList )
{
	Auth2Certificate::DataListCIter anIter = theDataList.begin();
	while( anIter != theDataList.end() )
	{
		CertificateDataBlock *aBlock = (*anIter)->Duplicate();
		mDataList.push_back( aBlock );
		++anIter;
	}
}

/*
// Auth2Certificate::ComputeBufSize
// Determines number of bytes required in buffer for PACK and UNPACK operations.
// For PACK, returns base class total + fixed size + length of public key.  For UNPACK,
// returns base class total + fixed size.
RawBuffer::size_type
Auth2Certificate::ComputeBufSize(SizeComputeMode theMode) const
{
	WTRACE("Auth2Certificate::ComputeBufSize");

	RawBuffer::size_type aRet = AuthCertificateBase::ComputeBufSize(theMode) + CERT_MINLEN;
	if (theMode == PACK)
		aRet += mPubKey.GetKeyLen();
		
	return aRet;
}
*/


// Auth2Certificate::PackData
// Packs member data into raw buffer in base class.  Returns true on success and
// false on failure.  Verifies member data and appends member data to buffer.
bool
Auth2Certificate::PackData()
{
	WTRACE("Auth2Certificate::PackData");
	if (! AuthCertificateBase::PackData()) return false;

	// UserId and be non-zero and at least one community's info must be known.
	WDBG_LL("Auth2Certificate::PackData Validating...");
	if ( mDataList.size() == 0 )
	{
		WDBG_LH("Auth2Certificate::PackData DataList is empty, pack fails.");
		return false;
	}

	unsigned short tmpDataListSize = mDataList.size();
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpDataListSize), sizeof(tmpDataListSize));

	Auth2Certificate::DataListCIter anIter = mDataList.begin();
	while( anIter != mDataList.end() )
	{
		if( ! (*anIter)->Serialize( mRawBuf ) ) return false;
		++anIter;
	}

	return true;
}


// Auth2Certificate::UnpackData
// Unpacks member data from raw buffer in base class.  Returns true on success and
// false on failure.  Verifies raw data length, reads memeber data, sets mDatalen.
bool
Auth2Certificate::UnpackData()
{
	WTRACE("Auth2Certificate::UnpackData");
	if (! AuthCertificateBase::UnpackData()) return false;

	// Get data pointer (skip header data)
	WDBG_LL("Auth2Certificate::UnpackData Unpack fixed fields.");
#ifdef macintosh
	const unsigned char*
#else
	const unsigned char const*
#endif // macintosh
		aDataPStart = mRawBuf.data();

	const unsigned char* aDataP = mRawBuf.data() + mDataLen;
	int                  aSize  = mRawBuf.size() - mDataLen;

	unsigned short aDataCount;


	// unpack count
	if( aSize < sizeof(aDataCount) ) return false;
	aDataCount = *(reinterpret_cast<const unsigned short*>(aDataP));
	aDataP += sizeof(aDataCount);
	aSize  -= sizeof(aDataCount);

	while( aDataCount-- )
	{
		// unpack a block, advance the data pointer, and adjust the size
		CertificateDataBlock *aDataBlock = CertificateDataBlock::UnpackDataBlock( aDataP, aSize );
		if( ! aDataBlock ) return false;
		if(aDataBlock->GetContentType()==CTAuthData)
		{
			AuthDataBlk *anAuthData = (AuthDataBlk*)aDataBlock;
			mUserId = anAuthData->GetUserId();
			mPubKey =  anAuthData->GetPubKey();
			mAccessList = anAuthData->GetAccessList();
			mUserName = anAuthData->GetUserName();
		}
		AddDataBlock( aDataBlock );
	}

	mDataLen = aDataP - aDataPStart; // save length of data portion of the buffer.

	return true;
}

// Auth2Certificate::Compare
// Compares two Auth2Certificate objects.  Performs base class compare,
// followed by compare of certificate contents.
int Auth2Certificate::Compare(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("Auth2Certificate::Compare");
	int aRet = AuthCertificateBase::Compare(theBufR);
	if (aRet != 0) return aRet;

	// If not of same type, have to PUNT
	const Auth2Certificate* aCmpP = dynamic_cast<const Auth2Certificate*>(&theBufR);
	if (! aCmpP) return aRet;

	aRet = mDataList.size() - aCmpP->mDataList.size();
	if (! aCmpP) return aRet;

	// compare data in the certificate
	Auth2Certificate::DataListCIter anIter1 = mDataList.begin();
	Auth2Certificate::DataListCIter anIter2 = aCmpP->mDataList.begin();
	while( anIter1 != mDataList.end() )
	{
		aRet = (*anIter1)->Compare( *(*anIter2) );
		if( aRet != 0 ) return aRet;
		++anIter1;
		++anIter2;
	}

	return 0;
}

// Auth2Certificate::Dump
// Streaming method.  Outputs certificate info.  Outputs lengths and intenral memebers.
void Auth2Certificate::Dump(std::ostream& os) const
{
	// Output buf and data length
	os << "(AuthCert1 RawLen=" << mRawBuf.size() << " DataLen=" << mDataLen << endl;
	os << "  Family=" << GetFamily() << endl;
	os << "  IssueTime=" << ctime(&mIssueTime);
	os << "  ExpireTime=" << ctime(&mExpireTime);
	os << "  DataCount=" << mDataList.size() << endl;

	Auth2Certificate::DataListCIter anIter = mDataList.begin();
	while( anIter != mDataList.end() )
	{
		(*anIter)->Dump( os );
		++anIter;
	}
}
