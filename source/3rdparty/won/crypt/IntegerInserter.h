#ifndef __MINICRYPT_INTEGERINSERTER_H__
#define __MINICRYPT_INTEGERINSERTER_H__

#pragma warning(disable: 4530)

#include <string>
#include "BigInteger.h"

namespace WONCryptMini
{

class IntegerInserter
{

private:
	RawBuffer out;

public:
	enum 
	{
		INTEGER=0x02,
		BIT_STRING=0x03,
		SEQUENCE=0x10,
		CONSTRUCTED = 0x20
	};

	void AddLength(RawBuffer &theBuf, int theLen);
public:
	IntegerInserter();

	void Insert(const BigInteger &theInt);
	RawBuffer Get();
};

}

#endif