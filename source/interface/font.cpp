///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Font system
//
// 29-JAN-1998
//


#include "font.h"
#include "fontsys.h"
#include "filesys.h"
#include "bitmap.h"
#include "vid_public.h"
#include "iface_util.h"


//#define LOG_FONT(x) LOG_DIAG(x)
#define LOG_FONT(x)

// render with trifans instead of bucketing trilists
#define DOTRIFAN

// Gutter around characters
static const S32 Gutter = 1;


///////////////////////////////////////////////////////////////////////////////
//
// Class Font
//


//
// Font::Font
//
Font::Font() 
: version(0),
  fontHeight(0),
  avgWidth(0)
{
  memset(hashTable, 0, sizeof(hashTable));
}


//
// Font::~Font
//
Font::~Font()
{
  Release();
}


//
// Font::Release
//
void Font::Release()
{
  for (U32 i = 0; i < HASH_COUNT; i++)
  {
    if (hashTable[i].data)
    {
      delete[] hashTable[i].data;
      hashTable[i].data = NULL;
    }

    hashTable[i].count = 0;
  }
}


//
// Font::FindChar
//
// Find character data for specified character
//
Font::CharData *Font::FindChar(int ch)
{
  U32 key = U32(ch) & HASH_MASK;

  if (hashTable[key].count)
  {
    ASSERT(hashTable[key].data)

    // crappy, replace with binary search
    for (U16 i = 0; i < hashTable[key].count; i++)
    {
      if (hashTable[key].data[i].charValue == ch)
      {
        return (&(hashTable[key].data[i]));
      }
    }
  }
  return (NULL);
}


//
// Return width of a string
//
S32 Font::Width(const CH *s, S32 len)
{
	// start with zero width
	S32 width = 0;
	
	// for each character...
	for (S32 c = 0; c < len; c++)
	{
    if (CharData *cd = FindChar(s[c]))
    {
  		// add character width to width
      width += cd->width;
    }
	}

	// return the width
	return (width);
}


//
// Return width of a character
//
S32 Font::Width(S32 c)
{
  if (CharData *cd = FindChar(c))
  {
    return (cd->width);
  }
  return (0);
}


//
// Font::Read
//
// Reload a font
//
Bool Font::Read()
{
  Release();
  return Read(path.str);
}


//
// Font::Read
//
// Read a font
//
Bool Font::Read(const char *fileName)
{
  Bool rc = FALSE;
  S32 i, x, y;

  // save the file path for mode change reloads
  path = fileName;

  // Clear hash table
  memset(hashTable, 0, sizeof(hashTable));

	// Open the file
	FileSys::DataFile *fp = FileSys::Open(fileName);
	
	// Did we find the file
	if (fp)
	{
    U8 *filePtr = (U8 *)fp->GetMemoryPtr();

    // Determine version
    U32 fileId = *(U32 *)filePtr; filePtr += 4;

    if (fileId == FILEID)
    {
      version = 1;
    }
    else

    if (fileId == FILEID2)
    {
      version = 2;
    }
    else
    {
			ERR_FATAL(("Font file is corrupt [%s]", fileName));
		}

    LOG_FONT(("Reading Font %s:", fileName))
    LOG_FONT((" - Version: %d", version))

    // Version specific header data
    U32 numChar = 0;

    switch (version)
    {
      case 1:
        // In version 1 numchars is 8 bit
        numChar = *(U8 *)filePtr; filePtr += 1;
        break;

      case 2:
        // In version 1 numchars is 16 bit
        numChar = *(U16 *)filePtr; filePtr += 2;
        break;
    }

    // Common to all versions
		fontHeight = *(U8 *)filePtr; filePtr += 1;

    // Next two members are no longer needed
    filePtr += 2;

    LOG_FONT((" - Chars: %d", numChar))

    // Reset Hash table counters
    U16 tableCount[HASH_COUNT];
    U16 tableBuilt[HASH_COUNT];

    memset(tableCount, 0, sizeof tableCount);
    memset(tableBuilt, 0, sizeof tableBuilt);

		// Read character headers
		CharHeader *charHeaderList = new CharHeader[numChar];

    for (i = 0; i < S32(numChar); i++)
    {
      switch (version)
      {
        case 1:
          // In version 1 charValue is 8 bits
          charHeaderList[i].charValue = *(U8 *)filePtr; filePtr += 1;
          break;

        case 2:
          // In version 2 charValue is 8 bits
          charHeaderList[i].charValue = *(U16 *)filePtr; filePtr += 2;
          break;
      }

      // Common to all versions
      charHeaderList[i].fullWidth = *(U8 *)filePtr; filePtr += 1;
      charHeaderList[i].rectX0    = *(U8 *)filePtr; filePtr += 1;
      charHeaderList[i].rectY0    = *(U8 *)filePtr; filePtr += 1;
      charHeaderList[i].rectX1    = *(U8 *)filePtr; filePtr += 1;
      charHeaderList[i].rectY1    = *(U8 *)filePtr; filePtr += 1;
    }

		// Setup pointers to character data in file
		CharImage **charImageList = new CharImage *[numChar];

		for (i = 0; i < (S32)numChar; i++)
		{
      // Setup a pointer to the character data
      charImageList[i] = (CharImage *)filePtr;

      // Sanity check
      if ((charImageList[i]->charWidth > 100) || (charImageList[i]->charHeight > 100))
      {
        ERR_FATAL(("Font file is corrupt [%s]", fileName))
      }

      // Accumulate hash table counters
      U32 key = U32(charHeaderList[i].charValue) & HASH_MASK;

      tableCount[key]++;

      // Advance by header size + data size
      filePtr += sizeof(CharImage) + (charImageList[i]->charWidth * charImageList[i]->charHeight);
		}

    // Allocate hash table
    for (i = 0; i < HASH_COUNT; i++)
    {
      if (tableCount[i])
      {
        hashTable[i].count = tableCount[i];
        hashTable[i].data = new CharData[tableCount[i]];
      }
    }

    // At this point charImageList could be sorted by height to optimize texture construction
    
    // ...

    // Create a bunch of 128x128 textures
    const S32 size = 128;

    // Texture handle of current character
    Bitmap *texture = NULL;
    U16 texHandle = U16_MAX;

    // Texture construction counters (intialized to 0 stop compiler complaining)
    int numTex = 0;
		S32 curX = 0;
		S32 curY = 0;
		S32 rowY = 0;

		// For each character in the font...
    i = 0;

    while (i < S32(numChar))
		{
      // Allocate new texture if necessary
      if (texture == NULL)
      {
        numTex++;
        texture = FontSys::AllocTexture(texHandle);

        LOG_FONT((" - Allocating texture %d (%dx%d)", numTex, size, size))

        if (!texture)
        {
          ERR_FATAL(("Out of font handles!"))
        }

        // Clear the contents
		    texture->Create(size, size, TRUE);
        texture->Clear(0);

		    // Lock the texture
		    texture->Lock();

        // Reset counters
		    curX = Gutter;
		    curY = Gutter;
		    rowY = 0;
      }

			// Get file character data structures
			CharHeader &charHeader = charHeaderList[i];
			CharImage *charImage = charImageList[i];

			// Get font character data structure
      U32 hashKey = U32(charHeader.charValue) & HASH_MASK;
      CharData &charData = hashTable[hashKey].data[tableBuilt[hashKey]];

			// If the image has nonzero width...
			if (charImage->charWidth > 0)
			{
			  // If the character won't fit horizontally...
        S32 cw = charImage->charWidth + Gutter;
        S32 ch = charImage->charHeight + Gutter;

				if (curX + cw > size - Gutter)
				{
					// Do line wrap
					curX = Gutter;
					curY += rowY;
					rowY = 0;
				}

				// Update row height
				if (rowY < ch)
				{
					rowY = ch;
				}

				if (curY + rowY > size - Gutter)
				{
					// Move to next texture
          texture->UnLock();
          texture->LoadVideo();
          texture = NULL;
					continue;
				}

				// Get character pixels
				U8 *pixel = charImage->charData;

				// For each row of the character...
				for (y = curY; y < curY + charImage->charHeight; y++)
				{
					// For each column of the row...
					for (x = curX; x < curX + charImage->charWidth; x++)
					{
						// Convert color to texture format
						U32 color = texture->MakeRGBA(0xFF, 0xFF, 0xFF, *pixel++);

            ASSERT(x < texture->Width())
            ASSERT(y < texture->Height())

						// Write pixel into the texture
						texture->PutPixel(x, y, color, &texture->GetClipRect());
					}
				}

        // Store texture handle
        charData.texHandle = texHandle;

				// Fill in font character data
				float scale = 1.0f / size;
				charData.width = charHeader.fullWidth;
				charData.rect.p0.x = charHeader.rectX0;
				charData.rect.p0.y = charHeader.rectY0;
				charData.rect.p1.x = charHeader.rectX1;
				charData.rect.p1.y = charHeader.rectY1;

				charData.u0 = F32(curX) * scale + texture->UVShiftWidth();
				charData.v0 = F32(curY) * scale + texture->UVShiftHeight();
				charData.u1 = F32(curX + charImage->charWidth)  * scale + texture->UVShiftWidth();
				charData.v1 = F32(curY + charImage->charHeight) * scale + texture->UVShiftHeight();

			  // Go to the next position
			  curX += cw;
			}
			else
			{
				// Fill in font character data
				charData.width = charHeader.fullWidth;
				charData.rect.p0.x = 0;
				charData.rect.p0.y = 0;
				charData.rect.p1.x = 0;
				charData.rect.p1.y = 0;
				charData.u0 = 0.0f;
				charData.v0 = 0.0f;
				charData.u1 = 0.0f;
				charData.v1 = 0.0f;
        charData.texHandle = 0;
			}

      // Common setup
      charData.charValue = charHeader.charValue;

      // Update built items in the hash table
      tableBuilt[hashKey]++;
      ASSERT(tableBuilt[hashKey] <= tableCount[hashKey])

      i++;
		}

#ifdef DEVELOPMENT

    // Verify hash table
    for (i = 0; i < HASH_COUNT; i++)
    {
      ASSERT(tableBuilt[i] == tableCount[i])
    }
    
#endif

		// Unlock the texture
    if (texture)
    {
		  texture->UnLock();
      texture->LoadVideo();
    }

    // Setup average character width
    avgWidth = Width(L'A');

		// Delete the character image list
		delete[] charImageList;

		// Delete the character header list
		delete[] charHeaderList;
		
		// Close the font file
		FileSys::Close(fp);
		
		rc = TRUE;
	}
	
	return rc;
}


//
// Font::Draw
//
// Render the string
//
void Font::Draw(S32 x, S32 y, const CH *s, U32 len, Color color, const ClipRect *clip, F32 alphaScale, S32 shadow)
{
  F32 fshadow = 0.0F;
  Color shadowClr;
  VertexTL point[4];
  
  if (shadow)
  {
    shadowClr.Set(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA));
    fshadow = F32(shadow);
  }

  // Scale the alpha down
  if (alphaScale < 1.0F)
  {
    color.a = U8(Utils::FtoL(F32(color.a) * alphaScale));
  }

  // Clip to screen dimensions if no clip rectangle specified
  if (clip == NULL)
  {
    clip = &Vid::backBmp.GetClipRect();
  }

	// for each character of the string...
	for (U32 c = 0; c < len; c++)
	{
		// get character data
    if (CharData *cd = FindChar(s[c]))
    {
		  // if the character image has nonzero width...
		  if (cd->rect.Width())
		  {
        Bitmap *texture = FontSys::FindTexture(cd->texHandle);

			  // calculate extents of the character
			  S32 x0 = x + cd->rect.p0.x;
			  S32 y0 = y + cd->rect.p0.y;
			  S32 x1 = x + cd->rect.p1.x;
			  S32 y1 = y + cd->rect.p1.y;

        // Clip
        if (x1 > clip->p1.x)
        {
          return;
        }
        else 
        
        if (x0 >= clip->p0.x)
        {
          // No filtering, regular clamp mode
          U16 vertOffset;
          VertexTL *pointBuf = IFace::GetVerts(4, texture, 0, 0, vertOffset);

          // Setup indices
          IFace::SetIndex(Vid::rectIndices, 6, vertOffset);

			    // top left corner
	        point[0].vv.z = 0.0f;
	        point[0].rhw = 1.0f;
	        point[0].diffuse = color;
	        point[0].specular = 0xFF000000;
			    point[0].vv.x = (F32)x0;
			    point[0].vv.y = (F32)y0;
			    point[0].u = cd->u0;
			    point[0].v = cd->v0;

			    // top right corner
	        point[1].vv.z = 0.0f;
	        point[1].rhw = 1.0f;
	        point[1].diffuse = color;
	        point[1].specular = 0xFF000000;
			    point[1].vv.x = (F32)x1;
			    point[1].vv.y = (F32)y0;
			    point[1].u = cd->u1;
			    point[1].v = cd->v0;

			    // bottom right corner
	        point[2].vv.z = 0.0f;
	        point[2].rhw = 1.0f;
	        point[2].diffuse = color;
	        point[2].specular = 0xFF000000;
			    point[2].vv.x = (F32)x1;
			    point[2].vv.y = (F32)y1;
			    point[2].u = cd->u1;
			    point[2].v = cd->v1;

			    // bottom left corner
	        point[3].vv.z = 0.0f;
	        point[3].rhw = 1.0f;
	        point[3].diffuse = color;
	        point[3].specular = 0xFF000000;
			    point[3].vv.x = (F32)x0;
			    point[3].vv.y = (F32)y1;
			    point[3].u = cd->u0;
			    point[3].v = cd->v1;

          // Copy vertices into buffer
          memcpy(pointBuf, point, 4 * sizeof(VertexTL));

          // Optional shadow
          if (shadow)
          {
            pointBuf = IFace::GetVerts(4, texture, 0, 0, vertOffset);

            // Setup indices 
            IFace::SetIndex(Vid::rectIndices, 6, vertOffset);

            // Modify color and positions for shadow
            for (U32 i = 0; i < 4; i++)
            {
              point[i].vv.x += fshadow;
              point[i].vv.y += fshadow;
              point[i].diffuse = shadowClr;
            }

            // Copy vertices into buffer
            memcpy(pointBuf, point, 4 * sizeof(VertexTL));
          }
        }
		  }

		  // update position
		  x += cd->width;
    }
	}
}
