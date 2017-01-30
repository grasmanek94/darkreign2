#include "BigNum.h"

#ifdef _MINI_CRYPT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <assert.h>
#include <memory>
#include <math.h>

#include "BigInteger.h"

using namespace std;
using namespace WONCryptMini;

const int BigNum::BITS = 32;

#define LOW_WORD(x)  (word)(x)
#define HIGH_WORD(x) (*(((word *)&(x))+1))

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::byteLength() const
{
	return ((bitLength(*this) + 7) / 8);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::check_state() const
{
    bitLength(*this);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::bitLength(const BigNum &n)
{
    int len = n.len;

    if (len == 0)
        return 0;

    int r = (len - 1) * BITS;

    unsigned int i = n.n[len-1];

    // Could probably speed this up with a binary search
    while (i != 0)
    {
        i >>= 1;
        ++r;
    }

    return r;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool BigNum::bit(const BigNum &n, int i)
{
    int bit = i % BITS;
    i /= BITS;

    if (i >= n.len || ((n.n[i] & (1L << bit)) == 0))
        return false;
	
    return true;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::setBit(BigNum &n, int i)
{
    int bit = i % BITS;
    i /= BITS;

	
    if (i >= n.len)
		return;
	
	n.n[i] |= (1L << bit);				
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::CopyBuffer(word *dest, const word *src, size_t count)
{
	if(dest!=src)
		memcpy(dest,src,sizeof(word)*count);
	else
		memmove(dest,src,sizeof(word)*count);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::NewBuffer(int theSize, const word *theOldBuf, int theOldBufSize) const
{
	word *oldN = n;
	
	if(theSize>STACKN_SIZE) 
	{
		n = new word[theSize];
		assert(n);
	}
	else 
		n = stackN;

	if(theOldBuf!=n)
		memset(n,0,theSize*sizeof(word));

	nlen = theSize;
	len = theOldBufSize;


	if(theOldBuf!=NULL && theOldBuf!=n)
	{
		memcpy(n,theOldBuf,sizeof(word)*theOldBufSize);
	}

	if(oldN!=stackN)
		delete oldN;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigNum::BigNum()
{
	n = NULL;

	NewBuffer(STACKN_SIZE);

    negative = false;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

BigNum::~BigNum()
{
	if(n!=stackN)
		delete n;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::copy(BigNum &dst, const BigNum &src)
{
    if (&dst == &src)
        return;

	dst.NewBuffer(src.nlen, src.n, src.len);
    dst.negative = src.negative;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::grow(const BigNum &a, int i, bool copyOld)
{
    if (i <= a.nlen)
        return;

	i+=16; // Leave extra space

	if(copyOld)
		a.NewBuffer(i,a.n,a.len);
	else
		a.NewBuffer(i);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::intoBinary(RawBuffer &buffer) const
{
    int len = (bitLength(*this)+7)/8;

	buffer.erase();

    int pos = 0;
    int bitpos = 0;

    // Index in reverse to get LSB first
    for (int i = len-1; i >= 0; --i)
    {
        int b = (((unsigned int)n[pos] >> bitpos) & 0xFFL);
        bitpos += 8;
        if (bitpos >= BITS)
        {
            bitpos -= BITS;
            pos++;
            if (bitpos > 0)
                b |= (n[pos] << (8-bitpos)) & 0xFFL;
        }
        buffer.insert((int)0,1,(unsigned char)b);
    }

	if(!negative && (char)buffer[0] < 0)
		buffer.insert((int)0,1,(unsigned char)0);

    return len;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::fromBinary(const RawBuffer &buffer)
{
    negative = false;    // Can't init negatives yet
    int alen = (buffer.length()*8 + BITS-1) / BITS;
    grow(*this, alen, false);
	len = alen;

    int pos = 0;
    n[pos] = 0;
    int bitpos = 0;
    // Index in reverse to get LSB first
    for (int i = buffer.length()-1; i >= 0; --i)
    {
        word b = buffer[i] & 0xFF;

        n[pos] |= (b << bitpos);
        bitpos += 8;
        if (bitpos >= BITS && i>0)
        {
            pos++;
            n[pos] = 0;
            bitpos -= BITS;
            if (bitpos > 0)
                n[pos] = b >> (8-bitpos);
        }
    }
    while (len > 0 && n[len-1] == 0)
        len--;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::zero(BigNum &a)
{
    a.n[0] = 0;
    a.negative = false;
    a.len = 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::one(BigNum &a)
{
    a.n[0] = 1;
    a.negative = false;
    a.len = 1;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static int AddWords(word *r, const word *a, const word *b, int NA, int NB)
{
	if(NA<NB) 
	{
		return AddWords(r,b,a,NB,NA);
	}

	word carry = 0;
	for(int i=0; i<NB; i++)
	{
		dword result = (dword)a[i] + b[i] + carry;
		r[i] = LOW_WORD(result);
		carry = HIGH_WORD(result);

//		NumAdds+=2;
	}

	for(; i<NA; i++)
	{
		dword result = (dword)a[i] + carry;
		r[i] = LOW_WORD(result);
		carry = HIGH_WORD(result);

//		NumAdds++;
	}

	if(carry)
	{
		r[i] = carry;
		return i+1;
	}

	return i;
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static word SubWords(word *r, const word *a, const word *b, int NA, int NB)
{
	if(NA<NB) return SubWords(r,b,a,NB,NA);

	word borrow = 0;
	for(int i=0; i<NB; i++)
	{
		dword result = (dword)a[i] - b[i] - borrow;
		r[i] = LOW_WORD(result);
		borrow = 0 - HIGH_WORD(result);

//		NumAdds+=2;
	}

	for(; i<NA; i++)
	{
		dword result = (dword)a[i] - borrow;
		r[i] = LOW_WORD(result);
		borrow = 0 - HIGH_WORD(result);

//		NumAdds++;
	}

	return borrow;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void MultiplyWords(word *r, const word *a, const word *b, int NA, int NB)
{
	memset(r,0,(NA+NB)*sizeof(word));
    for (int i = 0; i < NA; ++i)
    {
        word carry = 0;
        dword m1 = a[i];
        int ri = i;

        for (int j = 0; j < NB; ++j)
        {
            dword m2 = r[ri];
            m2 += b[j] * m1 + carry;
            carry = HIGH_WORD(m2);
            r[ri++] = LOW_WORD(m2);

//			NumMults++;
//			NumAdds++;
        }
        r[ri] = carry;
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void MultiplyWord(word *r, const word *a, const word b, int NA)
{
	word carry = 0;
	dword result;
	for(int i=0; i < NA; ++i)
	{
		result = (dword)b*a[i] + carry;
		r[i] = LOW_WORD(result);
		carry = HIGH_WORD(result);

//		NumMults++;
//		NumAdds++;
	}

	r[i] = carry;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static int CompareWords(const word *a, const word *b, int NA, int NB)
{
	if(NA!=NB)
		return NA>NB ? 1 : -1;

	for(int i=NA-1; i>=0; i--)
	{
		if(a[i]!=b[i])
			return a[i]>b[i] ? 1 : -1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void KaratsubaMultiplyWords(word *r, word *workspace, const word *a, const word *b, int N, int aLen, int bLen)
{

	if(N<=16)
	{
		MultiplyWords(r,a,b,N,N);
		return;
	}

	int N2 = N>>1;
	int TwoN = N<<1;
	int TwoN2 = N2<<1;
	
	const word *a0 = a;
	const word *b0 = b;
	const word *a1 = a + N2;
	const word *b1 = b + N2;

	word *c0 = r;
	word *c1 = workspace;
	word *c2 = r+2*N2;
	word *t = workspace+N;

	if(bLen<=N2)
	{
		memset(r,0,sizeof(word)*TwoN);
		KaratsubaMultiplyWords(r+N2,t,a1,b0,N2,N2,bLen);
		KaratsubaMultiplyWords(c1,t,a0,b0,N2,N2,bLen);
		AddWords(r,r,c1,N+N2,N);
	}
	else
	{
		bool needSubtract = false;
		if(CompareWords(a1,a0,N2,N2)>0)
		{
			SubWords(c0,a1,a0,N2,N2);
		}
		else
		{
			SubWords(c0,a0,a1,N2,N2);
			needSubtract = !needSubtract;
		}

		if(CompareWords(b1,b0,N2,N2)>0)
		{
			SubWords(c2,b1,b0,N2,N2);
			needSubtract = !needSubtract;
		}
		else
		{
			SubWords(c2,b0,b1,N2,N2);
		}

		KaratsubaMultiplyWords(c1,t,c0,c2,N2,N2,N2);
		KaratsubaMultiplyWords(c0,t,a0,b0,N2,N2,N2);		
		KaratsubaMultiplyWords(c2,t,a1,b1,N2,N2,N2);

		int tSize = AddWords(t,c0,c2,TwoN2,TwoN2);

		if(needSubtract)
		{
			SubWords(t,t,c1,tSize,TwoN2);
		}
		else
		{
			tSize = AddWords(t,t,c1,tSize,TwoN2);
		}

		AddWords(r+N2,r+N2,t,N+N2,tSize);
	}

	if(N&1)
	{	
		if(b[N-1]!=0)
		{
			MultiplyWord(c1,a,b[N-1],N-1);
			AddWords(r+N-1,r+N-1,c1,N,N);
		}

		MultiplyWord(t,b,a[N-1],N-1);
		AddWords(r+N-1,r+N-1,t,N,N);

		dword result = a[N-1]*b[N-1] + r[TwoN-2];
		r[TwoN-2] = LOW_WORD(result);
		r[TwoN-1] = HIGH_WORD(result);
	}
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int FindSize(const word *r, int size)
{
	while(size>0 && r[size-1]==0)
		size--;

	return size;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::add(BigNum &r, const BigNum &a, const BigNum &b)
{
    if (a.len == 0)
    {
        copy(r, b);
        return;
    }
    if (b.len == 0)
    {
        copy(r, a);
        return;
    }

    if (a.negative)
    {
        if (b.negative)
        {
            add_unsigned(r, a, b);
            r.negative = true;
        }
        else
        {
            sub_unsigned(r, b, a);
        }
    }
    else
    {
        if (b.negative)
        {
            sub_unsigned(r, a, b);
        }
        else
        {
            add_unsigned(r, a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::sub(BigNum &r, const BigNum &a, const BigNum &b)
{
    if (a.len == 0)
    {
        copy(r, b);
        if (b.len > 0)
            r.negative = true ^ b.negative;
        return;
    }
    if (b.len == 0)
    {
        copy(r, a);
        return;
    }

    if (a.negative)
    {
        if (b.negative)
        {
            sub_unsigned(r, b, a);
        }
        else
        {
            add_unsigned(r, b, a);
            r.negative = true;
        }
    }
    else
    {
        if (b.negative)
        {
            add_unsigned(r, a, b);
        }
        else
        {
            sub_unsigned(r, a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::cmp(const BigNum &a, const BigNum &b)
{
    if (a.len == 0 && b.len == 0) return 0;

    if (a.negative)
    {
        if (b.negative)
        {
            return ucmp(b, a);
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (b.negative)
        {
            return 1;
        }
        else
        {
            return ucmp(a, b);
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::ucmp(const BigNum &a, const BigNum &b)
{
    return CompareWords(a.n,b.n,a.len,b.len);
/*	int alen = a.len;
    int blen = b.len;

    if (alen < blen) return -1;
    if (alen > blen) return 1;

    word *an = a.n;
    word *bn = b.n;

    for (int i = alen-1; i >= 0; --i)
    {
        if (an[i] < bn[i]) return -1;
        if (an[i] > bn[i]) return 1;
    }
    return 0;*/
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::shiftLeft(BigNum &r, const BigNum &a, int n)
{
    if (a.len == 0)
    {
        zero(r);
        return;
    }
    int rem = n % BITS;
    int blocks = n / BITS;
    int len = a.len;

    grow(r, len + blocks);
    r.len = len + blocks;

    word *rn = r.n;

	CopyBuffer(rn+blocks,a.n,len);

    if (blocks > 0)
        for (int i = blocks-1; i >= 0 ; --i) { rn[i] = 0; }

    if (rem != 0)
    {
        word carry = 0;

        int rlen = r.len;
        for (int i = blocks; i < rlen; ++i)
        {
            word l = rn[i];

            rn[i] = ((l << rem) | carry);
            carry = (unsigned int)l >> (BITS-rem);
        }
        if (carry != 0)
        {
            rlen += 1;
            grow(r, rlen);
            r.n[rlen-1] = carry;
            r.len = rlen;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::shiftRight(BigNum &r, const BigNum &a, int n)
{
    int rem = n % BITS;
    int blocks = n / BITS;

    if (blocks >= a.len)
    {
        zero(r);
        return;
    }

    grow(r, a.len - blocks);
    r.len = a.len - blocks;

	CopyBuffer(r.n,a.n+blocks,r.len);

    if (rem != 0)
    {
        word carry = 0;
        word *rn = r.n;

        int rlen = r.len;
        for (int i = rlen-1; i > 0; --i)
        {
            word l = rn[i];

            rn[i] = ((unsigned int)l >> rem) | carry;
            carry = (l << (BITS-rem));
        }
        int l = rn[0];

        rn[0] = ((unsigned int)l >> rem) | carry;
        if (rlen > 0 && rn[rlen-1] == 0)
            r.len--;
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::add_unsigned(BigNum &r, const BigNum &a, const BigNum &b)
{
    // Ensure a is the longest
    if (a.len < b.len)
    {
		add_unsigned(r,b,a);
		return;
    }

    // Needed in case the result is same object as r
    int alen = a.len;
    int blen = b.len;

    grow(r, alen + 1);
   
	r.negative = false;

	r.len = AddWords(r.n,a.n,b.n,alen,blen);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::sub_unsigned(BigNum &r, const BigNum &a, const BigNum &b, bool determine_sign)
{
    switch (ucmp(a, b))
    {
    case 0:
        zero(r);
        return;
    case -1:
		if(determine_sign)
			r.negative = true;

		sub_unsigned(r,b,a,false);
		return;
    case 1:
        if(determine_sign)
			r.negative = false;
    }

    // Now a is the largest

    grow(r, a.len);

    word borrow = SubWords(r.n,a.n,b.n,a.len,b.len);
	r.len = FindSize(r.n,a.len);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::mul(BigNum &r, const BigNum &a, const BigNum &b)
{
    if (&r == &a || &r == &b)
		throw ArithmeticException("Result must not be either Parameter (a or b)");

    if (a.len == 0 || b.len == 0)
    {
        zero(r);
        return;
    }

    r.negative = a.negative ^ b.negative;


    grow(r, a.len + b.len,false);
    r.len = a.len + b.len;

	MultiplyWords(r.n,a.n,b.n,a.len,b.len);
	r.len = FindSize(r.n,r.len);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::kmul(BigNum &r, const BigNum &a, const BigNum &b, word *workspace)
{
	word *tn = workspace;

    if (&r == &a || &r == &b)
		throw ArithmeticException("Result must not be either Parameter (a or b)");

    if (a.len == 0 || b.len == 0)
    {
        mul(r,a,b);
        return;
    }

	if(a.len<b.len)
	{
		kmul(r,b,a,tn);
		return;
 	}

    r.negative = a.negative ^ b.negative;
	int aSize = a.len;

	if(tn==NULL)
		tn = new word[aSize*3];

	grow(a,aSize,true);
	grow(b,aSize,true);


	grow(r,aSize<<1,false);
	r.len = a.len+b.len;

	word *an = a.n;
	word *bn = b.n;

//	memset(an+a.len,0,(aSize-a.len)*sizeof(word));
//	memset(bn+b.len,0,(aSize-b.len)*sizeof(word));
//	memset(tn,0,aSize*3*sizeof(word));

//	memcpy(an,a.n,a.len*sizeof(word));
//	memcpy(bn,b.n,b.len*sizeof(word));


	KaratsubaMultiplyWords(r.n,tn,an,bn,a.len,a.len,b.len);
	r.len = FindSize(r.n,r.len);
		
	if(workspace==NULL)
		delete tn;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::mod(BigNum &r, const BigNum &m, const BigNum &d)
{
    if (&r == &m)
		throw(ArithmeticException("Result must not be the same object as m"));

    copy(r, m);
    if (ucmp(m, d) < 0)
	{
		if(m.negative)
		{
			r.negative = true;
			add(r,d,r);
		}
        return;
	}

	r.negative = false;
	
    int i = bitLength(m) - bitLength(d);

    BigNum ds;
    shiftLeft(ds, d, i);

    for (; i>= 0; --i)
    {
        if (cmp(r, ds) >= 0)
            sub(r, r, ds);
        shiftRight(ds, ds, (short)1);
    }
	
	if(m.negative)
	{
		r.negative = true;
		add(r,d,r);
	}
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::div(BigNum &dv, const BigNum &m, const BigNum &d)
{
	div(&dv, NULL, m, d);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::div(BigNum *dvOrig, BigNum *remOrig, const BigNum &m, const BigNum &d)
{
    if (d.len == 0)
		throw(ArithmeticException("div: Divide by zero"));

    if (cmp(m, d) < 0)
    {
        if (remOrig != NULL)
            copy(*remOrig, m);
        if (dvOrig != NULL)
            zero(*dvOrig);

        return;
    }

	BigNum *dvPtr = dvOrig, *remPtr = remOrig;

    if (dvPtr == NULL)
        dvPtr = new BigNum();
    if (remPtr == NULL)
        remPtr = new BigNum();

	auto_ptr<BigNum> aDelDv(dvPtr), aDelRem(remPtr);
	if(dvOrig!=NULL)
		aDelDv.release();
	if(remOrig!=NULL)
		aDelRem.release();


	BigNum &dv = *dvPtr;
	BigNum &rem = *remPtr;

    BigNum ds;
    copy(rem, m);
    zero(dv);

    int i = bitLength(m) - bitLength(d);
    shiftLeft(ds, d, i);

    for (; i >= 0; --i)
    {
        if (dv.len == 0)
        {
            if (cmp(rem, ds) >= 0)
            {
                one(dv);
                sub(rem, rem, ds);
            }
        }
        else
        {
            shiftLeft(dv, dv, 1);
            if (cmp(rem, ds) >= 0)
            {
                dv.n[0] |= 1;
                sub(rem, rem, ds);
            }
        }
        shiftRight(ds, ds, 1);
    }
    dv.negative = m.negative ^ d.negative;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::exp(BigNum &r, const BigNum &a, const BigNum &power)
{
    BigNum v1Orig;
	BigNum v2Orig;

	copy(v1Orig,a);

	BigNum *v1 = &v1Orig;
	BigNum *v2 = &v2Orig;
	BigNum *temp;
    
	int bits = bitLength(power);

    if ((power.n[0] & 1) != 0)
        copy(r,a);
    else
        one(r);


    for (int i = 1; i < bits; i++)
    {
        mul(*v2, *v1, *v1);
		temp = v1; v1 = v2; v2 = temp;

        if (bit(power, i))
		{
			copy(*v2,r);
			mul(r, *v1, *v2);
		}
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::modExp(BigNum &r, const BigNum &a, const BigNum &power, const BigNum &modulo)
{
    BigNum d;
	int nb = recip(d, modulo);

	modExp(r,a,power,modulo,d,nb);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::modExp(BigNum &r, const BigNum &a, const BigNum &power, const BigNum &modulo, const BigNum &d, short nb)
{
    BigNum v;

    mod(v, a, modulo);
    int bits = bitLength(power);

    if ((power.n[0] & 1) != 0)
        mod(r, a, modulo);
    else
        one(r);

	int aSize = modulo.len;
	word *workspace = new word[aSize*6];

/*
	BigNum nprime, bsquared;

	int nbits = MontgomeryRepresentation(nprime,bsquared,modulo);
	BigNum vMontgomery;
	BigNum rMontgomery;
	BigNum temp1, temp2;

	mul(temp1,v,bsquared);
	MontgomeryReduce(vMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);

	mul(temp1,r,bsquared);
	MontgomeryReduce(rMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);
*/

     for (int i = 1; i < bits; i++)
    {

        modMulRecip(v, v, v, modulo, d, nb,workspace);

//		kmul(temp1,vMontgomery,vMontgomery,workspace);
//		MontgomeryReduce(vMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);

	
        if (bit(power, i))
		{
			modMulRecip(r, r, v, modulo, d, (short)nb,workspace);
			//kmul(temp1,rMontgomery,vMontgomery,workspace);
			//MontgomeryReduce(rMontgomery,temp2,temp1,modulo,nbits,nprime,workspace);

		}
    }

//	MontgomeryReduce(r,temp2,rMontgomery,modulo,nbits,nprime,workspace);
	delete workspace;

	if(power.negative)
		inverseModN(r,r,modulo);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::recip(BigNum &r, const BigNum &m)
{
    BigNum t;
    one(t);

    int mbits = bitLength(m);

    shiftLeft(t, t, 2*mbits);
    div(&r, NULL, t, m);
    return mbits+1;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::euclid(BigNum &r, const BigNum &x, const BigNum &y)
{
    BigNum aOrig;
    BigNum bOrig;

    copy(aOrig, x);
    copy(bOrig, y);

	BigNum *a = &aOrig;
	BigNum *b = &bOrig;

    int shifts = 0;

    while (b->len != 0)
    {
        if ((a->n[0] & 1) != 0)    // a odd
            if ((b->n[0] & 1) != 0)    // b odd
            {
                sub(*a, *a, *b);
                shiftRight(*a, *a, 1);
                if (cmp(*a, *b) < 0)
                {
                    BigNum *t = a;
                    a = b;
                    b = t;
                }
            }
            else
            {
                shiftRight(*b, *b, 1);
                if (cmp(*a, *b) < 0)
                {
                    BigNum *t = a;
                    a = b;
                    b = t;
                }
            }
        else
            if ((b->n[0] & 1) != 0)    // b odd
            {
                shiftRight(*a, *a, 1);
                if (cmp(*a, *b) < 0)
                {
                    BigNum *t = a;
                    a = b;
                    b = t;
                }
            }
            else
            {
                shiftRight(*a, *a, 1);
                shiftRight(*b, *b, 1);
                shifts++;
            }
    }
    if (shifts > 0)
        shiftLeft(r, *a, shifts);
    else
        copy(r, *a);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::gcd(BigNum &r, const BigNum &a, const BigNum &b)
{
    if (cmp(a, b) > 0)
        euclid(r, a, b);
    else
        euclid(r, b, a);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::modMulRecip(BigNum &r, const BigNum &x, const BigNum &y, const BigNum &m, const BigNum &i,short nb, word *t)
{
    BigNum a;
    BigNum b;
    BigNum c;
    BigNum d;

    kmul(a, x, y,t);			
	shiftRight(d, a, nb-1);		
    kmul(b, d, i,t);			
    shiftRight(c, b, nb-1);	
    kmul(b, m, c,t);		
    sub(r, a, b);		

    int j = 0;
    while (cmp(r, m) >= 0)
    {
        if (j++ > 2)
		{
			throw ArithmeticException("modMulRecip");
		}

        sub(r, r, m);
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::inverseModN(BigNum &r, const BigNum &a, const BigNum &n)
{
    if (a.negative || n.negative)
		throw ArithmeticException("inverseModN");

	BigNum x1orig;
	BigNum x2orig;
	BigNum x3orig; copy(x3orig,a);

	BigNum y1orig;
	BigNum y2orig;
	BigNum y3orig; copy(y3orig,n);

    BigNum *x1 = &x1orig;
    BigNum *x2 = &x2orig;
    BigNum *x3 = &x3orig;
    BigNum *y1 = &y1orig;
    BigNum *y2 = &y2orig;
	BigNum *y3 = &y3orig;

    one(*x1); one(*y2);
    zero(*x2); zero(*y1);

	BigNum t1orig;
	BigNum t2orig;
	BigNum t3orig;
	BigNum qorig; 
	BigNum porig;

	BigNum *t1 = &t1orig;
	BigNum *t2 = &t2orig;
	BigNum *t3 = &t3orig;
	BigNum *q = &qorig;
	BigNum *p = &porig;


    while (y3->len != 0)
    {
        div(q, t3, *x3, *y3);
        mul(*t1, *q, *y2);		
        sub(*t2, *x2, *t1);
        mul(*p, *q, *y1);
        sub(*t1, *x1, *p);

		BigNum *tt1 = x1;
		BigNum *tt2 = x2;
		BigNum *tt3 = x3;

        x1 = y1; x2 = y2; x3 = y3;
        y1 = t1; y2 = t2; y3 = t3;
		t1 = tt1; t2 = tt2; t3 = tt3;
    }

    if (x1->negative)
        add(*x1, *x1, n);
	
    copy(r, *x1);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/*
void BigNum::sqrt(BigNum &r, const BigNum &x)
{
	int maxSqrtBits = ceil((bitLength(x) + 1)/2.0);

	BigNum aMax;
	BigNum aMin;
	BigNum aSqr;
	BigNum aTwo;
	BigNum aTemp;

	assign(aMax,1);
	assign(aMin,0);
	assign(aTwo,2);

	shiftLeft(aMax,aMax,maxSqrtBits);
	div(r,aMax,aTwo);

	while(cmp(r,aMin)!=0)
	{
		mul(aSqr,r,r);

		int aCmp = cmp(aSqr,x);
		if(aCmp==0)
			return;
		else if(aCmp>0)
			copy(aMax,r);
		else 
			copy(aMin,r);
			
		add(aTemp,aMax,aMin);
		div(r,aTemp,aTwo);
	}

	
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int BigNum::MontgomeryRepresentation(BigNum &nprime, BigNum &bsquared, const BigNum &n)
{
	int nBits = bitLength(n);
	if(nBits%BITS!=0)
		nBits+= BITS - nBits%BITS;

	BigNum b, binv, temp;
	
	one(b); 
	one(temp);


	shiftLeft(b,b,nBits);
	shiftLeft(temp,temp,nBits*2);

	DWORD aTick = GetTickCount();
	mod(bsquared,temp,n);
	printf("%d\n",GetTickCount()-aTick);

	inverseModN(binv,b,n);

	mul(temp,b,binv);
	div(nprime,temp,n);

	return nBits;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::MontgomeryReduce(BigNum &r, BigNum &workspace, const BigNum &t, const BigNum &n, int nBits, const BigNum &nprime, word *work)
{	
	ModPow2(r,t,nBits);				// r = t (mod B)
	kmul(workspace,r,nprime,work);		// workspace = [t (mod B)]*N' 
	ModPow2(r,workspace,nBits);		// r = tN' (mod B)
	kmul(workspace,r,n,work);			// workspace = tN'n (mod B)
	add(r,t,workspace);				// r = t + tN'n (mod B)
	DivPow2(r,r,nBits);				// r = (t + tN'n)/r

	if(cmp(r,n)>=0) 
		sub(r,r,n);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::ModPow2(BigNum &r, const BigNum &t, int nBits)
{
	int firstWord = nBits/BITS;

	copy(r,t);

	if(r.len<=firstWord)
		return;

	int anOffset = nBits%BITS;
	int aMask = 0xffffffff << anOffset;
	aMask^=0xffffffff;
	
	r.n[firstWord]&=aMask;

	memset(r.n + firstWord + 1, 0, r.len - firstWord - 1);
	r.len = firstWord + 1;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::DivPow2(BigNum &r, const BigNum &t, int nBits)
{
	shiftRight(r,t,nBits);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void BigNum::MulPow2(BigNum &r, const BigNum &t, int nBits)
{
	shiftLeft(r,t,nBits);
}


*/

#endif
