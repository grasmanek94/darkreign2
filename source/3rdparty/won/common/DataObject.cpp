// DataObject

#include "won.h"
#include "DataObject.h"

// Private namespace for using, constants, etc
namespace {
	using WONCommon::DataObject;
	using WONCommon::DataObjectRep;
};


// ** Constructors / Destructor **

DataObject::DataObject() :
	mRepP(new DataObjectRep)
{}

DataObject::DataObject(const DataType& theType) :
	mRepP(new DataObjectRep(theType))
{}

DataObject::DataObject(const DataType& theType, const Data& theData) :
	mRepP(new DataObjectRep(theType, theData))
{}

DataObject::DataObject(const DataObject& theObjR) :
	mRepP(theObjR.mRepP)
{
#ifdef WIN32
	InterlockedIncrement(&(theObjR.mRepP->mRefCt));
#else
	theObjR.mRepP->mRefCtCrit.Enter();
	theObjR.mRepP->mRefCt++;
	theObjR.mRepP->mRefCtCrit.Leave();
#endif
}

DataObject::~DataObject()
{
	long result;
#ifdef WIN32
	result = InterlockedDecrement(&(mRepP->mRefCt));
#else
	mRepP->mRefCtCrit.Enter();
	result = --(mRepP->mRefCt);
	mRepP->mRefCtCrit.Leave();
#endif

	if (result <= 0)
		delete mRepP;
}


// ** Protected Methods **

void
DataObject::CopyOnWrite()
{
	if (mRepP->mRefCt > 1)
	{
		DataObjectRep* aTmpP = mRepP;
		mRepP = new DataObjectRep(*aTmpP);

#ifdef WIN32
		InterlockedDecrement(&(aTmpP->mRefCt));
#else
		aTmpP->mRefCtCrit.Enter();
		aTmpP->mRefCt--;
		aTmpP->mRefCtCrit.Leave();
#endif
	}
}


// ** Public Methods **

DataObject&
DataObject::operator=(const DataObject& theObjR)
{
	if (mRepP != theObjR.mRepP)
	{
		long result;
#ifdef WIN32
		InterlockedIncrement(&(theObjR.mRepP->mRefCt));
		result = InterlockedDecrement(&(mRepP->mRefCt));
#else
		theObjR.mRepP->mRefCtCrit.Enter();
		theObjR.mRepP->mRefCt++;
		theObjR.mRepP->mRefCtCrit.Leave();

		mRepP->mRefCtCrit.Enter();
		result = --(mRepP->mRefCt);
		mRepP->mRefCtCrit.Leave();
#endif

		if (result <= 0)
			delete mRepP;

		mRepP = theObjR.mRepP;
	}

	return *this;
}


int
DataObject::Compare(const DataObject& theObjectR, bool compareData) const
{
	int aRet = mRepP->mDataType.compare(theObjectR.mRepP->mDataType);
	if ((compareData) && (aRet == 0))
		aRet = mRepP->mData.compare(theObjectR.mRepP->mData);

	return aRet;
}


void
DataObject::SetLifespan(unsigned long theLifespan, time_t theStartTime, bool copy)
{
	if (copy)
		CopyOnWrite();

	mRepP->mLifespan = theLifespan;
	if (theLifespan == 0) // 0 == infinite
		mRepP->mExpireTime = LONG_MAX;
	else
		mRepP->mExpireTime = ((theStartTime == 0) ? time(NULL) : theStartTime) + theLifespan;
}
