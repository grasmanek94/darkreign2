#ifndef __MINICRYPT_BLOWFISH_H__
#define __MINICRYPT_BLOWFISH_H__

#pragma warning(disable: 4530)

#include <string>
#include "MiniMisc.h"

namespace WONCryptMini
{

class Blowfish
{

public:
	enum
	{
		minKeyLen    = 5,    // min key length in bytes (40 bits)
		blockLength  = 8,   
		maxKeyLen    = 56    // max key length in bytes (448 bits)
	};

private:
	static const int Pinit[];
	static const int Sinit[];

	int S0[256];
    int S1[256];
    int S2[256];
    int S3[256];
    int P[18];
	
	RawBuffer XOR_MASK;
								 
    int P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15, P16, P17;
	
	bool mIsValid;

    void encryptTwoIntegers(int in[], int inOff, int out[], int outOff);
    void encryptBlock(const RawBuffer &in, int off, RawBuffer &out, int outOff);
    void decryptBlock(const RawBuffer &in, int off, RawBuffer &out, int outOff);	
	void initXORMask();

public:
	Blowfish();
	Blowfish(const RawBuffer& skey); 
	Blowfish(const void *theKey, int theKeyLen);

	bool setKey(const RawBuffer& skey); 
	bool setKey(const void *theKey, int theKeyLen);

	bool encrypt(const RawBuffer& in, RawBuffer &out);
	bool encrypt(const void *in, int inLen, RawBuffer &out);

	bool decrypt(const RawBuffer& in, RawBuffer &out);
	bool decrypt(const void *in, int inLen, RawBuffer &out);

	bool create(int theLen);
	
};

}
#endif