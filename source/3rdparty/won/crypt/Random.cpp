#include "Random.h"


#ifdef _MINI_CRYPT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <memory>
#include <time.h>
#include "Random.h"

using namespace std;
using namespace WONCryptMini;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


long Random::usageCount = 0;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Random::Random() 
{
	setSeed(getSeed(20));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Random::Random(const RawBuffer& seed) 
{
	setSeed(seed);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Random::setSeed(const RawBuffer& seed) 
{
	if (state.length()!=0)
		digest.update(state);

	state = digest.digest(seed);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Random::setSeed(__int64 seed) 
{
	if (seed != 0)
		setSeed(longToByteArray(seed));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Random::nextBytes(RawBuffer& bytes) 
{
	int numRequested = bytes.length();
	int numGot = 0;

	while (numGot < numRequested) 
	{
		/* If no more random bytes, make some more */
		if (randomBytes.length() == 0 || randomBytesUsed == randomBytes.length()) 
		{
			digest.update(state);
			randomBytes = digest.digest(longToByteArray(counter++));
			randomBytesUsed = 0;
		}

		bytes[numGot++] = randomBytes[randomBytesUsed++];
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Random::nextBytes(void* data, int theLen)
{
	char *bytes = (char*)data;

	int numRequested = theLen;
	int numGot = 0;

	while (numGot < numRequested) 
	{
		/* If no more random bytes, make some more */
		if (randomBytes.length() == 0 || randomBytesUsed == randomBytes.length()) 
		{
			digest.update(state);
			randomBytes = digest.digest(longToByteArray(counter++));
			randomBytesUsed = 0;
		}

		bytes[numGot++] = randomBytes[randomBytesUsed++];
	}
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned int Random::next(int numBits) 
{
	int numBytes = (numBits+7)/8;
	RawBuffer b(numBytes,0);
	
	unsigned int next = 0;

	nextBytes(b);
	for (int i=0; i<numBytes; i++)
		next = (next << 8) + (b[i] & 0xFF);

		return next >> (numBytes*8 - numBits);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

RawBuffer Random::getSeed(int numBytes) 
{
	RawBuffer aBuf(8,0);
	ULARGE_INTEGER aSysTime;
	long aUsageCount = InterlockedIncrement(&usageCount);

	// Calc bytesUsed and use larger of used/left as start of seed
	// (3rd param of GetDiskFreeSpaceEx returns total bytes on disk.)
	__int64 aSeed = 0;

	unsigned long sectorsPerCluster, bytesPerSector, freeClusters, usedClusters;
	GetDiskFreeSpace(NULL, &sectorsPerCluster, &bytesPerSector, &freeClusters, &usedClusters);
	usedClusters -= freeClusters;
	aSeed = (usedClusters < freeClusters ? freeClusters : usedClusters) * sectorsPerCluster * bytesPerSector;

	// Use the low bytes from sys tick as high bytes in seed
	unsigned long aSysTick = GetTickCount();
	*(reinterpret_cast<unsigned short*>(&aSeed)) = *(reinterpret_cast<unsigned short*>(&aSysTick) + 1);

	// Now xor seed with current time
	GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&aSysTime));
	aSeed ^= aSysTime.QuadPart;
	aSeed+=aUsageCount;

	return aBuf.assign((unsigned char*)&aSeed,8);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

RawBuffer Random::longToByteArray(__int64 l) 
{
	RawBuffer retVal(8,0);

	for (int i=0; i<8; i++) 
	{
		retVal[i] = (unsigned char) l;
		l >>= 8;
	}

	return retVal;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif
