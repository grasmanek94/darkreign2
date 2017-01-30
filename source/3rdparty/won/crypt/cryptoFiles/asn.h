#ifndef CRYPTOPP_ASN_H
#define CRYPTOPP_ASN_H

#include "cryptlib.h"
#include "queue.h"

NAMESPACE_BEGIN(CryptoPP)

// NOTE: these tags and flags are NOT COMPLETE!
enum ASNTag {INTEGER=0x02, BIT_STRING=0x03, SEQUENCE=0x10};
enum ASNIdFlag {CONSTRUCTED = 0x20};

unsigned int DERLengthEncode(unsigned int length, byte *output);
unsigned int DERLengthEncode(unsigned int length, BufferedTransformation &);

#define BERDecodeError() throw BERDecodeErr()

class BERDecodeErr : public Exception {public: BERDecodeErr() : Exception("BER decode error") {}};

bool BERLengthDecode(BufferedTransformation &, unsigned int &);

class BERSequenceDecoder : public BufferedTransformation
{
public:
	BERSequenceDecoder(BufferedTransformation &inQueue);
	~BERSequenceDecoder();

	void Put(byte) {}
	void Put(const byte *, unsigned int) {}

	unsigned long MaxRetrieveable()
		{return inQueue.MaxRetrieveable();}
	unsigned int Get(byte &outByte)
		{return inQueue.Get(outByte);}
	unsigned int Get(byte *outString, unsigned int getMax)
		{return inQueue.Get(outString, getMax);}
	unsigned int Peek(byte &outByte) const
		{return inQueue.Peek(outByte);}

private:
	BufferedTransformation &inQueue;
	bool definiteLength;
	unsigned int length;
};

class DERSequenceEncoder : public ByteQueue
{
public:
	DERSequenceEncoder(BufferedTransformation &outQueue);
	~DERSequenceEncoder();
private:
	BufferedTransformation &outQueue;
};

NAMESPACE_END

#endif
