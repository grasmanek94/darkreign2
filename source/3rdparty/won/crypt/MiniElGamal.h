#ifndef __MINICRYPT_ELGAMAL_H__
#define __MINICRYPT_ELGAMAL_H__

#pragma warning(disable: 4530)

#include "BigInteger.h"
#include "MiniMisc.h"

namespace WONCryptMini
{


class ElGamal
{

private:
	mutable Random mRandom;

	BigInteger p,q,g,y,x;
	BigInteger recip;
	int nb;

	int modulusLen;


	bool mIsPrivate;
	bool mIsPublic;

	bool EncodeDigest(const RawBuffer& digest, BigInteger &h) const;
	static BigInteger GetRandBetween(Random &rng, const BigInteger &min, const BigInteger &max);

	bool encrypt(const BigInteger &M, BigInteger ab[]) const;
	bool decrypt(const BigInteger &a, const BigInteger &b, BigInteger &out) const; 
	bool BogusSign(const BigInteger &M, BigInteger ab[]) const; 
	bool BogusVerify(const BigInteger &M, const BigInteger &a, const BigInteger &b) const;

public:
	ElGamal();
	void Invalidate();
	bool IsPrivate() const;
	bool IsPublic() const;

	bool SetPrivateKey(const RawBuffer& theKey);
	bool SetPrivateKey(const void *theKey, int theKeyLen);

	bool SetPublicKey(const RawBuffer& theKey);
	bool SetPublicKey(const void *theKey, int theKeyLen);

	bool encrypt(const RawBuffer &thePlainText, RawBuffer &out) const;
	bool encrypt(const void *thePlainText, int thePlainTextLen, RawBuffer &out) const;

	bool decrypt(const RawBuffer &theCipherText, RawBuffer &out) const;
	bool decrypt(const void *theCipherText, int theCipherTextLen, RawBuffer &out) const;

	bool verify(const RawBuffer& theMessage, const RawBuffer& theSignature) const;
	bool verify(const void *theMessage, int theMessageLen, const void *theSignature, int theSignatureLen) const;

	bool sign(const RawBuffer& theMessage, RawBuffer& out) const;
	bool sign(const void *theMessage, int theMessageLen, RawBuffer &out) const;

	bool GetRawPrivateKey(RawBuffer &out);
	bool GetRawPublicKey(RawBuffer& out);

	const BigInteger& GetG() { return g; }
	const BigInteger& GetP() { return p; }
	const BigInteger& GetY() { return y; }
	const BigInteger& GetX() { return x; }

	void SetX(const BigInteger &theX) { x = theX; }
};

}

#endif