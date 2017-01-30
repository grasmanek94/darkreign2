
///////////////////////////////////////////////////////////////////////////////////
//
// Ad serving API
//
// Contains class for downloading and displaying ads in any client.
//
// This class is currently an interface for the Aureate ad download and display system.
//
///////////////////////////////////////////////////////////////////////////////////

#include "AdAPI.h"
#include "RAImageDll.h"
#include <string>

using namespace std;
using namespace WONAPI;

// Local function prototypes
static unsigned long GetRadiateIdFromClientId(AdClient::ClientType theClientType);

AdClient::AdClient() :
	mRAImageInterfaceP(NULL),
	mInstHandle(NULL),
	mFuncPaintCallback(NULL),
	mDataPaintP(NULL),
	mOutputType(OutputType_Invalid)
{
	mRAImageInterfaceP = new RAImageDLL(); 
}

AdClient::~AdClient()
{
	// Make sure system is shut down if not already so
	Shutdown();

	delete mRAImageInterfaceP;
	mRAImageInterfaceP = NULL;
}

AdClient::AdResult 
AdClient::Init(ClientType theClientType, DWORD theVersion, AdType theAdType, OutputType theOutputType)
{
	if( mInstHandle != NULL )
	{
		Shutdown();
		mInstHandle = NULL;
	}
	
	unsigned long aAdType;
	switch (theAdType)
	{
		case AdType_Banner468x60: 
			aAdType = 1;
		break;
		case AdType_Banner120x60: 
			aAdType = 20;
		break;
		default: // Invalid
			aAdType = 0;
		break;
	}

	mOutputType = theOutputType;

	unsigned long aFlags = 0;
	switch (mOutputType)
	{
		case OutputType_RGB: 
			aFlags |= FLAG_RGB_BUFFER;
		break;
		case OutputType_Bitmap: 
		default: // Invalid
		break;
	}
	
	mInstHandle = mRAImageInterfaceP->Initialize(GetRadiateIdFromClientId(theClientType), theVersion, aAdType, aFlags);

	if ( mInstHandle != NULL )
	{
		// Set the internal callback to be called when painting is needed
		mRAImageInterfaceP->SetPaintCallback(mInstHandle, callback_paint, this);
		return  AdResult_Success;
	}
	else // error
	{
		return  AdResult_Failure;
	}
}

AdClient::AdResult 
AdClient::Shutdown()
{
	if( mInstHandle != NULL )
	{
		mRAImageInterfaceP->Shutdown(mInstHandle);
		mInstHandle = NULL;
	}

	return AdResult_Success;
}

//////////////////////////////////////////////////////////////////////
// Setting callbacks
//////////////////////////////////////////////////////////////////////

#ifdef _TITANAPI
	
// Want to use compleation routines if this is being built into the titan API
AdClient::AdResult 
AdClient::SetPaintCallback(PaintCallback thePaintCallbackP, void* theDataPaintP)
{
	mFuncPaintCallback = thePaintCallbackP;
	mDataPaintP = theDataPaintP;

	return AdResult_Success;
}

#else

AdClient::AdResult 
AdClient::SetPaintCallback(PaintCallback thePaintCallbackP, void* theDataPaintP)
{
	mFuncPaintCallback = thePaintCallbackP;
	mDataPaintP = theDataPaintP;

	return AdResult_Success;
}

#endif

AdClient::AdResult 
AdClient::SetLaunchCallback(LaunchCallback theLaunchCallbackP, const char* theFile)
{
	if( mInstHandle == NULL )
		return AdResult_NotInitialized;

	// Note: Currently there is not way to store the context for this callback so
	// we pass this one straight through.
	mRAImageInterfaceP->SetBrowserCallback(mInstHandle, theLaunchCallbackP);

	return AdResult_Success;
}


//////////////////////////////////////////////////////////////////////
// Download functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// SetThrottle
// 
// theBandwidth - Max number of bytes/sec that adds my download at
AdClient::AdResult 
AdClient::SetThrottle(unsigned long theBandwidth)
{
	return mRAImageInterfaceP->SetThrottle(theBandwidth) ? AdResult_Success : AdResult_Failure;
}

AdClient::AdResult 
AdClient::SetProxy(const char* theAddress, unsigned short thePort, const char* theUserName, const char* thePassword)
{
	return mRAImageInterfaceP->SetProxy(theAddress, thePort, theUserName, thePassword) ? AdResult_Success : AdResult_Failure;
}

//////////////////////////////////////////////////////////////////////
// Display functions
//////////////////////////////////////////////////////////////////////

AdClient::AdResult 
AdClient::AdClicked()
{
	if( mInstHandle == NULL )
		return AdResult_NotInitialized;

	mRAImageInterfaceP->AdActivated(mInstHandle);

	return AdResult_Success;
}

AdClient::AdResult 
AdClient::GetCurrentImage(HBITMAP& theBitmapR)
{
	if( mInstHandle == NULL )
		return AdResult_NotInitialized;
	if( mOutputType != OutputType_Bitmap)
		return AdResult_OutputTypeMisMatch;

	theBitmapR = mRAImageInterfaceP->GetBufferBitmap(mInstHandle);

	return AdResult_Success;
}

AdClient::AdResult 
AdClient::GetCurrentImage(BYTE*& theImageR, int& theXSizeR, int& theYSizeR)
{
	if( mInstHandle == NULL )
		return AdResult_NotInitialized;
	if( mOutputType != OutputType_RGB)
		return AdResult_OutputTypeMisMatch;

	theImageR = mRAImageInterfaceP->GetBufferRGB(mInstHandle, theXSizeR, theYSizeR);

	return AdResult_Success;
}

AdClient::AdResult 
AdClient::GetCurrentPalette(HPALETTE& thePalletteR)
{
	if( mInstHandle == NULL )
		return AdResult_NotInitialized;

	return AdResult_Success;
}

AdClient::AdResult 
AdClient::LockImage(bool theLock)
{
	if( mInstHandle == NULL )
		return AdResult_NotInitialized;

	if ( theLock )
	{
		mRAImageInterfaceP->LockBuffer(mInstHandle);
	}
	else
	{
		mRAImageInterfaceP->UnlockBuffer(mInstHandle);
	}

	return AdResult_Success;
}

//////////////////////////////////////////////////////////////////////
// Callback functions
//////////////////////////////////////////////////////////////////////

void __stdcall callback_paint(void *theData)
{
	AdClient* This = (AdClient*)theData; 
	if (This->GetPaintCallback() != NULL)
		This->GetPaintCallback()(This->GetDataPaint());
}

//////////////////////////////////////////////////////////////////////
// Static functions
//////////////////////////////////////////////////////////////////////

static unsigned long GetRadiateIdFromClientId(AdClient::ClientType theClientType)
{
	unsigned long aRadiateId;
	switch (theClientType)
	{
		case AdClient::ClientType_Test: // Test
			aRadiateId = 0;
		break;
		case AdClient::ClientType_Generic_WON: // Test
			aRadiateId = 753;
		break;
			
		case AdClient::ClientType_Ground_Control: // Ground Control
			aRadiateId = 753;
		break;
		default: // Test
			aRadiateId = 0;
		break;
	}

	return aRadiateId;
}

//////////////////////////////////////////////////////////////////////
// Installation functions
//////////////////////////////////////////////////////////////////////

bool InstallAdAPI(AdClient::ClientType theClientType, char* theClientName)
{
	STARTUPINFO aStartInfo;
	memset(&aStartInfo, 0, sizeof(STARTUPINFO));
	aStartInfo.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION aProcessInfo;
	
	char aCmdLine[256];

	sprintf(aCmdLine,"msipcsv.exe -install -sid=%u -name\"%s\"",GetRadiateIdFromClientId(theClientType),theClientName);
	return  CreateProcess( NULL, aCmdLine,	NULL, NULL, FALSE, 0, NULL, NULL, &aStartInfo, &aProcessInfo) ? true : false;
}

bool UnInstallAdAPI(AdClient::ClientType theClientType)
{
	STARTUPINFO aStartInfo;
	memset(&aStartInfo, 0, sizeof(STARTUPINFO));
	aStartInfo.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION aProcessInfo;

	char aCmdLine[256];

	sprintf(aCmdLine,"msipcsv.exe -uninstall -sid=%u",GetRadiateIdFromClientId(theClientType));
	return  CreateProcess( NULL, aCmdLine,	NULL, NULL, FALSE, 0, NULL, NULL, &aStartInfo, &aProcessInfo) ? true : false;
}

//void __stdcall callback_launch(const char *theFile)
//{
//	funcLaunchCallback(theFile);
//}
