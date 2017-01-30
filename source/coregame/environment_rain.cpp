///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Rain         particles around the camera with splats on ground and water
//
// 26-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "environment_rain_priv.h"
#include "utiltypes.h"
#include "stdload.h"
#include "effects_utils.h"
#include "console.h"
#include "gametime.h"
#include "terrain.h"
#include "random.h"
#include "iface.h"
#include "icwindow.h"
#include "sound.h"

namespace Environment
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Environment::Rain::
  //
  namespace Rain
  {
    // Rain:: constants
    //
    #define MAX_RAIN_GRIDS		16
    #define	MAX_RAIN_BEADS		16
    #define MAX_RAIN_SPLATS		128

    // Rain:: initialization Flag
    //
    static Bool sysInit  = FALSE;
    static Bool lockout  = FALSE;
    static Bool soundoff = TRUE;

    // Rain:: vars
    //
    VarInteger active;          // do ?

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Environment::Rain::Splat
    //
    namespace Splat
    {
      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat:: forward references
      //
      struct Object;

      // Rain::Splat:: vars
      //
      VarInteger      active;

      VarString       typeVar;
      VarInteger      groundVar;
      VarInteger      waterVar;
      VarInteger      groundVarPopup;
      VarInteger      waterVarPopup;

      VarInteger      countVar;					// number of splats in this set
	    VarInteger      colorVar;
      
      VarString       textureVar;
      VarString       blendVar;

      VarFloat        offsetVar;

		  VarFloat        minWidthVar;
      VarFloat        maxWidthVar;   		// Splat will grow using these limits
		  VarFloat        minHeightVar;
      VarFloat        maxHeightVar;

      VarFloat        lifeSpanVar;				// Average time to keep alive (or dead)

      // Rain::Splat:: data members
      //
      //
      BinTree<Type>   typeList;
      Type *   	      type;       // current Splat scope data for CmdHandler()        ;
      Type *   	      ground;
      Type *   	      water;

	    Array<Object>   splats;     // array of live splats

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Type  constructor
      //
      Type::Type( const char *s, TYPE _type, Type *copyType) // = NULL
      {
        name = s;

        // add it to the type list
        //
        typeList.Add( name.crc, this);

        if (copyType)
        {
          type      = copyType->type;
          count			= copyType->count;
	        color			= copyType->color;
	        minWidth	= copyType->minWidth;
	        maxWidth	= copyType->maxWidth;
	        minHeight	= copyType->minHeight;
	        maxHeight	= copyType->maxHeight;
	        texture   = copyType->texture;
	        blend     = copyType->blend;
	        lifeSpan	= copyType->lifeSpan;
	        centered	= copyType->centered;
	        minDist		= copyType->minDist;
          offset    = copyType->offset;
        }
        else if (_type == GROUND)
        {
          type      = _type;
          count			= 128;
	        color			= Color( U32(255), U32(255), U32(255), U32(255));
	        minWidth	= 0.08f;
	        maxWidth	= 0.5f;
	        minHeight	= 0.02f;
	        maxHeight	= 1.0f;
	        texture   = "engine_rain_splash.tga";
	        blend     = RS_BLEND_DEF;
	        lifeSpan	= 0.2f;
	        centered	= 0;
	        minDist		= 1.0f;
          offset    = 0.06f;
        }
        else
        {
          type      = _type;
	        count			= 64;
	        color			= Color( U32(0), U32(255), U32(255), U32(255));
	        minWidth	  = 0.1f;
	        maxWidth	  = 3.0f;
	        minHeight	= 0.1f;
	        maxHeight	= 3.0f;
	        texture	  = "engine_rain_ring.tga";
	        blend   	  = RS_BLEND_MODULATE;
	        lifeSpan	  = 0.5f;
	        centered	  = 1;
	        minDist		= 1.0f;
          offset     = 0.06f;
        }
      }

      void Type::SetupDialog()
      {
        lockout = TRUE;

        countVar = count;
	      colorVar = color;
	      minWidthVar = minWidth;
	      maxWidthVar = maxWidth;
	      minHeightVar = minHeight;
	      maxHeightVar = maxHeight;
	      textureVar = texture.str;

        GameIdent b;
        Effects::Blend::GetString( blend, b);
	      blendVar = b.str;

	      lifeSpanVar = lifeSpan;
        offsetVar = offset;

        if (type == GROUND)
        {
          groundVar = 1;
          waterVar  = 0;
        }
        else
        {
          groundVar = 0;
          waterVar  = 1;
        }

        lockout = FALSE;
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Type::Setup
      //
      void Type::Setup()
      {
        tex = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, texture.str);
        object = &splats[ type == GROUND ? 0 : MAX_RAIN_SPLATS];
        right = Matrix::I.right;
        up = Matrix::I.up;

        SetupDialog();
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Type::CmdHandler
      //
      Bool Type::CmdHandler(U32 pathCrc)
      {
        if (lockout)
        {
          return FALSE;
        }

        switch (pathCrc)
        {
        default:
          return FALSE;

    	  case 0x9F21D03B: // "rain.splat.report"
        {
			    Console::Message("name            = %s", name.str);
          Console::Message("type            = %s", type == GROUND ? "Ground" : "Water");
			    Console::Message("count           = %u", count);
			    Console::Message("texture         = %s", texture.str);
			    Console::Message("color           = %d %d %d %d", color.r, color.g, color.b, color.a);

          GameIdent string;
          Effects::Blend::GetString( blend, string);
          Console::Message("blend           = %s", string.str);

			    Console::Message("start wid/hgt   = %.0f %.0f", minWidth, minHeight);
			    Console::Message("end wid/hgt     = %.0f %.0f", maxWidth, maxHeight);
			    Console::Message("offset/centered = %.1f %u", offset, centered);
//			      Console::Message("velocity = %.1f %u", offset, centered);
			    Console::Message("lifeSpan        = %.1f", lifeSpan);
			    Console::Message("mindist         = %.1f", minDist);
			    break;
        }

        case 0x25688CEC: // "rain.splat.count"
          count = *countVar;
          break;

        case 0xB5A5DD26: // "rain.splat.lifespan"
          lifeSpan = *lifeSpanVar;
          break;

//          case 0xE0532883: // "rain.splat.centered"
//            break;

        case 0x5E9AC5B7: // "rain.splat.texture"
          texture = *textureVar;
          break;

        case 0xFF60004D: // "rain.splat.color"
          color = *colorVar;
          break;

        case 0x5D894A0F: // "rain.splat.blend"
          Effects::Blend::GetValue( *blendVar, blend);
          break;

        case 0x58F75381: // "rain.splat.offset"
          offset = *offsetVar;
          break;

        case 0xB7779B90: // "rain.splat.ground"
          lockout = TRUE;
          if (*groundVar)
          {
            waterVar = FALSE;
            type = GROUND;
          }
          else
          {
            waterVar = TRUE;
            type = WATER;
          }
          lockout = FALSE;

          {
            ICWindow * win = IFace::Find<ICWindow>("StudioTool::Weather");
            if (win)
            {
              IFace::PostEvent( win, NULL, IFace::NOTIFY, Crc::CalcStr("FillSplatTypes"));
            }
          }
          break;

        case 0x5CB60001: // "rain.splat.water"
          lockout = TRUE;
          if (*waterVar)
          {
            groundVar = FALSE;
            type = WATER;
          }
          else
          {
            groundVar = TRUE;
            type = GROUND;
          }
          lockout = FALSE;

          {
            ICWindow * win = IFace::Find<ICWindow>("StudioTool::Weather");
            if (win)
            {
              IFace::PostEvent( win, NULL, IFace::NOTIFY, Crc::CalcStr("FillSplatTypes"));
            }
          }
          break;

        case 0xA958F6FE: // "rain.splat.minwid"
          minWidth = *minWidthVar;
		      break;

        case 0xDF4AB887: // "rain.splat.maxwid"
          maxWidth = *maxWidthVar;
		      break;

        case 0x9C933AA7: // "rain.splat.minhgt"
          minHeight = *minHeightVar;
		      break;

        case 0xEA8174DE: // "rain.splat.maxhgt"
          maxHeight = *maxHeightVar;
		      break;

          //
        }   // switch( pathCrc)

        return TRUE;
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Object
      //
      // one for each actual visible splat
      //
	    struct Object
	    {
		    Color	        color;
		    Vector	      position;
		    F32	          width, height;
		    F32         	timeTillDeath, timeTillBirth, invLifeSpan;
	    };

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat:: function members
      //

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::CmdHandler
      //
      void CmdHandler(U32 pathCrc)
      {
        if (lockout)
        {
          return;
        }

        switch (pathCrc)
        {
        default:
          if (type && type->CmdHandler( pathCrc) && (type == ground || type == water))
          {
            type->Setup();
          }
          break;

        case 0x0C27C0B1: // "rain.popup.setup"
          if (type)
          {
            groundVarPopup = type->type == Type::GROUND ? TRUE : FALSE;
          }
          break;
        case 0xAF0C608B: // "rain.popup.ground"
          lockout = TRUE;
          waterVarPopup = *groundVarPopup ? FALSE : TRUE;
          lockout = FALSE;
          break;
        case 0x49F5AF4A: // "rain.popup.water"
          lockout = TRUE;
          groundVarPopup = *waterVarPopup ? FALSE : TRUE;
          lockout = FALSE;
          break;

        case 0xA1E988E7: // "rain.splat.delete"
          if (type && (!::Environment::Rain::type || (type != ::Environment::Rain::type->groundSplat && type != ::Environment::Rain::type->waterSplat)))
          {
            typeList.Dispose( type->name.crc);
            typeVar = typeList.GetCount() ? typeList.GetFirst()->name.str : "none";

            ICWindow * win = IFace::Find<ICWindow>("StudioTool::Weather");
            if (win)
            {
              IFace::PostEvent( win, NULL, IFace::NOTIFY, Crc::CalcStr("FillSplatTypes"));
            }
          }
          break;

        case 0xBAE53F36: // "rain.splat.create"
        {
          const char * n;
          if (!Console::GetArgString(1, n) || *n == '\0')
          {
            break;
          }
          Type * passType = type;
          if (type && ((type->type == Type::GROUND && *waterVarPopup) || (type->type == Type::WATER && *groundVarPopup)))
          {
            passType = NULL;
          }
          new Type( n, *groundVarPopup ? Type::GROUND : Type::WATER, passType);
          typeVar = n;
          break;
        }

        case 0x899D39D9: // "rain.splat.type"
        {
          Type *newType = typeList.Find( Crc::CalcStr( *typeVar));
          if (!newType)
          {
            break;
          }
          type = newType;
          if (type == ground || type == water)
          {
            type->Setup();
          }
          else
          {
            type->SetupDialog();
          }
          break;
        }
        case 0x54630FDE: // "rain.splat.listtypes"
        {
          char *s = NULL;
          Console::GetArgString(1, s);

          U32 len = 0;
          if (s)
          {
            len = strlen(s);
          }

          // Registered splat types
          CON_DIAG(("[Rain Splat Types]"))

          BinTree<Type>::Iterator i(&typeList);
          for (!i; *i; i++)
          {
            if (!s || !Utils::Strnicmp( (*i)->name.str, s, len))
	    	    {
              Console::Message("%s", (*i)->name.str);
            }
          }
          break;
        }
        //
        }   // switch( pathCrc)
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Init
      //
      void Init()
      {
        VarSys::RegisterHandler("rain.splat", CmdHandler);
        VarSys::RegisterHandler("rain.popup", CmdHandler);

        VarSys::CreateCmd("rain.splat.report");
        VarSys::CreateCmd("rain.splat.listtypes");
        VarSys::CreateCmd("rain.splat.create");
        VarSys::CreateCmd("rain.splat.delete");
        VarSys::CreateCmd("rain.popup.setup");

        VarSys::CreateInteger("rain.splat.active", 1, VarSys::DEFAULT, &active);

        VarSys::CreateString("rain.splat.type", "", VarSys::NOTIFY, &typeVar);
        VarSys::CreateInteger("rain.splat.ground", 1, VarSys::NOTIFY, &groundVar);
        VarSys::CreateInteger("rain.splat.water", 0, VarSys::NOTIFY, &waterVar);

        VarSys::CreateInteger("rain.popup.ground", 1, VarSys::NOTIFY, &groundVarPopup);
        VarSys::CreateInteger("rain.popup.water", 0, VarSys::NOTIFY, &waterVarPopup);

        VarSys::CreateInteger("rain.splat.count", 1, VarSys::NOTIFY, &countVar)->SetIntegerRange( 5, MAX_RAIN_SPLATS);
        VarSys::CreateInteger("rain.splat.color", 1, VarSys::NOTIFY, &colorVar);
      
        VarSys::CreateString("rain.splat.texture", "", VarSys::NOTIFY, &textureVar);
        VarSys::CreateString("rain.splat.blend", "", VarSys::NOTIFY, &blendVar);

        VarSys::CreateFloat("rain.splat.offset", 0, VarSys::NOTIFY, &offsetVar)->SetFloatRange( 0, 0.1f);
        VarSys::CreateFloat("rain.splat.minwid", 0, VarSys::NOTIFY, &minWidthVar)->SetFloatRange( .1f, 20.0f);
        VarSys::CreateFloat("rain.splat.maxwid", 0, VarSys::NOTIFY, &maxWidthVar)->SetFloatRange( .1f, 20.0f);
        VarSys::CreateFloat("rain.splat.minhgt", 0, VarSys::NOTIFY, &minHeightVar)->SetFloatRange(.1f, 20.0f);
        VarSys::CreateFloat("rain.splat.maxhgt", 0, VarSys::NOTIFY, &maxHeightVar)->SetFloatRange(.1f, 20.0f);

        VarSys::CreateFloat("rain.splat.lifespan", 0, VarSys::NOTIFY, &lifeSpanVar)->SetFloatRange( .1f, 5.0f);

        splats.Alloc(MAX_RAIN_SPLATS * 2);

	      // Reset object buffers
        Utils::Memset( &splats[0], 0, splats.count * sizeof(Object));

        // Since these are resurected on a timer, if we perform genocide, they will be ok soon...
	      // all we have to do is set the time till resurection... and Process() will generate these correctly
	      U32	i;
	      for (i = 0; i < splats.count ; i++)
	      {
          splats[i].timeTillBirth	= Random::nonSync.Float();
	      }

        type = NULL;
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Done
      //
      void Done()
      {
        splats.Release();

        typeList.DisposeAll();

        VarSys::DeleteItem("rain.popup");
        VarSys::DeleteItem("rain.splat");
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Process
      //
      void Process(F32 dt)
      {
        if (!*active)
        {
          return;
        }

	      if (ground)
        {
          ground->Process( dt);
        }
	      if (water)
        {
          water->Process( dt);
        }
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Render
      //
      void Render()
      {
	      if (!*active)
        {
		      return;
        }

        Camera &camera = Vid::CurCamera();

        if (ground)
        {
	        // Align ground sprites along y-axis and facing camera
  	      Vid::SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
	  	      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_NOSORT | ground->blend);

          Vid::SetBucketMaterial( Vid::defMaterial);
  	      Vid::SetBucketTexture( ground->tex, TRUE, 0, ground->blend);

	        ground->right.Set(camera.WorldMatrix().Front().z, 0, -camera.WorldMatrix().Front().x);
	        //	(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
	        //	splatInfo.groundSet.right = (camera.WorldMatrix().Front().Cross(splatInfo.groundSet.up));

          ground->Render();
        }

        if (water)
        {
	        // Align water splats in x-z plane, then draw them
  	      Vid::SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
	  	      DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | DP_DONOTLIGHT | RS_NOSORT | water->blend);

          Vid::SetBucketMaterial( Vid::defMaterial);
  	      Vid::SetBucketTexture( water->tex, TRUE, 0, water->blend);

          water->Render();
        }
      }

    } // Environment::Rain::Splat


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain:: forward references
    //
    struct Type;
    struct Object;

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain:: data members
    //
    // type data
    //
    
    VarString     typeVar;

	  VarFloat      directionVar;
	  VarFloat      rotationVar;

	  VarFloat      blockVar;
	  VarFloat      blockHeightVar;
	  VarInteger    gridsVar;					      
	  VarInteger    countVar;						
	  VarFloat      scaleVar;						
	  VarFloat      widthVar;						
	  VarFloat      heightVar;
	  VarFloat      speedVar;

    VarFloat      chanceOfStrikeVar;
    VarString     soundVar;

	  VarInteger    colorVar;
	  VarString     blendVar;
    VarString     textureVar;
    VarFloat      animRateVar;

    VarString     groundSplatVar;
    VarString     waterSplatVar;


    BinTree<Type> typeList;           // available Rain::Type 's
    Type *        type;               // current Rain::Type:: scope
    Sound::Digital::Effect * effect;  // current Rain:: sound effect

    // live data
    //
    Array<Object> rains;              // live rain string data
    F32           intensity;          // how intense is this type
    U32           index;              // animating texture index
    U32           objCount;           // how many strings
	  Bounds 	      blockBounds;				// bounds of all rains
	  F32   	      blockRadius;				// Radius of bounding sphere (half of angled string)
    F32           invSpeed;
    U32           strikeCounter;      // Thunder and Lightning
    Vector        jitter[MAX_RAIN_BEADS];
    F32           texAnim;

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type  constructor
    //
    Type::Type( const char *s, Type * copyType) // = NULL
    {
      name = s;

      if (copyType)
      {
	      blockSize   = copyType->blockSize;
	      blockHeight = blockSize * 1.4f;
	      grids       = copyType->grids;
        count       = copyType->count; 
        width       = copyType->width; 
        height      = copyType->height; 
        scale       = copyType->scale; 
        speed       = copyType->speed; 
	      color			  = copyType->color;
        blend       = copyType->blend;

        texture     = copyType->texture;

        minDist     = copyType->minDist; 
        hasSplats   = copyType->hasSplats;

        dirVal      = copyType->dirVal;
        rotVal      = copyType->rotVal;

        sound       = copyType->sound;

        chanceOfStrike  = copyType->chanceOfStrike;

        groundSplatName = copyType->groundSplatName;
        waterSplatName  = copyType->waterSplatName;
      }
      else
      {
	      blockSize   = 30.0;
	      blockHeight = blockSize * 1.4f;
	      grids       = 8;
        count       = 4; 
        width       = 0.04f; 
        height      = 5.0f; 
        scale       = 1.0f;
        speed       = 25.0f; 
	      color			  = Color( U32(222), U32(222), U32(222), U32(255));
        blend       = RS_BLEND_DEF;

        texture     = "engine_rain.tga";

        minDist     = 0.2f; 
        hasSplats   = TRUE;

        dirVal = 0.2f;
        rotVal = 0.0f;

        sound       = "engine_rain.wav";

        chanceOfStrike  = 0.01f;

        groundSplatName = "groundDef";
        waterSplatName  = "waterDef";

        if (!Splat::typeList.Find( waterSplatName.crc))
        {
          new Splat::Type(  waterSplatName.str, Splat::Type::WATER);
        }
        if (!Splat::typeList.Find( groundSplatName.crc))
        {
          new Splat::Type(  groundSplatName.str, Splat::Type::GROUND);
        }
      }

      // add it to the type list
      //
      typeList.Add( name.crc, this);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type::Setup
    //
    void Type::Setup()
    {
      tex = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, texture.str);

      groundSplat = Splat::typeList.Find( groundSplatName.crc);
      waterSplat  = Splat::typeList.Find( waterSplatName.crc);

      if (groundSplat)
      {
        groundSplat->Setup();
      }
      Splat::ground = groundSplat;

      if (waterSplat)
      {
        waterSplat->Setup();
      }
      Splat::water = waterSplat;

      hasSplats = !groundSplat && !waterSplat ? FALSE : TRUE;

      Quaternion d( PI + dirVal * PI, Matrix::I.right);
      Quaternion r( rotVal * 2 * PI, Matrix::I.up);
      Matrix mat( d * r);
      direction = mat.up;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type::CmdHandler
    //
    Bool Type::CmdHandler(U32 pathCrc)
    {
      if (lockout)
      {
        return FALSE;
      }

      switch (pathCrc)
      {
      default:
        return FALSE;

    	case 0xE7DD5D67: // "rain.report"
      {
			  Console::Message("blocksize       = %.0f",	blockSize);
			  Console::Message("blockheight     = %.0f",	blockHeight);
			  Console::Message("grids/count     = %u %u", grids, count);
			  Console::Message("width/height    = %.2f %.2f",	width, height);
			  Console::Message("scale           = %.2f",	scale);
			  Console::Message("speed           = %.2f",	speed);
			  Console::Message("texture         = %s",	  texture.str);
			  Console::Message("color           = %d %d %d %d", color.r, color.g, color.b, color.a);

        GameIdent string;
        Effects::Blend::GetString( blend, string);
        Console::Message("blend           = %s", string.str);

			  Console::Message("mindist         = %.1f", minDist);
			  Console::Message("sound           = %s", sound.str);
			  Console::Message("groundsplat     = %s", groundSplatName.str);
			  Console::Message("watersplat      = %s", waterSplatName.str);
		  }
		  return FALSE;

      case 0x3A49486B: // "rain.direction"
        dirVal = *directionVar;
        Setup();
        break;
      case 0xBDBCA85B: // "rain.rotation"
        rotVal = *rotationVar;
        Setup();
        break;
	    case 0xF3CC1F83: // "rain.block"
		    blockSize = *blockVar;
        break;
	    case 0x963405DD: // "rain.blockheight"
		    blockHeight = *blockHeightVar;
        break;
      case 0x0D218958: // "rain.grids"
			  grids = *gridsVar;
        break;
      case 0xA363FACC: // "rain.count"
        count = *countVar;
        break;
      case 0x5764026B: // "rain.scale"
        scale  = *scaleVar;
        width  = (*widthVar  + .04f) * *scaleVar;
        height = (*heightVar + .04f) * *scaleVar;
        break;
      case 0x9F0E7132: // "rain.width"
        width  = (*widthVar  + .04f) * *scaleVar;
        break;
      case 0x5831EB08: // "rain.height"
        height = (*heightVar + .04f) * *scaleVar;
        break;
      case 0x9BF5B083: // "rain.speed"
        speed = *speedVar;
        break;
      case 0x796B766D: // "rain.color"
        color = *colorVar;
        break;
      case 0x8CA55977: // "rain.blend"
        blend = Effects::Blend::GetValue( *blendVar, blend);
        break;
      case 0xCA88FC48: // "rain.chanceofstrike"
		    chanceOfStrike = *chanceOfStrikeVar;
        break;
      case 0x6CA02FB0: // "rain.sound"
        effect->StopByEffect(); 
        soundoff = TRUE;
  	    sound = *soundVar;
        break;
      case 0x64AB69E8: // "rain.texture"
        texture = *textureVar;
        break;
      case 0xB66E4743: // "rain.groundsplat"
        groundSplatName = *groundSplatVar;
        break;
      case 0x3275323A: // "rain.watersplat"
        waterSplatName = *waterSplatVar;
        break;
      }   // switch( pathCrc)

      return TRUE;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Object
    //
    // one for each visible string of drops
    //
	  struct Object
	  {
		  F32         height;
		  F32	        width;
		  Color	      color;
		  F32	        spacing[MAX_RAIN_BEADS];	// Parametrized to 0-1 within a block
		  Vector	    start;					        // Position on a block
		  Vector	    end;						        // End of string (along direction, blockDist lower)
		  Vector	    direction;				      // Unit vector
		  F32	        s;						          // Parameter 0-1, wraps at 1 back to 0
		  F32	        speed;					        // meters-per-second
		  F32	        len;						        // length of the string
    };

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain:: function members
    //
    #define JITTER(x) ((Random::nonSync.Float() - 0.5f) * x)

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Setup
    //
    void Setup()
    {
      if (!type)
      {
        Splat::ground = NULL;
        Splat::water  = NULL;

        return;
      }

	    // Sanity check (array bounds)
	    if (type->grids > MAX_RAIN_GRIDS)
      {
        type->grids = MAX_RAIN_GRIDS;
      }
	    if (type->count > MAX_RAIN_BEADS)
      {
        type->count = MAX_RAIN_BEADS;
      }
	    type->direction.Normalize();

	    // Init string positions and speeds
	    F32 dx = type->blockSize / type->grids;
	    F32 dz = type->blockSize / type->grids;
	    F32 ds = 1.0f / (F32)(type->count);

      blockBounds.SetOffset( 
        Vector
        ( 
          .5f * type->blockSize,
          .5f * type->blockHeight,
          .5f * type->blockSize
        )
      );
      blockBounds.Set
      (
 	      .5f * type->blockSize,
        .5f * type->blockHeight,
        .5f * type->blockSize
      );

      objCount = type->grids * type->grids;

	    for (U32 j = 0; j < objCount; j++)
	    {
		    Object *obj = &rains[j];

		    obj->height			  = type->height;
		    obj->width			  = type->width;
		    obj->color        = type->color;
		    obj->start.x		  = (j%type->grids) * dx + JITTER(dx) + .5f * dx;
		    obj->start.y		  = type->blockHeight    + JITTER(ds * type->blockHeight * .5f);
		    obj->start.z		  = (j/type->grids) * dz + JITTER(dz) + .5f * dz;
		    obj->direction.x	= type->direction.x + JITTER(.2f);
		    obj->direction.y	= type->direction.y + JITTER(.2f);
		    obj->direction.z	= type->direction.z + JITTER(.2f);
		    obj->len			    = type->blockHeight / (F32)fabs(obj->direction.y);
		    obj->end.x		    = obj->start.x + obj->direction.x * obj->len;
		    obj->end.y		    = obj->start.y + obj->direction.y * obj->len;
		    obj->end.z		    = obj->start.z + obj->direction.z * obj->len;
		    obj->speed		    = type->speed + JITTER(type->speed*0.2f);
		    obj->s			      = 0;

		    for (U32 k = 0; k < MAX_RAIN_BEADS; k++)
        {
			    obj->spacing[k] = k * ds + JITTER(ds*0.5f);
        }
	    }

      Vector jj = type->direction * type->blockSize * .2f; 

      for (j = 0; j < type->count; j++)
	    {
		    jitter[j].Set( JITTER( jj.x), 0, JITTER( jj.z));
      }

      invSpeed = 1.0f / type->speed;

      type->Setup();

      if (*type->sound.str)
      {
        effect->Setup( type->sound.str);
      }

      texAnim = 0;

      lockout = TRUE;

      typeVar   = type->name.str;

      blockVar  = type->blockSize;
      blockHeightVar = type->blockHeight;
      gridsVar  = type->grids;
      countVar  = type->count;		
      scaleVar  = type->scale;
      widthVar  = type->width  / type->scale -.04f;
      heightVar = type->height / type->scale -.04f;
      speedVar  = type->speed;
      chanceOfStrikeVar = type->chanceOfStrike;
      soundVar = type->sound.str;

      directionVar = type->dirVal;
      rotationVar = type->rotVal;

      colorVar  = type->color;
      GameIdent b;
      Effects::Blend::GetString( type->blend, b);
      blendVar = b.str;
      textureVar = type->texture.str;
      animRateVar = type->animRate;

      groundSplatVar = type->groundSplatName.str;
      waterSplatVar  = type->waterSplatName.str;

      lockout = FALSE;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Strike
    //
    void Strike()
    {
      if (!strikeCounter)
      {
        strikeCounter = 2 + Random::nonSync.Integer( 5);
        Sound::Digital::Effect snd("engine_thunder.wav");
        snd.Play2D(Sound::Digital::DEFAULT_VOLUME, Sound::Digital::NO_OWNER, F32_MAX);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::CmdHandler
    //
    void CmdHandler(U32 pathCrc)
    {
      if (lockout)
      {
        return;
      }

      switch (pathCrc)
      {
      case 0xBBD88C38: // "rain.active"
        if (*active)
        {
          effect->StopByEffect();
          soundoff = TRUE;

          if (typeList.GetCount() == 0)
          {
            lockout = TRUE;
            active = FALSE;
            lockout = FALSE;

            LOG_WARN(("called 'rain.active 1' with no current rain type"));
          }
          else 
          {
            Setup();

            if (Vid::renderState.status.weather)
            {
              effect->Play2D(Sound::Digital::DEFAULT_VOLUME, Sound::Digital::NO_OWNER, F32_MAX, 0);
              soundoff = FALSE;
            }
          }
        }
        else
        {
          effect->StopByEffect();
          soundoff = TRUE;
        }
        break;


      case 0x8E33250F: // "rain.strike"
        Strike();
        break;

      case 0xD91505BB: // "rain.delete"
        if (type)
        {
          typeList.Dispose( type->name.crc);
          typeVar = typeList.GetCount() ? typeList.GetFirst()->name.str : "none";

          ICWindow * win = IFace::Find<ICWindow>("StudioTool::Weather");
          if (win)
          {
            IFace::PostEvent( win, NULL, IFace::NOTIFY, Crc::CalcStr("FillRainTypes"));
          }
        }
        break;

      case 0xC219B26A: // "rain.create"
      {
        const char * n;
        if (!Console::GetArgString(1, n) || *n == '\0')
        {
          break;
        }
        new Type( n, type);
        typeVar = n;
        break;
      }
      case 0xE8B35674: // "rain.type"
      {
        if (Crc::CalcStr( *typeVar) == 0xC9EF9119) // "none"
        {
          active = FALSE;
          type = NULL;
          Setup();
          break;
        }
        Type *newType = typeList.Find( Crc::CalcStr( *typeVar));
        if (!newType)
        {
          break;
        }
        if (newType != type)
        {
          effect->StopByEffect(); 
          soundoff = TRUE;
        }
        type = newType;

        Setup();

        Splat::typeVar = type->groundSplatName.str;

        if (*active && soundoff && Vid::renderState.status.weather)
        {
          effect->Play2D(Sound::Digital::DEFAULT_VOLUME, Sound::Digital::NO_OWNER, F32_MAX, 0);
          soundoff = FALSE;
        }

        pathCrc = 0xE7DD5D67; // "rain.report"
        // fall through
      }

      case 0xE7DD5D67: // "rain.report"
        CON_DIAG(("[%s]", type->name.str));
        // fall through

      default:
        if (type && type->CmdHandler( pathCrc))
        {
          Setup();

          if (*active && soundoff && Vid::renderState.status.weather)
          {
            effect->Play2D(Sound::Digital::DEFAULT_VOLUME, Sound::Digital::NO_OWNER, F32_MAX, 0);
            soundoff = FALSE;
          }
        }
        break;

      case 0x32870F91: // "rain.listtypes"
      {
        char *s = NULL;
        Console::GetArgString(1, s);

        U32 len = 0;
        if (s)
        {
          len = strlen(s);
        }

        // Registered splat types
        CON_DIAG(("[Rain Types]"))

        BinTree<Type>::Iterator i(&typeList);
        for (!i; *i; i++)
        {
          if (!s || !Utils::Strnicmp( (*i)->name.str, s, len))
	    	  {
    			  Console::Message("%s", (*i)->name.str);
          }
        }
        break;
      }
      //
      }   // switch( pathCrc)
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Init
    //
    void Init()
    {
      VarSys::RegisterHandler("rain", CmdHandler);

      VarSys::CreateCmd("rain.report");
      VarSys::CreateCmd("rain.strike");
      VarSys::CreateCmd("rain.listtypes");
      VarSys::CreateCmd("rain.create");
      VarSys::CreateCmd("rain.delete");

      VarSys::CreateInteger("rain.active", 0, VarSys::NOTIFY, &active);

      VarSys::CreateString("rain.type",  "", VarSys::NOTIFY, &typeVar);

      // in Rain::Type::CmdHandler
      //
      VarSys::CreateFloat("rain.direction", 0, VarSys::NOTIFY, &directionVar)->SetFloatRange(0.0f, 1.0f);
      VarSys::CreateFloat("rain.rotation", 0, VarSys::NOTIFY, &rotationVar)->SetFloatRange(0.0f, 1.0f);

      VarSys::CreateFloat("rain.block", 30.0f, VarSys::NOTIFY, &blockVar)->SetFloatRange(2.0f, 100.0f);
      VarSys::CreateFloat("rain.blockheight", 60.0f, VarSys::NOTIFY, &blockHeightVar)->SetFloatRange(4.0f, 200.0f);
      VarSys::CreateInteger("rain.grids", 8, VarSys::NOTIFY, &gridsVar)->SetIntegerRange(2, MAX_RAIN_GRIDS);
      VarSys::CreateInteger("rain.count", 4, VarSys::NOTIFY, &countVar)->SetIntegerRange(2, MAX_RAIN_BEADS);
      VarSys::CreateFloat("rain.scale", 1.0f,   VarSys::NOTIFY, &scaleVar)->SetFloatRange(1, 22.0f);
      VarSys::CreateFloat("rain.width",  .04f , VarSys::NOTIFY, &widthVar)->SetFloatRange(0, 1);
      VarSys::CreateFloat("rain.height", 5.0f, VarSys::NOTIFY, &heightVar)->SetFloatRange(0, 5);
      VarSys::CreateFloat("rain.speed", 25.0f, VarSys::NOTIFY, &speedVar)->SetFloatRange(0.1f, 35.0f);
      VarSys::CreateInteger("rain.color", Color( U32(222), U32(222), U32(222), U32(255)), VarSys::NOTIFY, &colorVar);
      VarSys::CreateString("rain.blend", "modulate", VarSys::NOTIFY, &blendVar);
      VarSys::CreateFloat("rain.chanceofstrike", 0.01f, VarSys::NOTIFY, &chanceOfStrikeVar)->SetFloatRange(0, .1f);
      VarSys::CreateString("rain.sound", "", VarSys::NOTIFY, &soundVar);
      VarSys::CreateString("rain.texture", "engine_rain.tga", VarSys::NOTIFY, &textureVar);
      VarSys::CreateFloat("rain.animrate", 0.1f, VarSys::NOTIFY, &animRateVar)->SetFloatRange(0.01f, 1.0f);

      VarSys::CreateString("rain.groundsplat", "", VarSys::NOTIFY, &groundSplatVar);
      VarSys::CreateString("rain.watersplat",  "", VarSys::NOTIFY, &waterSplatVar);

      Splat::Init();

	    // One time initialization
      rains.Alloc( MAX_RAIN_GRIDS * MAX_RAIN_GRIDS);

      strikeCounter = 0;
      intensity = 1.0f;

      // default rain type
      //
      effect = new Sound::Digital::Effect( "engine_rain.wav");

      texAnim = 0;

      type = NULL;

      sysInit = TRUE;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Done
    //
    void Done()
    {
      if (!soundoff)
      {
        effect->StopByEffect();
        soundoff = TRUE;
      }

      Splat::Done();

      rains.Release();

      delete effect;

      typeList.DisposeAll();

      VarSys::DeleteItem("rain");

      sysInit = FALSE;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Process
    //
    void Process()
    {
      ASSERT( sysInit);

      if (strikeCounter)
      {
        strikeCounter--;
        F32 light = 0.2f + Random::nonSync.Float();
        Vid::SetAmbientColor(light, light, 1.5f * light);
      }

      if (!*active)
      {
        return;
      }
      if (!Vid::renderState.status.weather)
      {
        if (!soundoff)
        {
          effect->StopByEffect();
          soundoff = TRUE;
        }
        return;
      }
      else if (soundoff)
      {
        effect->Play2D(Sound::Digital::DEFAULT_VOLUME, Sound::Digital::NO_OWNER, F32_MAX, 0);
        soundoff = FALSE;
      }

      if (!strikeCounter && Random::nonSync.Float() <= type->chanceOfStrike)
      {
        Strike();
      }

      F32 dt = GameTime::SimTime();

	    // Process offset along each string
	    for (U32 j = 0; j < objCount; j++)
	    {
		    Object * obj = &rains[j];
		    obj->s = (F32)fmod( obj->s + dt * (obj->speed/type->blockHeight), 1.);
	    }

      // texture animation
      texAnim += dt * type->animRate;
      if (texAnim > .1f)
      {
        texAnim -= .1f;
        if (type->tex)
        {
          type->tex = type->tex->GetNext();
        }
      }

      if (type->hasSplats)
      {
        Splat::Process(dt);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Render
    //
    void Render()
    {
      ASSERT( sysInit);

  //	  INTERVAL_IN(" Rain");
      
      if (!*active || !Vid::renderState.status.weather)
      {
		    return;
      }
      type->Render();

      if (type->hasSplats)
      {
        Splat::Render();
      }

  //	  INTERVAL_OUT(" Rain");
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Environment::Rain:: render functions
    //

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Environment::Rain::Type::Render
    //
    void Type::Render()
    {
	    #define RAIN_GRID_COUNT	9
	    static F32	gridOffset[RAIN_GRID_COUNT][3] =
	    {
		    -1.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f,  1.0f, 0.0f,-1.0f,	// around you
		    -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
		    -1.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,
	    };

      const Matrix & cam = Vid::CurCamera().WorldMatrix();
	    Vector worldPos
      (
        (F32)(type->blockSize   * .5f * floor( 2 * cam.posit.x / type->blockSize   ) - type->direction.x  * (type->blockSize + (type->blockHeight - type->blockSize) * .4f)),
	      (F32)(type->blockHeight * .5f * floor( 2 * cam.posit.y / type->blockHeight ) - (type->blockHeight - type->blockSize) ),
	      (F32)(type->blockSize   * .5f * floor( 2 * cam.posit.z / type->blockSize   ) - type->direction.z  * (type->blockSize + (type->blockHeight - type->blockSize) * .4f))
      );

	    // Setup bucket
	    Vid::SetWorldTransform(Matrix::I);
	    Vid::SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
		    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_NOINDEXED | RS_NOSORT | blend);

      Vid::SetBucketMaterial( Vid::defMaterial);
      Vid::SetBucketTexture( type->tex, TRUE, 0, blend);

	    // Compute vertices in a triangle aligned along the rain fall direction
	    // and facing the camera
	    Vector	p0, p1, p2, dir0, dir1;
      Vid::Math::viewMatrix.Rotate( dir0, type->direction);

      // particle size
	    F32	w = type->width * 0.5f;
	    F32	h = type->height* 0.5f;
	    F32	wh= w/h;
      Bool isRound = h > 0.000001f && wh > 0.9f && wh < 1.1f;
      if (!isRound)
      {
        h *= 2;
      }

	    // Generate the rain drop triangles
	    for (U32 grid = 0; grid < RAIN_GRID_COUNT; grid++)
	    {
		    // Compute origin of this block in world space
		    Vector blk
        (   
          gridOffset[grid][0] * type->blockSize   + worldPos.x,
				  gridOffset[grid][1] * type->blockHeight + worldPos.y,
				  gridOffset[grid][2] * type->blockSize   + worldPos.z
        );

		    //Don't draw blocks which are not within camera view
//		    Vector	blkPos;
//		    blkPos.Set(blkX+blockCenter.x, blkY+blockCenter.y, blkZ+blockCenter.z);
//        viewMatrix.Transform( blkPos);
//		    if (camera.SphereTest(blkPos, blockRadius) != clipOUTSIDE)
		    {
			    for (U32 j = 0; j < objCount; j++)
			    {
				    Object * obj = &rains[j];

				    // Compute starting point of this string in camera space
            Vector start0 = blk + obj->start;
		        Vid::Math::viewMatrix.Transform( start0);

            // obj direction in camera space
		        Vid::Math::viewMatrix.Rotate( dir1, obj->direction);

            Vid::SetTranBucketZ( start0.z, Vid::sortEFFECT0 + 222);

	          // get memory
            U32 vCount = count * 4;
            VertexTL * vertmem;
            if (!Vid::LockPrimitiveMem( (void **) &vertmem, vCount))
            {
              return;
            }
            VertexTL vtx0, vtx1, vtx2, * dstV = vertmem;

    			  // Generate the beads along this string
					  for (U32 k = 0; k < count; k++)
					  {
						  // Compute offset along the string and convert it to a position in world space
						  F32	t = (F32)(obj->len * fmod(obj->s + obj->spacing[k], 1));

              Vector start1 = start0 + dir1 * t + jitter[k];
//              start1 = start0 + dir1 * t;
              Vid::ProjectFromCamera_II( vtx0, start1);

              if (isRound)
              {
                if (start1.z < Vid::Math::nearPlane)
                {
                  // nearplane clip
                  continue;
                }

                // ...round rain drops (e.g. snow flakes) are aligned to face the camera
                // don't let drops shrink beyond 2 pix
                F32 hh = Vid::Project( h, start1.z) < 1 ? Vid::ProjectInv( 1, start1.z) : h;
                F32 ww = Vid::Project( w, start1.z) < 1 ? Vid::ProjectInv( 1, start1.z) : w;

                vtx1 = vtx0;
                vtx2 = vtx0;

                vtx0.vv.y -= hh;

                vtx1.vv.x += ww;
                vtx1.vv.y += hh;
                
                vtx2.vv.x -= ww;
                vtx2.vv.y += hh;
              }
              else
              {
                Vector end = start1 + dir0 * h;

                // nearplane clip
                if (start1.z < Vid::Math::nearPlane)
                {
                  if (end.z < Vid::Math::nearPlane)
                  {
                    continue;
                  }
                  t = (Vid::Math::nearPlane - start1.z) / (end.z - start1.z);
                  start1 += (end - start1) * t;
                  Vid::ProjectFromCamera_II( vtx0, start1);
                }
                else if (end.z < Vid::Math::nearPlane)
                {
                  t = (Vid::Math::nearPlane - end.z) / (start1.z - end.z);
                  end += (start1 - end) * t;
                }
                Vid::ProjectFromCamera_II( vtx1, end);

                // don't let drops shrink beyond 2 pix
                end.x = vtx1.vv.x - vtx0.vv.x;
                end.y = vtx1.vv.y - vtx0.vv.y;
                if (fabs( end.x) < 2)
                {
                  vtx1.vv.x = vtx0.vv.x + Utils::FSign( end.x) * 2;
                }
                if (fabs( end.y) < 2)
                {
                  vtx1.vv.y = vtx0.vv.y + Utils::FSign( end.y) * 2;
                }
                vtx2 = vtx1;

                Vector dw = dir0 * w;
                dw.x = -Vid::Project( dw.x, end.z) /* * (cam.front.x > 0 ? -1 : 1)*/;
                dw.y =  Vid::Project( dw.y, end.z) /* * (cam.front.x > 0 ? -1 : 1)*/;
//                dw.x = dw.x < 0 ? Min<F32>( -.5, dw.x) : Max<F32>( .5, dw.x);
//                dw.y = dw.y < 0 ? Min<F32>( -.5, dw.y) : Max<F32>( .5, dw.y);

                if (cam.front.x > 0)
                {
                  vtx1.vv.x -= dw.y;
                  vtx1.vv.y += dw.x;

                  vtx2.vv.x += dw.y;
                  vtx2.vv.y -= dw.x;
                }
                else
                {
                  vtx1.vv.x += dw.y;
                  vtx1.vv.y -= dw.x;

                  vtx2.vv.x -= dw.y;
                  vtx2.vv.y += dw.x;
                }
              }

              dstV->vv  = vtx0.vv;
              dstV->rhw = vtx0.rhw;
              dstV->diffuse  = color;
              dstV->specular = 0xFF000000;
              dstV->uv.u = 0.5f;
              dstV->uv.v = 0.0f;
              dstV++;

              dstV->vv  = vtx1.vv;
              dstV->rhw = vtx1.rhw;
              dstV->diffuse  = color;
              dstV->specular = 0xFF000000;
              dstV->uv.u = 0.0f;
              dstV->uv.v = 1.0f;
              dstV++;

              dstV->vv  = vtx2.vv;
              dstV->rhw = vtx2.rhw;
              dstV->diffuse  = color;
              dstV->specular = 0xFF000000;
              dstV->uv.u = 1.0f;
              dstV->uv.v = 1.0f;
              dstV++;

					  } //...loop over beads on a string

	          Vid::UnlockPrimitiveMem( dstV - vertmem);

  			  } //...loop over rains within grid block
	  	  } //...if grid block is within viewing volume
  	  } //...loop over 3x3x2 grid

    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Environment::Rain::Splat::
    //
    namespace Splat
    {

      void Type::Process(F32 dt)
      {
        const Matrix & cam = Vid::CurCamera().WorldMatrix();

        // calculate splat placement
        //
        Vector pos = cam.posit;

        if (!Terrain::MeterOnMap(pos.x, pos.z))
        {
          return;
        }

        F32 dy = pos.y - Terrain::FindFloorWithWater( pos.x, pos.z);
        Vector front( cam.front + cam.front * dy * offset * Rain::type->blockSize * .6f);

        F32 block = Rain::type->blockSize * (1 + dy / Rain::type->blockSize);
	      F32	z0 = pos.z + front.z - 0.5f * block;
        F32 x0 = pos.x + front.x - 0.5f * block;

        // clip to map
        if (z0 < 0.0f)
        {
          z0 = 0.0f;
        }
        else if (z0 > Terrain::MeterHeight() - block)
        {
          z0 = Terrain::MeterHeight() - block;
        }
        if (x0 < 0.0f)
        {
          x0 = 0.0f;
        }
        else if (x0 > Terrain::MeterWidth() - block)
        {
          x0 = Terrain::MeterWidth() - block;
        }

        // Ground/Water block distribution
	      U32	n	 = (U32)sqrt(count);
        F32	dw = block / (F32)n;    // maximum jitter

	      // Generate the splat quads
	      Object * obj = object;

	      for (U32 j = 0; j < n; j++, z0 += dw)
	      {
		      F32 x = x0;
		      for (U32 i = 0; i < n; i++, obj++, x += dw)
		      {
			      // Process live sprites
			      if (obj->timeTillDeath > 0)
			      {
				      // Process time to live
				      obj->timeTillDeath -= dt;

				      if (obj->timeTillDeath >= 0)
				      {
					      // Scale alpha based on time-to-live and life-span
                F32 s = obj->invLifeSpan * obj->timeTillDeath;
                Float2Int	ss( s * color.a + Float2Int::magic);
                obj->color.a = (U8) ss.i; 

					      // Process position (if we have velocity)
					      //TODO: windshield moves away from center
					      //		ground stays
					      //		water may want to drift with wind...

					      // Scale size based on time-to-live and life-span
					      obj->width = maxWidth  + s * (minWidth  - maxWidth);
					      obj->height= maxHeight + s * (minHeight - maxHeight);
				      }  //...object still be alive on next frame
				      else
				      {
					      // How long do we want to rest six feet under
                obj->timeTillBirth = Random::nonSync.Float() * 0.25f;
				      }  //...object just passed away
			      } //...object alive

			      // Process dormant sprites (just update how long have we been dead)
			      else if (obj->timeTillBirth > 0)
			      {
				      obj->timeTillBirth -= dt;
			      } //...object is dormant

			      // Must be Easter, time for resurection
			      else
			      {
				      Vector vv( 0, 0, 0);

				      // Generate position within one block of user
				      // ...according to the ground type at this position
				      TYPE currType = END;
				      switch (type)
				      {
				      case GROUND:
					      {
                  vv.x = x  + JITTER( dw);
						      vv.z = z0 + JITTER( dw);

                  if (!Terrain::MeterOnMap( vv.x, vv.z))
                  {
                    break;
                  }

                  F32 wy, gy = Terrain::FindFloor( vv.x, vv.z);
                  if (!Terrain::FindWater( vv.x, vv.z, &wy) || gy > wy)
						      {
                    vv.y = gy;
							      currType = GROUND;
						      }
					      }
					      break;

				      case WATER:
					      {
						      vv.x = x  + JITTER( dw);
						      vv.z = z0 + JITTER( dw);

                  if (!Terrain::MeterOnMap( vv.x, vv.z))
                  {
                    break;
                  }

                  F32 wy;
						      if (Terrain::FindWater( vv.x, vv.z, &wy))
                  {
                    vv.y = wy;
							      currType = WATER;
                  }
					      }
					      break;
				      }

				      // If the type found matches 'type'
				      if (currType == type)
				      {
					      obj->color      = color;
					      obj->position	  = vv;
					      obj->width		  = minWidth;
					      obj->height		  = minHeight;

					      // Generate life expectancy
                obj->timeTillDeath	= lifeSpan + Random::nonSync.Float() * lifeSpan;
					      obj->invLifeSpan	= 1.f/obj->timeTillDeath;
				      }
				      else
				      {
					      // Generated point is over incorrect water/ground type so go to sleep
					      // hoping that soon we will be elsewhere
					      obj->timeTillBirth = 0.500f;
				      }
			      } //...object is ready to be resurected
		      } //...loop over sprites
	      }
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Splat::Type::Render
      //
      void Type::Render()
      {
        // camera space nearplane
        const Plane & nearPlane = Vid::CurCamera().Planes()[5];

        VertexTL * vtxBuf;
        U16	* idxBuf;
        U32	iCount = count * 6, vCount = count * 4;

				if (type == GROUND)
				{
	        // Setup buffer for face list
          if (!Vid::LockIndexedPrimitiveMem( (void **) &vtxBuf, vCount, &idxBuf, iCount))
          {
            return;
          }
	        VertexTL 	* dstV = vtxBuf;
	        U16				* dstI = idxBuf;

	        // Generate the splat quads
          vCount = 0;
	        for (Object * obj = object, * oe = object + count; obj < oe; obj++)
	        {
		        if (obj->timeTillDeath >= 0)
		        {
              // pos in camera space
              Vector pos;
              Vid::Math::viewMatrix.Transform( pos, obj->position);

			        // Compute min distance of this string to the camera
			        F32	dist = nearPlane.Evalue( pos);

			        // Don't draw sprites which are too close to the camera or too far
			        if (dist > minDist)
			        {
                Vid::SetTranBucketZ( pos.z, Vid::sortEFFECT0 + 223);

				        Color color = obj->color;

                // pos, h, w, in screen space
                F32	h = Vid::Project( obj->height, pos.z);
                F32	w = Vid::Project( obj->width * .5f, pos.z);
                VertexTL temp;
                Vid::ProjectFromCamera_II( temp, pos);

                dstV->vv.Set( temp.vv.x - w, temp.vv.y, temp.vv.z);
                dstV->rhw = temp.rhw;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 0.0f;
                dstV->uv.v = 1.0f;
                dstV++;

                dstV->vv.Set( temp.vv.x - w, temp.vv.y - h, temp.vv.z);
                dstV->rhw = temp.rhw;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 1.0f;
                dstV->uv.v = 1.0f;
                dstV++;

                dstV->vv.Set( temp.vv.x + w, temp.vv.y - h, temp.vv.z);
                dstV->rhw = temp.rhw;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 1.0f;
                dstV->uv.v = 0.0f;
                dstV++;

                dstV->vv.Set( temp.vv.x + w, temp.vv.y, temp.vv.z);
                dstV->rhw = temp.rhw;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 0.0f;
                dstV->uv.v = 0.0f;
                dstV++;

				        // Fill the next face's indices
				        dstI[0] = U16( vCount+0);
				        dstI[1] = U16( vCount+1);
				        dstI[2] = U16( vCount+2);
				        dstI[3] = U16( vCount+0);
				        dstI[4] = U16( vCount+2);
				        dstI[5] = U16( vCount+3) ;

				        // Advance pointers
				        dstI += 6;
				        vCount += 4;
			        } //...if object not too close and not too far
		        } //...object is alive
	        } //...loop over sprites in this splatter set

          Vid::UnlockIndexedPrimitiveMem( vCount, dstI - idxBuf);
        }
        else
        {
	        // Setup buffer for face list
          U32 heapSize = Vid::Heap::ReqVertex( &vtxBuf, &idxBuf);

	        VertexTL 	* dstV = vtxBuf;
	        U16				* dstI = idxBuf;

  	        // Generate the splat quads
	        Object * obj = &object[0];

	        for (U32 i = 0, iCount = vCount = 0; i < count; i++, obj++)
	        {
					  F32 w = obj->width * 0.5f;
            Vector posC;
            Vid::Math::viewMatrix.Transform( posC, obj->position);
            posC.z -= w * 1.7f;

		        if (obj->timeTillDeath >= 0)
		        {
			        // Compute min distance of this string to the camera
			        F32	dist = nearPlane.Evalue( posC);

			        // Don't draw sprites which are too close to the camera or too far
			        if (dist > minDist)
              {
                Vid::SetTranBucketZ( dist);

                Vector pos  = obj->position;
				        Color color = obj->color;

                dstV->vv.x = pos.x - w;
                dstV->vv.y = pos.y;
                dstV->vv.z = pos.z - w;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 0.0f;
                dstV->uv.v = 0.0f;
                dstV++;

                dstV->vv.x = pos.x - w;
                dstV->vv.y = pos.y;
                dstV->vv.z = pos.z + w;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 1.0f;
                dstV->uv.v = 0.0f;
                dstV++;

                dstV->vv.x = pos.x + w;
                dstV->vv.y = pos.y;
                dstV->vv.z = pos.z + w;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 1.0f;
                dstV->uv.v = 1.0f;
                dstV++;

                dstV->vv.x = pos.x + w;
                dstV->vv.y = pos.y;
                dstV->vv.z = pos.z - w;
                dstV->diffuse = color;
                dstV->specular = 0xFF000000;
                dstV->uv.u = 0.0f;
                dstV->uv.v = 1.0f;
                dstV++;

				        // Fill the next face's indices
				        dstI[0] = U16( vCount+0);
				        dstI[1] = U16( vCount+1);
				        dstI[2] = U16( vCount+2);
				        dstI[3] = U16( vCount+0);
				        dstI[4] = U16( vCount+2);
				        dstI[5] = U16( vCount+3);

				        // Advance pointers
				        dstI += 6;
				        iCount += 6;
				        vCount += 4;

                if (iCount + 6 > Vid::renderState.maxIndices || vCount + 4 > Vid::renderState.maxVerts)
                {
                  // flush
        		      Vid::ProjectClip( vtxBuf, vCount, idxBuf, iCount);

                  dstV = vtxBuf;
                  dstI = idxBuf;
                  vCount = iCount = 0;
                }

			        } //...if object not too close and not too far
		        } //...object is alive
	        } //...loop over sprites in this splatter set

          if (iCount)
          {
            // flush last
  		      Vid::ProjectClip( vtxBuf, vCount, idxBuf, iCount);
          }

          Vid::Heap::Restore( heapSize);
        }
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Type::Splat::LoadInfo
      //
      void Type::LoadInfo( FScope * fScope)
      {

        GameIdent t;
        t = StdLoad::TypeString( fScope, "Type", t.str);
        type = Utils::Stricmp( t.str, "Water") ? GROUND : WATER;

        count = StdLoad::TypeU32( fScope, "Count", count);
        color = StdLoad::TypeColor( fScope, "Color", color);
        texture = StdLoad::TypeString( fScope, "Texture", texture.str);

        GameIdent blendStr = StdLoad::TypeString( fScope, "Blend", blendStr.str);
        blend = Effects::Blend::GetValue( blendStr.str, blend);

        centered = StdLoad::TypeU32( fScope, "Centered", centered);
        offset = StdLoad::TypeF32( fScope, "Offset", offset);
        minWidth  = StdLoad::TypeF32( fScope, "MinWidth", minWidth);
        minHeight = StdLoad::TypeF32( fScope, "MinHeight", minHeight);
        maxWidth  = StdLoad::TypeF32( fScope, "MaxWidth", maxWidth);
        maxHeight = StdLoad::TypeF32( fScope, "MaxHeight", maxHeight);

        lifeSpan = StdLoad::TypeF32( fScope, "LifeSpan", lifeSpan);
        minDist  = StdLoad::TypeF32( fScope, "MinDist", minDist);

        //        F32 maxDist = StdLoad::TypeF32( fScope, "MaxDist", maxDist);
        //        StdLoad::TypeVector( fScope, "Velocity", maxHeight);
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Type::Splat::SaveInfo
      //
      void Type::SaveInfo( FScope * fScope)
      {
        fScope->AddArgString( name.str);

        StdSave::TypeString( fScope, "Type", type == GROUND ? "Ground" : "Water");
        StdSave::TypeU32( fScope, "Count", count);
        StdSave::TypeColor( fScope, "Color", color);
        StdSave::TypeString( fScope, "Texture", texture.str);

        GameIdent blendStr;
        Effects::Blend::GetString( blend, blendStr);
        StdSave::TypeString( fScope, "Blend", blendStr.str);

        StdSave::TypeU32( fScope, "Centered", centered);
        StdSave::TypeF32( fScope, "Offset", offset);
        StdSave::TypeF32( fScope, "MinWidth", minWidth);
        StdSave::TypeF32( fScope, "MinHeight", minHeight);
        StdSave::TypeF32( fScope, "MaxWidth", maxWidth);
        StdSave::TypeF32( fScope, "MaxHeight", maxHeight);

        StdSave::TypeF32( fScope, "LifeSpan", lifeSpan);
        StdSave::TypeF32( fScope, "MinDist", minDist);

        //        StdSave::TypeF32( fScope, "MaxDist", maxDist);
        //        StdSave::TypeVector( fScope, "Velocity", maxHeight);
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Rain::Type::Splat::PostLoad
      //
      void Type::PostLoad()
      {
      }

    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type::LoadInfo
    //
    void Type::LoadInfo( FScope * fScope)
    {
      blockSize   = StdLoad::TypeF32( fScope, "BlockSize", blockSize);
      blockHeight = blockSize * 1.4f;

      grids = StdLoad::TypeU32( fScope, "Grids", grids);
      count = StdLoad::TypeU32( fScope, "Count", count);
      width = StdLoad::TypeF32( fScope, "Width", width);
      height = StdLoad::TypeF32( fScope, "Height", height);
      scale  = StdLoad::TypeF32( fScope, "Scale", scale);
      speed = StdLoad::TypeF32( fScope, "Speed", speed);
      StdLoad::TypeColor( fScope, "Color", color);

      texture = StdLoad::TypeString( fScope, "Texture", "");

      GameIdent blendStr;
      blendStr = StdLoad::TypeString( fScope, "Blend", blendStr.str);
      blend = Effects::Blend::GetValue( blendStr.str, blend);

      minDist = StdLoad::TypeF32( fScope, "MinDist", minDist);
      StdLoad::TypeVector( fScope, "Direction", direction);

      chanceOfStrike = StdLoad::TypeF32( fScope, "ChanceOfStrike", chanceOfStrike);

      sound = StdLoad::TypeString( fScope, "Sound", sound.str);

      groundSplatName = StdLoad::TypeString( fScope, "GroundSplat", groundSplatName.str);
      waterSplatName = StdLoad::TypeString( fScope, "WaterSplat",  waterSplatName.str);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type::SaveInfo
    //
    void Type::SaveInfo( FScope * fScope)
    {
      fScope->AddArgString( name.str);

      StdSave::TypeF32( fScope, "BlockSize", blockSize);

      StdSave::TypeU32( fScope, "Grids", grids);
      StdSave::TypeU32( fScope, "Count", count);
      StdSave::TypeF32( fScope, "Width", width);
      StdSave::TypeF32( fScope, "Height", height);
      StdSave::TypeF32( fScope, "Scale", scale);
      StdSave::TypeF32( fScope, "Speed", speed);
      StdSave::TypeColor( fScope, "Color", color);

      StdSave::TypeString( fScope, "Texture", texture.str);

      GameIdent blendStr;
      Effects::Blend::GetString( blend, blendStr);
      StdSave::TypeString( fScope, "Blend", blendStr.str);

      StdSave::TypeF32( fScope, "MinDist", minDist);
      F32 maxDist = 0;
      StdSave::TypeF32( fScope, "MaxDist", maxDist);
      StdSave::TypeVector( fScope, "Direction", direction);

      StdSave::TypeF32( fScope, "ChanceOfStrike", chanceOfStrike);

      StdSave::TypeString( fScope, "Sound", sound.str);

      StdSave::TypeString( fScope, "GroundSplat", groundSplatName.str);
      StdSave::TypeString( fScope, "WaterSplat",  waterSplatName.str);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::Type::PostLoad
    //
    void Type::PostLoad()
    {
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::LoadInfo
    //
    void LoadInfo( FScope * fScope)
    {
#if 0
      if (!fScope)
      {
        // default splat types
        //
        Splat::ground = new Splat::Type( "groundDef", Splat::Type::GROUND);
        Splat::water  = new Splat::Type( "waterDef", Splat::Type::WATER);

        Splat::water->Setup();
        Splat::ground->Setup();

        Splat::type = Splat::ground;

        type = new Type( "rainDef");
      }
      else
#endif
      if (fScope)
      {
        FScope *sScope;
        while ((sScope = fScope->NextFunction()) != NULL)
        {
          switch (sScope->NameCrc())
          {
            case 0xD76A1098: // "RainType"
            {
              const char *s = sScope->NextArgString();

              type = typeList.Find( Crc::CalcStr( s));
              if (!type)
              {
                type = new Type( s);
              }
              type->LoadInfo( sScope);
              break;
            }
            case 0x5472DE17: // "SplatType"
            {
              const char *s = sScope->NextArgString();

              Splat::type = Splat::typeList.Find( Crc::CalcStr( s));
              if (!type)
              {
                Splat::type = new Splat::Type( s, Splat::Type::GROUND);
              }
              Splat::type->LoadInfo( sScope);
              break;
            }
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::SaveInfo
    //
    void SaveInfo( FScope * fScope)
    {
      BinTree<Type>::Iterator irt(&typeList);
      for (!irt; *irt; irt++)
      {
        FScope * sScope = fScope->AddFunction( "RainType");
        (*irt)->SaveInfo( sScope);
      }
      BinTree<Splat::Type>::Iterator ist(&Splat::typeList);
      for (!ist; *ist; ist++)
      {
        FScope * sScope = fScope->AddFunction( "SplatType");
        (*ist)->SaveInfo( sScope);
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Rain::PostLoad
    //
    void PostLoad()
    {
      BinTree<Type>::Iterator irt(&typeList);
      for (!irt; *irt; irt++)
      {
        (*irt)->PostLoad();
      }
      BinTree<Splat::Type>::Iterator ist(&Splat::typeList);
      for (!ist; *ist; ist++)
      {
        (*ist)->PostLoad();
      }
    }

  } // Environment::Rain

} // Environment::


///////////////////////////////////////////////////////////////////////////////
//
// stuff
//

  #if 0
      void RenderWShieldSet(Camera *camera, SplatSet &splatSet)
      {
	      // Setup buffer for face list
	      U32				iCount = 0, vCount = 0;
	      VertexTL		*dstV;
	      U16				*dstI;
	      if (!Vid::LockIndexedPrimitiveMem((void **)&dstV, 4*count, &dstI, 6*count))
		      return;

	      // Generate the splat quads
	      Object	*obj = &object[0];

	      for (U32 i = 0; i < count; i++, obj++)
	      {
		      if (obj->timeTillDeath >= 0)
		      {
			      // Generate the next face's vertices, anc clip to camera viewport
			      F32	w = obj->width * 0.5f;
			      F32	h = obj->height* 0.5f;
			      F32	x0 = obj->position.x - w,
					      y0 = obj->position.y - h,
					      x1 = obj->position.x + w,
					      y1 = obj->position.y + h,
					      u0 = 0,	v0 = 0, u1 = 1, v1 = 1;
			      if (!Cliobj(camera.ViewRect(), x0, y0, x1, y1, u0, v0, u1, v1))
				      continue;	//...don't draw

			      // Fill the next face (if clipper passed)
			      BLD_TLVERTEX(dstV + 0, x0, y0, 0, 1, obj->color, 0xFF000000, u0, v0);
			      BLD_TLVERTEX(dstV + 1, x1, y0, 0, 1, obj->color, 0xFF000000, u1, v0);
			      BLD_TLVERTEX(dstV + 2, x1, y1, 0, 1, obj->color, 0xFF000000, u1, v1);
			      BLD_TLVERTEX(dstV + 3, x0, y1, 0, 1, obj->color, 0xFF000000, u0, v1);

			      // Fill the next face's indices
			      dstI[0] = vCount+0;
			      dstI[1] = vCount+1;
			      dstI[2] = vCount+2;
			      dstI[3] = vCount+0;
			      dstI[4] = vCount+2;
			      dstI[5] = vCount+3;

			      // Advance pointers
			      dstI += 6;
			      iCount += 6;
			      dstV += 4;
			      vCount += 4;
			      ASSERT(iCount <= Vid::renderState.maxIndices);
			      ASSERT(vCount <= Vid::renderState.maxVerts);
		      } //...object is alive
	      } //...loop over sprites in this splatter set

	      // Flush the buffer
	      Vid::UnlockIndexedPrimitiveMem(vCount, iCount, FALSE, TRUE);
      }
  #endif
