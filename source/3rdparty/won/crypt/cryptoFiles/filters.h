#ifndef CRYPTOPP_FILTERS_H
#define CRYPTOPP_FILTERS_H

#include "cryptlib.h"
#include "misc.h"
#include "smartptr.h"

NAMESPACE_BEGIN(CryptoPP)

class Filter : public BufferedTransformation
{
public:
	Filter(BufferedTransformation *outQ = NULL);
	Filter(const Filter &source);

	bool Attachable() {return true;}
	//void Detach(BufferedTransformation *newOut = NULL);
	//void Attach(BufferedTransformation *newOut);
	void Close()
		{InputFinished(); outQueue->Close();}

	unsigned long MaxRetrieveable()
		{return outQueue->MaxRetrieveable();}

	unsigned int Get(byte &outByte)
		{return outQueue->Get(outByte);}
	unsigned int Get(byte *outString, unsigned int getMax)
		{return outQueue->Get(outString, getMax);}

	unsigned int Peek(byte &outByte) const
		{return outQueue->Peek(outByte);}

	BufferedTransformation *OutQueue() {return outQueue.get();}

protected:
	member_ptr<BufferedTransformation> outQueue;

private:
	void operator=(const Filter &);	// assignment not allowed
};

/*
class StreamCipherFilter : public Filter
{
public:
	StreamCipherFilter(StreamCipher &c,
					   BufferedTransformation *outQueue = NULL)
		: cipher(c), Filter(outQueue) {}

	void Put(byte inByte)
		{outQueue->Put(cipher.ProcessByte(inByte));}

	void Put(const byte *inString, unsigned int length);

private:
	StreamCipher &cipher;
};

class HashFilter : public Filter
{
public:
	HashFilter(HashModule &hm, BufferedTransformation *outQueue = NULL)
		: hash(hm), Filter(outQueue) {}

	void InputFinished();

	void Put(byte inByte)
		{hash.Update(&inByte, 1);}

	void Put(const byte *inString, unsigned int length)
		{hash.Update(inString, length);}

private:
	HashModule &hash;
};

class Source : public Filter
{
public:
	Source(BufferedTransformation *outQ = NULL)
		: Filter(outQ) {}

	void Put(byte)
		{Pump(1);}
	void Put(const byte *, unsigned int length)
		{Pump(length);}
	void InputFinished()
		{PumpAll();}

	virtual unsigned int Pump(unsigned int size) =0;
	virtual unsigned long PumpAll() =0;
};

class Sink : public BufferedTransformation
{
public:
	unsigned long MaxRetrieveable()
		{return 0;}
	unsigned int Get(byte &)
		{return 0;}
	unsigned int Get(byte *, unsigned int)
		{return 0;}
	unsigned int Peek(byte &) const
		{return 0;}
};

class BitBucket : public Sink
{
public:
	void Put(byte) {}
	void Put(const byte *, unsigned int) {}
};

BufferedTransformation *Insert(const byte *in, unsigned int length, BufferedTransformation *outQueue);
unsigned int Extract(Source *source, byte *out, unsigned int length);
*/

NAMESPACE_END

#endif
