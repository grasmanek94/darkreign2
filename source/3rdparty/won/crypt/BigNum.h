#ifndef __MINICRYPT_BIGNUM_H__
#define __MINICRYPT_BIGNUM_H__

#define STACKN_SIZE 4

#pragma warning(disable: 4530)

#include <string>
#include <assert.h>
#include "MiniMisc.h"

namespace WONCryptMini
{

class BigInteger;	
typedef unsigned long word;
typedef	unsigned __int64 dword;


class BigNum
{

private:
	BigNum(const BigNum&) { assert(false); }
	void operator =(BigNum&) { assert(false); }

public:
protected:
	static const int   BITS; 
    static const dword RADIX;
    static const word  MASK; 


	mutable word stackN[STACKN_SIZE];
	mutable word *n;
	mutable unsigned short nlen, len;
    bool negative;

    BigNum();

	void NewBuffer(int theSize, const word *theOldBuf = NULL, int theOldBufSize = 0) const;
	static void CopyBuffer(word *dest, const word *src, size_t count);
    static void copy(BigNum &dst, const BigNum &src);
    
	void fromBinary(const RawBuffer &buffer);

public:

	virtual void debugInitStr() { }
	virtual void initStr() { }

	virtual ~BigNum();

    int byteLength() const;
    void check_state() const;

	static int bitLength(const BigNum &n);
    static bool bit(const BigNum &n, int i);
    static void setBit(BigNum &n, int i);
	static void grow(const BigNum &a, int i, bool copyOld = true);
    
	int intoBinary(RawBuffer &buffer) const;
    
    static void zero(BigNum &a);
    static void one(BigNum &a);

    static int cmp(const BigNum &a, const BigNum &b);
    static int ucmp(const BigNum &a, const BigNum &b);

    static void add(BigNum &r ,const BigNum &a, const BigNum &b);
    static void add_unsigned(BigNum &r, const BigNum &a, const BigNum &b);
    static void sub(BigNum &r, const BigNum &a, const BigNum &b);
    static void sub_unsigned(BigNum &r, const BigNum &a, const BigNum &b, bool determine_sign = true);

    static void shiftLeft(BigNum &r, const BigNum &a, int n);
    static void shiftRight(BigNum &r, const BigNum &a, int n);

    static void mul(BigNum &r, const BigNum &a, const BigNum &b);
	static void kmul(BigNum &r, const BigNum &a, const BigNum &b, word *tn = NULL);
    static void mod(BigNum &r, const BigNum &m, const BigNum &d);
    static void div(BigNum &dv, const BigNum &m, const BigNum &d);
    static void div(BigNum *dv, BigNum *rem, const BigNum &m, const BigNum &d);

    static int recip(BigNum &r, const BigNum &m);
    static void modMulRecip(BigNum &r, const BigNum &x, const BigNum &y, const BigNum &m, const BigNum &i, short nb, word *t = NULL);
    static void exp(BigNum &r, const BigNum &a, const BigNum &power);
    static void modExp(BigNum &r, const BigNum &a, const BigNum &power, const BigNum &modulo);
    static void modExp(BigNum &r, const BigNum &a, const BigNum &power, const BigNum &modulo, const BigNum &recip, short nb);

    static void inverseModN(BigNum &r, const BigNum &a, const BigNum &n);
    static void euclid(BigNum &r, const BigNum &x, const BigNum &y);
    static void gcd(BigNum &r, const BigNum &a, const BigNum &b);
/*
	static void sqrt(BigNum &r, const BigNum &x);

	static void MontgomeryReduce(BigNum &r, BigNum &workspace, const BigNum &t, const BigNum &n, int bBits, const BigNum &nprime, word *work); 
	static int MontgomeryRepresentation(BigNum &nprime, BigNum &bsquared, const BigNum &n);
	static void ModPow2(BigNum &r, const BigNum &t, int nBits);
	static void DivPow2(BigNum &r, const BigNum &t, int nBits);
	static void MulPow2(BigNum &r, const BigNum &t, int nBits);*/
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ArithmeticException : public exception
{
public:
	std::string what;
	ArithmeticException() : exception() { }
	ArithmeticException(const std::string &theWhat) { what = theWhat; }
};

}
#endif