#include "windows.h"
#include "string.h"

typedef struct {
   long wStructLen;   // The length of the structure
   long hWnd;         // Handle to main window
   long wMaxReplaces; // Maximum number of replaces
   long wRepNameWidth;// The width of a replace name
   long wRepStrWidth; // The width of each replace string
   long wCurrReps;    // Current number of replace strings
   char *lpszRepName;  // The actual replace names
   char *lpszRepStr;   // The actual replace values
   long bRunMode;     // The installation mode
   long fLogFile;     // A file handle to the log file
   char *lpszParam;    // String parameter from Wise Installation System
} DLLCALLPARAMS;

void GetVariable(DLLCALLPARAMS* lpDllParams,char *szVariable,char *szValue);
void SetVariable(DLLCALLPARAMS* lpDllParams,char *szVariable,char *szValue);

extern "C"
{
	__declspec(dllexport) bool ValidateCDKeyWise(DLLCALLPARAMS* lpDllParams);
}

__declspec(dllexport) bool ValidateCDKeyWise(DLLCALLPARAMS* lpDllParams)
{
   bool bResult = false;

   return bResult;
}

void GetVariable(DLLCALLPARAMS* lpDllParams,char *szVariable,char *szValue)
{
   short i;
   char szVar[32];

   *szVar = '%';
   lstrcpy(&szVar[1],szVariable);
   lstrcat(szVar,"%");
   for (i = 0 ; (i < lpDllParams->wCurrReps) &&
      (lstrcmp(&lpDllParams->lpszRepName[i * lpDllParams->wRepNameWidth],szVar) != 0) ; i++) ;
   if (i < lpDllParams->wCurrReps) {
      lstrcpy(szValue,&lpDllParams->lpszRepStr[i * lpDllParams->wRepStrWidth]);
   } else *szValue = '\0';
}

void SetVariable(DLLCALLPARAMS* lpDllParams,char *szVariable,char *szValue)
{
   WORD i;
   char szVar[32];

   *szVar = '%';
   lstrcpy(&szVar[1],szVariable);
   lstrcat(szVar,"%");
   for (i = 0 ; (i < lpDllParams->wCurrReps) &&
      (lstrcmp(&lpDllParams->lpszRepName[i * lpDllParams->wRepNameWidth],szVar) != 0) ; i++) ;
   if (i >= lpDllParams->wCurrReps) {
      if (i >= lpDllParams->wMaxReplaces) return; // Too many variables
      lstrcpy(&lpDllParams->lpszRepName[i * lpDllParams->wRepNameWidth],szVar);
      lpDllParams->wCurrReps++;
   }
   lstrcpy(&lpDllParams->lpszRepStr[i * lpDllParams->wRepStrWidth],szValue);
}
