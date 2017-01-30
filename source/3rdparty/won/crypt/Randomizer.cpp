#ifndef _MINI_CRYPT

// Randomizer

// Static class that maintains a random number pool for generating keys within then
// crypt library.  Randomizer is used to generate Symmetric keys and Public/Private
// key pairs.


#include "common/won.h"
#include "cryptoFiles/cryptlib.h"
#include "cryptoFiles/randpool.h"
#include "Randomizer.h"

// private namespace for using and constants
namespace {
	using namespace CryptoPP;
	using WONCrypt::Randomizer;
};

#ifdef WIN32
// GetDiskFreeSpaceEx() function definition
typedef BOOL (WINAPI * pfnGetDiskFreeSpaceEx)(LPCTSTR, PULARGE_INTEGER, 
	PULARGE_INTEGER, PULARGE_INTEGER);
#endif

// Static members
auto_ptr<CryptoPP::RandomPool> Randomizer::mPoolP(NULL);

// ** Private Methods **

// Randomizer::AllocatePool
// Generates a seed from noise as follows:
//	1) Initial seed is larger of BytesLeft and BytesUsed on current hard disk
//	2) Seed high 2 bytes are replaced by low 2 bytes of system ticks
//	3) Seed is xored with current time.
__int64
Randomizer::GenerateSeed()
{
	__int64 aSeed = 0;

#ifdef WIN32
	WTRACE("Randomizer::GenerateSeed");
	ULARGE_INTEGER aSysTime;

	// Calc bytesUsed and use larger of used/left as start of seed
	// (3rd param of GetDiskFreeSpaceEx returns total bytes on disk.)

	// GetDiskFreeSpaceEx is not available in all versions of Windows.  Use
	// LoadLibrary/GetProcAddress to determine if it's available.
	HMODULE hKernel = ::LoadLibrary("kernel32");
	pfnGetDiskFreeSpaceEx fnGetDiskFreeSpaceEx = 
								reinterpret_cast<pfnGetDiskFreeSpaceEx>
								(::GetProcAddress(hKernel, "GetDiskFreeSpaceExA"));

	if (fnGetDiskFreeSpaceEx != NULL)
	{
		WDBG_LM("Randomizer::GenerateSeed - using GetDiskFreeSpaceEx");
		ULARGE_INTEGER aBytesLeft, aBytesUsed;
		fnGetDiskFreeSpaceEx(NULL, &aSysTime, &aBytesUsed, &aBytesLeft);
		aBytesUsed.QuadPart -= aBytesLeft.QuadPart;
		aSeed = (aBytesLeft.QuadPart < aBytesUsed.QuadPart ? aBytesUsed.QuadPart
														   : aBytesLeft.QuadPart);
	}
	else
	{
		WDBG_LM("Randomizer::GenerateSeed - using GetDiskFreeSpace");
		unsigned long sectorsPerCluster, bytesPerSector, freeClusters, usedClusters;
		GetDiskFreeSpace(NULL, &sectorsPerCluster, &bytesPerSector, &freeClusters, &usedClusters);
		usedClusters -= freeClusters;
		aSeed = (usedClusters < freeClusters ? freeClusters : usedClusters) * sectorsPerCluster * bytesPerSector;
	}
	::FreeLibrary(hKernel);

	// Use the low bytes from sys tick as high bytes in seed
	WDBG_LM("Randomizer::GenerateSeed Initial Seed=" << aSeed);
	unsigned long aSysTick = GetTickCount();
	*(reinterpret_cast<unsigned short*>(&aSeed)) = *(reinterpret_cast<unsigned short*>(&aSysTick) + 1);
	WDBG_LL("Randomizer::GenerateSeed Use low bytes from SysTicks=" << aSysTick);

	// Now xor seed with current time
	GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&aSysTime));
	WDBG_LL("Randomizer::GenerateSeed XOR Seed with current time.");
	aSeed ^= aSysTime.QuadPart;

	// Seed the pool
	WDBG_LM("Randomizer::GenerateSeed Seed=" << aSeed);
#else//if defined(_LINUX)
	time_t t;
	time(&t);
	aSeed = t;
#endif
	return aSeed;
}


void
Randomizer::AllocatePool()
{
	// Allocate pool. (Delete existing pool first if needed)
	WTRACE("Randomizer::AllocatePool");
	WDBG_LL("Randomizer::AllocatePool Allocating new RandomPool.");
	auto_ptr<RandomPool> tempAutoPool(new RandomPool);
	delete mPoolP.release();
	mPoolP = tempAutoPool;

	// Seed the pool
	WDBG_LM("Randomizer::AllocatePool Seeding pool.");
	__int64 aSeed = GenerateSeed();
	mPoolP->Put(reinterpret_cast<byte*>(&aSeed), sizeof(aSeed));
}


// ** Public Methods **

// Randomizer::GetByte()
// Returns a random byte.
unsigned char
Randomizer::GetByte() throw(Exception)
{
	try
	{
		if (! mPoolP.get()) AllocatePool();
		return mPoolP->GetByte();
	}
#ifdef _DEBUG
	catch (Exception& anEx)
#else
	catch (Exception&)
#endif
	{
		WDBG_AH("Randomizer::GetByte Caught CryptoLib exception: " << anEx.what());
		throw;
	}
}


// Randomizer::GetShort()
// Returns a random short (2 bytes).
unsigned short
Randomizer::GetShort() throw(Exception)
{
	try
	{
		if (! mPoolP.get()) AllocatePool();
		return mPoolP->GetShort();
	}
#ifdef _DEBUG
	catch (Exception& anEx)
#else
	catch (Exception&)
#endif
	{
		WDBG_AH("Randomizer::GetShort Caught CryptoLib exception: " << anEx.what());
		throw;
	}
}


// Randomizer::GetLong()
// Returns a random long (4 bytes).
unsigned long
Randomizer::GetLong() throw(Exception)
{
	try
	{
		if (! mPoolP.get()) AllocatePool();
		return mPoolP->GetLong();
	}
#ifdef _DEBUG
	catch (Exception& anEx)
#else
	catch (Exception&)
#endif
	{
		WDBG_AH("Randomizer::GetLong Caught CryptoLib exception: " << anEx.what());
		throw;
	}
}


// Randomizer::GetBlock()
// Returns a random block (theLen bytes).
void
Randomizer::GetBlock(unsigned char* theBlock, unsigned long theLen) throw(Exception)
{
	try
	{
		if (! mPoolP.get()) AllocatePool();
		mPoolP->GetBlock(theBlock, theLen);
	}
#ifdef _DEBUG
	catch (Exception& anEx)
#else
	catch (Exception&)
#endif
	{
		WDBG_AH("Randomizer::GetBlock Caught CryptoLib exception: " << anEx.what());
		throw;
	}
}


// Randomizer::Reset()
// Resets the random pool (regenerates it).
void
Randomizer::Reset() throw(Exception)
{
	try
	{
		AllocatePool();
	}
#ifdef _DEBUG
	catch (Exception& anEx)
#else
	catch (Exception&)
#endif
	{
		WDBG_AH("Randomizer::Reset Caught CryptoLib exception: " << anEx.what());
		throw;
	}
}


// Randomizer::GetPool()
// Returns the random pool.  This method is meant to be used internally in the
// crypt lib, not by external objects.
RandomPool&
Randomizer::GetPool()
{
	if (! mPoolP.get()) AllocatePool();
	return *mPoolP;
}

#endif
