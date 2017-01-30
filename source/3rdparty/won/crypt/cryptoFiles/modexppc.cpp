#ifndef _MINI_CRYPT

// modexppc.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "modexppc.h"
#include "asn.h"

#include "algebra.cpp"
#include "eprecomp.cpp"

NAMESPACE_BEGIN(CryptoPP)

ModExpPrecomputation::~ModExpPrecomputation() {}

ModExpPrecomputation::ModExpPrecomputation(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage)
{
	Precompute(modulus, base, maxExpBits, storage);
}

ModExpPrecomputation::ModExpPrecomputation(const ModExpPrecomputation &mep)
	: mr(new MontgomeryRepresentation(*mep.mr)), 
	  mg(new MR_MG(*mr)), 
	  ep(new ExponentiationPrecomputation<MR_MG>(*mg, *mep.ep))
{
}

void ModExpPrecomputation::Precompute(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage)
{
	if (!mr.get() || mr->GetModulus()!=modulus)
	{
		mr.reset(new MontgomeryRepresentation(modulus));
		mg.reset(new MR_MG(*mr));
		ep.reset(NULL);
	}

	if (!ep.get() || ep->storage < storage)
		ep.reset(new ExponentiationPrecomputation<MR_MG>(*mg, mr->ConvertIn(base), maxExpBits, storage));
}

/*
void ModExpPrecomputation::Load(const Integer &modulus, BufferedTransformation &bt)
{
	if (!mr.get() || mr->GetModulus()!=modulus)
	{
		mr.reset(new MontgomeryRepresentation(modulus));
		mg.reset(new MR_MG(*mr));
	}

	ep.reset(new ExponentiationPrecomputation<MR_MG>(*mg));
	BERSequenceDecoder seq(bt);
	ep->storage = (unsigned int)(Integer(seq).ConvertToLong());
	ep->exponentBase.BERDecode(seq);
	ep->g.resize(ep->storage);
	for (unsigned i=0; i<ep->storage; i++)
		ep->g[i].BERDecode(seq);
}

void ModExpPrecomputation::Save(BufferedTransformation &bt) const
{
	assert(ep.get());
	DERSequenceEncoder seq(bt);
	Integer(ep->storage).DEREncode(seq);
	ep->exponentBase.DEREncode(seq);
	for (unsigned i=0; i<ep->storage; i++)
		ep->g[i].DEREncode(seq);
}
*/

Integer ModExpPrecomputation::Exponentiate(const Integer &exponent) const
{
	assert(mr.get() && ep.get());
	return mr->ConvertOut(ep->Exponentiate(exponent));
}

Integer ModExpPrecomputation::CascadeExponentiate(const Integer &exponent, const ModExpPrecomputation &pc2, const Integer &exponent2) const
{
	assert(mr.get() && ep.get());
	return mr->ConvertOut(ep->CascadeExponentiate(exponent, *pc2.ep, exponent2));
}

NAMESPACE_END

#endif
