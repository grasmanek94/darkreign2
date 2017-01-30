///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// EarthQuakes
//
// 01-NOV-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "environment_quake.h"
#include "utiltypes.h"
#include "stdload.h"
#include "effects_utils.h"
#include "console.h"
#include "gametime.h"
#include "terrain.h"
#include "random.h"
#include "sound.h"
#include "perfstats.h"
#include "lopassfilter.h"

namespace Environment
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Environment::Quake::
  //
  namespace Quake
  {
    // Quake:: initialization Flag
    //
    static Bool sysInit  = FALSE;
    static Bool soundoff = TRUE;

    // Quake:: vars
    //
    VarInteger active;          // do ?

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake:: forward references
    //
    struct Type;
    struct Object;

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake:: data members
    //
    // type data
    //

    struct QuakeKey : public KeyFrame
    {
      F32 intensity, speed;

      QuakeKey()
      {
        frame = 0.0f;
        intensity = speed = 0.0f;
      }
      QuakeKey( F32 f, F32 i, F32 s)
      {
        frame = f;
        intensity = i;
        speed = s;
      }
      void Setup( const Effects::Data & data)
      {
        data;
      }

      void Interpolate( const QuakeKey & k1, const QuakeKey & k2, F32 dt)
      {
        intensity = k1.intensity + (k2.intensity - k1.intensity) * dt;
        speed     = k1.speed     + (k2.speed     - k1.speed)     * dt;
      }
    };

    BinTree<Type>       typeList;

    VarString           typeVar;
    Type *              curType;
    KeyAnim<QuakeKey>   quakeAnim;    // animating curType

    Sound::Digital::Effect * effect;

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake::Type
    //
    //
    struct Type
    { 
      ///////////////////////////////////////////////////////////////////////////////
      //
      // Quake::Type:: data members
      //
      // type data
      //
      GameIdent         name;

      KeyList<QuakeKey> keys;

      F32               lifeTime;

      GameIdent         sound;

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Quake::Type  constructor
      //
      Type( FScope * fScope)
      {
        name = StdLoad::TypeString( fScope);

        Effects::Data data;
        FScope * sScope;
        while ((sScope = fScope->NextFunction()) != NULL)
        {
          switch (sScope->NameCrc())
          {
          default:
          {
            // effects helper structure
            //
            if (data.Configure( sScope))
            {
              lifeTime = data.lifeTime;
              sound    = data.objectId;
            }
            break;
          }

          case 0x9FECAD2F: // "QuakeKey"
          {
            F32 f, i, s;

            f = StdLoad::TypeF32( sScope);    // frame
            i = StdLoad::TypeF32( sScope);    // intensity
            s = StdLoad::TypeF32( sScope);    // speed

            keys.Append( new QuakeKey( f, i, s));
            break;
          }
            //
          } // switch
        }

        ASSERT( keys.GetCount() >= 2);

        keys.PostLoad();      // initialize

        typeList.Add( name.crc, this);
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // Quake::Type  destructor
      //
      ~Type()
      {
        keys.DisposeAll();
      }

    }; // Quake::Type


    void SetCurrent( Type * t, Bool activate = TRUE)
    {
      ASSERT( t);

      if (t != curType)
      {
        if (!effect)
        {
          effect = new Sound::Digital::Effect();         
        }
        if (!soundoff)
        {
          effect->StopByEffect(); 
          soundoff = TRUE;
        }

        curType = t;
        quakeAnim.Setup( t->lifeTime, &t->keys, NULL, Effects::flagLOOP);

        if (*t->sound.str)
        {
          effect->Setup(t->sound.str);
        }
      }
      if (activate && !*active)
      {
        active = TRUE;
      }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake::CmdHandler
    //
    void CmdHandler(U32 pathCrc)
    {
      switch (pathCrc)
      {
      case 0x3860F45E: // "quake.active"
        if (*active)
        {
          effect->StopByEffect();
          effect->Play2D(Sound::Digital::DEFAULT_VOLUME, Sound::Digital::NO_OWNER, F32_MAX, 0);
          soundoff = FALSE;
          quakeAnim.SetFrame(0);
        }
        else
        {
          effect->StopByEffect();
          soundoff = TRUE;
        }
        break;

      case 0x030664FD: // "quake.type"
      {
        Type * t = typeList.Find( Crc::CalcStr( *typeVar));
        if (t)
        {
          SetCurrent( t);
        }
        break;
      }

      case 0x44327F38: // "quake.listtypes"
      {
        char *s = NULL;
        Console::GetArgString(1, s);

        U32 len = 0;
        if (s)
        {
          len = strlen(s);
        }

        // Registered quake types
        CON_DIAG(("[Quake Types]"))

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
    // Quake::Init
    //
    void Init()
    {
      VarSys::RegisterHandler("quake", CmdHandler);

      VarSys::CreateCmd("quake.listtypes");

      VarSys::CreateInteger("quake.active", 0, VarSys::NOTIFY, &active);
      VarSys::CreateString("quake.type",   "", VarSys::NOTIFY, &typeVar);

      curType = NULL;
      effect  = NULL;

      sysInit = TRUE;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake::Done
    //
    void Done()
    {
      if (!soundoff)
      {
        effect->StopByEffect(); 
        soundoff = TRUE;
      }
      if (effect)
      {
        delete effect;
      }

      typeList.DisposeAll();

      VarSys::DeleteItem("quake");

      sysInit = FALSE;
    }

    static LoPassFilterF32 filterX( 0.5f, 0);
    static LoPassFilterF32 filterY( 0.5f, 0);
    static LoPassFilterF32 filterZ( 0.5f, 0);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake::Simulate
    //
    void Simulate( F32 dt)
    {
      ASSERT( sysInit);

      if (!*active)
      {
        return;
      }

      // advance quake animating parameters
      //
      if (!quakeAnim.Simulate( dt))
      {
        active = FALSE;
        return;
      }

      // update shake filters
      //
      if (filterX.CheckThresh() && filterY.CheckThresh() && filterZ.CheckThresh())
      {
        // reached dest offset; set new random offset
        //
        filterX.SetSpeed( quakeAnim.Current().speed);
        filterY.SetSpeed( quakeAnim.Current().speed);
        filterZ.SetSpeed( quakeAnim.Current().speed);

        filterX.Set( Random::nonSync.Float() * quakeAnim.Current().intensity);
        filterY.Set( Random::nonSync.Float() * quakeAnim.Current().intensity);
        filterZ.Set( Random::nonSync.Float() * quakeAnim.Current().intensity);
      }
      else
      {
        // still moving towards the last calculated random offset
        //
        filterX.Update();
        filterY.Update();
        filterZ.Update();
      }

    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Quake::SetQuake
    //
    // shake it up baby now
    //
    Bool SetWorldMatrix( FamilyNode & node, const Matrix & matrix)
    {
      ASSERT( sysInit);

      if (!*active)
      {
        // setup world position
        //
        node.SetWorldAll(matrix);

        return FALSE;
      }

      Quaternion q0( matrix);
      Quaternion q1( quakeAnim.Current().intensity * 0.2f, Vector( filterX.Current(), filterY.Current(), filterZ.Current()));
      Matrix mat( q0 * q1);
      mat.posit.x = matrix.posit.x + filterX.Current();
      mat.posit.y = matrix.posit.y + filterY.Current();
      mat.posit.z = matrix.posit.z + filterZ.Current();

      // setup world position
      //
      node.SetWorldAll(mat);

      return TRUE;
    }

    void ProcessCreate(FScope *fScope)
    {
      Type * t = new Type( fScope);

      SetCurrent( t, FALSE);
    }

  } // Environment::Quake

} // Environment::

