#ifndef _Auth1Certificate_H
#define _Auth1Certificate_H

// Auth1Certificate

// WON Authentication Certificate for Auth Family 1.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//	User Info - UserId, CommunityId, TrustLevel
//	Client's Public Key (ElGamal)

// Used as system ticket for various authentication protocols within the WON
// system.


#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthCertificateBase.h"

// In the WONAuth namespace
namespace WONAuth {


class Auth1Certificate : public AuthCertificateBase
{
public:
	// Default constructor - may provide user info
	Auth1Certificate(unsigned long theUserId=0, unsigned long theCommunityId=0,
	                 unsigned short theTrustLevel=0);

	// Construct from raw representation (calls Unpack())
	Auth1Certificate(const unsigned char* theRawP, unsigned short theLen);

	// Copy Constructor
	Auth1Certificate(const Auth1Certificate& theCertR);

	// Destructor
	~Auth1Certificate();

	// Operators
	Auth1Certificate& operator=(const Auth1Certificate& theCertR);

	// Compare (overridden from base class)
	int Compare(const AuthFamilyBuffer& theBufR) const;
	int LenientCompare(const Auth1Certificate& theCertR) const; // only compares this class's data, not expire date, etc.

	// Fetch certificate family
	unsigned short GetFamily() const;

	// User information access
	const unsigned long  GetCommunityId() const;
	const unsigned short GetTrustLevel() const;

	// Public Key access
	const WONCrypt::EGPublicKey& GetPubKey() const;

	// Member update
	void SetCommunityId(unsigned long theId);
	void SetTrustLevel(unsigned short theLevel);

	// Dump to stream
	void Dump(std::ostream& os) const;

private:
	// Compute size of buffer needed form pack and unpack operations.
	WONCommon::RawBuffer::size_type ComputeBufSize(SizeComputeMode theMode) const;

	// Pack local members into base raw buffer
	bool PackData();

	// Unpack local members from base raw buffer
	bool UnpackData();
};


inline const unsigned long
Auth1Certificate::GetCommunityId() const
{ return mAccessList.front().mCommunityId; }

inline const unsigned short
Auth1Certificate::GetTrustLevel() const
{ return mAccessList.front().mTrustLevel; }

inline const WONCrypt::EGPublicKey&
Auth1Certificate::GetPubKey() const
{ return mPubKey; }

inline void
Auth1Certificate::SetCommunityId(unsigned long theId)
{ mAccessList.front().mCommunityId = theId; }

inline void
Auth1Certificate::SetTrustLevel(unsigned short theLevel)
{ mAccessList.front().mTrustLevel = theLevel; }

};  // Namespace WONAuth

#endif