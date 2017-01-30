#ifndef _Auth2Certificate_H
#define _Auth2Certificate_H

// Auth2Certificate

// WON Authentication Certificate. Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//   generic content object

// Used for various authentication protocols within the WON system.

#include <list>
#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthCertificateBase.h"

// In the WONAuth namespace
namespace WONAuth {

enum CertContentType
{
		CTAuthData = 0,
		CTMax
};

// base class for Certificate Data Blocks
class CertificateDataBlock {
public:

	    static CertificateDataBlock *UnpackDataBlock(
				const unsigned char * &aBuf,
				int                   &aLength );

		virtual int Compare(const CertificateDataBlock& theDataR) const = 0;

	    virtual CertificateDataBlock * Duplicate() const = 0;
		virtual CertContentType GetContentType()   const = 0;

		// base clase packed length is 2 byte tag + 2 byte length
		virtual unsigned long ComputeSize() const = 0 { return( 4 ); }

		// Serialize the data into a RawBuffer.
		virtual bool Serialize( WONCommon::RawBuffer &theRawBuf ) const = 0;
		virtual bool UnpackData( const unsigned char *theBuf, int theLength ) = 0;

		// Dump to stream
		virtual void Dump(std::ostream& os) const = 0;
};

class AuthDataBlk : public CertificateDataBlock {
public:
	// Default constructor
	AuthDataBlk() : mUserId(0) {}

	// Construct from raw representation (calls Unpack())
//	explicit AuthDataBlk(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	explicit AuthDataBlk(const AuthDataBlk& theCertR);

	// Operators
	AuthDataBlk& operator=(const AuthDataBlk & theCertR);

	int Compare(const CertificateDataBlock& theDataR) const;

	// type identification
	CertContentType GetContentType()  const { return CTAuthData; }
	unsigned long ComputeSize()       const;

	// User information access
	unsigned long     GetUserId()     const { return mUserId; }
	const wstring&    GetUserName()   const { return mUserName; }
    const AuthCertificateBase::AccessList& GetAccessList() const { return mAccessList; }

	// Public Key access
	const WONCrypt::EGPublicKey& GetPubKey() const { return mPubKey; }

	// Member update - will invalidate certificate until pack is called again
	void SetUserId(unsigned long theId)        { mUserId = theId; }
	void SetUserName( const wstring &theName ) { mUserName = theName; }
    void AddAccessInfo(unsigned long theCommunityId, unsigned short theTrustLevel) 
	{ mAccessList.push_back(AuthCertificateBase::CommunityAccess(theCommunityId, theTrustLevel)); }

	void SetPublicKey(const WONCrypt::EGPublicKey& theKeyR) { mPubKey = theKeyR; }

	CertificateDataBlock * Duplicate() const;

	bool UnpackData( const unsigned char *theBuf, int theLength );
	bool Serialize( WONCommon::RawBuffer &theRawBuf ) const;
    void Dump( std::ostream& os ) const;

private:
    unsigned long  mUserId;
	WONCrypt::EGPublicKey mPubKey; // Public key
    wstring        mUserName;
	AuthCertificateBase::AccessList     mAccessList;    // List of Community/trust level objects
};

class Auth2Certificate : public AuthCertificateBase {
public:

    // Note: Auth2Certificate maintains ownership of objects in the DataList

    typedef std::list<CertificateDataBlock*> DataList;
    typedef std::list<CertificateDataBlock*>::const_iterator DataListCIter;
    typedef std::list<CertificateDataBlock*>::iterator DataListIter;

	// Default constructor - may provide user info
	Auth2Certificate();

	// Construct from raw representation (calls Unpack())
	Auth2Certificate(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	Auth2Certificate(const Auth2Certificate& theCertR);

	// Operators
	Auth2Certificate& operator=(const Auth2Certificate& theCertR);

	// Destructor
	~Auth2Certificate();

	// Compare (overridden from base class)
	int Compare(const AuthFamilyBuffer& theBufR) const;
	int LenientCompare(const Auth2Certificate& theCertR) const; // only compares this class's data, not expire date, etc.

	// Fetch certificate family
	unsigned short GetFamily() const { return WONAuth::AuthFamily2; }  // From AuthFamilies.h
	const std::wstring& GetUserName() const { return mUserName; }

	const DataList &GetDataList() const { return mDataList; }

    void AddDataBlock( CertificateDataBlock *theData ) { mDataList.push_back( theData ); }

	// Dump to stream
	void Dump(std::ostream& os) const;

private:
    void AppendDataList( const DataList &theDataList );
    void FreeDataList();

    DataList mDataList; // List of data objects in the certificate
	std::wstring mUserName;

	// Compute size of buffer needed for pack and unpack operations.
	// WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

	// Pack local members into base raw buffer
	bool PackData();

	// Unpack local members from base raw buffer
	bool UnpackData();
};

};  // Namespace WONAuth

#endif // _Auth2Certificate_H
