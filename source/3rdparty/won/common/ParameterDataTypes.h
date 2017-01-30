#ifndef _PARSERVERDATATYPES_H
#define _PARSERVERDATATYPES_H

//////////////////////////////////////////////////////////////////////
// ParServerDataTypes.h
//////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include <set>

#include "common/won.h"

namespace WONCommon {

//////////////////////////////////////////////////////////////////////
// Global Constants
//////////////////////////////////////////////////////////////////////

// Query flags
const unsigned short LogicalConfigsMustExist	= 1; 
const unsigned short ParamsMustExist			= 2;

//////////////////////////////////////////////////////////////////////
// Global Enums
//////////////////////////////////////////////////////////////////////

enum ParamTypeEnum
{
	ParamType_Binary	= 'R',
	ParamType_Bool		= 'B',
	ParamType_List		= 'L',
	ParamType_Long		= 'I',
	ParamType_LongLong	= 'H',
	ParamType_String	= 'S',
	ParamType_WString	= 'W'
};

//////////////////////////////////////////////////////////////////////
// Types
//////////////////////////////////////////////////////////////////////

class Parameter;

typedef std::list<std::string>	NameList;
typedef std::set<std::string>	NameSet;
typedef std::list<Parameter>	ParameterList;
typedef std::set<Parameter>		ParameterSet;

typedef WONCommon::RawBuffer ParamValue;

//////////////////////////////////////////////////////////////////////
// Parameter
//////////////////////////////////////////////////////////////////////

class Parameter
{

// Methods
public:

	// Constructors/Destructors
	Parameter() :
		mName(),
		mType(ParamType_Binary),
		mValue()
	{
	}

	Parameter(const Parameter& theParameter)  :
		mName(theParameter.mName),
		mType(theParameter.mType),
		mValue(theParameter.mValue)
	{
	}

	virtual ~Parameter()
	{
	}


	// Operator Overloads
	Parameter& operator=(const Parameter& theParameter)
	{
		mName = theParameter.mName;
		mType = theParameter.mType;
		mValue = theParameter.mValue;
		return *this;
	}

	bool Parameter::operator==(const Parameter& theParameterR) const
	{
		return mName == theParameterR.mName;
	}

	bool Parameter::operator<(const Parameter& theParameter) const
	{
		return mName < theParameter.mName;
	}

	long GetPackSize() const
	{
		long aSize = sizeof(short) + mName.size(); // Name
		aSize += 1; // Type
		aSize += sizeof(short) + mValue.size(); 
		return aSize;
	}
	

// Member data
public:
	std::string		mName;
	ParamTypeEnum	mType;
	ParamValue		mValue;
};

}

#endif