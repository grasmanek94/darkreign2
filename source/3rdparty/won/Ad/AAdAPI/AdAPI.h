#ifndef _ADAPI_H
#define _ADAPI_H

#include "windows.h"

namespace WONAPI
{

// Prototypes
class RAImageDLL;

class AdClient
{
public:
	// Error code of the operation
	enum AdResult
	{
		AdResult_Success = 0,			// Success
		AdResult_Failure = 1,			// General failure
		AdResult_NotInitialized = 2,	// Failure because not properly initialized
		AdResult_NotSupported = 3,		// Current version of client does not support this functionality
		AdResult_OutputTypeMisMatch = 4	// Output type asked for does not match the type specified at initialization.  
	};

	// Type of the ads being served
	enum AdType
	{
		AdType_Invalid = 0,				// Invalid type		
		AdType_Banner468x60 = 1,		// Banner size 468x60	
		AdType_Banner120x60 = 2			// Banner size 120x60
	};

	// Id of game or software that is displaying ads
	// Every client will need an id to be added so it can be determined
	// what adds to display for them.
	enum ClientType
	{
		ClientType_Test = 0,				// Test client
		ClientType_Generic_WON = 1,			// Generic WON
		ClientType_Ground_Control = 100		// Ground Control
	};

	// Type of output that display client will be asking for
	enum OutputType
	{
		OutputType_Invalid = 0,
		OutputType_Bitmap = 1,
		OutputType_RGB = 2
	};

	// callback function typedefs
	typedef void (__stdcall *PaintCallback)(void *theData);
	typedef void (__stdcall *LaunchCallback)(const char *theFile);

	// Constructor/Destructor
	AdClient();
	~AdClient();

	// Public Methods
public: 
	// Initializes the ad delivery system.  As soon at this function is called ads
	// start to download and be displayed.
	AdResult Init(	ClientType theClientType,		// Type of client that is serving ads.  This is used to determine what adds to show to the client
					DWORD theVersion,				// The version of the client program
					AdType theAdType,				// This the type of ad.  In general this is used to determine banner size
					OutputType theOutputType);		// Type of display to be used in the GetDisplayImage call

	// This function should be called when your program is about to shut down and
	// ads no longer need to be displayed.
	AdResult Shutdown();

	// Setting callbacks
	// Sets the function to be called when an animation frame of an ad needs to be painted.
	AdResult SetPaintCallback(	PaintCallback thePaintCallbackP,	// Function pointer of the Paint Callback
								void* theDataPaintP);				// Data to be passed into Paint Callback

	// Sets the function to be called when when the AdApi wants to launch the browser.  This gives
	// the client the chance to use it's own browser.
	AdResult SetLaunchCallback(	LaunchCallback theLaunchCallbackP,  // Function pointer to Launch Callback
								const char* theFile);				// The file/url to be displayed

	// Download functions
	// Sets the throttle bandwidth for the ad delivery system.
	AdResult SetThrottle(unsigned long theBandwidth);	// Maximum bandwidth in bytes/sec that the ad system is allowed to use
	
	// Sets the proxy settings for the ad delivery system.  This should be used if the user's communication to the internet requires as proxy server.
	AdResult SetProxy(	const char* theAddress,			// Machine name of IP address of the proxy.  Set to null to cancel use of a proxy.
						unsigned short thePort,			// TCP listen port of the proxy
						const char* theUserName,		// Username to provide to proxy
						const char* thePassword);		// Password to provide to proxy

	// Display functions
	// Call this function when the user clicks on the ad.  This will log some information and start the browser.
	AdResult AdClicked();

	// Used to get the current animation frame in bitmap form
	AdResult GetCurrentImage(HBITMAP& theBitmapR);		// Handle to the bitmap

	// Used to get the current animation frame in 24 bit raw form
	// Data is stored in this format
	//#pragma pack(1)		// single byte packing
	//
	//typedef struct _tagRGB
	//{
	//	BYTE Red;
	//	BYTE Green;
	//	BYTE Blue;
	//} RGBval;
	//
	//BYTE *image = new BYTE [image_width(468) * image_height(60) * sizeof(RGBval)];
	//
	// pixels are stored in row-major order starting with the top left corner.  So first row then next and so on..
	//
	AdResult GetCurrentImage(	BYTE*& theImageR,		// Address of pointer to block of memory that contains the image on return
								int& theXSizeR,			// Size of the image returned in the X direction.
								int& theYSizeR);		// Size of the image returned in the Y direction.

	// Used to get the current pallet used by the animation frame.
	AdResult GetCurrentPalette(HPALETTE& thePaletteR);	// Handle to the palette

	// Lock or unlock the current image so that it can be displayed.  This is done so that 
	// the image is not changed while it is being displayed.
	// Should always lock the image before you get and blit it and then unlock it afterwords
	AdResult LockImage(bool theLock);					// true locks the image, false unlocks it.

	// Accessor functions
	// These are used internally and should not be used
public:

	// Inlines
	void* GetDataPaint()
	{
		return mDataPaintP;
	}
	PaintCallback GetPaintCallback()
	{
		return mFuncPaintCallback;
	}

	// Member data
private:
	RAImageDLL*		mRAImageInterfaceP;
	HANDLE			mInstHandle; // Handle for this current instance

	// Callback pointer
	PaintCallback	mFuncPaintCallback;

	// Callback data pointer
	void*			mDataPaintP;

	// Type of output expected
	OutputType		mOutputType;
};

}
// Registration Unregistration functions
// These functions are not included in the AdClient object so that an AdClient object will not have to be
// instantiated at install time.
bool InstallAdAPI(WONAPI::AdClient::ClientType theClientType,		// Type of client that is serving ads.  This is used to determine what adds to show to the client.  This is the same as the id passed into the Init function
				  char* theClientName);								// The name of the game

bool UnInstallAdAPI(WONAPI::AdClient::ClientType theClientType);	// Type of client that is serving ads.  This is used to determine what adds to show to the client.  This is the same as the id passed into the Init function

// Callback
// callback function
void __stdcall callback_paint(void *theData);

#endif