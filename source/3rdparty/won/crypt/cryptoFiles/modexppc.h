#ifndef CRYPTOPP_MODEXPPC_H
#define CRYPTOPP_MODEXPPC_H

#include "modarith.h"
#include "eprecomp.h"
#include "smartptr.h"

NAMESPACE_BEGIN(CryptoPP)

class ModExpPrecomputation
{
public:
	ModExpPrecomputation() {}
	ModExpPrecomputation(const ModExpPrecomputation &mep);
	ModExpPrecomputation(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage);
	~ModExpPrecomputation();

	void operator=(const ModExpPrecomputation &);
	
	void Precompute(const Integer &modulus, const Integer &base, unsigned int maxExpBits, unsigned int storage);
	//void Load(const Integer &modulus, BufferedTransformation &storedPrecomputation);
	//void Save(BufferedTransformation &storedPrecomputation) const;

	Integer Exponentiate(const Integer &exponent) const;
	Integer CascadeExponentiate(const Integer &exponent, const ModExpPrecomputation &pc2, const Integer &exponent2) const;

private:
	typedef MultiplicativeGroup<MontgomeryRepresentation> MR_MG;
	member_ptr<MontgomeryRepresentation> mr;
	member_ptr<MR_MG> mg;
	member_ptr< ExponentiationPrecomputation<MR_MG> > ep;
};

NAMESPACE_END

#endif
