#include "IntegerExtractor.h"	

#ifdef _MINI_CRYPT

using namespace std;
using namespace WONCryptMini;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

IntegerExtractor::IntegerExtractor(const RawBuffer &input, bool isBufferedTransform)
{
	this->input = input;	
	
	offset = 0;
	mIsValid = true;
	
	if(isBufferedTransform)
	{	
		if(GetByte() != (SEQUENCE | CONSTRUCTED))
			return;
		else if(!LengthDecode())
			return;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned char IntegerExtractor::GetByte()
{
	if(offset<input.length())
		return input[offset++];

	mIsValid = false;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned char IntegerExtractor::PeekByte()
{
	if(offset<input.length())
		return input[offset];

	mIsValid = false;
	return 0;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned char IntegerExtractor::Decrement(RawBuffer& A, int N)
{
	unsigned char t = A[0];
	A[0] = (unsigned char)(t-1);
	if (A[0] <= t)
		return 0;
	
	for (int i=1; i<N; i++)
		if (A[i]-- != 0)
			return 0;
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void IntegerExtractor::TwosComplement(RawBuffer& A, int N)
{
	Decrement(A, N);
	for (int i=0; i<N; i++)
		A[i] = (unsigned char)~A[i];
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool IntegerExtractor::LengthDecode()
{
	int length;
	unsigned char b = GetByte();

	if ((b & 0x80) == 0)
		length = b;
	else
	{
		int lengthBytes = b & 0x7f;
		if (input.length() - offset < lengthBytes)
			return false;

		b = GetByte();
		while (b==0 && lengthBytes>1)
		{
			b = GetByte();
			lengthBytes--;
		}

		switch (lengthBytes)
		{
			case 0:
				return false;   // indefinite length
			case 1:
				length = b;
				break;
			case 2:
				length = b << 8;
				length |= GetByte();
				break;
			default:
				return false;
		}
	}
	return mIsValid;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool IntegerExtractor::Decode(int inputLen, BigInteger &theInt)
{
	if(!mIsValid)
		return false;

	if(input.length()<=offset)
		return false;

	bool negative = ((PeekByte() & 0x80)!=0) ? true : false;

	while (inputLen>0 && PeekByte()==0)
	{
		offset++;
		inputLen--;
	}

	RawBuffer reg(inputLen,0);
			
	for (int i=0; i<inputLen; i++)
	{
		reg[i] = GetByte();
	}

/*		
	if (negative)
	{
		for (int i=inputLen; i<reg.length; i++)
			reg[i] = (byte)0xff;
		
		TwosComplement(reg, reg.length);
	}*/

	theInt.fromByteArray(reg);
	return mIsValid;
}	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


bool IntegerExtractor::Extract(BigInteger &theInt)
{	
	if(!mIsValid)
		return false;

	if (GetByte() != INTEGER)
		return false;


	int bc;
	if ((PeekByte() & 0x80)==0)
		bc = GetByte();
	else
	{
		int lengthBytes = GetByte() & 0x7f;
		if (lengthBytes > 2)
			return false;
	
		bc = GetByte();
		if (lengthBytes > 1)
		{
			bc = (bc << 8) | GetByte();
		}
	}


	return Decode(bc,theInt);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif

