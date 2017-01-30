#ifndef __MINICRYPT_RANDOM_H__
#define __MINICRYPT_RANDOM_H__

#include "MiniMisc.h"
#include "MD5Digest.h"

namespace WONCryptMini
{

class Random 
{

private:
    RawBuffer state;
    MD5Digest digest;

	RawBuffer randomBytes;
	int randomBytesUsed;
	__int64 counter;

	static long usageCount;
	
public:

	Random();
	Random(const RawBuffer& seed); 
	void setSeed(const RawBuffer& seed);
	void setSeed(__int64 seed);

	void nextBytes(RawBuffer& bytes);
	void nextBytes(void* data, int theLen);

	unsigned int next(int numBits);
	
	static RawBuffer getSeed(int numBytes); 
	RawBuffer longToByteArray(__int64 l); 


};

}
#endif