#ifndef _MINI_CRYPT

// filters.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "filters.h"
#include "queue.h"
#include <memory>

NAMESPACE_BEGIN(CryptoPP)

Filter::Filter(BufferedTransformation *outQ)
	: outQueue(outQ ? outQ : new ByteQueue) 
{
}

Filter::Filter(const Filter &source)
	: outQueue(new ByteQueue)
{
}

/*
void Filter::Detach(BufferedTransformation *newOut)
{
	std::auto_ptr<BufferedTransformation> out(newOut ? newOut : new ByteQueue);
	outQueue->Close();
	outQueue->TransferTo(*out);
	outQueue.reset(out.release());
}

void Filter::Attach(BufferedTransformation *newOut)
{
	if (outQueue->Attachable())
		outQueue->Attach(newOut);
	else
		Detach(newOut);
}

void StreamCipherFilter::Put(const byte *inString, unsigned int length)
{
	SecByteBlock temp(length);
	cipher.ProcessString(temp, inString, length);
	outQueue->Put(temp, length);
}

void HashFilter::InputFinished()
{
	SecByteBlock buf(hash.DigestSize());
	hash.Final(buf);
	outQueue->Put(buf, hash.DigestSize());
}

BufferedTransformation *Insert(const byte *in, unsigned int length, BufferedTransformation *outQueue)
{
	outQueue->Put(in, length);
	return outQueue;
}

unsigned int Extract(Source *source, byte *out, unsigned int length)
{
	while (source->MaxRetrieveable() < length && source->Pump(1));
	return source->Get(out, length);
}
*/

NAMESPACE_END

#endif
