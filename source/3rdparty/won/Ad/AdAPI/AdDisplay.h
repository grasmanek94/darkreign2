#ifndef _ADDISPLAY_H
#define _ADDISPLAY_H

#include "AdApi.h"
#include "ThreadBase.h"
#include "CriticalSection.h"
#include "Image.h"

#pragma warning (disable : 4786)
#include <set>

namespace WONAPI
{

// Base class that all ad display classes are derived from
	class AdDisplay : public WONCommon::ThreadBase
{
public:
	// Constructor/Destructor
	AdDisplay();
	virtual ~AdDisplay();

	// Set the paint callback
	void SetPaintCallback(AdApi::PaintCallback thePaintCallback, void* theDataPaintP);

	// Load a new image from file
	bool LoadNewImage(const std::string& theFileName);

	// Get and release for the various formats
	bool GetDisplayImage(HDC hDC, HBITMAP& theBitmapHR);
	bool GetDisplayImage(BYTE* &theRawPR, int& theWidthR, int& theHeightR);

	void ReleaseDisplayImage(HBITMAP theBitmapH);
	void ReleaseDisplayImage(BYTE* theRawP);

	// Allow or disallow skipping of Ad Scenes when timing issues interfere with the proper presentation
	void AllowSceneSkipping(bool isSceneSkippingAllowed = true);
	bool IsSceneSkippingAllowed(void);

	// Thread over-rides
	void startThread();
	void stopThread();

	// Member data
private:
	AdApi::PaintCallback	mFuncPaintCallback;
	void*					mDataPaintP;

	Magick::Image           mBaseImage;
	Magick::Image           mCurrentImage;
	MagickLib::Image*       mCurrentImpageP;
	int                     mNumScenes;
	int                     mCurrentSceneNum;
	DWORD                   mTimeToNextScene;
	bool					mAllowSceneSkipping;
	bool					mShownCurrentScene;

	WONCommon::CriticalSection mCrit;

	typedef std::set<BYTE*> RAW_SET;
	RAW_SET mRawSet;

	typedef std::set<HBITMAP> BITMAPH_SET;
	BITMAPH_SET mBitmapHSet;

	bool AdvanceScene(void);

	HANDLE hLoadTrigger;
	HANDLE getLoadEvent();

	int threadProcess();
};

inline HANDLE AdDisplay::getLoadEvent()
{
	return hLoadTrigger;
}

inline void AdDisplay::AllowSceneSkipping(bool isSceneSkippingAllowed /* = true */)
{
	mAllowSceneSkipping = isSceneSkippingAllowed;
}

inline bool AdDisplay::IsSceneSkippingAllowed(void)
{
	return mAllowSceneSkipping;
}

} // WONAPI

#endif // _ADDISPLAY_H