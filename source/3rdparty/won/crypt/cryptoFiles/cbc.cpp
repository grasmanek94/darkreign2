#ifndef _MINI_CRYPT

// cbc.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "cbc.h"

NAMESPACE_BEGIN(CryptoPP)

CBCPaddedEncryptor::CBCPaddedEncryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue)
	: Filter(outQueue), CipherMode(cipher, IV), counter(0)
{
}

void CBCPaddedEncryptor::ProcessBuf()
{
	cipher.ProcessBlock(reg);
	outQueue->Put(reg, S);
	counter = 0;
}

void CBCPaddedEncryptor::Put(byte inByte)
{
	reg[counter++] ^= inByte;
	if (counter == S)
		ProcessBuf();
}

void CBCPaddedEncryptor::Put(const byte *inString, unsigned int length)
{
	while (counter && length)
	{
		CBCPaddedEncryptor::Put(*inString++);
		length--;
	}

	while (length >= S)
	{
		xorbuf(reg, inString, S);
		ProcessBuf();
		inString += S;
		length -= S;
	}

	while (length--)
		CBCPaddedEncryptor::Put(*inString++);
}

void CBCPaddedEncryptor::InputFinished()
{
	// pad last block
	byte pad = S-counter;
	do
		Put(pad);
	while (counter != 0);
}

CBCPaddedDecryptor::CBCPaddedDecryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue)
	: Filter(outQueue), CipherMode(cipher, IV), counter(0), temp(S)
{
}

void CBCPaddedDecryptor::ProcessBuf()
{
	cipher.ProcessBlock(buffer, temp);
	xorbuf(temp, reg, S);
	outQueue->Put(temp, S);
	reg.swap(buffer);
	counter = 0;
}

void CBCPaddedDecryptor::Put(byte inByte)
{
	if (counter == S)
		ProcessBuf();
	buffer[counter++] = inByte;
}

void CBCPaddedDecryptor::Put(const byte *inString, unsigned int length)
{
	while (counter!=S && length)
	{
		CBCPaddedDecryptor::Put(*inString++);
		length--;
	}

	while (length >= S)
	{
		ProcessBuf();
		memcpy(buffer, inString, S);
		counter = S;
		inString += S;
		length -= S;
	}

	while (length--)
		CBCPaddedDecryptor::Put(*inString++);
}

void CBCPaddedDecryptor::InputFinished()
{
	// unpad last block
	cipher.ProcessBlock(buffer);
	xorbuf(buffer, reg, S);
	if (buffer[S-1] > S)
		buffer[S-1] = 0;     // something's wrong with the padding
	outQueue->Put(buffer, S-buffer[S-1]);
}

// ********************************************************
/*
CBC_CTS_Encryptor::CBC_CTS_Encryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue)
	: Filter(outQueue), CipherMode(cipher, IV), counter(0), bFirstBlock(true)
{
}

void CBC_CTS_Encryptor::ProcessBuf()
{
	if (bFirstBlock)
		bFirstBlock = false;
	else
		outQueue->Put(reg, S);

	cipher.ProcessBlock(buffer, reg);
	counter = 0;
}

void CBC_CTS_Encryptor::Put(byte inByte)
{
	buffer[counter] = reg[counter] ^ inByte;
	if (++counter == S)
		ProcessBuf();
}

void CBC_CTS_Encryptor::Put(const byte *inString, unsigned int length)
{
	while (counter && length)
	{
		CBC_CTS_Encryptor::Put(*inString++);
		length--;
	}

	while (length >= S)
	{
		xorbuf(buffer, inString, reg, S);
		ProcessBuf();
		inString += S;
		length -= S;
	}

	while (length--)
		CBC_CTS_Encryptor::Put(*inString++);
}

void CBC_CTS_Encryptor::InputFinished()
{
	// padded with '\0's if plaintext is too short
	while (bFirstBlock)
		Put('\0');

	if (counter)
	{
		// output last full ciphertext block first
		memcpy(buffer+counter, reg+counter, S-counter);
		cipher.ProcessBlock(buffer);
		outQueue->Put(buffer, S);
		// steal ciphertext from next to last block
		outQueue->Put(reg, counter);
	}
	else
		outQueue->Put(reg, S);	// no need to do ciphertext stealing
}

CBC_CTS_Decryptor::CBC_CTS_Decryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue)
	: Filter(outQueue), CipherMode(cipher, IV), counter(0), bFirstBlock(true), temp(S), buffer2(S)
{
}

void CBC_CTS_Decryptor::ProcessBuf()
{
	if (bFirstBlock)
		bFirstBlock = false;
	else
	{
		cipher.ProcessBlock(buffer, temp);
		xorbuf(temp, reg, S);
		outQueue->Put(temp, S);
		reg.swap(buffer);
	}

	buffer.swap(buffer2);
	counter = 0;
}

void CBC_CTS_Decryptor::Put(byte inByte)
{
	buffer2[counter++] = inByte;
	if (counter == S)
		ProcessBuf();
}

void CBC_CTS_Decryptor::Put(const byte *inString, unsigned int length)
{
	while (counter && length)
	{
		CBC_CTS_Decryptor::Put(*inString++);
		length--;
	}

	while (length >= S)
	{
		memcpy(buffer2, inString, S);
		ProcessBuf();
		inString += S;
		length -= S;
	}

	while (length--)
		CBC_CTS_Decryptor::Put(*inString++);
}

void CBC_CTS_Decryptor::InputFinished()
{
	if (counter)
	{
		// decrypt last partial plaintext block
		cipher.ProcessBlock(buffer, temp);
		xorbuf(temp, buffer2, counter);

		// decrypt next to last plaintext block
		memcpy(buffer2+counter, temp+counter, S-counter);
		cipher.ProcessBlock(buffer2);
		xorbuf(buffer2, reg, S);

		outQueue->Put(buffer2, S);
		outQueue->Put(temp, counter);
	}
	else
		ProcessBuf();	// no need to do ciphertext stealing
}
*/

NAMESPACE_END

#endif
