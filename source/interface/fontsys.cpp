///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Font system
//
// 15-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fontsys.h"
#include "filesys.h"
#include "console.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace FontSys
//
namespace FontSys
{
  struct FontFile
  {
    // Filename of .fff file
    GameIdent name;

    // Font data
    Font font;

    NBinTree<FontFile>::Node node;
  };

  // Font item
  struct FontItem
  {
    // Name of font alias
    GameIdent name;

    // Pointer to font data
    Font *font;

    NBinTree<FontItem>::Node node;
  };
 
  // System init flag
  static Bool sysInit = FALSE;

  // List of registered font files
  static NBinTree<FontFile> fontFiles(&FontFile::node);

  // List of registered font symbols
  static NBinTree<FontItem> fontItems(&FontItem::node);

  // Max number of textures
  static const MAX_TEX = 1000;

  // Array of textures
  static Bitmap *textures[MAX_TEX];

  // Current texture handle
  static U16 currTex;


  //
  // Allocate a texture
  //
  Bitmap *AllocTexture(U16 &handle)
  {
    if (currTex == MAX_TEX)
    {
      return (NULL);
    }

    // Store handle
    handle = currTex;

    // Advance texture handle
    currTex++;

    // Allocate unmanaged bitmap
    textures[handle] = new Bitmap(bitmapTEXTURE | bitmapNOMANAGE);

    return (textures[handle]);
  }

  //
  // Find a texture
  //
  Bitmap *FindTexture(U16 handle)
  {
    ASSERT(handle < MAX_TEX)
    return (textures[handle]);
  }


  //
  // Release all textures
  //
  static void ReleaseAllTextures()
  {
    for (U16 i = 0; i < MAX_TEX; i++)
    {
      if (textures[i])
      {
        delete textures[i];
        textures[i] = NULL;
      }        
    }

    currTex = 0;
  }



  //
  // Initialise the font system
  //
  void Init()
  {
    ASSERT(!sysInit)

    // Reset texture table
    memset(textures, 0, sizeof(textures));
    currTex = 0;

    sysInit = TRUE;
  }


  //
  // Shut down the font system
  //
  void Done()
  {
    ASSERT(sysInit);
    ASSERT(!fontItems.GetCount())
    ASSERT(!fontFiles.GetCount())

    // Done
    sysInit = FALSE;
  }


  //
  // Create and load a font
  //
  Bool Create(const char *name, const char *path, U32)
  {
    ASSERT(sysInit);

    // Search the tree for this font
    if (fontItems.Find(Crc::CalcStr(name)))
    {
      LOG_ERR(("Font [%s] already exists", name))
      return (TRUE);
    }

    // Is the file already loaded?
    FontFile *fontFile;

    if ((fontFile = fontFiles.Find(Crc::CalcStr(path))) == NULL)
    {
      fontFile = new FontFile;
      fontFile->name = path;
      
      if (!fontFile->font.Read(path))
      {
        ERR_FATAL(("Error reading font [%s]", path))
      }

      fontFiles.Add(fontFile->name.crc, fontFile);
    }

    // Create new alias
    FontItem *fontItem = new FontItem;
    fontItem->name = name;
    fontItem->font = &(fontFile->font);

    // Add it to the tree
    fontItems.Add(fontItem->name.crc, fontItem);

    return TRUE;
  }


  //
  // Create and load a font from an fscope
  //
  void Create(FScope *fScope)
  {
    // Create a font with name specified in argument list
    const char *fontName = fScope->NextArgString();

    // Compulsory Functions inside the scope
    FScope *sScope = fScope->GetFunction("File");
    const char *file = sScope->NextArgString();

    if (!Create(fontName, file))
    {
      ERR_FATAL(("Could not create font [%s]", fontName))
    }
  }


  //
  // DeleteAll
  //
  // Delete all fonts
  //
  void DeleteAll()
  {
    // Release all textures
    ReleaseAllTextures();

    // Dispose all thingies
    fontFiles.DisposeAll();
    fontItems.DisposeAll();
  }


  //
  // The font name must be passed in as uppercase
  //
  Font *GetFont(const char *name)
  {
    ASSERT(sysInit);

    if (Font *font = GetFont(Crc::CalcStr(name)))
    {
      return (font);
    }
    ERR_FATAL(("Font [%s] not found", name));
  }


  //
  // Find the font
  //
  Font *GetFont(U32 crc)
  {
    FontItem *fi = fontItems.Find(crc);

    if (fi)
    {
      return (fi->font);
    }
    else
    {
      return (NULL);
    }
  }

  
  //
  // callback for video mode changes
  //
  void OnModeChange()
  {
    // free and reload font textures
    ReleaseAllTextures();

    for (NBinTree<FontFile>::Iterator i(&fontFiles); *i; i++)
    {
      //(*i)->font.Release();
      (*i)->font.Read();
    }
  }


  //
  // List all fonts to the console
  //
  void Report()
  {
    /*
    for (NBinTree<FontItem>::Iterator i(&fontItems); *i; i++)
    {
      FontItem *fi = *i;
      CON_MSG((
        "%-20s: %3dx%3d : %s", 
        fi->name.str, 
        fi->font->Texture()->Width(), 
        fi->font->Texture()->Height(), 
        fi->font->Texture()->GetIdent().str))
    }
    */
  }


  //
  // Write fonts to disk
  //
  void WriteFonts()
  {
    /*
    for (NBinTree<FontFile>::Iterator i(&fontFiles); *i; i++)
    {
      FontFile *f = *i;
      FilePath path;
      Dir::PathMake(path, "", "", (*i)->name.str, "tga");
      const_cast<Bitmap *>(f->font.Texture())->WriteTGA(path.str);
    }
    */
  }
}
