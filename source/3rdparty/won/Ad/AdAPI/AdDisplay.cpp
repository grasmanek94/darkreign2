
#pragma warning (disable : 4786)
#include "AdDisplay.h"

using namespace WONAPI;

AdDisplay::AdDisplay() :
	hLoadTrigger(NULL),
	mFuncPaintCallback(NULL),
	mDataPaintP(NULL),
	mCurrentImpageP(NULL),
	mNumScenes(0),
	mCurrentSceneNum(0),
	mTimeToNextScene(INFINITE),
	mAllowSceneSkipping(true),
	mShownCurrentScene(false),
	ThreadBase()
{
    hLoadTrigger = CreateEvent(NULL, false, false, NULL);
	startThread();
}

AdDisplay::~AdDisplay()
{
	stopThread();
	if(hLoadTrigger) CloseHandle(hLoadTrigger); hLoadTrigger = NULL;
	for(RAW_SET::iterator aRawIt(mRawSet.begin()); aRawIt != mRawSet.end(); aRawIt++)
		delete [] *aRawIt;
	mRawSet.clear();
	for(BITMAPH_SET::iterator anBMPIt(mBitmapHSet.begin()); anBMPIt != mBitmapHSet.end(); anBMPIt++)
		DeleteObject(*anBMPIt);
	mBitmapHSet.clear();
}

// If compiled to use single-threaded libraries,
// startThread() will fail to link.
#ifdef _MT
void AdDisplay::startThread()
{
	ThreadBase::startThread();
	ResetEvent(hLoadTrigger);
}
#endif

void AdDisplay::stopThread()
{
	SetEvent(hLoadTrigger);		
	ThreadBase::stopThread();
}

//////////////////////////////////////////////////////////////////////
// Setting callbacks
//////////////////////////////////////////////////////////////////////
	
void
AdDisplay::SetPaintCallback(AdApi::PaintCallback thePaintCallback, void* theDataPaintP)
{
	mFuncPaintCallback = thePaintCallback;
	mDataPaintP = theDataPaintP;
}

//////////////////////////////////////////////////////////////////////
// Display functions
//////////////////////////////////////////////////////////////////////

// Used to get the current animation frame in bitmap form
bool 
AdDisplay::GetDisplayImage(HDC hDC, HBITMAP& theBitmapHR)
{
	// Crit section
	WONCommon::AutoCrit aCrit(mCrit);

	theBitmapHR = NULL;
	try
	{
		if(mCurrentImage.isValid())
		{
			Magick::Blob aBlob;
			mCurrentImage.write(&aBlob, "BMP");
			const void* aPtr = aBlob.data();

			BITMAPFILEHEADER* aFileHeaderP   = (BITMAPFILEHEADER*)aPtr;
			BITMAPINFOHEADER* anInfoHeaderP  = (BITMAPINFOHEADER*)(((BYTE*)aPtr) + sizeof(BITMAPFILEHEADER));
			BYTE*             aBitmapP       = ((BYTE*)(aPtr)) + aFileHeaderP->bfOffBits;

			theBitmapHR = CreateDIBitmap(hDC, anInfoHeaderP, CBM_INIT, aBitmapP, (BITMAPINFO*)(anInfoHeaderP), DIB_RGB_COLORS);
			mBitmapHSet.insert(theBitmapHR);
			mShownCurrentScene = true;
		}
	}
	catch(...)
	{
		if(theBitmapHR)
		{
			mBitmapHSet.erase(theBitmapHR);
			DeleteObject(theBitmapHR);
		}
		return false;
	}
	return (theBitmapHR != NULL);
}

// Used to get the current animation frame in raw form
bool 
AdDisplay::GetDisplayImage(BYTE* &theRawPR, int& theWidthR, int& theHeightR)
{
	// Crit section
	WONCommon::AutoCrit aCrit(mCrit);

	theRawPR = NULL;
	try
	{
		if(mCurrentImage.isValid())
		{
			// Interpret ImageMajicks raw data
			MagickLib::Image* aCurrentImageP = mCurrentImage.image();
			theWidthR = mCurrentImage.size().width();
			theHeightR = mCurrentImage.size().height();
			theRawPR = new BYTE[theWidthR * theHeightR * 3];
			MagickLib::GetPixels(aCurrentImageP, 0, 0, theWidthR, theHeightR, "RGB", MagickLib::CharPixel, theRawPR);
			mRawSet.insert(theRawPR);
			mShownCurrentScene = true;
		}
	}
	catch(...)
	{
		if(theRawPR)
		{
			mRawSet.erase(theRawPR);
			delete [] theRawPR;
		}
		return false;
	}
	return (theRawPR != NULL);
}

void 
AdDisplay::ReleaseDisplayImage(HBITMAP theBitmapH)
{
	if(mBitmapHSet.erase(theBitmapH) > 0) 
		DeleteObject(theBitmapH);
}

void 
AdDisplay::ReleaseDisplayImage(BYTE* theRawP)
{
	if(mRawSet.erase(theRawP) > 0)
		delete [] theRawP;
}

int
AdDisplay::threadProcess()
{
	HANDLE aWaitArray[2] = {getStopEvent(), getLoadEvent()};
	while(true)
	{
		DWORD aResult = WaitForMultipleObjects(2, aWaitArray, false, mTimeToNextScene);
		if(aResult == WAIT_FAILED)
			return 1;

		switch(aResult - WAIT_OBJECT_0)
		{
		// Stop requested
		case 0:
			return 0;
		// Load Image
		case 1:
			break;
		// Timer
		default:
			AdvanceScene();
			if(mFuncPaintCallback)
				mFuncPaintCallback(mDataPaintP);
			break;
		}
	}
}

bool 
AdDisplay::LoadNewImage(const std::string& theFileName)
{
	// Crit section
	WONCommon::AutoCrit aCrit(mCrit);

	mTimeToNextScene = INFINITE;
	SetEvent(hLoadTrigger);		
	bool isNewLoad(false);
	try
	{
		Magick::Image anImage(theFileName);
		mBaseImage = anImage;
		mCurrentImage = mBaseImage;
		mCurrentImpageP = mBaseImage.image();
		mNumScenes = MagickLib::GetNumberScenes(mCurrentImpageP);
		mCurrentSceneNum = 0;

		if(mFuncPaintCallback)
			mFuncPaintCallback(mDataPaintP);

		isNewLoad = true;
	}
	catch(...)
	{
	}
	if(mCurrentImpageP && mCurrentImage.isValid() && mNumScenes > 1)
		mTimeToNextScene = mCurrentImpageP->delay * 10;
	else
		mTimeToNextScene = INFINITE;
	SetEvent(hLoadTrigger);		
	return isNewLoad;
}

bool 
AdDisplay::AdvanceScene(void)
{
	// Crit section
	WONCommon::AutoCrit aCrit(mCrit);

	if(mShownCurrentScene == false && mAllowSceneSkipping == false)
	{
		mTimeToNextScene = 100;
		return false;
	}

	mShownCurrentScene = false;
	if(mCurrentSceneNum > 0)
	{
		// We have exceeded the number of scenes, time to start over from the beginning
		if(mCurrentSceneNum == mNumScenes)
		{
			mCurrentImpageP = mBaseImage.image();
			mCurrentImage = mBaseImage;
			mCurrentSceneNum = 1;
		}
		// Composite the new image over the last image
		else
		{
			mCurrentImpageP = MagickLib::GetNextImage(mCurrentImpageP);
			mCurrentSceneNum++;
			Magick::Image aTempImage(MagickLib::CloneImage(mCurrentImpageP, mCurrentImpageP->columns, mCurrentImpageP->rows, true), mBaseImage.options());
			// Don't ask me why, but this fixes an odd display problem with some GIFs
			if(mCurrentImpageP->matte == 0)
				mCurrentImage.composite(aTempImage, mCurrentImpageP->page_info.x, mCurrentImpageP->page_info.y, Magick::AtopCompositeOp);
			else
				mCurrentImage.composite(aTempImage, mCurrentImpageP->page_info.x, mCurrentImpageP->page_info.y, Magick::OverCompositeOp);
		}
	}
	// First image is displayed as is
	else
		mCurrentSceneNum++;

	mTimeToNextScene = mCurrentImpageP->delay * 10;
	return true;
}
