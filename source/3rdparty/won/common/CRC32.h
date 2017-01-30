#ifndef _H_CRC32
#define	_H_CRC32

// CRC32

// Class implements a 32 bit CRC on an arbitrary stream of bytes.  This code was
// taken from the following location (web):
//		http://www.w3c.rl.ac.uk/isopng/D-PNG-CRCAppendix.html


#include "won.h"
#include "WONEndian.h"

namespace WONCommon {

class CRC32
{
public:

	explicit CRC32(unsigned long theSeed=0xffffffff);
	CRC32(const CRC32& theCRCR);
	~CRC32();

	// Assignment operator
	CRC32& operator=(const CRC32& theCRCR);

	// Reset for new CRC calc
	void Reset();

	// Get current CRC
	unsigned long GetCRC();

	// Validate an existing CRC with this one
	bool ValidateCRC(unsigned long theCRC);

	// Build a CRC from data.  Can call Put() repeatably to build a CRC
	void Put(const unsigned char* theDataP, unsigned int theLen);
	void Put(unsigned char theData);
	void Put(unsigned long theData);
	void Put(unsigned short theData);
	void Put(const char* theStrP);
	void Put(const std::string& theStrR);
	void Put(const std::wstring& theStrR);
	void Put(const WONCommon::RawBuffer& theDataR);

private:
	unsigned long mRegister;
	unsigned long mSeed;

	// Process byte stream for CRC
	void ProcessBytes(const unsigned char* theBytesP, unsigned long theNumBytes);

	// Load CRC table
	static void LoadCRCTable();
};


// Inlines

inline void
CRC32::Reset()
{ mRegister = mSeed; }

inline unsigned long
CRC32::GetCRC()
{ return (mRegister ^ 0xffffffff); }

inline void
CRC32::Put(unsigned char theData)
{ Put(reinterpret_cast<const unsigned char*>(&theData), sizeof(theData)); }

inline void
CRC32::Put(unsigned long theData)
{
	unsigned long tmpData = getLittleEndian(theData);
	Put(reinterpret_cast<const unsigned char*>(&tmpData), sizeof(tmpData));
}

inline void
CRC32::Put(unsigned short theData)
{
	unsigned short tmpData = getLittleEndian(theData);
	Put(reinterpret_cast<const unsigned char*>(&tmpData), sizeof(tmpData));
}

inline void
CRC32::Put(const char* theStrP)
{ Put(reinterpret_cast<const unsigned char*>(theStrP), strlen(theStrP)); }

inline void
CRC32::Put(const std::string& theStrR)
{ Put(reinterpret_cast<const unsigned char*>(theStrR.data()), theStrR.size()); }

inline void
CRC32::Put(const std::wstring& theStrR)
{
	std::wstring tmpS;
	makeLittleEndianWString(tmpS);
	Put(reinterpret_cast<const unsigned char*>(tmpS.data()), (tmpS.size() * sizeof(unsigned short)));
}

inline void
CRC32::Put(const WONCommon::RawBuffer& theDataR)
{ Put(theDataR.data(), theDataR.size()); }

inline bool
CRC32::ValidateCRC(unsigned long theCRC)
{ return (GetCRC() == theCRC); }


};  // Namespace WONCommon

#endif // _H_CRC16
