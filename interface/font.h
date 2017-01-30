///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Font system
//
// 29-JAN-1998
//


#ifndef __FONT_H
#define __FONT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "bitmapdec.h"
#include "bitmapclip.h"


///////////////////////////////////////////////////////////////////////////////
//
// class Font
//
class Font
{
public:

  // Version 1 file header
	enum { FILEID  = 'TNOF' };

  // Version 2 file header
  enum { FILEID2 = 'KROB' };

  // Size of hash table
  enum { HASH_COUNT = 256 };

  // Hash table mask
  enum { HASH_MASK = 255 };
	
	//
	// File structures
	//
#pragma pack(push, 1)
	
	// File font header
	struct FontHeader
	{
    // unique font identifier
		U32 fontIdent;	

    // number of characters in the font
		U16 numChar;

    // maximum height of each character
		U8 fontHeight;

    // character ascent above the baseline
		U8 fontAscent;

    // character descent below the baseline
		U8 fontDescent;
	};

	// File character header
	struct CharHeader
	{
    // character value
		U16 charValue;

    // full character width
		U8 fullWidth;

    // left x position in image
		U8 rectX0;

    // top y position in image
		U8 rectY0;

    // right x position in image
		U8 rectX1;

    // bottom y position in image
		U8 rectY1;
	};

  #pragma warning(disable : 4200)

	// File character image
	struct CharImage
	{
    // width of the character image
		U8 charWidth;

    // height of the character image
		U8 charHeight;

    // character pixels
		U8 charData[0];
	};

  #pragma warning(default : 4200)
	
#pragma pack(pop)
	
protected:

	struct CharData
	{
    // Character value
    U16 charValue;

    // Width in pixels
    U16 width;

    // Texture handle
    U16 texHandle;

    // Dimensions
		ClipRect rect;
		F32 u0;
		F32 v0;
		F32 u1;
		F32 v1;
	};

  // Hash table item
  struct HashItem
  {
    // Number of items
    U16 count;

    // Array of items
    CharData *data;
  };

protected:

  // Filename
  PathString path;

  // File version
  U32 version;

  // Font height
  U32 fontHeight;

  // Average character width
  U32 avgWidth;

  // Hash table 
  HashItem hashTable[HASH_COUNT];

  // Find character data for specified character
  CharData *FindChar(int ch);

  // Release memory
  void Release();

public:

  // Constructor
	Font();
	~Font();

  // Read from disk
	Bool Read(const char *fileName);
	Bool Read();

  // Width 
	S32 Width(const CH *s, S32 len);
	S32 Width(S32 c);

  // Average width
	S32 AvgWidth() 
  { 
    return (avgWidth); 
  }

  // Font height
	S32 Height() 
  { 
    return (fontHeight); 
  }

  // Draw the font
	void Draw(S32 x, S32 y, const CH *s, U32 len, Color color, const ClipRect *clip = NULL, F32 alphaScale = 1.0F, S32 shadow = 0);
};

#endif