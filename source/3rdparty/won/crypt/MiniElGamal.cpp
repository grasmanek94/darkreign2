#include "MiniElGamal.h"

#ifdef _MINI_CRYPT

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable: 4530)
#include <windows.h>
#include <iostream>


#include "IntegerExtractor.h"
#include "IntegerInserter.h"
#include "MiniElGamal.h"
#include "MD5Digest.h"


using namespace std;
using namespace WONCryptMini;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ElGamal::ElGamal()
{
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ElGamal::Invalidate()
{
	mIsPrivate = false;
	mIsPublic = false;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::IsPrivate() const
{
	return mIsPrivate;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::IsPublic() const
{
	return mIsPublic;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::GetRawPrivateKey(RawBuffer &out)
{
	if(!IsPrivate())
		return false;

	IntegerInserter anInserter;
	anInserter.Insert(p);
	anInserter.Insert(q);
	anInserter.Insert(g);
	anInserter.Insert(y);
	anInserter.Insert(x);

	out = anInserter.Get();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	
bool ElGamal::GetRawPublicKey(RawBuffer& out)
{
	if(!IsPublic())
		return false;

	IntegerInserter anInserter;
	anInserter.Insert(p);
	anInserter.Insert(q);
	anInserter.Insert(g);
	anInserter.Insert(y);

	out = anInserter.Get();
	return true;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::SetPrivateKey(const RawBuffer &theKey)
{		
	Invalidate();

	IntegerExtractor anExtractor(theKey);
	bool aSuccess = true;
	aSuccess = anExtractor.Extract(p) && anExtractor.Extract(q) && anExtractor.Extract(g) 
			   && anExtractor.Extract(y) && anExtractor.Extract(x);	
	
	if(!aSuccess)
		return false;
	
	modulusLen = p.bitLength()/8;

	recip = p.recip(nb);
	

	mIsPrivate = true;
	mIsPublic = true;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::SetPrivateKey(const void *theKey, int theKeyLen)
{
	return SetPrivateKey(RawBuffer((unsigned char*)theKey,theKeyLen));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::SetPublicKey(const RawBuffer& theKey)
{		
	Invalidate();

	IntegerExtractor anExtractor(theKey);
	bool aSuccess = true;
	aSuccess = anExtractor.Extract(p) && anExtractor.Extract(q) && anExtractor.Extract(g)
		&& anExtractor.Extract(y);
	
	if(!aSuccess)
		return false;

	modulusLen = p.bitLength()/8;

	recip = p.recip(nb);

	mIsPublic = true;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::SetPublicKey(const void *theKey, int theKeyLen)
{
	return SetPublicKey(RawBuffer((unsigned char*)theKey,theKeyLen));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::encrypt(const RawBuffer& thePlainText, RawBuffer& out) const
{
	if(!IsPublic())
		return false;

	int aBlockLen = modulusLen - 3;
	int aNumBlock = thePlainText.length() / aBlockLen;
	if ((thePlainText.length() % aBlockLen) != 0) aNumBlock++;

	out.reserve(4+modulusLen*2*aNumBlock);
	out.erase();

	int anOffset = 0;
	
	int tmpNumBlock = getLittleEndian(aNumBlock);
	out.append((unsigned char*)&tmpNumBlock,4);

	while(anOffset < thePlainText.length())
	{	
		int thisBlockLen = aBlockLen;
		
		if(thePlainText.length() - anOffset < aBlockLen)
			thisBlockLen = thePlainText.length() - anOffset;
		
		RawBuffer anEncryptBlock(modulusLen-1,0);

		for(int k=0,j=modulusLen-2-thisBlockLen; j<modulusLen-2; j++,k++)
			anEncryptBlock[j] = thePlainText[anOffset+k];

		anEncryptBlock[modulusLen - 2] = (unsigned char)thisBlockLen;


		BigInteger ab[2];
		
		if(!encrypt(BigInteger(anEncryptBlock),ab))
			return false;


		RawBuffer aa = ab[0].toByteArray();
		RawBuffer bb = ab[1].toByteArray();
		
		if(aa.length()==modulusLen)
			out+=aa;
		else if(aa.length()>modulusLen)
			out.append(aa.data()+aa.length()-modulusLen,modulusLen);
		else
		{
			for(int i=aa.length(); i<modulusLen; i++)
				out+=(unsigned char)0;
			
			out+=aa;
		}
		

		if(bb.length()==modulusLen)
			out+=bb;
		else if(bb.length()>modulusLen)
			out.append(bb.data()+bb.length()-modulusLen,modulusLen);
		else
		{
			for(int i=bb.length(); i<modulusLen; i++)
				out+=(unsigned char)0;
			
			out+=bb;
		}						
		anOffset+=thisBlockLen;		
	}		

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::encrypt(const void *thePlainText, int thePlainTextLen, RawBuffer &out) const
{
	return encrypt(RawBuffer((unsigned char*)thePlainText, thePlainTextLen),out);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::decrypt(const RawBuffer& theCipherText, RawBuffer& out) const
{
	if(!IsPrivate())
		return false;

	const RawBuffer& in = theCipherText;
	int inOffset = 0;

	if(in.length()-inOffset<4) return false;
	int aNumBlocks = *(int*)in.data(); inOffset+=4;

	if(in.length()-inOffset < aNumBlocks*modulusLen*2-inOffset)
		return false;

	RawBuffer aBuf(modulusLen,0);
	RawBuffer bBuf(modulusLen,0);

	out.erase();

	BigInteger a;;
	BigInteger b;
		
	BigInteger aPlainText;

	for(int i=0; i<aNumBlocks; i++)
	{
		aBuf.assign(in.data()+inOffset,modulusLen); inOffset+=modulusLen;
		bBuf.assign(in.data()+inOffset,modulusLen); inOffset+=modulusLen;
				
		
		a.fromByteArray(aBuf);
		b.fromByteArray(bBuf);

		if(!decrypt(a,b,aPlainText))
			return false;
		
		RawBuffer aBigIntArray = aPlainText.toByteArray();
		

		if(aBigIntArray.length()==0) return false;
		int aPlainLen = aBigIntArray[aBigIntArray.length() - 1];
		
		if(aPlainLen>modulusLen - 3)
			return false;
		
		if(aBigIntArray.length() - 1 - aPlainLen < 0)
		{
			int extra = aPlainLen - (aBigIntArray.length() - 1);
			for(int j=0; j<extra; j++)
				out+=(unsigned char)0;
			
			out+=aBigIntArray;
		}
		else
			out.append(aBigIntArray.data()+aBigIntArray.length()-1-aPlainLen,aPlainLen);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::decrypt(const void *theCipherText, int theCipherTextLen, RawBuffer &out) const
{
	return decrypt(RawBuffer((unsigned char*)theCipherText,theCipherTextLen),out);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::EncodeDigest(const RawBuffer& digest, BigInteger &h) const
{
	IntegerExtractor aDecoder(digest,false);
	if(digest.length()*8 < q.bitLength())
	{
		if(!aDecoder.Decode(digest.length(),h))
			return false;
	}
	else
	{
		if(!aDecoder.Decode(q.byteLength(),h))
			return false;

		h = h.shiftRight(q.byteLength()*8 - q.bitLength() + 1);
	}

	return true;
}	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::verify(const RawBuffer& theMessage, const RawBuffer& theSignature) const
{
	if(!IsPublic())
		return false;

	if(theSignature.length()!=q.byteLength()*2)
		return false;
	
	
	MD5Digest anMD5;
	anMD5.update(theMessage);
	RawBuffer aDigest = anMD5.digest();
			

	BigInteger M;
	if(!EncodeDigest(aDigest,M))
		return false;

	const RawBuffer &in = theSignature;
	if(in.length()<q.byteLength()*2)
		return false;

	RawBuffer aBuf(q.byteLength(),0);
	RawBuffer bBuf(q.byteLength(),0);

	aBuf.assign(in.data(),aBuf.length());
	bBuf.assign(in.data()+aBuf.length(),bBuf.length());
					
	BigInteger a(aBuf);
	BigInteger b(bBuf);
	
	return BogusVerify(M,a,b);	
}		
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::verify(const void *theMessage, int theMessageLen, const void *theSignature, int theSignatureLen) const
{
	return verify(RawBuffer((unsigned char*)theMessage,theMessageLen),
		RawBuffer((unsigned char*)theSignature,theSignatureLen));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::sign(const RawBuffer& theMessage, RawBuffer &out) const
{		
	
	MD5Digest anMD5;
	anMD5.update(theMessage);
	RawBuffer aDigest = anMD5.digest();
	
	BigInteger M;
	if(!EncodeDigest(aDigest,M))
		return false;
	
	BigInteger ab[2];
	
	if(!BogusSign(M,ab))
		return false;

	out.erase();
	
	
	int qLen = q.byteLength();
	RawBuffer a = ab[0].toByteArray();
	RawBuffer b = ab[1].toByteArray();

	if(a.length()==qLen)
		out+=a;
	else if(a.length()>qLen)
		out.append(a.data()+a.length()-qLen,qLen);
	else
	{
		for(int i=a.length(); i<qLen; i++)
			out+=(unsigned char)0;
		
		out+=a;
	}
	

	if(b.length()==qLen)
		out+=b;
	else if(b.length()>qLen)
		out.append(b.data()+b.length()-qLen,qLen);
	else
	{
		for(int i=b.length(); i<qLen; i++)
			out+=(unsigned char)0;
		
		out+=b;
	}						
	
	return true;
}		

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	
bool ElGamal::sign(const void *theMessage, int theMessageLen, RawBuffer &out) const
{
	return sign(RawBuffer((unsigned char*)theMessage,theMessageLen),out);	
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
	
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ElGamal::encrypt(const BigInteger &M, BigInteger ab[]) const
{
	try
	{
		BigInteger p_minus_1 = p.sub(BigInteger::ONE);
		// choose a random k, relatively prime to p-1.
		BigInteger k;
		do {
			// values of k with the same number of bits as p won't be chosen, but
			// that shouldn't be a problem.
			k = BigInteger(p.bitLength()-1, mRandom);
			if (!(k.testBit(0)))
				k = k.setBit(0); // make sure k is odd
		} while (!(k.gcd(p_minus_1).equals(BigInteger::ONE)));

		ab[0] /* a */ = g.modExp(k, p, recip, nb);
		ab[1] /* b */ = y.modExp(k, p, recip, nb).modMulRecip(M,p,recip,nb);

		return true;
	}
	catch(ArithmeticException&)
	{
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::decrypt(const BigInteger &a, const BigInteger &b, BigInteger &out) const
{
    try 
	{
		out = b.modMulRecip(a.modExp(x, p, recip, nb).inverseModN(p), p, recip, nb);
		return true;
    } 
	catch (ArithmeticException &) 
	{
    }
	
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BigInteger ElGamal::GetRandBetween(Random &rng, const BigInteger &min, const BigInteger &max)
{
	BigInteger range = max.sub(min);
	int nBits = range.bitLength();

	BigInteger anInt;
	do
	{
		anInt = BigInteger(nBits,rng);
	}
	while (anInt.cmp(range)>0);

	anInt = anInt.add(min);
	return anInt;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool ElGamal::BogusSign(const BigInteger &M, BigInteger ab[]) const
{
	try 
	{
		BigInteger TWO(2);		
		BigInteger k;
		
		do
		{
			k = GetRandBetween(mRandom, TWO, q.sub(TWO));			
			ab[0] = g.modExp(k,p);
			
			
			ab[0] = ab[0].add(M);
			
			ab[0] = ab[0].mod(p);
			
			ab[0] = ab[0].mod(q);
			
			ab[1] = k.sub(x.mul(ab[0]));

			
			ab[1] = ab[1].mod(q);
			
			
			//r = (gpc.Exponentiate(k) + m) % q;
			//s = (k - x*r) % q;
		} while (ab[0].cmp(BigInteger::ZERO)==0);			// make sure r != 0

		return true;
	}
	catch(ArithmeticException &)
	{
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
public static void sign(BigInteger M, BigInteger[] ab,
                 BigInteger p, BigInteger g, BigInteger x,
                 Random rng) 
{
    BigInteger p_minus_1 = p.subtract(ONE);
    // choose a random k, relatively prime to p-1.
    BigInteger k;
    do 
	{
        // values of k with the same number of bits as p won't be chosen, but
        // that shouldn't be a problem.
        k = new BigInteger(p.bitLength()-1, rng);
        if (!(k.testBit(0)))
            k = k.setBit(0); // make sure k is odd
    } while (!(k.gcd(p_minus_1).equals(ONE)));

    BigInteger a = g.modPow(k, p);
    ab[0] = a;

    // solve for b in the equation:
    //     M = (x.a + k.b) mod (p-1)
    // i.e.
    //     b = (inverse of k mod p-1).(M - x.a) mod (p-1)
    try 
	{
        ab[1] = k.modInverse(p_minus_1)
                         .multiply(M.subtract(x.multiply(a)).mod(p_minus_1))
                         .mod(p_minus_1);
    } 
	catch (ArithmeticException e) 
	{
		ab[0] = null;
		ab[1] = null;
    }
}*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
public static boolean verify(BigInteger M, BigInteger a, BigInteger b,
                      BigInteger p, BigInteger g, BigInteger y) 
{
    BigInteger p_minus_1 = p.subtract(ONE);
    // sanity checks
    if (M.compareTo(ZERO) < 0 || M.compareTo(p_minus_1) >= 0 ||
        a.compareTo(ZERO) < 0 || a.compareTo(p_minus_1) >= 0 ||
        b.compareTo(ZERO) < 0 || b.compareTo(p_minus_1) >= 0)
        return false;

    //accept iff y^a.a^b = g^M (mod p)
	

    return y.modPow(a, p).multiply(a.modPow(b, p)).mod(p)
            .equals(g.modPow(M, p));
}*/

bool ElGamal::BogusVerify(const BigInteger &M, const BigInteger &a, const BigInteger &b) const 
{

// check a != 0 && a == (g^b * y^a + m) mod q
	
	try
	{
		if(a.cmp(BigInteger::ZERO)==0)
			return false;

//		BigInteger a1 = a.mod(q);
//		BigInteger b1 = b.mod(q);
		
		BigInteger gbq = g.modExp(b,p,recip,nb);
		BigInteger yaq = y.modExp(a,p,recip,nb);

	/*
		cout << "p = " << p << endl;
		cout << "g = mod(" << g << ",p)" << endl;
		cout << "a = " << a << endl;
		cout << "b = " << b << endl;
		cout << "y = " << y << endl;

		cout << "gbq = " << gbq << endl;
		cout << "yaq = " << yaq << endl;*/
		
		BigInteger result;

		result = gbq.modMulRecip(yaq,p,recip,nb);
//		result = gbq.mul(yaq);
		result = result.add(M);
//		result = result.mod(p);
		result = result.mod(q);
		
		if(a.cmp(result)==0)
			return true;
		else
			return false;
	}
	catch(ArithmeticException &)
	{
	}

	return false;
}


#endif
