#ifndef _MINI_CRYPT

// nr.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "nr.h"
#include "asn.h"
#include "nbtheory.h"

NAMESPACE_BEGIN(CryptoPP)

/*
NRDigestVerifier::NRDigestVerifier(const Integer &p, const Integer &q,
			   const Integer &g, const Integer &y)
	: p(p), q(q), g(g), y(y),
	  gpc(p, g, q.BitCount(), 1), ypc(p, y, ExponentBitLength(), 1)
{
}
*/

void NRDigestVerifier::Precompute(unsigned int precomputationStorage)
{
	gpc.Precompute(p, g, ExponentBitLength(), precomputationStorage);
	ypc.Precompute(p, y, ExponentBitLength(), precomputationStorage);
}

void NRDigestVerifier::LoadPrecomputation(BufferedTransformation &bt)
{
	//gpc.Load(p, bt);
	//ypc.Load(p, bt);
}

void NRDigestVerifier::SavePrecomputation(BufferedTransformation &bt) const
{
	//gpc.Save(bt);
	//ypc.Save(bt);

}

Integer NRDigestVerifier::EncodeDigest(const byte *digest, unsigned int digestLen) const
{
	Integer h;
	if (digestLen*8 < q.BitCount())
		h.Decode(digest, digestLen);
	else
	{
		h.Decode(digest, q.ByteCount());
		h >>= q.ByteCount()*8 - q.BitCount() + 1;
	}
	assert(h < q);
	return h;
}

unsigned int NRDigestVerifier::ExponentBitLength() const
{
	return q.BitCount();
}

NRDigestVerifier::NRDigestVerifier(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	p.BERDecode(seq);
	q.BERDecode(seq);
	g.BERDecode(seq);
	y.BERDecode(seq);
	gpc.Precompute(p, g, ExponentBitLength(), 1);
	ypc.Precompute(p, y, ExponentBitLength(), 1);
}

void NRDigestVerifier::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	p.DEREncode(seq);
	q.DEREncode(seq);
	g.DEREncode(seq);
	y.DEREncode(seq);
}

bool NRDigestVerifier::VerifyDigest(const byte *digest, unsigned int digestLen, const byte *signature) const
{
	assert(digestLen <= MaxDigestLength());

	Integer h = EncodeDigest(digest, digestLen);
	unsigned int qLen = q.ByteCount();
	Integer r(signature, qLen);
	Integer s(signature+qLen, qLen);
	return RawVerify(h, r, s);
}

bool NRDigestVerifier::RawVerify(const Integer &m, const Integer &r, const Integer &s) const
{
	// check r != 0 && r == (g^s * y^r + m) mod q
	return !!r && r == (gpc.CascadeExponentiate(s, ypc, r) + m) % q;
}

// ******************************************************************

/*
NRDigestSigner::NRDigestSigner(const Integer &p, const Integer &q, const Integer &g, const Integer &y, const Integer &x)
	: NRDigestVerifier(p, q, g, y), x(x)
{
}
*/

NRDigestSigner::NRDigestSigner(RandomNumberGenerator &rng, unsigned int pbits)
{
	PrimeAndGenerator pg(1, rng, pbits, 2*DiscreteLogWorkFactor(pbits));
	p = pg.Prime();
	q = pg.SubPrime();
	g = pg.Generator();
	x.Randomize(rng, 2, q-2, Integer::ANY);
	gpc.Precompute(p, g, ExponentBitLength(), 1);
	y = gpc.Exponentiate(x);
	ypc.Precompute(p, y, ExponentBitLength(), 1);
}

/*
NRDigestSigner::NRDigestSigner(RandomNumberGenerator &rng, const Integer &pIn, const Integer &qIn, const Integer &gIn)
{
	p = pIn;
	q = qIn;
	g = gIn;
	x.Randomize(rng, 2, q-2, Integer::ANY);
	gpc.Precompute(p, g, ExponentBitLength(), 1);
	y = gpc.Exponentiate(x);
	ypc.Precompute(p, y, ExponentBitLength(), 1);
}
*/

NRDigestSigner::NRDigestSigner(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	p.BERDecode(seq);
	q.BERDecode(seq);
	g.BERDecode(seq);
	y.BERDecode(seq);
	x.BERDecode(seq);
	gpc.Precompute(p, g, ExponentBitLength(), 1);
	ypc.Precompute(p, y, ExponentBitLength(), 1);
}

void NRDigestSigner::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	p.DEREncode(seq);
	q.DEREncode(seq);
	g.DEREncode(seq);
	y.DEREncode(seq);
	x.DEREncode(seq);
}

void NRDigestSigner::SignDigest(RandomNumberGenerator &rng, const byte *digest, unsigned int digestLen, byte *signature) const
{
	assert(digestLen <= MaxDigestLength());

	Integer h = EncodeDigest(digest, digestLen);
	Integer r;
	Integer s;

	RawSign(rng, h, r, s);
	unsigned int qLen = q.ByteCount();
	r.Encode(signature, qLen);
	s.Encode(signature+qLen, qLen);
}

void NRDigestSigner::RawSign(RandomNumberGenerator &rng, const Integer &m, Integer &r, Integer &s) const
{
	do
	{
		Integer k(rng, 2, q-2, Integer::ANY);
		r = (gpc.Exponentiate(k) + m) % q;
		s = (k - x*r) % q;
	} while (!r);			// make sure r != 0
}

NAMESPACE_END

#endif
