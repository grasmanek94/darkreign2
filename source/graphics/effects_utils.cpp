///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
// effects_utils.cpp
// 
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
//#include "vid_publicmore.h"
#include "vid_public.h"
#include "mesh.h"
#include "effects_utils.h"
#include "stdload.h"
//----------------------------------------------------------------------------

namespace Effects
{
  Data::Data()
  {
    flags = dataNONE;

    lifeTime = 1.0f;

    blend = RS_BLEND_DEF;

    color  = 0xffffffff;

    scale = 1.0f;

    animRate = 1.0f;
    texTime = 0;

    root = NULL;
    texture = NULL;
    textReduce = 2;   // max

    uvScale = 1.0f;
    uvAnimRate = 0.0f;

    noRestore = FALSE;
    noX = noY = noZ = FALSE;

    vector.Set( 0, 1, 0);
    offset.Set( 0, 0, 0);

    animFlags = flagDESTROY;
    sorting = Vid::sortSMOKE0 + 2;

    random = 0;

    noZBuffer = 0;
    interpolate = 0;
  }

  Bool Data::Configure(FScope *fScope, U32 counter) // = 1
  {
    counter;

    switch (fScope->NameCrc())
    {
    default:
      return FALSE;

    case 0x90B3C656: // "Interpolate"
      interpolate = StdLoad::TypeU32(fScope, interpolate);
      break;

    case 0x12CAD0FD: // "LifeTime"
      lifeTime = StdLoad::TypeF32( fScope, lifeTime);
      break;

    case 0x1655D08E: // "NoRestore"
      noRestore = StdLoad::TypeU32(fScope, noRestore);
      break;

    case 0x156D2BE8: // "NoX"
      noX = StdLoad::TypeU32(fScope, noX);
      break;
    case 0x11AC365F: // "NoY"
      noY = StdLoad::TypeU32(fScope, noY);
      break;
    case 0x1CEF1086: // "NoZ"
      noZ = StdLoad::TypeU32(fScope, noZ);
      break;

    case 0xDCC321D2: // "Sound"           
    case 0xC939D9B6: // "GeometryName"
    case 0x63E93484: // "GodFile"
      objectId = StdLoad::TypeStringD(fScope, objectId.str);
      flags |= dataOBJECT;
      break;

    case 0xB9702877: // "UVScale"
      uvScale = StdLoad::TypeF32( fScope, uvScale);
      break;
    case 0x9274F424: // "UVAnimRate"
      uvAnimRate = StdLoad::TypeF32( fScope, uvAnimRate);
      break;

    case 0xCB2A656E: // "TextureReduce"
    case 0x386C31B7: // "TextReduce"
      textReduce = Min<U32>( StdLoad::TypeU32(fScope, textReduce), 2);
//      LOG_DIAG(("TEXREDUCE: %d", textReduce));
      break;

    case 0x251D4D16: // "TextureName"
    case 0x7951FC0B: // "Texture"
      textureId = StdLoad::TypeStringD(fScope, textureId.str);
      flags |= dataTEXTURE;
      break;

    case 0x52F4890F: // "TextureBlend"
    case 0x6BE1324D: // "Blend"
    {
      StrBuf<128> str;

      str = StdLoad::TypeStringD(fScope, "");
      if (*str.str)
      {
        blend = Effects::Blend::GetValue(str.str, blend);
      }
      flags |= dataBLEND;
      break;
    }

    case 0xA7816640: // "NoZBuffer"
      noZBuffer = StdLoad::TypeU32(fScope, (U32)noZBuffer);
      break;

    case 0x86010476: // "Height"
      scale = StdLoad::TypeF32(fScope, scale);
      flags |= dataHEIGHT;
      break;

    case 0x362FA3AA: // "Size"
    case 0xE7070C09: // "Scale"
    case 0xBFA39551: // "Radius"
      scale = StdLoad::TypeF32(fScope, scale);
      flags |= dataSCALE;
      break;

    case 0xF6B933DD: // "AnimateTime"
    case 0xC6B5F4F8: // "AnimTime"
    case 0x409FED81: // "AnimRate"
      animRate = StdLoad::TypeF32(fScope, animRate);
      flags |= dataANIMRATE;
      break;

    case 0x7F4E4933: // "ComponentSize"
    case 0xC4AA3473: // "Vector"
    case 0x798A5CF0: // "Plane"
      vector.x = StdLoad::TypeF32(fScope, vector.x);
      vector.y = StdLoad::TypeF32(fScope, vector.y);
      vector.z = StdLoad::TypeF32(fScope, vector.z);
      flags |= dataVECTOR;
      break;

    case 0xFE3B31A3: // "Offset"
      offset.x = StdLoad::TypeF32(fScope, offset.x);
      offset.y = StdLoad::TypeF32(fScope, offset.y);
      offset.z = StdLoad::TypeF32(fScope, offset.z);
      break;

    case 0xFC38C807: // "Loop"
      animFlags |= (StdLoad::TypeU32(fScope, U32(0)) ? flagLOOP : 0);
      break;

    case 0x6F459C35: // "Sorting"
      if (fScope->IsNextArgString())
      {
        GameIdent sort = fScope->NextArgString();
        switch (sort.crc)
        {
          case 0x1C329AB7: // "Debug"
            sorting = Vid::sortDEBUG0;
            break;
          case 0x978BE78D: // "Flare"
            sorting = Vid::sortFLARE0;
            break;
          case 0x920892B8: // "Light"
            sorting = Vid::sortLIGHT0;
            break;
          case 0x6FF6291D: // "Fog"
            sorting = Vid::sortLIGHT0;
            break;
          case 0x3A5D41B6: // "Flame"
            sorting = Vid::sortFLAME0;
            break;
          case 0xE3DB018F: // "Smoke"
            sorting = Vid::sortSMOKE0;
            break;
          case 0xEDA4DC57: // "Effect"
            sorting = Vid::sortEFFECT0;
            break;
          case 0x5EB76DCD: // "Normal"
            sorting = Vid::sortNORMAL0;
            break;
          case 0x6728DE39: // "Surface"
            sorting = Vid::sortSURFACE0;
            break;
          case 0x6ADE7843: // "Water"
            sorting = Vid::sortWATER0;
            break;
          case 0x80528818: // "Brush"
            sorting = Vid::sortBRUSH0;
            break;
          case 0x11BBF9B2: // "Ground"
            sorting = Vid::sortGROUND0;
            break;
          case 0xB31C6654: // "Terrain"
            sorting = Vid::sortTERRAIN0;
            break;
        }
        U32 index = 0;
        // add in extra
        index = Min<S32>( 4095, StdLoad::TypeU32( fScope, index));
        sorting += index;
      }
      else
      {
        sorting = StdLoad::TypeU32(fScope, sorting);
      }
      break;
    }
    return TRUE;
  }

  void Data::PostLoad()
  {
    if (flags & dataOBJECT)
    {
      root = Mesh::Manager::FindRead( objectId.str, Vid::renderState.scaleFactor, Vid::renderState.status.mrmGen);        
    }
    if (flags & dataTEXTURE)
    {
//      texture = Bitmap::Manager::FindCreate( textureId.str, Mesh::Manager::mipCount);
      texture = Bitmap::Manager::FindCreate( textReduce, textureId.str);
//      LOG_DIAG(("Effect::Data: %s : %d", textureId.str, textReduce ));
    }
    if (noZBuffer)
    {
      blend |= RS_NOZBUFFER;
    }
  }

  Bool Data::Configure(FScope *fScope, KeyList<ColorKey> &keys, U32 counter) // = 1)
  {
    switch (fScope->NameCrc())
    {
    default:
      return FALSE;

    case 0xBA4990AD: // "ColorKey2"
      if (counter != 2)
      {
        return FALSE;
      }
      counter = 1;
      // fall through

    case 0xEB38EC6B: // "ColorKey"
      if (counter == 1)
      {
        F32 f = StdLoad::TypeF32( fScope);
        U32 r = StdLoad::TypeU32( fScope);
        U32 g = StdLoad::TypeU32( fScope);
        U32 b = StdLoad::TypeU32( fScope);
        U32 a = StdLoad::TypeU32( fScope, 255);

        keys.Append( new ColorKey( f, r, g, b, a));
      }
      break;

    case 0x9A8F5250: // "StartColor2"
      if (counter != 2)
      {
        return FALSE;
      }
      counter = 1;

    case 0x2770ACA2: // "StartColor"
      if (counter == 1)
      {
        ASSERT( keys.GetCount() == 0);

        U32 r = StdLoad::TypeU32( fScope);
        U32 g = StdLoad::TypeU32( fScope);
        U32 b = StdLoad::TypeU32( fScope);
        U32 a = StdLoad::TypeU32( fScope, 255);

        keys.Append( new ColorKey( 0.0f, r, g, b, a));
      }
      break;

    case 0x8E997D8B: // "FinishColor2"
      if (counter != 2)
      {
        return FALSE;
      }
      counter = 1;

    case 0x730F2950: // "FinishColor"
      if (counter == 1)
      {
        ASSERT( keys.GetCount() == 1);

        U32 r = StdLoad::TypeU32( fScope);
        U32 g = StdLoad::TypeU32( fScope);
        U32 b = StdLoad::TypeU32( fScope);
        U32 a = StdLoad::TypeU32( fScope, 255);

        keys.Append( new ColorKey( 1.0f, r, g, b, a));
      }
      break;
    }
    return TRUE;
  }
  void Data::PostLoad( KeyList<ColorKey> &keys)
  {
    // keys lists must have at least 2 entries
    //
    if (keys.GetCount() == 0)
    {
      keys.Append( new ColorKey( 0.0f, color.r, color.g, color.b, color.a) );
    }
    if (keys.GetCount() == 1)
    {
      ColorKey *key = keys.GetHead();
      Color c = key->color;

      keys.Append( new ColorKey( 1.0f, c.r, c.g, c.b, c.a) );
    }
    keys.KeyList<ColorKey>::PostLoad();
  }

  Bool Data::Configure(FScope *fScope, KeyList<ScaleKey> &keys, U32 counter) // = 1)
  {
    switch (fScope->NameCrc())
    {
    default:
      return FALSE;

    case 0xD7A2677F: // "RadiusKey2"
    case 0x4C7DA445: // "ScaleKey2"
      if (counter != 2)
      {
        return FALSE;
      }
      counter = 1;

    case 0x4FB72CBC: // "RadiusKey"
    case 0x920CA5B2: // "ScaleKey"
    case 0x44FC1D1A: // "TimeKey"
    case 0x5E205EB4: // "StateKey"
      if (counter == 1)
      {
        F32 f = StdLoad::TypeF32( fScope);
        F32 s = StdLoad::TypeF32( fScope);

        keys.Append( new ScaleKey( f, s));
      }
      break;

    case 0xD445F92F: // "StartRadius2"
    case 0x315642BA: // "StartScale2"
      if (counter != 2)
      {
        return FALSE;
      }
      counter = 1;

    case 0x3E50BE22: // "StartRadius"
    case 0x097FD8A4: // "StartScale"
      if (counter == 1)
      {
        ASSERT( keys.GetCount() == 0);

        F32 s = StdLoad::TypeF32( fScope);

        keys.Append( new ScaleKey( 0.0f, s));
      }
      break;

    case 0x9D7F8F83: // "FinishRadius2"
    case 0x25406D61: // "FinishScale2"
      if (counter != 2)
      {
        return FALSE;
      }
      counter = 1;

    case 0x2A4691F9: // "FinishRadius"
    case 0x5D005D56: // "FinishScale"
      if (counter == 1)
      {
        ASSERT( keys.GetCount() == 1);

        F32 s = StdLoad::TypeF32( fScope);

        keys.Append( new ScaleKey( 1.0f, s));
      }
      break;
    }
    return TRUE;
  }

  void Data::PostLoad(KeyList<ScaleKey> &keys)
  {
    // keys lists must have at least 2 entries
    //
    if (keys.GetCount() == 0)
    {
      keys.Append( new ScaleKey( 0.0f, scale) );
    }
    if (keys.GetCount() == 1)
    {
      keys.Append( new ScaleKey( 1.0f, keys.GetHead()->scale) );
    }
    keys.KeyList<ScaleKey>::PostLoad();
  }

  namespace Blend
  {
	  struct MapString
	  {
		  char *match;
		  U32 value;
	  };

    typedef enum _D3DTEXTUREBLEND 
    {
        D3DTBLEND_DECAL            = 1,
        D3DTBLEND_MODULATE         = 2,
        D3DTBLEND_DECALALPHA       = 3,
        D3DTBLEND_MODULATEALPHA    = 4,
        D3DTBLEND_DECALMASK        = 5,
        D3DTBLEND_MODULATEMASK     = 6,
        D3DTBLEND_COPY             = 7,
        D3DTBLEND_ADD              = 8,
        D3DTBLEND_FORCE_DWORD      = 0x7fffffff, /* force 32-bit size enum */
    } D3DTEXTUREBLEND;

	  static const MapString src_mode[] =
	  {
		  { "zero",          RS_SRC_ZERO          },
		  { "one",           RS_SRC_ONE           },
		  { "srccolor",      RS_SRC_SRCCOLOR      },
		  { "invsrccolor",   RS_SRC_INVSRCCOLOR   },
		  { "srcalpha",      RS_SRC_SRCALPHA      },
		  { "invsrcalpha",   RS_SRC_INVSRCALPHA   },
		  { "dstcolor",      RS_SRC_DSTCOLOR     },
		  { "invdstcolor",   RS_SRC_INVDSTCOLOR  },
		  { "dstalpha",      RS_SRC_DSTALPHA     },
		  { "invdstalpha",   RS_SRC_INVDSTALPHA  },
		  { "srcalphasat",   RS_SRC_SRCALPHASAT   },
      { NULL, 0 }
	  };

	  static const MapString dst_mode[] =
	  {
		  { "zero",          RS_DST_ZERO          },
		  { "one",           RS_DST_ONE           },
		  { "srccolor",      RS_DST_SRCCOLOR      },
		  { "invsrccolor",   RS_DST_INVSRCCOLOR   },
		  { "srcalpha",      RS_DST_SRCALPHA      },
		  { "invsrcalpha",   RS_DST_INVSRCALPHA   },
		  { "dstcolor",      RS_DST_DSTCOLOR     },
		  { "invdstcolor",   RS_DST_INVDSTCOLOR  },
		  { "dstalpha",      RS_DST_DSTALPHA     },
		  { "invdstalpha",   RS_DST_INVDSTALPHA  },
		  { "srcalphasat",   RS_DST_SRCALPHASAT   },
      { NULL, 0 }
	  };

	  static const MapString tex_mode[] =
	  {
		  { "decal",         RS_TEX_DECAL         },
		  { "decalalpha",    RS_TEX_DECALALPHA    },
		  { "modulate",      RS_TEX_MODULATE      },
		  { "modulate2x",    RS_TEX_MODULATE2X    },
		  { "modulate4x",    RS_TEX_MODULATE4X    },
		  { "modulatealpha", RS_TEX_MODULATEALPHA },
		  { "add",           RS_TEX_ADD           },
      { NULL, 0 }
	  };

	  static const MapString defModes[] =
	  {
		  { "modulate",      RS_BLEND_MODULATE    },
		  { "modulate2x",    RS_BLEND_MODULATE2X  },
		  { "modulate4x",    RS_BLEND_MODULATE4X  },
		  { "add",           RS_BLEND_ADD         },
		  { "glow",          RS_BLEND_GLOW        },
		  { "decal",         RS_BLEND_DECAL       },
      { NULL, 0 }
	  };

    //
    // Convert a blend value into a name string
    //
    void GetString( U32 blend, GameIdent &string, Bool defs) // = TRUE
    {
      Bool hit = FALSE;

      const MapString *mp;
      if (defs)
      {
        for (mp = defModes; mp->match; mp++)
        {
          if (blend == mp->value)
          {
            Utils::Strcpy( string.str, mp->match);
            return;
          }
        }
      }

      for (mp = src_mode; mp->match; mp++)
      {
        if ((blend & RS_SRC_MASK) == mp->value)
        {
          Utils::Strcat( string.str, mp->match);
          hit = TRUE;
          break;
        }
      }
      for (mp = dst_mode; mp->match; mp++)
      {
        if ((blend & RS_DST_MASK) == mp->value)
        {
          if (hit)
          {
            Utils::Strcat( string.str, " | ");
          }
          Utils::Strcat( string.str, mp->match);
          break;
        }
      }
      for (mp = tex_mode; mp->match; mp++)
      {
        if ((blend & RS_TEX_MASK) == mp->value)
        {
          if (hit)
          {
            Utils::Strcat( string.str, " | ");
          }
          Utils::Strcat( string.str, mp->match);
          break;
        }
      }
    }

    //
    // Convert a string to a D3D blend value
    //
    U32 GetValue( const char * string, U32 defval)
    {
      ASSERT(string);

      // default modes
      const MapString *mp;
      for (mp = defModes; mp->match; mp++)
		  {
			  if (Utils::Strnicmp( string, mp->match, sizeof(mp->match)) == 0)
			  {
				  return mp->value;
			  }
		  }

		  // split data string
		  char src_buf[16];
		  char dst_buf[16];
		  char tex_buf[16];
		  sscanf(string, "%s %s %s", src_buf, dst_buf, tex_buf);

		  U32 src = defval & RS_SRC_MASK;

      // for each source mode...
      for (mp = src_mode; mp->match; mp++)
		  {
			  // if the source mode text buffer matches...
			  if (Utils::Strnicmp(src_buf, mp->match, sizeof(src_buf)) == 0)
			  {
				  // set the source mode
				  src = mp->value;
				  break;
			  }
		  }

		  // get default destination mode
		  U32 dst = defval & RS_DST_MASK;

		  // for each destination mode...
      for (mp = dst_mode; mp->match; mp++)
		  {
			  // if the destination mode text buffer matches...
			  if (Utils::Strnicmp(dst_buf, mp->match, sizeof(dst_buf)) == 0)
			  {
				  // set the destination mode
				  dst = mp->value;
				  break;
			  }
		  }

		  // get default texture mode
		  U32 tex = defval & RS_TEX_MASK;

		  // for each texture mode...
      for (mp = tex_mode; mp->match; mp++)
		  {
			  // if the texture mode text buffer matches...
			  if (Utils::Strnicmp(tex_buf, mp->match, sizeof(tex_buf)) == 0)
			  {
				  // set the texture mode
				  tex = mp->value;
				  break;
			  }
		  }

		  // combine the source, destination and texture modes
		  return (src | dst | tex);
    }
  }
  //----------------------------------------------------------------------------

}