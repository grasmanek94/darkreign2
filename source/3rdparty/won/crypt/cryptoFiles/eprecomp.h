#ifndef CRYPTOPP_EPRECOMP_H
#define CRYPTOPP_EPRECOMP_H

#include "integer.h"
#include <vector>

NAMESPACE_BEGIN(CryptoPP)

// Please do not directly use the following class.  It should be
// considered a private class for the library.  The following
// classes are public and use ExponentiationPrecomputation internally.
//
// ModExpPrecomputation;
// EcPrecomputation<EC2N>;
// EcPrecomputation<ECP>;

template <class T> class ExponentiationPrecomputation
{
public:
	typedef T Group;
	typedef typename Group::Element Element;

	class CryptoPair
	{
	public:
		Integer first;
		Element second;

		CryptoPair()
			:	first(Integer()), second(Element())	{ }
		CryptoPair(const Integer& f, const Element& s)
			:	first(f), second(s)					{ }
        CryptoPair(const CryptoPair& pr)
			:	first(pr.first), second(pr.second)	{ }

		bool operator==(const CryptoPair& cp) const
		{ return (first == cp.first && second == cp.second); }

		bool operator!=(const CryptoPair& cp) const
		{ return !(*this == cp); }

		bool operator<(const CryptoPair& cp) const
		{ return (first < cp.first || !(cp.first < first) && second < cp.second); }

		bool operator>(const CryptoPair& cp) const
		{ return (cp < *this); }

		bool operator<=(const CryptoPair& cp) const
		{ return (!(cp < *this)); }

		bool operator>=(const CryptoPair& cp) const
		{ return (!(*this < cp)); }

	};

	ExponentiationPrecomputation(const Group &group) : group(group) {}

	ExponentiationPrecomputation(const Group &group, const Element &base, unsigned int maxExpBits, unsigned int storage)
		: group(group), storage(storage), g(storage) {Precompute(base, maxExpBits);}

	ExponentiationPrecomputation(const Group &group, const ExponentiationPrecomputation &pc)
		: group(group), storage(pc.storage), exponentBase(pc.exponentBase), g(pc.g)	{}

	void Precompute(const Element &base, unsigned int maxExpBits);
	Element Exponentiate(const Integer &exponent) const;
	Element CascadeExponentiate(const Integer &exponent, const ExponentiationPrecomputation<Group> &pc2, const Integer &exponent2) const;

	const Group &group;
	unsigned int storage;	// number of precalculated bases
	Integer exponentBase;	// what base to represent the exponent in
	std::vector<Element> g;		// precalculated bases
};

NAMESPACE_END

#endif
