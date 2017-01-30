#ifndef _MD5HASH_H
#define _MD5HASH_H

#ifndef _MINI_CRYPT

// MD5Hash

// Class that encapsulates a MD5 hash implementation.  Anum number of bytes may be
// hashed to a constant length value.  Use the Put() methods to add bytes and the
// Hash() method to generate the hash value.  Multiple calls to Put() may be made
// before calling Hash(); use this techinque to queue up bytes for hashing.


#include "common/won.h"
#include "common/WONEndian.h"

// From Crypto++
namespace CryptoPP {
	class MD5;
};


// In WONCrypt namespace
namespace WONCrypt
{

class MD5Hash
{
public:
	// Default constructor
	MD5Hash();

	// Constructor - add bytes for hashing.  Must still call Hash().
	MD5Hash(const unsigned char* theDataP, unsigned int theLen);

	// Destructor
	~MD5Hash();

	// Put data in for hashing
	void Put(const unsigned char* theDataP, unsigned int theLen);
	void Put(unsigned char theData);
	void Put(unsigned long theData);
	void Put(unsigned short theData);
	void Put(const char* theStrP);
	void Put(const std::string& theStrR);
	void Put(const std::wstring& theStrR);
	void Put(const WONCommon::RawBuffer& theDataR);

	// Perform the Hash, returned pointer contains the Hash value and has a length
	// of HashLength() bytes.  Value is maintained until next call to Hash() or until
	// instance is destructed.
	// Returns NULL if hash fails.
	const unsigned char* Hash();
	const unsigned char* Hash(const unsigned char* theDataP, unsigned int theLen);

	// Reset for new hash (automatic after Hash() is called)
	void Reset();

	// Get length of hash buffer
	static unsigned int HashLength();

private:
	CryptoPP::MD5* mHashP;  // Hashing object
	unsigned char* mBufP;   // Buffer for hashing
	bool           mFail;   // Flag for failed hashing

	// Disbale copy/assign
	MD5Hash(const MD5Hash& theHashR);
	MD5Hash& operator=(const MD5Hash& theHashR);
};


// Inlines

inline void
MD5Hash::Put(unsigned char theData)
{ Put(reinterpret_cast<const unsigned char*>(&theData), sizeof(theData)); }

inline void
MD5Hash::Put(unsigned long theData)
{
	unsigned long tmpData = getLittleEndian(theData);
	Put(reinterpret_cast<const unsigned char*>(&tmpData), sizeof(tmpData));
}

inline void
MD5Hash::Put(unsigned short theData)
{
	unsigned short tmpData = getLittleEndian(theData);
	Put(reinterpret_cast<const unsigned char*>(&tmpData), sizeof(tmpData));
}

inline void
MD5Hash::Put(const char* theStrP)
{ Put(reinterpret_cast<const unsigned char*>(theStrP), strlen(theStrP)); }

inline void
MD5Hash::Put(const std::string& theStrR)
{ Put(reinterpret_cast<const unsigned char*>(theStrR.data()), theStrR.size()); }

inline void
MD5Hash::Put(const std::wstring& theStrR)
{
	std::wstring tmpS;
	makeLittleEndianWString(tmpS);
	Put(reinterpret_cast<const unsigned char*>(tmpS.data()), (tmpS.size() * sizeof(unsigned short)));
}

inline void
MD5Hash::Put(const WONCommon::RawBuffer& theDataR)
{ Put(theDataR.data(), theDataR.size()); }

inline const unsigned char*
MD5Hash::Hash(const unsigned char* theDataP, unsigned int theLen)
{ Put(theDataP, theLen);  return Hash(); }


};  //namespace WONCrypt

#else


#include "MiniMisc.h"

namespace WONCryptMini
{

class MD5Digest
{

private:
    enum {
		HASH_LENGTH = 16,
		DATA_LENGTH = 64
	};

	RawBuffer out;

    int data[DATA_LENGTH/4];
    int mDigest[HASH_LENGTH/4];
    unsigned char tmp[DATA_LENGTH];
	__int64 count;

private:
	static int F(int x,int y,int z) { return (z ^ (x & (y^z))); }
	static int G(int x,int y,int z) { return (y ^ (z & (x^y))); }
	static int H(int x,int y,int z) { return (x ^ y ^ z); }
	static int I(int x,int y,int z) { return (y  ^  (x | ~z)); }

	static int FF(int a,int b,int c,int d,int k,int s,int t);
	static int GG(int a,int b,int c,int d,int k,int s,int t);
	static int HH(int a,int b,int c,int d,int k,int s,int t);
	static int II(int a,int b,int c,int d,int k,int s,int t);

	void transform();
	void transform(int M[]);
	void ReadData();

	static void byte2int(const unsigned char src[], int srcOffset,int dst[], int dstOffset, int length);
public:
	MD5Digest();
	void reset();

	void update(const RawBuffer& input);

	RawBuffer digest();
	RawBuffer digest(const RawBuffer& in);
	RawBuffer digest(const RawBuffer& in, int pos);



};

}

#endif
#endif