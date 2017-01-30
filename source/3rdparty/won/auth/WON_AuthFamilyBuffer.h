#ifndef _WON_AUTHFAMILYBUFFER_H
#define _WON_AUTHFAMILYBUFFER_H

// WON_AuthFamilyBuffer

// Base interface class for Auth buffers within the WONAuth library.  AuthFamilyBuffer
// is an abstract base class, all constructors are protected.

// This class simply makes use of the AuthFamilyBuffer class.  It eliminates the STL
// and VC5 ehancements from the interface.

// *IMPORTANT**
// Implementation relies on the fact that mBufP will NEVER be NULL after construction.

// *NOTE*
// This header is compatible with VC4.  Ergo, no using bool, string, etc.


#include "WON_AuthFactory.h"


class DLL_OPER WON_AuthFamilyBuffer
{
public:
	// Constants
	enum {
		RETURN_FALSE = 0,
		RETURN_TRUE  = (! RETURN_FALSE)
	};

	// Operators
	int operator==(const WON_AuthFamilyBuffer& theBufR) const;
	int operator!=(const WON_AuthFamilyBuffer& theBufR) const;
	int operator< (const WON_AuthFamilyBuffer& theBufR) const;
	int operator<=(const WON_AuthFamilyBuffer& theBufR) const;
	int operator> (const WON_AuthFamilyBuffer& theBufR) const;
	int operator>=(const WON_AuthFamilyBuffer& theBufR) const;

	// Check validity.  Returns true if last call to Pack/Unpack was successful,
	// otherwise false.  Will return false if Pack/Unpack have not been called.
	int IsValid() const;

	// Compare two buffers
	int Compare(const WON_AuthFamilyBuffer& theBufR) const;

	// Pack into raw form.  Must specifiy raw form of ElGamal private key to use
	// for signature generation.  Returns true on success, false on failure.
	int Pack(const unsigned char* thePrivKeyP, unsigned short theLen);

	// Unpack from raw form.  Returns true on success, false on failure.
	int Unpack(const unsigned char* theRawP, unsigned short theLen);

	// Verifies signature using specified raw form of ElGamal public key.  Must be
	// valid (IsValid() returns true).  Returns true on success, false on failure.
	int Verify(const unsigned char* thePubKeyP, unsigned short theLen) const;

	// Fetch Auth Family ID
	unsigned short GetFamily() const;

	// Lifespan access
	time_t        GetIssueTime() const;
	time_t        GetExpireTime() const;
	unsigned long GetLifespan() const;

	// Lifespan update
	void SetLifespan(time_t theIssueTime, unsigned long theLifespan);
	void SetIssueTime(time_t theTime);
	void SetExpireTime(time_t theTime);

	// Fetch raw buffer.  Return NULL/0 if IsValid is false
	const unsigned char* GetRaw() const;
	unsigned short       GetRawLen() const;

	// Fetch data or sig portion of buffer.  Both will return NULL/0
	// if IsValid() is false.
	const unsigned char* GetData() const;       // Return data block only
	unsigned short       GetDataLen() const;
	const unsigned char* GetSignature() const;  // Return sig block only
	unsigned short       GetSignatureLen() const;

protected:
	// Time to trick the compiler.  Need mBufP to point to a class in the WONAuth
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mBufP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mKeyP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	// MUST be allocated by derivec classes in constructor!  Cannot BE NULL after
	// construction!!
	WONAuth::AuthFamilyBuffer* mBufP;  // Real buffer object (allocated)
#else
	void* mBufP;
#endif

	// Protected Constructors
	WON_AuthFamilyBuffer();
	WON_AuthFamilyBuffer(const WON_AuthFamilyBuffer& theKeyR);

	// Destructor
	virtual ~WON_AuthFamilyBuffer();

	// Protected Operators
	WON_AuthFamilyBuffer& operator=(const WON_AuthFamilyBuffer& theBufR);

private:
};


// Inlines
inline int
WON_AuthFamilyBuffer::operator==(const WON_AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) == 0); }

inline int
WON_AuthFamilyBuffer::operator!=(const WON_AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) != 0); }

inline int
WON_AuthFamilyBuffer::operator<(const WON_AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) < 0); }

inline int
WON_AuthFamilyBuffer::operator<=(const WON_AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) <= 0); }

inline int
WON_AuthFamilyBuffer::operator>(const WON_AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) > 0); }

inline int
WON_AuthFamilyBuffer::operator>=(const WON_AuthFamilyBuffer& theBufR) const
{ return (Compare(theBufR) >= 0); }


#endif