#ifndef __MINICRYPT_BIGINTEGER_H__
#define __MINICRYPT_BIGINTEGER_H__

#pragma warning(disable: 4530)

#include <iostream>
#include "BigNum.h"
#include "Random.h"


namespace WONCryptMini
{

class BigInteger : public BigNum
{

protected:
  static RawBuffer randomBits(int numBits, Random &rndSrc);
  static bool debugStr;

#ifdef BIGINT_DEBUG_STR
  mutable std::string mVal;
#endif

public:
	void debugInitStr() const;
	void initStr() const;


    static const BigInteger ZERO;
    static const BigInteger ONE; 

	BigInteger(const BigInteger&);
	const BigInteger& operator =(const BigInteger&);
	BigInteger();

	BigInteger(__int64 from, bool negative = false);
	BigInteger(const std::string& theString);
	BigInteger(const RawBuffer& buffer);
	BigInteger(int numBits, Random &rndSrc);
	bool testBit(int theBit);
	BigInteger& setBit(int theBit);
	int bitLength() const;

	void fromString(const std::string& inHex);
	std::string toString() const;

	BigInteger gcd(const BigInteger &b) const;
	BigInteger modExp(const BigInteger &power, const BigInteger &modulo) const;
	BigInteger modExp(const BigInteger &power, const BigInteger &modulo, const BigInteger &recip, short nb) const;
	BigInteger inverseModN(const BigInteger &n) const;
	BigInteger mod(const BigInteger &b) const;
	BigInteger mul(const BigInteger &b) const;
	BigInteger div(const BigInteger &b) const;
	BigInteger divrem(const BigInteger &a, BigInteger &rem) const;
	BigInteger add(const BigInteger &b) const;
	BigInteger sub(const BigInteger &b) const;
	bool equals(const BigInteger &b) const;
	int cmp(const BigInteger &a) const;
	bool operator<(const BigInteger &a) const;

	BigInteger negate() const;

	BigInteger recip(int &nb) const;
    BigInteger modMulRecip(const BigInteger &y, const BigInteger &m, const BigInteger &i, short nb) const;
	BigInteger sqrt() const;
	BigInteger exp(const BigInteger &power) const;

	unsigned __int64 GetInt64() const;

	BigInteger& shiftLeft(int n);
	BigInteger& shiftRight(int n);

	RawBuffer toByteArray() const;
	void fromByteArray(const RawBuffer &buf);
};

std::ostream& operator<<(std::ostream&,const BigInteger&);

};

#endif
