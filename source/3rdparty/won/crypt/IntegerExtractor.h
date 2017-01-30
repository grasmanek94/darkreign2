#ifndef __MINICRYPT_INTEGEREXTRACTOR_H__
#define __MINICRYPT_INTEGEREXTRACTOR_H__

#pragma warning(disable: 4530)

#include <string>
#include "BigInteger.h"

namespace WONCryptMini
{

class IntegerExtractor
{

private:
	RawBuffer input;
	int offset;
	bool mIsValid;

	bool LengthDecode();

	static unsigned char Decrement(RawBuffer& A, int N);
	static void TwosComplement(RawBuffer& A, int N);

	unsigned char GetByte();
	unsigned char PeekByte();

public:
	enum 
	{
		INTEGER=0x02,
		BIT_STRING=0x03,
		SEQUENCE=0x10,
		CONSTRUCTED = 0x20
	};

public:

	IntegerExtractor(const RawBuffer &input, bool isBufferedTransform = true);

	bool Decode(int inputLen, BigInteger &theInt);
	bool Extract(BigInteger &theInt);
};

}

#endif