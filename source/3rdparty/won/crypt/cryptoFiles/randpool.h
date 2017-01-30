#ifndef CRYPTOPP_RANDPOOL_H
#define CRYPTOPP_RANDPOOL_H

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

class RandomPool : public RandomNumberGenerator,
				   public BufferedTransformation
{
public:
	// poolSize must be greater than 16
	RandomPool(unsigned int poolSize=384);

	// interface for BufferedTransformation
	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	unsigned int Get(byte &outByte);
	unsigned int Get(byte *outString, unsigned int getMax);
	unsigned int Peek(byte &outByte) const;

	// return 0 to prevent infinite loops
	unsigned long MaxRetrieveable() {return 0;}

	// interface for RandomNumberGenerator
	byte GetByte()
		{byte b; RandomPool::Get(b); return b;}
	void GetBlock(byte *output, unsigned int size)
		{Get(output, size);}

	// help compiler disambiguate
	word16 GetShort(word16 min=0, word16 max=0xffff)
		{return RandomNumberGenerator::GetShort(min, max);}
	word32 GetLong(word32 min=0, word32 max=0xffffffffL)
		{return RandomNumberGenerator::GetLong(min, max);}

protected:
	void Stir();

private:
	SecByteBlock pool, key;
	unsigned int addPos, getPos;
};

NAMESPACE_END

#endif
