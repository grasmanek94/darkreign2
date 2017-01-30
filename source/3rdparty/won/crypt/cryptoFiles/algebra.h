#ifndef CRYPTOPP_ALGEBRA_H
#define CRYPTOPP_ALGEBRA_H

NAMESPACE_BEGIN(CryptoPP)

class Integer;

template <class T> class AbstractGroup
{
public:
	typedef T Element;

	virtual ~AbstractGroup() {}

	virtual bool Equal(const Element &a, const Element &b) const =0;
	virtual Element Zero() const =0;
	virtual Element Add(const Element &a, const Element &b) const =0;
	virtual Element Inverse(const Element &a) const =0;

	virtual Element Double(const Element &a) const;
	virtual Element Subtract(const Element &a, const Element &b) const;
	virtual Element& Accumulate(Element &a, const Element &b) const;
	virtual Element& Reduce(Element &a, const Element &b) const;

	virtual Element IntMultiply(const Element &a, const Integer &e) const;
	virtual Element CascadeIntMultiply(const Element &x, const Integer &e1, const Element &y, const Integer &e2) const;
};

template <class T> class AbstractRing : public AbstractGroup<T>
{
public:
	typedef T Element;

	virtual bool IsUnit(const Element &a) const =0;
	virtual Element One() const =0;
	virtual Element Multiply(const Element &a, const Element &b) const =0;
	virtual Element MultiplicativeInverse(const Element &a) const =0;

	virtual Element Square(const Element &a) const;
	virtual Element Divide(const Element &a, const Element &b) const;

	virtual Element Exponentiate(const Element &a, const Integer &e) const;
	virtual Element CascadeExponentiate(const Element &x, const Integer &e1, const Element &y, const Integer &e2) const;
};

// ********************************************************

// VC60 workaround: incomplete member template support
template <class Element, class Iterator>
	Element GeneralCascadeMultiplication(const AbstractGroup<Element> &group, Iterator begin, Iterator end);
template <class Element, class Iterator, class ConstIterator>
	void SimultaneousMultiplication(Iterator result, const AbstractGroup<Element> &group, const Element &base, ConstIterator expBegin, ConstIterator expEnd);
template <class Element, class Iterator>
	Element GeneralCascadeExponentiation(const AbstractRing<Element> &ring, Iterator begin, Iterator end);
template <class Element, class Iterator, class ConstIterator>
	void SimultaneousExponentiation(Iterator result, const AbstractRing<Element> &ring, const Element &base, ConstIterator expBegin, ConstIterator expEnd);

// ********************************************************

template <class T> class AbstractField : public AbstractRing<T>
{
public:
	typedef AbstractRing<T> Element;
	
	bool IsUnit(const Element &a) const
		{return !Equal(a, Zero());}
};

template <class T> class AbstractEuclideanDomain : public AbstractRing<T>
{
public:
	typedef T Element;

	virtual void DivisionAlgorithm(Element &r, Element &q, const Element &a, const Element &d) const =0;

	virtual Element Mod(const Element &a, const Element &b) const;
	virtual Element Gcd(const Element &a, const Element &b) const;
};

// ********************************************************

template <class T> class MultiplicativeGroup : public AbstractGroup<typename T::Element>
{
public:
	typedef T Ring;
	typedef AbstractGroup<typename T::Element>::Element Element;

	MultiplicativeGroup(const Ring &m_ring)
		: m_ring(m_ring) {}

	const Ring & GetRing() const
		{return m_ring;}

	bool Equal(const Element &a, const Element &b) const
		{return m_ring.Equal(a, b);}

	Element Zero() const
		{return m_ring.One();}

	Element Add(const Element &a, const Element &b) const
		{return m_ring.Multiply(a, b);}

	Element& Accumulate(Element &a, const Element &b) const
		{return a = m_ring.Multiply(a, b);}

	Element Inverse(const Element &a) const
		{return m_ring.MultiplicativeInverse(a);}

	Element Subtract(const Element &a, const Element &b) const
		{return m_ring.Divide(a, b);}

	Element& Reduce(Element &a, const Element &b) const
		{return a = m_ring.Divide(a, b);}

	Element Double(const Element &a) const
		{return m_ring.Square(a);}

protected:
	const Ring &m_ring;
};

template <class T> class EuclideanDomainOf : public AbstractEuclideanDomain<T>
{
public:
	typedef T Element;

	EuclideanDomainOf() {}

	bool Equal(const Element &a, const Element &b) const
		{return a==b;}

	Element Zero() const
		{return Element::Zero();}

	Element Add(const Element &a, const Element &b) const
		{return a+b;}

	Element& Accumulate(Element &a, const Element &b) const
		{return a+=b;}

	Element Inverse(const Element &a) const
		{return -a;}

	Element Subtract(const Element &a, const Element &b) const
		{return a-b;}

	Element& Reduce(Element &a, const Element &b) const
		{return a-=b;}

	Element Double(const Element &a) const
		{return a.Doubled();}

	Element One() const
		{return Element::One();}

	Element Multiply(const Element &a, const Element &b) const
		{return a*b;}

	Element Square(const Element &a) const
		{return a.Squared();}

	bool IsUnit(const Element &a) const
		{return a.IsUnit();}

	Element MultiplicativeInverse(const Element &a) const
		{return a.MultiplicativeInverse();}

	Element Divide(const Element &a, const Element &b) const
		{return a/b;}

	Element Mod(const Element &a, const Element &b) const
		{return a%b;}

	void DivisionAlgorithm(Element &r, Element &q, const Element &a, const Element &d) const
		{Element::Divide(r, q, a, d);}
};

template <class T> class QuotientRing : public AbstractRing<typename T::Element>
{
public:
	typedef T EuclideanDomain;
	typedef typename T::Element Element;

	QuotientRing(const EuclideanDomain &domain, const Element &modulus)
		: m_domain(domain), m_modulus(modulus) {}

	const EuclideanDomain & GetDomain() const
		{return m_domain;}

	const Element & GetModulus() const
		{return m_modulus;}

	bool Equal(const Element &a, const Element &b) const
		{return m_domain.Equal(m_domain.Mod(m_domain.Subtract(a, b), m_modulus), m_domain.Zero());}

	Element Zero() const
		{return m_domain.Zero();}

	Element Add(const Element &a, const Element &b) const
		{return m_domain.Add(a, b);}

	Element& Accumulate(Element &a, const Element &b) const
		{return m_domain.Accumulate(a, b);}

	Element Inverse(const Element &a) const
		{return m_domain.Inverse(a);}

	Element Subtract(const Element &a, const Element &b) const
		{return m_domain.Subtract(a, b);}

	Element& Reduce(Element &a, const Element &b) const
		{return m_domain.Reduce(a, b);}

	Element Double(const Element &a) const
		{return m_domain.Double(a);}

	bool IsUnit(const Element &a) const
		{return m_domain.IsUnit(m_domain.Gcd(a, m_modulus));}

	Element One() const
		{return m_domain.One();}

	Element Multiply(const Element &a, const Element &b) const
		{return m_domain.Mod(m_domain.Multiply(a, b), m_modulus);}

	Element Square(const Element &a) const
		{return m_domain.Mod(m_domain.Square(a), m_modulus);}

	Element MultiplicativeInverse(const Element &a) const;

protected:
	const EuclideanDomain &m_domain;
	Element m_modulus;
};

NAMESPACE_END

#endif
