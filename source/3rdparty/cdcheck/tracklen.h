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
--------------------------------------------------------------------------*/

#ifndef tracklen_h
#define tracklen_h

#include <windows.h>

/* Enable for debug */
/* #define tracklen_LOGGING */
/* #define tracklen_CRYPTLOG */

#ifdef __cplusplus
extern "C" {
#endif

/* Assume any CD we protect has less than MAXTRACKS tracks */
#define tracklen_MAXTRACKS 128

/* Standard (and obscure) name for the encrypted track length file */
#define tracklen_DATFILE "texture.dat"

#ifdef tracklen_LOGGING
#define tracklen_LOGFILE "cd.log"
/* Used by logging code inside tracklen.cpp; set by main() or tracklen_CheckTrackLengths() */
extern FILE *tracklen_fp;
extern char tracklen_buf[];

/*--------------------------------------------------------------------------
 Encrypt or decrypt the given null-terminated string.
 If the input is a printable ASCII string, so is the output.
 Used to protect the log file created by the end-user test program tracklen_t.
--------------------------------------------------------------------------*/
char *tracklen_cryptAscii(char *s);

#endif

/*--------------------------------------------------------------------------
 Encrypt or decrypt the given binary buffer.  Used to protect the
 track length file.
--------------------------------------------------------------------------*/
void tracklen_cryptBinary(char *data, size_t len);

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
int tracklen_GetTrackLengths(DWORD *trackLenBuf, char whichDrive);

/*--------------------------------------------------------------------------
 Return TRUE if a CD in some drive is longer than 74 minutes, can be
 seeked to near the end of the disc, and matches the encrypted track length 
 stored in the file named tracklen_DATFILE.
--------------------------------------------------------------------------*/
__declspec(dllexport) BOOL __cdecl tracklen_CheckTrackLengths(const char *fileName);

/*--------------------------------------------------------------------------
 Save the current autoplay setting, then disable autoplay.
--------------------------------------------------------------------------*/
void tracklen_AutoPlay_Disable();

/*--------------------------------------------------------------------------
 Restore the autoplay setting saved by AutoPlay_Disable().
--------------------------------------------------------------------------*/
void tracklen_AutoPlay_Restore();

#ifdef __cplusplus
}
#endif

#endif //tracklen_h
