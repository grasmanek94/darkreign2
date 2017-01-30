#ifndef CRYPTOPP_CBC_H
#define CRYPTOPP_CBC_H

#include "filters.h"
#include "modes.h"

NAMESPACE_BEGIN(CryptoPP)

class CBCPaddedEncryptor : public Filter, protected CipherMode
{
public:
	CBCPaddedEncryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	void InputFinished();

private:
	void ProcessBuf();
	unsigned int counter;
};

class CBCPaddedDecryptor : public Filter, protected CipherMode
{
public:
	CBCPaddedDecryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	void InputFinished();

private:
	friend class DefaultDecryptor;	// need access to ProcessBuf()
	void ProcessBuf();
	unsigned int counter;
	SecByteBlock temp;
};

/// CBC mode encryptor with ciphertext stealing

/** Ciphertext stealing requires at least cipher.BlockSize() bytes of plaintext.
	Shorter plaintext will be padded with '\0's
*/
/*
class CBC_CTS_Encryptor : public Filter, protected CipherMode
{
public:
	CBC_CTS_Encryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	void InputFinished();

private:
	void ProcessBuf();
	unsigned int counter;
	bool bFirstBlock;
};

class CBC_CTS_Decryptor : public Filter, protected CipherMode
{
public:
	CBC_CTS_Decryptor(const BlockTransformation &cipher, const byte *IV, BufferedTransformation *outQueue = NULL);

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	void InputFinished();

private:
	friend class DefaultDecryptor;	// need access to ProcessBuf()
	void ProcessBuf();
	unsigned int counter;
	bool bFirstBlock;
	SecByteBlock temp;
	SecByteBlock buffer2;
};
*/

NAMESPACE_END

#endif
