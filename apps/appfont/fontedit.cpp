///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1997-98 Activision Studios
//
// Font editor utility
//
// 29-JAN-1998
//


#include "defines.h"

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <limits.h>
#include <stdio.h>
#include "resource.h"



// Font file identifier
static const DWORD FONT_ID = 'KROB';

// Application class name
const char *appClass = "FontSelClass";

// Default window text
const char *DefaultCaption = "Font Selector";

// Progress caption
const char *ProgressCaption = "Capturing %d characters [%d%%]";

// Application file filter
const char *appSaveFilter = "Bitmap Font Files (.FFF)\0*.FFF\0\0";

#pragma pack(push, 1)

// FILE FONT HEADER
struct FontHeader
{
	DWORD fontIdent;	// unique font identifier
	CH numChar;		// number of characters in the font
	BYTE fontHeight;	// maximum height of each character
	BYTE fontAscent;	// character ascent above the baseline
	BYTE fontDescent;	// character descent below the baseline
};

// FILE CHARACTER HEADER
struct CharHeader
{
	U16 charValue;		// character ascii value
	BYTE fullWidth;		// full character width
	BYTE rectX0;		// left x position in image
	BYTE rectY0;		// top y position in image
	BYTE rectX1;		// right x position in image
	BYTE rectY1;		// bottom y position in image
};

// FILE CHARACTER IMAGE
struct CharImage
{
	BYTE charWidth;		// width of the character image
	BYTE charHeight;	// height of the character image
	BYTE charData[0];	// character pixels
};

#pragma pack(pop)

// APPLICATION DATA
HWND		appWnd;
HINSTANCE	appInst;
HFONT		appFont;
LOGFONT		appLogFont;
HBRUSH		appBrush[256];
HBRUSH      appBrushEmpty;
HMENU       appMenu;

int         appZoom = 10;
int         appChar = 'A';

// current display mode
enum EDispMode
{
	DISP_CAPTURE, 
	DISP_CHAR,
}
dispMode;

// Font scale for antialiasing
int appFontScale = 0;
int appFontScale2 = appFontScale * appFontScale;

// file font header
FontHeader fontHeader;

// file character headers
CharHeader *charHeaderList;

// character images
CharImage **charImageList;

// DLL handles
HINSTANCE hGdiDll = NULL;

// Function pointers
typedef DWORD (WINAPI GetFontUnicodeRangesProc)(HDC, LPGLYPHSET);
GetFontUnicodeRangesProc *fnGetFontUnicodeRanges = NULL;

//DWORD GetFontUnicodeRanges(
//  HDC hdc,         // handle to DC
//  LPGLYPHSET lpgs  // glyph set
//);



// Is windows version unicode capable 
Bool unicodeCapable = FALSE;


//
// Get unicode mode setting
//
static Bool UnicodeMode()
{
  return (GetMenuState(appMenu, IDM_FONT_UNICODE, MF_BYCOMMAND) & MF_CHECKED);
}


//
// Set unicode mode
//
static void SetUnicodeMode(Bool mode)
{
  if (mode)
  {
    if (unicodeCapable)
    {
      CheckMenuItem(appMenu, IDM_FONT_UNICODE, MF_CHECKED);
    }
    else
    {
      MessageBox(appWnd, "Operating system is not able to capture unicode fonts", "Uh-oh", MB_OK);
    }
  }
  else
  {
    CheckMenuItem(appMenu, IDM_FONT_UNICODE, MF_UNCHECKED);
  }
}


//
// Create font data
//
static void CreateFontData(int count)
{
	// Create font header
	fontHeader.fontIdent = FONT_ID;
	fontHeader.numChar = count;

	// Create character header list
	charHeaderList = new CharHeader[count];

	// Create character image list
	charImageList = new CharImage *[count];

	// Clear character images
	memset(charImageList, 0, count * sizeof(CharImage *));
}


//
// Delete font data
//
static void DeleteFontData(void)
{
	int i;

	// Delete character images
	for (i = 0; i < fontHeader.numChar; i++)
	{
		delete charImageList[i];
	}

	// Delete character image list
	delete charImageList;
	charImageList = NULL;

	// Delete character header list
	delete charHeaderList;
	charHeaderList = NULL;

	// Delete font header
	memset(&fontHeader, 0, sizeof(fontHeader));
}


//
//
//
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//
//
//
BOOL OnAntiAliasFont(int id)
{
	static int lastid = 0;

	if (lastid)
	{
		CheckMenuItem(appMenu, lastid, MF_UNCHECKED);
	}

	lastid = id;

	if (lastid)
	{
		CheckMenuItem(appMenu, lastid, MF_CHECKED);
	}

	// set font scale factor
	appFontScale = lastid - IDM_FONT_ANTIALIAS_1 + 1;
	appFontScale2 = appFontScale * appFontScale;

	return TRUE;
}


//
// Capture one character into given array index
//
void CaptureOneChar(HDC hDC, int i, int charValue, int yShift)
{
	int x, y, xx, yy;
	SIZE charSize;

	// Display the character
	InvalidateRect(appWnd, NULL, TRUE);
	UpdateWindow(appWnd);

  if (UnicodeMode())
  {
    CH str[2] = { CH(charValue), 0 };

    GetTextExtentPoint32W(hDC, str, 1, &charSize);
		TextOutW(hDC, 0, 0, str, 1);
  }
  else
  {
    char str[2] = { char(charValue), 0};

    GetTextExtentPoint32(hDC, str, 1, &charSize);
		TextOut(hDC, 0, 0, str, 1);
  }
	GdiFlush();

	// Clear character rectangle
	RECT charRect;
	charRect.left = LONG_MAX;
	charRect.top = LONG_MAX;
	charRect.right = LONG_MIN;
	charRect.bottom = LONG_MIN;
	bool setRect = false;

	// Determine clipping rectangle
	for (y = 0; y < charSize.cy; y++)
	{
		for (x = 0; x < charSize.cx; x++)
		{
			// Capture the pixel
			COLORREF clr = GetPixel(hDC, x, y + yShift);

			// If the pixel is not white...
			if (GetRValue(clr) < 255)
			{
				// Update the character rectangle
				if (charRect.left > x)
					charRect.left = x;
				if (charRect.top > y)
					charRect.top = y;
				if (charRect.right < x)
					charRect.right = x;
				if (charRect.bottom < y)
					charRect.bottom = y;
				setRect = true;
			}
		}
	}

	if (setRect)
	{
		// Calculate x shift
		int xShift = charRect.left % appFontScale;

		// Rescale the size
		charSize.cx = (charSize.cx + appFontScale - 1) / appFontScale;
		charSize.cy = (charSize.cy + appFontScale - 1) / appFontScale;

		// Rescale the rectangle
		charRect.left = (charRect.left - xShift) / appFontScale;
		charRect.top = charRect.top / appFontScale;
		charRect.right = (charRect.right - xShift) / appFontScale + 1;
		charRect.bottom = (charRect.bottom) / appFontScale + 1;

		// Get width and height
		int w = charRect.right - charRect.left;
		int h = charRect.bottom - charRect.top;

		// Create a new character image
		CharImage *charImage = (CharImage *)
			new BYTE [sizeof(CharImage) + w * h];

		// Push the character image into the character list
		charImageList[i] = charImage;

		// Store the character dimensions
		charImage->charWidth = (BYTE)w;
		charImage->charHeight = (BYTE)h;

		// Start in the upper left corner
		BYTE *pixel = charImage->charData;

		// Capture the character
		for (y = charRect.top; y < charRect.bottom; y++)
		{
			for (x = charRect.left; x < charRect.right; x++)
			{
				// Clear the pixel sum
				DWORD sum = 0;

				// Average value over block
				for (yy = (y) * appFontScale; yy < (y + 1) * appFontScale; yy++)
				{
					for (xx = (x) * appFontScale; xx < (x + 1) * appFontScale; xx++)
					{
						// Capture the pixel
						COLORREF clr = GetPixel(hDC, xx + xShift, yy + yShift);

						// Add pixel intensity to sum
						sum += GetRValue(clr);
					}
				}

				// Get the average pixel intensity
				sum = (sum + sum + appFontScale2) / (appFontScale2 + appFontScale2);

				// Store the pixel value
				*pixel++ = U8(0xFF - (BYTE)sum);
			}
		}

		// Get the current character header
		CharHeader *charHeader = &charHeaderList[i];

		// Fill in the character header
		charHeader->charValue = (CH)charValue;
		charHeader->fullWidth = (BYTE)charSize.cx;
		charHeader->rectX0 = (BYTE)charRect.left;
		charHeader->rectY0 = (BYTE)charRect.top;
		charHeader->rectX1 = (BYTE)charRect.right;
		charHeader->rectY1 = (BYTE)charRect.bottom;
	}
	else
	{
		// Rescale the size
		charSize.cx = (charSize.cx + appFontScale - 1) / appFontScale;
		charSize.cy = (charSize.cy + appFontScale - 1) / appFontScale;

		// Create a new character image
		CharImage *charImage = (CharImage *)
			new BYTE [sizeof(CharImage)];

		// Push the character image into the character list
		charImageList[i] = charImage;

		// Zero width and height
		charImage->charWidth = 0;
		charImage->charHeight = 0;

		// Get the current character header
		CharHeader *charHeader = &charHeaderList[i];

		// Fill in the character header
		charHeader->charValue = (CH)charValue;
		charHeader->fullWidth = (BYTE)charSize.cx;
		charHeader->rectX0 = 0;
		charHeader->rectY0 = 0;
		charHeader->rectX1 = 0;
		charHeader->rectY1 = 0;
	}
}



//
//
//
BOOL OnCaptureFont()
{
	int i;
	CHOOSEFONT cf;
	
	// Init CHOOSEFONT structure
	memset(&cf, 0, sizeof(cf));
	
	cf.lStructSize    = sizeof(cf);
	cf.lpLogFont      = &appLogFont;
	cf.hwndOwner      = appWnd;
	cf.Flags          = CF_SCREENFONTS;
	
	if (appFont)
  {
		cf.Flags |= CF_INITTOLOGFONTSTRUCT;
  }
	
	// Select a font
	if (ChooseFont(&cf))
	{
		// if a font already exists...
		if (appFont)
		{
			// delete the old font
			DeleteObject(appFont);
			appFont = NULL;
		}

		// remember the point size
		int size = appLogFont.lfHeight;

		// scale the point size
		appLogFont.lfHeight = size * appFontScale;

		// create the new font
		appFont = CreateFontIndirect(&appLogFont);

		// restore the point size
		appLogFont.lfHeight = size;
	}
	else
	{
		return FALSE;
	}

	// Delete any existing font data
	DeleteFontData();

	// Capture it now
	HDC hDC;
	
	if (!appFont)
  {
		return FALSE;
  }
	
	hDC = GetDC(appWnd);
	if (hDC)
	{
		EDispMode oldMode;
		
		/*
		{
		KERNINGPAIR kp[256];
		DWORD np;
		
		  np = GetKerningPairs(hDC, 256, kp);
		  for (i = 0; i < np; i++)
		  {
		  LOG_DIAG(("Kerning pair: %c -> %c = %d", (char)kp[i].wFirst, (char)kp[i].wSecond, kp[i].iKernAmount));
		  }
		  }
		*/
		
		oldMode = dispMode;
		dispMode = DISP_CAPTURE;
		
		// Set up DC
		SelectObject(hDC, appFont);
		SelectObject(hDC, GetStockObject(BLACK_BRUSH));
		
		// Calculate max sizes of any font character
		TEXTMETRICW tmw;
    TEXTMETRIC tma;
		RECT cli;
		RECT win;

    int tmMaxCharWidth;
    int tmHeight;
    int tmLastChar;
    int tmFirstChar;
    int tmAscent;
    int tmDescent;
    int tmCharSet;
    GLYPHSET *gs = NULL;

    if (UnicodeMode())
    {
      gs = (GLYPHSET *)malloc(sizeof(GLYPHSET) + sizeof(WCRANGE) * 5000);

      if (!fnGetFontUnicodeRanges || !fnGetFontUnicodeRanges(hDC, gs))
      {
        MessageBox(appWnd, "GetFontUnicodeRanges failed", "Error", MB_OK);
        return (FALSE);
      }

		  GetTextMetricsW(hDC, &tmw);

      tmMaxCharWidth = tmw.tmMaxCharWidth;
      tmHeight = tmw.tmHeight;
      tmLastChar = tmw.tmLastChar;
      tmFirstChar = tmw.tmFirstChar;
      tmAscent = tmw.tmAscent;
      tmDescent = tmw.tmDescent;
      tmCharSet = tmw.tmCharSet;

		  // Create new font data
		  CreateFontData(gs->cGlyphsSupported);
    }
    else
    {
      GetTextMetrics(hDC, &tma);

      tmMaxCharWidth = tma.tmMaxCharWidth;
      tmHeight = tma.tmHeight;
      tmLastChar = tma.tmLastChar;
      tmFirstChar = tma.tmFirstChar;
      tmAscent = tma.tmAscent;
      tmDescent = tma.tmDescent;
      tmCharSet = tma.tmCharSet;

		  // Create new font data
		  CreateFontData(tmLastChar - tmFirstChar + 1);
    }

		GetClientRect(appWnd, &cli);
		GetWindowRect(appWnd, &win);
		
		if (cli.right < tmMaxCharWidth || cli.bottom < tmHeight)
		{
			int newW;// = max(cli.right, tm.tmMaxCharWidth);
			int newH;// = max(cli.bottom, tm.tmHeight);
			
			newW = (win.right - win.left) - (cli.right - cli.left) + max(cli.right, tmMaxCharWidth);
			newH = (win.bottom - win.top) - (cli.bottom - cli.top) + max(cli.bottom, tmHeight);
			
			MoveWindow(appWnd, win.left, win.top, newW, newH, TRUE);
		}
		
		// Calculate y shift
		int yShift = -tmAscent % appFontScale;

    if (GetAsyncKeyState(VK_MENU) < 0)
    {
      char buf[1024];
      char *chSet = "Unknown";

      switch (tmCharSet)
      {
        case ANSI_CHARSET:          chSet = "ANSI"; break;
        case DEFAULT_CHARSET:       chSet = "DEFAULT"; break;
        case SYMBOL_CHARSET:        chSet = "SYMBOL"; break;
        case SHIFTJIS_CHARSET:      chSet = "SHIFTJIS"; break;
        case HANGUL_CHARSET:        chSet = "HANGUL"; break;
        case GB2312_CHARSET:        chSet = "GP2312"; break;
        case CHINESEBIG5_CHARSET:   chSet = "CHINESEBIG5"; break;
        case OEM_CHARSET:           chSet = "OEM"; break;
        case JOHAB_CHARSET:         chSet = "JOHAB"; break;
        case HEBREW_CHARSET:        chSet = "HEBREW"; break;
        case ARABIC_CHARSET:        chSet = "ARABIC"; break;
        case GREEK_CHARSET:         chSet = "GREEK"; break;
        case TURKISH_CHARSET:       chSet = "TURKISH"; break;
        case VIETNAMESE_CHARSET:    chSet = "VIETNAMESE"; break;
        case THAI_CHARSET:          chSet = "THAI"; break;
        case EASTEUROPE_CHARSET:    chSet = "EASTEUROPE"; break;
        case RUSSIAN_CHARSET:       chSet = "RUSSIAN"; break;
        case MAC_CHARSET:           chSet = "MAC"; break;
        case BALTIC_CHARSET:        chSet = "BALTIC"; break;
      }

      if (gs)
      {
        wsprintf(buf, "size=%d\ncharset=%s\nranges=%d\ntotal=%d", tmHeight, chSet, gs->cRanges, gs->cGlyphsSupported);
      }
      else
      {
        wsprintf(buf, "size=%d\nchars range=%d .. %d\ncharset=%s\ntotal=%d", tmHeight, tmFirstChar, tmLastChar, chSet, fontHeader.numChar);
      }

      MessageBox(appWnd, buf, "Info", MB_OK);
    }

		// Fill in the font header
		fontHeader.fontHeight   = (BYTE)((tmHeight + yShift) / appFontScale);
		fontHeader.fontAscent   = (BYTE)((tmAscent + yShift) / appFontScale);
		fontHeader.fontDescent  = (BYTE)((tmDescent) / appFontScale);

    S32 pct = -1;

    if (UnicodeMode())
    {
      int i = 0;

      for (int range = 0; range < gs->cRanges; range++)
      {
        for (int gch = 0; gch < gs->ranges[range].cGlyphs; gch++)
        {
          CaptureOneChar(hDC, i, gch + gs->ranges[range].wcLow, yShift);
          i++;
        }
      }

    }
    else
    {
		  for (i = 0; i < fontHeader.numChar; i++)
		  {
        CaptureOneChar(hDC, i, i + tmFirstChar, yShift);
      }
    }

    /*
		// Display and capture each letter
		for (i = 0; i < fontHeader.numChar; i++)
		{
			int x, y, xx, yy;
			SIZE charSize;

			// Generate character value
			int charValue = i + tmFirstChar;

			// Display the character
			InvalidateRect(appWnd, NULL, TRUE);
			UpdateWindow(appWnd);

      if (UnicodeMode())
      {
        CH str[2] = { CH(charValue), 0 };

        GetTextExtentPoint32W(hDC, str, 1, &charSize);
			  TextOutW(hDC, 0, 0, str, 1);
      }
      else
      {
        char str[2] = { char(charValue), 0};

        GetTextExtentPoint32(hDC, str, 1, &charSize);
			  TextOut(hDC, 0, 0, str, 1);
      }
			GdiFlush();

			// Clear character rectangle
			RECT charRect;
			charRect.left = LONG_MAX;
			charRect.top = LONG_MAX;
			charRect.right = LONG_MIN;
			charRect.bottom = LONG_MIN;
			bool setRect = false;

			// Determine clipping rectangle
			for (y = 0; y < charSize.cy; y++)
			{
				for (x = 0; x < charSize.cx; x++)
				{
					// Capture the pixel
					COLORREF clr = GetPixel(hDC, x, y + yShift);

					// If the pixel is not white...
					if (GetRValue(clr) < 255)
					{
						// Update the character rectangle
						if (charRect.left > x)
							charRect.left = x;
						if (charRect.top > y)
							charRect.top = y;
						if (charRect.right < x)
							charRect.right = x;
						if (charRect.bottom < y)
							charRect.bottom = y;
						setRect = true;
					}
				}
			}

			if (setRect)
			{
				// Calculate x shift
				int xShift = charRect.left % appFontScale;

				// Rescale the size
				charSize.cx = (charSize.cx + appFontScale - 1) / appFontScale;
				charSize.cy = (charSize.cy + appFontScale - 1) / appFontScale;

				// Rescale the rectangle
				charRect.left = (charRect.left - xShift) / appFontScale;
				charRect.top = charRect.top / appFontScale;
				charRect.right = (charRect.right - xShift) / appFontScale + 1;
				charRect.bottom = (charRect.bottom) / appFontScale + 1;

				// Get width and height
				int w = charRect.right - charRect.left;
				int h = charRect.bottom - charRect.top;

				// Create a new character image
				CharImage *charImage = (CharImage *)
					new BYTE [sizeof(CharImage) + w * h];

				// Push the character image into the character list
				charImageList[i] = charImage;

				// Store the character dimensions
				charImage->charWidth = (BYTE)w;
				charImage->charHeight = (BYTE)h;

				// Start in the upper left corner
				BYTE *pixel = charImage->charData;

				// Capture the character
				for (y = charRect.top; y < charRect.bottom; y++)
				{
					for (x = charRect.left; x < charRect.right; x++)
					{
						// Clear the pixel sum
						DWORD sum = 0;

						// Average value over block
						for (yy = (y) * appFontScale; yy < (y + 1) * appFontScale; yy++)
						{
							for (xx = (x) * appFontScale; xx < (x + 1) * appFontScale; xx++)
							{
								// Capture the pixel
								COLORREF clr = GetPixel(hDC, xx + xShift, yy + yShift);

								// Add pixel intensity to sum
								sum += GetRValue(clr);
							}
						}

						// Get the average pixel intensity
						sum = (sum + sum + appFontScale2) / (appFontScale2 + appFontScale2);

						// Store the pixel value
						*pixel++ = U8(0xFF - (BYTE)sum);
					}
				}

				// Get the current character header
				CharHeader *charHeader = &charHeaderList[i];

				// Fill in the character header
				charHeader->charValue = (CH)charValue;
				charHeader->fullWidth = (BYTE)charSize.cx;
				charHeader->rectX0 = (BYTE)charRect.left;
				charHeader->rectY0 = (BYTE)charRect.top;
				charHeader->rectX1 = (BYTE)charRect.right;
				charHeader->rectY1 = (BYTE)charRect.bottom;
			}
			else
			{
				// Rescale the size
				charSize.cx = (charSize.cx + appFontScale - 1) / appFontScale;
				charSize.cy = (charSize.cy + appFontScale - 1) / appFontScale;

				// Create a new character image
				CharImage *charImage = (CharImage *)
					new BYTE [sizeof(CharImage)];

				// Push the character image into the character list
				charImageList[i] = charImage;

				// Zero width and height
				charImage->charWidth = 0;
				charImage->charHeight = 0;

				// Get the current character header
				CharHeader *charHeader = &charHeaderList[i];

				// Fill in the character header
				charHeader->charValue = (CH)charValue;
				charHeader->fullWidth = (BYTE)charSize.cx;
				charHeader->rectX0 = 0;
				charHeader->rectY0 = 0;
				charHeader->rectX1 = 0;
				charHeader->rectY1 = 0;
			}

      // Update window text
      S32 newPct = S32(F32(i) * 100.0F / F32(fontHeader.numChar));

      if (pct != newPct)
      {
        char buf[256];
        pct = newPct;
        sprintf(buf, ProgressCaption, fontHeader.numChar, pct);
        SetWindowText(appWnd, buf);
      }
		}
    */

    // Restore window text
    SetWindowText(appWnd, DefaultCaption);
		
		dispMode = oldMode;
		InvalidateRect(appWnd, NULL, TRUE);
		
		ReleaseDC(appWnd, hDC);
	}

	return TRUE;
}


//
//
//
BOOL OnLoadFont()
{
	int i;
	char path[260] = {0};
	
	// Set up file name requester data
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize   = sizeof(ofn);
	ofn.hwndOwner     = appWnd;
	ofn.lpstrFilter   = appSaveFilter;
	ofn.lpstrFile     = path;
	ofn.nMaxFile      = sizeof(path);
	
	// Get open file name
	if (GetOpenFileName(&ofn))
	{
		// Open the font file for read
		FILE *file = fopen(path, "rb");

		// If the file couldn't be opened...
		if (!file)
		{
			// Exit
			return FALSE;
		}

		// Read the font header
		fread(&fontHeader, sizeof(fontHeader), 1, file);

		// If the font identifier doesn't match...
		if (fontHeader.fontIdent != FONT_ID)
		{
			// Bail out
			fclose(file);
			return FALSE;
		}

		// Create font data
		CreateFontData(fontHeader.numChar);

		// Read all the font character headers
		fread(charHeaderList, sizeof(CharHeader), fontHeader.numChar, file);

		// Read all character images
		for (i = 0; i < fontHeader.numChar; i++)
		{
			// Get character image size
			CharImage size;
			fread(&size, sizeof(size), 1, file);

			// Allocate character image
			CharImage *charImage = (CharImage *)
				new BYTE [sizeof(CharImage) + size.charWidth * size.charHeight];

			// Add character image to list
			charImageList[i] = charImage;

			// Copy character image size
			charImage->charWidth = size.charWidth;
			charImage->charHeight = size.charHeight;

			// Read character image data
			fread(charImage->charData, 1, charImage->charWidth * charImage->charHeight, file);
		}

		// Close the font file
		fclose(file);

		// File was read successfully
		return TRUE;
	}
	else
	{
		// File was not read successfully
		return FALSE;
	}
}


//
//
//
BOOL OnSaveFont()
{
	int i;
	char path[260] = {0};

	// Set up file name requester data
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize   = sizeof(ofn);
	ofn.hwndOwner     = appWnd;
	ofn.lpstrFilter   = appSaveFilter;
	ofn.lpstrFile     = path;
	ofn.nMaxFile      = sizeof(path);
	
	// Get save file name
	if (GetSaveFileName(&ofn))
	{
		// Open the font file for write
		FILE *file = fopen(path, "wb");

		// If the file couldn't be opened...
		if (!file)
		{
			// Exit
			return FALSE;
		}

		// Write the font header
		fwrite(&fontHeader, sizeof(fontHeader), 1, file);

		// Write all the font character headers
		fwrite(charHeaderList, sizeof(CharHeader), fontHeader.numChar, file);

		// Write all character images
		for (i = 0; i < fontHeader.numChar; i++)
		{
			// Get character image data
			CharImage *charImage = charImageList[i];

			// Write character image data
			fwrite(charImage, 1, sizeof(CharImage) + 
				charImage->charWidth * charImage->charHeight, file);
		}

		// Close the font file
		fclose(file);

		// File was written successfully
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//
//
//
void OnDrawChar(HDC hDC, PAINTSTRUCT * /*ps*/)
{
	int i, x, y;
	CharHeader *charHeader = NULL;
	CharImage *charImage = NULL;

	// Search for the matching character
	for (i = 0; i < fontHeader.numChar; i++)
	{
		if (charHeaderList[i].charValue == appChar)
		{
			charHeader = &charHeaderList[i];
			charImage = charImageList[i];
			break;
		}
	}

	if (charHeader && charImage)
	{
		// Fill background
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = (charHeader->fullWidth + 1) * appZoom;
		rc.bottom = (fontHeader.fontHeight + 1) * appZoom;
		FillRect(hDC, &rc, appBrushEmpty);

		// Get pixel data
		BYTE *pixel = charImage->charData;

		// Draw character data
		for (y = charHeader->rectY0; y < charHeader->rectY1; y++)
		{
			for (x = charHeader->rectX0; x < charHeader->rectX1; x++)
			{
				int x0 = appZoom * x;
				int y0 = appZoom * y;

				// Get grid rectangle
				RECT rc = { x0, y0, x0+appZoom, y0+appZoom };

				// Draw foreground color with desired intensity
				FillRect(hDC, &rc, appBrush[*pixel++]);
			}
		}
		
		if (appZoom > 1)
		{
			// Draw grid
			for (x = rc.left; x <= rc.right; x += appZoom)
			{
				MoveToEx(hDC, x, rc.top, NULL);
				LineTo(hDC, x, rc.bottom + 1);
			}
			for (y = rc.top; y <= rc.bottom; y += appZoom)
			{
				MoveToEx(hDC, rc.left, y, NULL);
				LineTo(hDC, rc.right + 1, y);
			}
		}
	}
}


//
//
//
BOOL AppInit()
{
	int i;

	// Initialise globals
	appFont = NULL;
	appWnd  = NULL;
	appMenu = NULL;

	// Create color ramp brushes
	for (i = 0; i < 256; i++)
	{
		appBrush[i] = CreateSolidBrush(RGB(255 - i, 255 - i, 255 - i));
	}

	// Create empty color brush
	appBrushEmpty = CreateSolidBrush(RGB(255, 0, 0));
	
	// Set display mode
	dispMode = DISP_CHAR;
	
	// Set initial zoom
	appZoom = 10;
	
	// Register Window Class
	WNDCLASSEX wc;

  memset(&wc, 0, sizeof(wc));

  wc.cbSize        = sizeof(wc);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = appInst;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = appClass;
	
	if (!RegisterClassEx(&wc))
	{
    U32 x = GetLastError();
		return FALSE;
	}
	
	// Create the window
	appWnd = CreateWindow
		(
		appClass, 
		DefaultCaption,
		WS_VISIBLE|WS_OVERLAPPEDWINDOW, 
		0, 0, 640, 480,
		NULL, 
		NULL, 
		appInst, 
		NULL
		);
	
	if (appWnd == NULL)
	{
		return FALSE;
	}
	
	// Get menu
	appMenu = GetMenu(appWnd);

	// set initial antialiasing
	OnAntiAliasFont(IDM_FONT_ANTIALIAS_1);

  // Resolve unicode function pointers
  unicodeCapable = TRUE;

  if ((hGdiDll = LoadLibrary("gdi32.dll")) != NULL)
  {
    if ((fnGetFontUnicodeRanges = (GetFontUnicodeRangesProc *)GetProcAddress(hGdiDll, "GetFontUnicodeRanges")) == NULL)
    {
      unicodeCapable = FALSE;
    }
  }

  // Default to non-unicode mode
  SetUnicodeMode(FALSE);

	return TRUE;
}

void AppDone()
{
	int i;

	// Delete color ramp brushes
	for (i = 0; i < 256; i++)
	{
		DeleteObject(appBrush[i]);
	}

	// Delete empty color brush
	DeleteObject(appBrushEmpty);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//
void OnDestroy(HWND /*hwnd*/)
{
	PostQuitMessage(0);
}

void OnClose(HWND hwnd)
{
	DestroyWindow(hwnd);
}

void OnCommand(HWND /*hwnd*/, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
	switch (id)
	{
    case IDM_FILE_OPEN:
		OnLoadFont();
		return;
    case IDM_FILE_SAVE:
		OnSaveFont();
		return;
    case IDM_FONT_CAPTURE:
		OnCaptureFont();
		return;
	case IDM_FONT_ANTIALIAS_1:
	case IDM_FONT_ANTIALIAS_2:
	case IDM_FONT_ANTIALIAS_3:
	case IDM_FONT_ANTIALIAS_4:
		OnAntiAliasFont(id);
		return;

  case IDM_FONT_UNICODE:
    SetUnicodeMode(!UnicodeMode());    
    return;
	}
}

void OnPaint(HWND hwnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	
	hDC = BeginPaint(hwnd, &ps);
	
	switch (dispMode)
	{
    case DISP_CHAR:
		OnDrawChar(hDC, &ps);
		break;
	}
	
	EndPaint(hwnd, &ps);
}

void OnKey(HWND /*hwnd*/, UINT vk, BOOL fDown, int /*cRepeat*/, UINT /*flags*/)
{
	if (fDown)
	{
		switch (vk)
		{
		case VK_LEFT:
			appChar = (BYTE)(appChar - 1);
			InvalidateRect(appWnd, NULL, TRUE);
			return;
		case VK_RIGHT:
			appChar = (BYTE)(appChar + 1);
			InvalidateRect(appWnd, NULL, TRUE);
			return;
		case VK_UP:
			if (appZoom < 20)
				appZoom++;
			InvalidateRect(appWnd, NULL, TRUE);
			return;
		case VK_DOWN:
			if (appZoom > 1)
				appZoom--;
			InvalidateRect(appWnd, NULL, TRUE);
			return;
		}
	}
}


//
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_KEYDOWN, OnKey);
	}
	
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


//
//
//
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE /*hPrevInst*/, LPSTR /*cmdLine*/, int /*cmdShow*/)
{
	appInst = hInst;
	
	if (!AppInit())
	{
		return 1;
	}
	
	// Message pump
	MSG msg;
	
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	AppDone();
	
	return msg.wParam;
}
