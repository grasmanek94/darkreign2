/****************************************************************************
 * CDKeyValidate.cpp - Main source file for CDKeyValidate.DLL				*
 *																			*
 * Date: April, 2000								Questions/Comments to 	*
 * A. Install Shield Specific							 SDK@WON.NET		*
 * B. WISE Installed Specific												*
 ****************************************************************************/

#define DllExport   __declspec( dllexport )

#define WIN32_LEAN_AND_MEAN
#include <windows.h>						// windows
#include "authentication/clientcdkey.h"		// TITAN 


 

/****************************************************************************
 *						 A. INSTALLSHIELD SPECIFIC							*
 ****************************************************************************/

extern "C"
{
	// Prototype: aProductName, aCDKeyString, isBetaAllowed
	DllExport BOOL ValidateCDKey(char*,char*,BOOL);
}

// Takes a product name, CD Key and isBeta flag and returns TRUE for a valid CD Key
__declspec (dllexport) BOOL ValidateCDKey(char* aProductName, char* aCDKeyString, BOOL isBetaAllowed) 
{
	// Validate the string.  For beta builds, allow beta keys (IsBeta() == true).
	// For retail builds, don't.
	WONCDKey::ClientCDKey aCDKeyFromUser(aProductName);
	aCDKeyFromUser.Init(aCDKeyString);

	if (!aCDKeyFromUser.IsValid() || (aCDKeyFromUser.IsBeta() && !isBetaAllowed) )
	{
		// Invalid key.
		return false;
	}

	// Store the valid CD key in the registry (encrypted)
	return aCDKeyFromUser.Save();
}





/****************************************************************************
 *						 B. WISE INSTALLER SPECIFIC							*
 ****************************************************************************/

// Structure of parameter data that WISE will pass to our DLL function
typedef struct {
   DWORD wStructLen;	// The length of the structure
   DWORD hWnd;			// Handle to main window
   DWORD wMaxReplaces;	// Maximum number of replaces
   DWORD wRepNameWidth;	// The width of a replace name
   DWORD wRepStrWidth;	// The width of each replace string
   DWORD wCurrReps;		// Current number of replace strings
   char *lpszRepName;	// The actual replace names
   char *lpszRepStr;	// The actual replace values
   DWORD bRunMode;		// The installation mode
   DWORD fLogFile;		// A file handle to the log file
   char *lpszParam;		// String parameter from Wise Installation System
} DLLCALLPARAMS;
typedef DLLCALLPARAMS *LPDLLCALLPARAMS;


// Extracts a data item from the DLLCALLPARAMS structure (INTERNAL USE ONLY)
void GetVariable(LPDLLCALLPARAMS lpDllParams /*in*/,char *szVariable /*in*/,
				 char *szValue /*out*/)
{
	// The variable name, must be surrounded by '%' 
	char szVar[32];			
	
	szVar[0] = '%';					// initial  '%'
	lstrcpy(&szVar[1],szVariable);	// variable name
	lstrcat(szVar,"%");				// trailing '%'
	
	// Search from the first string in the buffer to the last until we find ours
	for (WORD strnum=0; (strnum < lpDllParams->wCurrReps) &&
      (lstrcmp(&lpDllParams->lpszRepName[strnum * lpDllParams->wRepNameWidth],szVar) != 0);
		strnum++) ;

	// Did the loop end because we found our string?
	if (strnum < lpDllParams->wCurrReps) 
		// Yes, copy the data into the buffer
		lstrcpy(szValue,&lpDllParams->lpszRepStr[strnum * lpDllParams->wRepStrWidth]);
	else 
		// String not found, return NULL
		*szValue = '\0';
}


extern "C" {
	// Validation Function Prototype
	DllExport BOOL ValidateCDKeyWISE(LPDLLCALLPARAMS);
}

// Extracts a product name, CD Key and isBeta flag from the PARAMS object
DllExport BOOL ValidateCDKeyWISE(LPDLLCALLPARAMS lpDllParams) 
{
	// Validate the string.  For beta builds, allow beta keys (IsBeta() == true).
	// For retail builds, don't.

	// Allocating more than enough space
	char szCDKey[1023];			// the cd key
	char szProductName[1023];	// the product name (i.e. Homeworld)
	char szBetaParam[1023];		// isBetaAllowed as a string
	bool isBetaAllowed=false;	// isBetaAllowed as a boolean

	// Load the variables from the PARAMS structure
	GetVariable(lpDllParams,"CDKEY",szCDKey);				// cd key
	GetVariable(lpDllParams,"PRODUCTNAME",szProductName);	// product name
	GetVariable(lpDllParams,"ISBETAALLOWED",szBetaParam);	// isBetaAllowed (string)

	// Create a user key from the Product Name and CDKey
	WONCDKey::ClientCDKey aCDKeyFromUser(szProductName);
	aCDKeyFromUser.Init(szCDKey);

	// Initialize our boolen isBetaAllowed from the string data
	if (szBetaParam[0]=='1')
		isBetaAllowed = true;	// '1' sent from WISE Installer script
	else
		isBetaAllowed = false;  // anything but '1'

	//       Is it invalid?       or      Is it beta and beta is not allowed?
	if (!aCDKeyFromUser.IsValid() || (aCDKeyFromUser.IsBeta() && !isBetaAllowed) )
	{
		// Invalid Key
		return false;
	}

	// Store the valid CD key in the registry (encrypted)
	return aCDKeyFromUser.Save();
}

