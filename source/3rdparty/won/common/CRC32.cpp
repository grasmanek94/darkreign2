// CRC32

// Class implements a 32 bit CRC on an arbitrary stream of bytes.  This code was
// taken from the following location (web):
//		http://www.w3c.rl.ac.uk/isopng/D-PNG-CRCAppendix.html


#include "common/won.h"
#include "common/CriticalSection.h"
#include "CRC32.h"

// Private namespace
namespace {
	using WONCommon::CRC32;

	// Constants
	const unsigned long POLYNOMIAL    = 0xedb88320L;
	const unsigned int  CRCTABLE_SIZE = 256;

	// CRC table
	unsigned long              gCRCTable[CRCTABLE_SIZE];
	bool                       gIsInit = false;
	WONCommon::CriticalSection gInitCrit;
};


// ** Constructors / Destructor **

CRC32::CRC32(unsigned long theSeed) :
	mRegister(theSeed),
	mSeed(theSeed)
{
	LoadCRCTable();
}


// Copy Constructor
CRC32::CRC32(const CRC32& theCRCR) :
	mRegister(theCRCR.mRegister),
	mSeed(theCRCR.mSeed)
{
	LoadCRCTable();
}


// Destructor
CRC32::~CRC32()
{}


// ** Private Methods **

// CRC32::LoadCRCTable
// Initializes the global CRC tablewith appropriate values.  This is a static method.
void
CRC32::LoadCRCTable()
{
	WONCommon::AutoCrit aCrit(gInitCrit);
	if (gIsInit)
		return;

	for (unsigned long i = 0; i < CRCTABLE_SIZE; i++)
	{
		unsigned long c = i;
		for (int j = 0; j < 8; j++)
		{
			if (c & 1)
				c = POLYNOMIAL ^ (c >> 1);
			else
				c = c >> 1;
		}
		gCRCTable[i] = c;
	}
   
	gIsInit = true;
}


// ** Public Methods **

// Assignment operator
CRC32&
CRC32::operator=(const CRC32& theCRCR)
{
	if (this != &theCRCR)
	{
		mRegister = theCRCR.mRegister;
		mSeed     = theCRCR.mSeed;
	}
	return *this;
}


// CRC32::Put
// Perform CRC calculations on theLen bytes.  Running CRC is kept in mRegister.
void
CRC32::Put(const unsigned char* theDataP, unsigned int theLen)
{
	unsigned long c = mRegister;
	for (unsigned int i = 0; i < theLen; i++)
	{
		mRegister = gCRCTable[(c ^ theDataP[i]) & 0xff] ^ (c >> 8);
	}
}
