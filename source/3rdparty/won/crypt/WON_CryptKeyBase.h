#ifndef _WON_CRYPTKEYBASE_H
#define _WON_CRYPTKEYBASE_H

// WON_CryptKeyBase

// Base interface class for keys within the WONCrypt library.  CryptKeyBase is an
// abstract base class, all constructors are protected and there is a pure vitural
// method to be overridden.

// This class simply makes use of the CryptKeyBase class.  It eliminates the STL
// and exceptions from the interface.

// *NOTE*
// This header is compatible with VC4.  Ergo, no using bool, string, etc.


#include "WON_CryptFactory.h"


class DLL_OPER WON_CryptKeyBase
{
public:
	// Constants
	enum {
		RETURN_FALSE = 0,
		RETURN_TRUE  = (! RETURN_FALSE)
	};

	// Types
	// CryptReturn - Data class used to return variable length binary blocks.
	// Allows fetch of data pointer and length of data.  Note that CryptReturn
	// takes ownership of theDataP in its default constructor; it will delete this
	// pointer when destructed.  Also note, that ownership of mData is exchanged
	// by the copy constructor and asignment operator.  The target object of the
	// copy/assignment will now own/delete mData; the original object will not delete
	// it.  As a result, outside of the WONCrypt lib, ONLY PASS CryptReturn instances
	// BY REFERENCE!
	class DLL_OPER CryptReturn
	{
	public:
		CryptReturn(unsigned char* theDataP=NULL, unsigned long theLen=0);
		CryptReturn(const CryptReturn&);
		~CryptReturn();
		CryptReturn& operator=(const CryptReturn&);
		const unsigned char* GetData() const { return mData; }
		unsigned long        GetLen()  const { return mLen; }
	private:
		unsigned char* mData;
		unsigned long  mLen;
		unsigned char  mOwnIt;
	};

	// Operators
	WON_CryptKeyBase& operator=(const WON_CryptKeyBase& theKeyR);
	int operator==(const WON_CryptKeyBase& theKeyR) const;
	int operator!=(const WON_CryptKeyBase& theKeyR) const;
	int operator< (const WON_CryptKeyBase& theKeyR) const;

	// Fetch raw key
	const unsigned char* GetKey()    const;
	unsigned short       GetKeyLen() const;

	// Get last error.  Returned pointer will be NULL if no error occured.  Returned
	// pointer is only valid until next call to a method that makes use of last error.
	// See derived headers for methods that will reset lastError.
	const char* GetLastError() const;

	// Class Methods
	// Fetch default key length
	static unsigned short GetDefKeyLen();

protected:
	char* mLastErrP;  // Text of last error (NULL if no error occurred)

	// Time to trick the compiler.  Need mKeyP to point to a class in the WONCrypt
	// namespace.  Also need this header to compile in VC4.2 (i.e., no namespaces).
	// Ergo, define mKeyP to be void* for normal compiles and real pointer for
	// source compiles.  This will work since mKeyP is NEVER referenced outside
	// of the source module for this header.
#ifdef _WON_SOURCE_
	WONCrypt::CryptKeyBase* mKeyP;  // Real key object (allocated)
#else
	void* mKeyP;
#endif

	// Protected Constructors
	WON_CryptKeyBase();
	WON_CryptKeyBase(const WON_CryptKeyBase& theKeyR);

	// Protected Destructor
	virtual ~WON_CryptKeyBase();

	// Protected Methods
	void SetLastError(const char* theErrP);

	// Pure virtual method to allocate key object
	virtual void AllocateKey() = 0;

private:
	// Factory is a friend
	friend class WON_CryptFactory;
};


// Inlines
inline int
WON_CryptKeyBase::operator!=(const WON_CryptKeyBase& theKeyR) const
{ return (! operator==(theKeyR)); }

inline const char*
WON_CryptKeyBase::GetLastError() const
{ return mLastErrP; }


#endif