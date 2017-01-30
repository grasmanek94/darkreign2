/*--------------------------------------------------------------------------
 Module to retrieve info about a CD-ROM and verify it's an 80-minute disc.
 Has methods to store and encrypted profile of the CD-ROM to a file,
 and to compare the CD-ROM to that profile.

 The companion files tracklen_*.* provide the following tools:
 tracklen_w.cpp - program to generate texture.dat encrypted track len file
 tracklen_t.cpp - test program suitable for sending to end users
 tracklen_d.cpp - program to decrypt main.out created by tracklen_t.exe
 tracklen_m.bat - batch file to make above programs

 Dan Kegel 14 July 1999
 Ben Siron 1998, 1999

 This copy protection scheme has two parts:
 1. It checks the audio track lengths against a file on the hard drive.
 If they don't match, perhaps someone burned a backup without redbook audio.
 2. The game disc has an audio track that goes beyond 75 minutes (the usual
 limit is 74 minutes).  

 Issues: 
 1. Some DVD's have trouble reading CD's longer than 74 minutes (Hitachi?).
 2. Normal 74 minute CD blanks actually are 75 or so minutes long, so you
 have to have an audio track that extends past 76 minutes or so for this to
 be effective.  This means the last burn has to be tweaked to get the
 length right.
 3. The check causes the last second of sound to play, so the last second
 or two of sound on the disc MUST be silence to avoid speaker noises.
 4. Some normal CD drives crash if you seek to the end (this code avoids that). 

 NOTE: the pioneer CD-ROM DR-U10X drive has a fit if you try to play from
 the last 8 sectors (107 milliseconds or closer) at the end of the last
 track.  If you tell it to do so, the system hangs, and then 10 seconds later,
 your system will sometimes autoplays the CD.  Frankly this is wierd, and we
 think it is a problem with the firmware on the drive.  This is not a common
 drive anymore, but this problem should be avoided by playing no later than the
 last 1000ms of the last track.
 
 Most drives also have this trouble to some degree.  my CD-ROM drive, for
 example, a Toshiba XM-6202B (24x) can handle seeks to no closer than 8ms
 from the end of the last track.  One DVD drive can seek to 2ms, and one
 has trouble seeking closer than 486ms (!).
 
 1000ms is a good threshhold because all redbook audio capable hardware 
 must support it to properly run the cdplayer.exe application, which
 everybody has, and can seek to the last second on the last track.
--------------------------------------------------------------------------*/

#include <stdio.h>

#include "windows.h"
#include "mbstring.h"
#include "mmsystem.h"
#include "cheatkey.h"

#include "tracklen.h"

#ifdef tracklen_LOGGING

FILE *tracklen_fp = NULL;

char tracklen_buf[512];

/*--------------------------------------------------------------------------
 Encrypt or decrypt the given null-terminated string.
 If the input is a printable ASCII string, so is the output.
 Used to protect the log file created by the end-user test program tracklen_t.
--------------------------------------------------------------------------*/
char *tracklen_cryptAscii(char *s)
{
	static char *keyy = "Checking aspiration formants.\n";

	int i = 0;
	char *p = s;
	static char buf[256];
	char *q = buf;

	while (*p) {
		int k = keyy[i++] & 31;
		if (*p >= 32)
			*q++ = *p++ ^ k;
		else
			*q++ = *p++;
		if (!keyy[i]) i = 0;
	}
	*q++ = 0;
	return buf;
}

#ifdef tracklen_CRYPTLOG
#define tracklen_DPRINT(ss) fprintf(tracklen_fp, "%s:%d ", "tar", 1000+__LINE__); sprintf ss; fprintf(tracklen_fp, "%s", tracklen_cryptAscii(tracklen_buf))
#else
#define tracklen_DPRINT(ss) fprintf(tracklen_fp, "%s:%d ", "tar", 1000+__LINE__); sprintf ss; fprintf(tracklen_fp, "%s", tracklen_buf)
#endif

/* Or, to integrate with anet's DPRINT, you could use
#define tracklen_DPRINT(ss) sprintf ss; DPRINT(("%s:%d %s", __FILE__, __LINE__, tracklen_buf))
*/

static void DMCIError(int error)
{
	char szScratch[ MAX_PATH ];
	mciGetErrorString( error, szScratch, MAX_PATH );
	tracklen_DPRINT((tracklen_buf,  "last MCI error = %s\n", szScratch ));
}


#else
#define tracklen_DPRINT(ss) 
#define DMCIError(error)
#endif 

/*--------------------------------------------------------------------------
 Encrypt or decrypt the given binary buffer.  Used to protect the
 track length file.
--------------------------------------------------------------------------*/
void tracklen_cryptBinary(char *data, size_t len)
{
	BYTE *p = cheatkey_get();

	for (size_t index = 0; index < len; index++ )
		data[ index ] ^= p[ index % 400 ];
}

/*--------------------------------------------------------------------------
 Read the track lengths, in milliseconds, of the open CD-ROM drive with
 the given handle.
 Number of tracks stored in trackLenBuf[0].
 First track length stored in trackLenBuf[1], etc.
 Verify that disc is longer than 74:30 long.
 Device is left open; caller must stop and close device.

 Return 0 on success;
 return error code on failure.
--------------------------------------------------------------------------*/
static int tracklen_GetTrackLengthsViaHandle( DWORD *trackLenBuf, unsigned int wDeviceID )
{
	MCI_STATUS_PARMS msp;
	MCI_SET_PARMS mp;
	DWORD i;
	BOOL bAllTracksOK = TRUE;
	long totalLen_ms;
	int iRet;

	/* Set the device to use milliseconds as its time format */
	memset(&mp, 0, sizeof(mp));
	mp.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
	iRet = mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&mp);
	if( iRet ) {
		tracklen_DPRINT((tracklen_buf, "GetTrackLengths: failed to set time format.\n" ));
		return __LINE__;
	}
	tracklen_DPRINT((tracklen_buf, "GetTrackLengths: set time format to milliseconds.\n"));

	/* Get the number of tracks. */
	memset(&msp, 0, sizeof(msp));
	msp.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	if( iRet = mciSendCommand( wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&msp ) ) {
		tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: failed to read # of tracks\n"));
		DMCIError( iRet );
		return __LINE__;
	}
	trackLenBuf[0] = msp.dwReturn;
	tracklen_DPRINT((tracklen_buf, "GetTrackLengths: %d tracks\n", trackLenBuf[0]));
	if (trackLenBuf[0] > tracklen_MAXTRACKS) {
		tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: too many tracks\n"));
		return __LINE__;
	}

	/* Read length of each track */
	for (i = 1; i <= trackLenBuf[0]; i++) {
		memset(&msp, 0, sizeof(msp));
		msp.dwItem = MCI_STATUS_LENGTH;
		msp.dwTrack = i;
		iRet = mciSendCommand( wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, (DWORD)&msp );
		if ( iRet ) {
			tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: can't get track %d len\n", i));
			DMCIError( iRet );
			trackLenBuf[i] = 0;
			continue;
		}
		trackLenBuf[i] = msp.dwReturn;
		tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: track %d len %d\n", i, trackLenBuf[i]));
	}

	// Get the total length of the CD
	msp.dwItem = MCI_STATUS_LENGTH;
	iRet = mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&msp);
	if( iRet ) {
		tracklen_DPRINT((tracklen_buf, "GetTrackLengths: Can't get disc len.\n" ));
		DMCIError( iRet );
		return __LINE__;
	}
	totalLen_ms = msp.dwReturn;
	tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: got disc len %d\n", totalLen_ms ));

	// Note: At this point we used to attempt to play the last track on the CD as an additional
	//       validation test; However, we decided to remove this test, since it appeared to
	//       cause erratic behavior with some CD-ROM drives (e.g. Samsung's).

	// Verify that disc is between 74:30 and 80:30 minutes:seconds long. 
	if (totalLen_ms < ((74 * 60 + 30) * 1000)) {
		tracklen_DPRINT((tracklen_buf, "GetTrackLengths: disc is too short\n"));
		return __LINE__;
	}
	if (totalLen_ms > ((80 * 60 + 30) * 1000)) {
		tracklen_DPRINT((tracklen_buf, "GetTrackLengths: disc is too long\n"));
		return __LINE__;
	}

	/* Success. */
	return 0;
}

/*--------------------------------------------------------------------------
 Read the track lengths, in milliseconds, of the drive with the given
 drive letter.  
 Verify that disc is longer than 74:30 long.
 Number of tracks stored in trackLenBuf[0].
 First track length stored in trackLenBuf[1], etc.
 trackLenBuf must be an array of length tracklen_MAXTRACKS.
 Return 0 on success;
 return error code on failure.
--------------------------------------------------------------------------*/
int tracklen_GetTrackLengths(DWORD *trackLenBuf, char whichDrive)
{
	MCI_OPEN_PARMS mop;
	char szDrive[16];
	int iRet;

#ifdef tracklen_LOGGING
	if (!tracklen_fp) tracklen_fp = fopen(tracklen_LOGFILE, "w");
#endif


	/* Open the device.  Block until it really opens. */
	memset(&mop, 0, sizeof(mop));
	mop.lpstrDeviceType = (LPCSTR)(MCI_DEVTYPE_CD_AUDIO);
	mop.lpstrAlias = NULL;
	szDrive[0] = whichDrive;
	szDrive[1] = ':';
	szDrive[2] = 0;
	mop.lpstrElementName = szDrive;
	mop.dwCallback = NULL;
	mop.wDeviceID = 0;
	if( iRet = mciSendCommand( 0, MCI_OPEN, MCI_WAIT | MCI_OPEN_ELEMENT | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_SHAREABLE, (DWORD)&mop ) )
	{
		tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: failed to open MCI device.\n" ));
		DMCIError( iRet );
		return __LINE__;
	}
	tracklen_DPRINT((tracklen_buf,  "GetTrackLengths: Opened MCI device\n" ));

	/* Get the track lengths, or try to. */
	iRet = tracklen_GetTrackLengthsViaHandle( trackLenBuf, mop.wDeviceID);

	/* Close the device.  Ignore errors on close. */
	mciSendCommand( mop.wDeviceID, MCI_CLOSE, 0, NULL );

	tracklen_DPRINT((tracklen_buf, "GetTrackLengths: Returning %d\n", iRet ));
	return iRet;
}

/*--------------------------------------------------------------------------
 Given an array trackLenBuf[] with
 number of tracks stored in trackLenBuf[0],
 first track length stored in trackLenBuf[1], etc.
 return zero if a CD matching that description is in some drive,
 the disc is longer than 74 minutes.

 On failure, return nonzero error code.
--------------------------------------------------------------------------*/

static int tracklen_CheckTrackLengths2( DWORD *trackLenBuf)
{
	if( !trackLenBuf )
		return __LINE__;

	tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: begin\n" ));

	// shut down CD player if necessary, so we can read track lengths
	int i;
	HWND hCD;
	for (i=0; (hCD=FindWindow( "SJE_CdPlayerClass",NULL)) && i<100; i++) {
		tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: Shutting down CD player\n" ));
		SendMessage( hCD, WM_CLOSE, 0, 0 );
	}

	/* Loop over all drives */
	int iDrive;
	for (iDrive=0; iDrive<26; iDrive++) {
		char szDrive[ MAX_PATH ];
		int iDriveType;
		int iRet;

		/* Check to see if this is a CD-ROM drive. */
		szDrive[0] = iDrive + 'A';
		szDrive[1] = ':';
		szDrive[2] = '\\';
		szDrive[3] = 0;
		iDriveType = GetDriveType( szDrive );
		if( iDriveType != DRIVE_CDROM ) {
			tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: %s type %d, not cd.\n", szDrive, iDriveType ));
			continue;
		}
		tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: device %s is cd.\n", szDrive ));

		/* Compare its track lengths */
		DWORD newbuf[tracklen_MAXTRACKS];
		iRet = tracklen_GetTrackLengths(newbuf, szDrive[0]);
		if (iRet) {
			tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: drive %s fails check %d\n", szDrive, iRet ));
			continue;
		}
		if (newbuf[0] != trackLenBuf[0]) {
			tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: drive %s has %d tracks, wanted %d\n", szDrive, newbuf[0], trackLenBuf[0] ));
			continue;
		}
		/* Skip data track (track 1), as it's different from build to build 
		 * and from US to UK
		 */
		for (i=2; i< (int)(newbuf[0]); i++) {
			if (newbuf[i] != trackLenBuf[i]) {
				tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: drive %s track %d has len %d, wanted %d\n", szDrive, i, newbuf[i], trackLenBuf[i]));
				break;
			}
		}
		/* Did we match 'em all?  If so, we succeeded! */
		if (i == ((int) newbuf[0])) {
			tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: success!\n"));
			return 0;
		}
	}

	tracklen_DPRINT((tracklen_buf,  "CheckTrackLengths2: failure!\n"));
	return __LINE__;
}

/*--------------------------------------------------------------------------
 Read in a tracklen buffer from the given file and decrypt it.
 If the filename does not contain a colon, it is assumed to be in the same
 directory as the current executable module.
--------------------------------------------------------------------------*/
static DWORD *tracklen_LoadEncryptedKey( DWORD *trackLenBuf, const char *szFile )
{
	char szTemp[ MAX_PATH ];

  strcpy( szTemp, szFile );

	HANDLE hFile = CreateFile( szTemp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if( hFile == INVALID_HANDLE_VALUE ) {
		tracklen_DPRINT((tracklen_buf, "tracklen_LoadEncryptedKey: can't open %s\n", szTemp));
		return NULL;
	}

	DWORD dwSize = GetFileSize( hFile, NULL );
	if ((dwSize < 4) || (dwSize % 4)) {
		tracklen_DPRINT((tracklen_buf, "tracklen_LoadEncryptedKey: %s bad size\n", szTemp));
		CloseHandle( hFile );
		return NULL;
	}


	DWORD ignore;
	ReadFile( hFile, trackLenBuf, dwSize, &ignore, NULL );
	CloseHandle( hFile );
	if( ignore < dwSize ) {
		tracklen_DPRINT((tracklen_buf, "tracklen_LoadEncryptedKey: file %s bad size 2\n", szTemp));
		return NULL;
	}

	*trackLenBuf = dwSize/4;

	tracklen_cryptBinary((char *)(trackLenBuf+1), dwSize-4);

	tracklen_DPRINT((tracklen_buf, "tracklen_LoadEncryptedKey: success\n"));
	return trackLenBuf;
}

/*--------------------------------------------------------------------------
 Return TRUE if a CD in some drive is longer than 74 minutes, can be
 seeked to near the end of the disc, and matches the encrypted track length 
 stored in the file named tracklen_DATFILE.
--------------------------------------------------------------------------*/

BOOL __cdecl tracklen_CheckTrackLengths(const char *fileName)
{
	int rv;
	DWORD trackLenBuf[tracklen_MAXTRACKS];

#ifdef tracklen_LOGGING
	if (!tracklen_fp) tracklen_fp = fopen(tracklen_LOGFILE, "w");
#endif

	if (!tracklen_LoadEncryptedKey( trackLenBuf, fileName))
		return FALSE;
	rv = tracklen_CheckTrackLengths2( trackLenBuf );
	if (rv)
		return FALSE;
	return TRUE;
}

/* An illegal value, and two useful values of the autorun flags word. */
#define AUTORUN_UNKNOWN 0		/* not really illegal, but definitely unusual */
#define AUTORUN_DEFAULT 0x95
#define AUTORUN_DISABLE 0xff

/* Old value of autoplay registry entry. 
 * Only the first call to AutoPlay_Disable sets this value.
 */
static ULONG s_oldAutoRunValue = AUTORUN_UNKNOWN;

/*--------------------------------------------------------------------------
 Save the current autoplay setting, then disable autoplay.
--------------------------------------------------------------------------*/
void tracklen_AutoPlay_Disable()
{	
	int res;
	HKEY hkey;

	/* Look for AutoPlay registry key.  
	 * Get its current value into oldAutoRunValue.
	 * Set its value to disable autorun from any medium type.
	 */
	hkey = NULL;
	res = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_ALL_ACCESS, &hkey);
	if (res == ERROR_SUCCESS) {	
		// The autorun key exists.  
		// If this is the first call, get the autorun key's value.
		// Otherwise, just keep the value from the first call.
		// This guards against multiple calls.
		if (s_oldAutoRunValue == AUTORUN_UNKNOWN) {
			unsigned long ulDataSize = sizeof(s_oldAutoRunValue);
			res = RegQueryValueEx(hkey, "NoDriveTypeAutoRun", 0, NULL, (unsigned char*)&s_oldAutoRunValue, &ulDataSize);
		}
		// Now set its value to DISABLED.
		if (res == ERROR_SUCCESS) {
			unsigned long ulDisableAutoRun = AUTORUN_DISABLE;
			RegSetValueEx(hkey, "NoDriveTypeAutoRun", 0, REG_BINARY, (const unsigned char*)&ulDisableAutoRun, 4);
		}
		RegFlushKey(hkey);
		RegCloseKey(hkey);
	 }
}

/*--------------------------------------------------------------------------
 Restore the autoplay setting saved by AutoPlay_Disable().
--------------------------------------------------------------------------*/
void tracklen_AutoPlay_Restore()
{	
	int res;
	HKEY hkey = NULL;

	// If AutoPlay_Disable() was never called, just return.
	if (s_oldAutoRunValue == AUTORUN_UNKNOWN) {
		return;
	}

	// Restore AutoPlay setting
	res = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", 0, KEY_ALL_ACCESS, &hkey);
	if (res == ERROR_SUCCESS) {	
		RegSetValueEx(hkey, "NoDriveTypeAutoRun", 0, REG_BINARY, (const unsigned char*)&s_oldAutoRunValue, 4);
		RegFlushKey(hkey);
		RegCloseKey(hkey);
	 }
}
