#ifndef _AuthCertificateBase_H
#define _AuthCertificateBase_H

// AuthCertificateBase

// Abstract base class for WON authentication certificates.  A place holder currently
// as it ads no functionality to AuthFamilyBuffer.  It is implemented to allow any
// common certificate functionality to be added later.


#include "AuthFamilyBuffer.h"
#include "Crypt/EGPublicKey.h"
#include <list>

// In the WONAuth namespace
namespace WONAuth {


class AuthCertificateBase : public AuthFamilyBuffer
{
public:
	// Default constructor
	AuthCertificateBase();

	// Copy Constructor
	AuthCertificateBase(const AuthCertificateBase& theCertR);

	// Destructor
	virtual ~AuthCertificateBase();

	// Operators
	AuthCertificateBase& operator=(const AuthCertificateBase& theCertR);

	struct CommunityAccess 
	{
        unsigned long  mCommunityId;
        unsigned short mTrustLevel;

        CommunityAccess() : mCommunityId(0), mTrustLevel(0) {}
        CommunityAccess(unsigned long theCommunityId, unsigned short theTrustLevel) : mCommunityId(theCommunityId), mTrustLevel(theTrustLevel) {}
    };
    typedef std::list<CommunityAccess> AccessList;

	const unsigned long  GetUserId() const { return mUserId; }
	const WONCrypt::EGPublicKey& GetPubKey() const { return mPubKey; }
	const AccessList& GetAccessList() const { return mAccessList; }

	void SetUserId(unsigned long theId) { mUserId = theId; }
	void SetPublicKey(const WONCrypt::EGPublicKey& theKeyR) { mPubKey = theKeyR; }

protected:
	AccessList mAccessList;
	unsigned long mUserId;
	WONCrypt::EGPublicKey mPubKey;

private:
};



};  // Namespace WONAuth

#endif