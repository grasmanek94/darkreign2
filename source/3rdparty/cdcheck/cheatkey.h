#ifndef cheatkey_h
#define cheatkey_h


#ifdef __cplusplus
extern "C" {
#endif

// size of the cheatkey in DWORDs.
#define CHEATKEY_LENGTH 100


BYTE *WINAPI cheatkey_get();


#ifdef __cplusplus
}
#endif


#endif // cheatkey_h