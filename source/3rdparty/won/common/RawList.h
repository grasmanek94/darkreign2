#ifndef _RAWLIST_H_
#define _RAWLIST_H_

#pragma warning(disable : 4786)

#include <vector>
#include "common/won.h"

namespace WONCommon {

// constants
const long LENGTH_BYTES = 2;
const long MAX_LENGTH = 65535; // Max length that can fit in 2 bytes
const long MAX_ELEMENTS = 255; // Max number of elements in a list

const int RawError = -1;

// The purpose of this class is to hold a list of elements in RawBuffer form and 
class RawList
{
// Supported types
// These types mirror those in the options info class
public:
	enum ListTypeEnum
	{
		RawUnknown,			// This is an error type
		RawUnicode, 
		RawString, 
		RawLong, 
		RawBool,
		RawBinary,
		RawListLong, 
		RawListString 
	};

	typedef std::vector<std::wstring>				UnicodeList;
	typedef std::vector<std::string>				StringList;
	typedef std::vector<long>						LongList;
	typedef std::vector<bool>						BoolList;
	typedef std::vector<RawBuffer>		BinaryList;
	typedef std::vector<std::vector<long> >			LongListList;
	typedef std::vector<std::vector<std::string> >	StringListList;


// Methods
public:
	// Constructors
	RawList(const RawBuffer& theRawBufferR);

	RawList(const wstring& theWStringR, ListTypeEnum theListTypeEnum);

	RawList(const UnicodeList& theUnicodeListR);
	RawList(const StringList& theStringListR);
	RawList(const LongList& theLongListR);
	RawList(const BoolList& theBoolListR);
	RawList(const BinaryList& theBinaryListR);
	RawList(const LongListList& theLongListListR);
	RawList(const StringListList& theStringListListR);

	virtual ~RawList();

	long GetListSize();

	RawBuffer GetRawList();
	wstring GetString();

// Inlines
public:
	ListTypeEnum GetListType()
	{
		return mListType;
	}

	// Note: These methods will return empty lists if wrong type is asked for
	UnicodeList& GetUnicodeList()
	{
		return mUnicodeList;
	}

	StringList& GetStringList()
	{
		return mStringList;
	}

	LongList& GetLongList()
	{
		return mLongList;
	}

	BoolList& GetBoolList()
	{
		return mBoolList;
	}

	BinaryList& GetBinaryList()
	{
		return mBinaryList;
	}

	LongListList& GetLongListList()
	{
		return mLongListList;
	}

	StringListList& GetStringListList()
	{
		return mStringListList;
	}

	bool Error()
	{
		return mListType == RawUnknown;
	}

// Pravate Util Methods
private:

	// Append to RawBuffer methods
	bool AppendUnicode(RawBuffer& theRawBufferR, const wstring& theInWStringR);
	bool AppendString(RawBuffer& theRawBufferR, const string& theInStringR);
	bool AppendLong(RawBuffer& theRawBufferR, const long theInLong);
	bool AppendBool(RawBuffer& theRawBufferR, const long theInBool);
	bool AppendBinary(RawBuffer& theRawBufferR, const RawBuffer& theInRawBufferR);

	bool AppendUnicodeList(RawBuffer& theRawBufferR, const UnicodeList& theInUnicodeListR);
	bool AppendStringList(RawBuffer& theRawBufferR, const StringList& theInStringListR);
	bool AppendLongList(RawBuffer& theRawBufferR, const LongList& theInLongListR);
	bool AppendBoolList(RawBuffer& theRawBufferR, const BoolList& theInBoolListR);
	bool AppendBinaryList(RawBuffer& theRawBufferR, const BinaryList& theInBinaryListR);

	// Append to String methods
	bool AppendUnicode(wstring& theWStringR, const wstring& theInWStringR);
	bool AppendString(wstring& theWStringR, const string& theInStringR);
	bool AppendLong(wstring& theWStringR, const long theInLong);
	bool AppendBool(wstring& theWStringR, const bool theInBool);
	bool AppendBinary(wstring& theWStringR, const RawBuffer& theInRawBufferR);

	bool AppendUnicodeList(wstring& theWStringR, const UnicodeList& theInUnicodeListR);
	bool AppendStringList(wstring& theWStringR, const StringList& theInStringListR);
	bool AppendLongList(wstring& theWStringR, const LongList& theInLongListR);
	bool AppendBoolList(wstring& theWStringR, const BoolList& theInBoolListR);
	bool AppendBinaryList(wstring& theWStringR, const BinaryList& theInBinaryListR);

	// Read from RawBuffer methods
	int ReadUnicode(const RawBuffer& theRawBufferR, wstring& theOutWStringR);
	int ReadString(const RawBuffer& theRawBufferR, string& theOutStringR);
	int ReadLong(const RawBuffer& theRawBufferR, long& theOutLongR);
	int ReadBool(const RawBuffer& theRawBufferR, bool& theOutBoolR);
	int ReadBinary(const RawBuffer& theRawBufferR, RawBuffer& theOutRawBufferR);

	int ReadList(const RawBuffer& theRawBufferR, void* theListP, ListTypeEnum theListType);
	//int ReadUnicodeList(const RawBuffer& theRawBufferR, UnicodeList& theOutUnicodeListR);
	//int ReadStringList(const RawBuffer& theRawBufferR, StringList& theOutStringListR);
	//int ReadLongList(const RawBuffer& theRawBufferR, LongList& theOutLongListR);

	// Read from String methods
	int ReadUnicode(const wstring& theWStringR, wstring& theOutWStringR);
	int ReadString(const wstring& theWStringR, string& theOutStringR);
	int ReadLong(const wstring& theWStringR, long& theOutLongR);
	int ReadBool(const wstring& theWStringR, bool& theOutBoolR);
	int ReadBinary(const wstring& theWStringR, RawBuffer& theOutRawBufferR);

	int ReadList(const wstring& theWStringR, void* theListP, ListTypeEnum theListType);
	int ReadListList(const wstring& theWStringR, void* theListListP, ListTypeEnum theListListType);
	

// Members
private:
	// List stored in binary form
	//RawBuffer		mRawList;

	ListTypeEnum	mListType;

	// List will be stored in one of these lists based on type
	UnicodeList		mUnicodeList;
	StringList		mStringList;
	LongList		mLongList;
	BoolList		mBoolList;
	BinaryList		mBinaryList;
	LongListList	mLongListList;
	StringListList	mStringListList;
};

};

#endif