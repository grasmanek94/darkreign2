///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


#include "worldctrl.h"
#include "particlesystem.h"
#include "particlesystem_priv.h"
#include "utiltypes.h"
#include "console.h"
#include "gamegod.h"

#include "smokesimulate.h"
#include "embersimulate.h"
#include "chunksimulate.h"
#include "bloodsimulate.h"
#include "dustsimulate.h"

#include "particle.h"
#include "particlerender.h"
#include "spriterender.h"
#include "geometryrender.h"
#include "trailrender.h"
#include "beamrender_base.h"
#include "beamrender_plain.h"
#include "beamrender_weapon.h"
#include "beamrender_runner.h"
#include "beamrender_shroud.h"
#include "groundspriterender.h"
#include "waterspriterender.h"
#include "sight.h"
#include "team.h"
#include "vid_private.h"

// DEBUGGING
#include "terrain.h"
#include "input.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ParticleSystem
//
namespace ParticleSystem
{
  static Bool sysInit = FALSE;

  // Derived class structure
  struct DerivedClass
  {
    // Identifier for particle type
    GameIdent ident;

    // Identifier of parent
    GameIdent parent;

    // Configuration scope
    FScope *fScope;

    // Default constructor
    DerivedClass() : fScope(NULL) {}

    // Destructor
    ~DerivedClass()
    {
      if (fScope)
      {
        delete fScope;
      }
    }

  };

  // List of all derived particle classes
  BinTree<DerivedClass> derivedParticles;

  // List of all derived render classes
  BinTree<DerivedClass> derivedRenderers;

  // List of all particle types
  BinTree<ParticleClass> particleTypes;

  // List of all renderer types
  BinTree<ParticleRenderClass> renderTypes;

  // List of all active particle simulators
  NList<Particle> simulators(&Particle::node);

  // List of all active renderers
  NList<ParticleRender> renderSim(&ParticleRender::node);

  // List of all active renderers
  NList<ParticleRender> renderInt(&ParticleRender::node);

  // Enable/disable display of particles
  VarInteger drawParticles;

  // Enable/disable creation of particles
  VarInteger enableParticles;

  // Last particle created
  static ParticlePtr lastParticle;

  // Are cineractives enabled
  static Bool cineractiveMode;

  // Accumulated game skipping time
  static F32 fastModeElapsed;

  // Console Command handler
  void CmdHandler(U32 pathCrc);


  //
  // Initialise particle system
  //
  void Init()
  {
    ASSERT(!sysInit);

    // Register command scope
    VarSys::RegisterHandler("coregame.particle", CmdHandler);

    // Create commands
    VarSys::CreateCmd("coregame.particle.listtypes");
    VarSys::CreateCmd("coregame.particle.create");
    VarSys::CreateCmd("coregame.particle.track");
    VarSys::CreateCmd("coregame.particle.delete");

    // Create vars
    VarSys::CreateInteger("coregame.particle.enabled", TRUE, VarSys::DEFAULT, &enableParticles);
    VarSys::CreateInteger("coregame.particle.draw", TRUE, VarSys::DEFAULT, &drawParticles);

    cineractiveMode = FALSE;
    fastModeElapsed = 0.0F;

    sysInit = TRUE;
  }


  //
  // Shutdown particle system
  //
  void Done()
  {
    ASSERT(sysInit);

    // Dispose of derived classes
    derivedParticles.DisposeAll();
    derivedRenderers.DisposeAll();

    // Dispose of all particle types
    particleTypes.DisposeAll();
    renderTypes.DisposeAll();

    // Remove var scope
    VarSys::DeleteItem("coregame.particle");

    sysInit = FALSE;
  }


  //
  // Mission is closing
  //
  void CloseMission()
  {
    ASSERT(sysInit)

//    simulators.DisposeAll();
//    renderSim.DisposeAll();
//    renderInt.DisposeAll();

    // Dispose of particles
    for (NList<Particle>::Iterator i(&simulators); *i;)
    {
      delete (i++);
    }

    // Dispose of renderers
    for (NList<ParticleRender>::Iterator j(&renderSim); *j;)
    {
      delete (j++);
    }

    // Dispose of renderers
    for (NList<ParticleRender>::Iterator k(&renderInt); *j;)
    {
      delete (k++);
    }
  }


  //
  // Particle Simulator Babel
  //
  static ParticleClass *CreateSimulateClass(GameIdent &classId)
  {
    ParticleClass *p = NULL;

    switch (classId.crc)
    {
      case 0xE3DB018F: // "Smoke"
        p = new SmokeSimulateClass;
        break;

      case 0xE7504E38: // "Ember"
        p = new EmberSimulateClass;
        break;

      case 0x7BDC6C08: // "Chunk"
        p = new ChunkSimulateClass;
        break;

      case 0x13D4B6D6: // "Particle"
        p = new ParticleClass;
        break;

      case 0xB0E7F4D5: // "Stain"
        p = new BloodSimulateClass;
        break;

      case 0xD9152A7F: // "Dust"
        p = new DustSimulateClass;
        break;
    }

    return p;
  }


  //
  // Particle Renderer Babel
  //
  static ParticleRenderClass *CreateRenderClass(GameIdent &classId)
  {
    ParticleRenderClass *p = NULL;

    switch (classId.crc)
    {
      case 0xDD21EFC9: // "Sprite"
        p = new SpriteRenderClass;
        break;

      case 0x09C72CDE: // "Geometry"
        p = new GeometryRenderClass;
        break;

      case 0x0A23AA56: // "GeometryScale"
        p = new GeometryScaleRenderClass;
        break;

      case 0x82698073: // "Trail"
        p = new TrailRenderClass;
        break;

      case 0xDA0BA8B0: // "Beam"
        p = new BeamRenderWeaponClass;
        break;

      case 0xCC14C288: // "BeamPlain"
        p = new BeamRenderPlainClass;
        break;

      case 0x17267313: // "BeamBase"
        p = new BeamRenderBaseClass;
        break;

      case 0x8123AC3D: // "BeamRunner"
        p = new BeamRenderRunnerClass;
        break;

      case 0x04DC14BB: // "BeamShroud"
        p = new BeamRenderShroudClass;
        break;

      case 0xB0E7F4D5: // "Stain"
        p = new AirGroundSpriteRenderClass;
        break;

      case 0xE16208B1: // "GroundSprite"
        p = new GroundSpriteRenderClass;
        break;

      case 0xB03BF3A7: // "WaterSprite"
        p = new WaterSpriteRenderClass;
        break;
    }

    return p;
  }


  //
  // Process a CreateParticleType scope
  //
  Bool ProcessCreateParticleType(FScope *fScope)
  {
    ASSERT(sysInit);
    ASSERT(fScope);

    // The first argument is the type identifier
    GameIdent typeId = fScope->NextArgString();

    // The second argument is the simulator base class
    GameIdent simClass = fScope->NextArgString();

    // Create the Particle Simulator type
    ParticleClass *newParticle = NULL;

    if (particleTypes.Exists(typeId.crc))
    {
      ERR_CONFIG(("Particle type '%s' already defined, ignoring definition", typeId.str))
      return (FALSE);
    }
    else
    {
      if ((newParticle = CreateSimulateClass(simClass)) != NULL)
      {
        // Configure the type
        newParticle->Setup(fScope);

        // Add it to list of derived types
        DerivedClass *newDerived = new DerivedClass;

        newDerived->ident  = typeId;
        newDerived->parent = simClass;
        newDerived->fScope = fScope->Dup();

        derivedParticles.Add(typeId.crc, newDerived);
      }
      else
      {
        // Find the base class
        DerivedClass *derived = derivedParticles.Find(simClass.crc);

        if (derived)
        {
          // FIXME: doesnt handle multiple levels of inheritance
          newParticle = CreateSimulateClass(derived->parent);
          newParticle->Setup(derived->fScope);
          newParticle->Setup(fScope);
        }
        else
        {
          ERR_CONFIG(("Simulate base class [%s] does not exist", simClass.str));
        }
      }
    }

    ASSERT(newParticle);

    // Save type id
    newParticle->typeId = typeId;

    // Add to list
    particleTypes.Add(newParticle->typeId.crc, newParticle);

    //LOG_DIAG(("Registered Particle Simulator [%s]", newParticle->typeId.str));

    // Success
    return (TRUE);
  }


  //
  // Process a CreateParticleRenderType scope
  //
  Bool ProcessCreateParticleRenderType(FScope *fScope)
  {
    ASSERT(sysInit);
    ASSERT(fScope);

    // The first argument is the type identifier
    GameIdent typeId = fScope->NextArgString();

    // The second argument is the render base class
    GameIdent rendClass = fScope->NextArgString();

    if (renderTypes.Exists(typeId.crc))
    {
      ERR_CONFIG(("Particle render type '%s' already defined, ignoring definition", typeId.str))
      return (FALSE);
    }

    // Create the renderer type
    ParticleRenderClass *newRender = NULL;

    if ((newRender = CreateRenderClass(rendClass)) != NULL)
    {
      // Configure the Render type
      newRender->Setup(fScope);

      // Add it to list of derived render types
      DerivedClass *newDerived = new DerivedClass;

      newDerived->ident  = typeId;
      newDerived->parent = rendClass;
      newDerived->fScope = fScope->Dup();

      derivedRenderers.Add(typeId.crc, newDerived);
    }
    else
    {
      // Find the base class
      DerivedClass *derived = derivedRenderers.Find(rendClass.crc);

      if (derived)
      {
        // FIXME: doesnt handle multiple levels of inheritance
        newRender = CreateRenderClass(derived->parent);
        newRender->Setup(derived->fScope);
        newRender->Setup(fScope);
      }
      else
      {
        ERR_CONFIG(("Render base class [%s] does not exist", rendClass.str));
      }
    }

    ASSERT(newRender);

    // Save type id
    newRender->typeId = typeId;

    // Add to list
    renderTypes.Add(newRender->typeId.crc, newRender);

    //LOG_DIAG(("Registered Particle Renderer [%s]", newRender->typeId.str));

    // Success
    return (TRUE);
  }


  Bool VisTest( Bool build, const Vector & p0, const ParticleClass * p, const Vector * p1) // = NULL
  {
    ASSERT( p);

    if (WorldCtrl::MetreOnMap( p0.x, p0.z))
    {
      if (build && p->makeUnderFog)
      {
        // Always create under fog
        return (TRUE);
      }
      else if (!build && p->showUnderFog)
      {
        return (TRUE);
      }
      else if (cineractiveMode)
      {
        if (GameTime::GetFastMode() && !GameTime::GetDisplayMode())
        {
          // Don't create particle if in fast forward mode
          return (FALSE);
        }
        else if (!p1)
        {
          // Create if within viewing frustrum
          Point<S32> cell(
            Utils::FastFtoL(p0.x), 
            Utils::FastFtoL(p0.z));
          Area<S32> rect;
          Vid::CurCamera().GetVisibleRect(rect);

          Bool in = rect.In(cell);

          return in;
        }
        return TRUE;
      }
      else if (!Team::GetDisplayTeam()
       || Sight::Visible(
            WorldCtrl::MetresToCellX( p0.x), 
            WorldCtrl::MetresToCellZ( p0.z),
            Team::GetDisplayTeam())
       || (p1 && WorldCtrl::MetreOnMap( p1->x, p1->z)
         && Sight::Visible(
            WorldCtrl::MetresToCellX( p1->x), 
            WorldCtrl::MetresToCellZ( p1->z),
            Team::GetDisplayTeam())))
      {
        // Create if display team can see cell
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // Construct a new particle instance
  //
  Particle *New(ParticleClass *p, const Matrix &matrix, const Vector &veloc, const Vector &omega, const Vector &length, F32 timer, void *data) // = NULL)
  {
    ASSERT(p);

    if (enableParticles)
    {
      return (p->Build(matrix, veloc, omega, length, timer, data));
    }
    return (NULL);
  }


  //
  // Particle system post load
  //
  void PostLoad()
  {
    GameGod::Loader::SubSystem("#game.loader.particles", 1 + renderTypes.GetCount());

    // Post load simulator classes
    for (BinTree<ParticleClass>::Iterator i(&particleTypes); *i; i++)
    {
      (*i)->PostLoad();
    }
    GameGod::Loader::Advance();

    // Post load renderer classes
    for (BinTree<ParticleRenderClass>::Iterator j(&renderTypes); *j; j++)
    {
      GameGod::Loader::Advance();
      (*j)->PostLoad();
    }
  }


  //
  // Return the type specified
  //
  ParticleClass *FindType(U32 id)
  {
    return (particleTypes.Find(id));
  }


  //
  // Return the type specified
  //
  ParticleRenderClass *FindRenderType(U32 id)
  {
    return (renderTypes.Find(id));
  }


  //
  // Add a simulator to the system
  //
  void AddSimulator(Particle *p)
  {
    ASSERT(p);
    simulators.Append(p);
  }


  //
  // Delete a simulator from the system
  //
  void DeleteSimulator(Particle *p)
  {
    ASSERT(p);
    simulators.Unlink(p);
  }


  //
  // Add a renderer to the system
  //
  void AddRenderer(ParticleRender *p)
  {
    ASSERT(p);

    if (p->proto->data.interpolate)
    {
      renderInt.Append(p);
    }
    else
    {
      renderSim.Append(p);
    }
  }


  //
  // Delete a renderer from the system
  //
  void DeleteRenderer(ParticleRender *p)
  {
    ASSERT(p);

    if (p->proto->data.interpolate)
    {
      renderInt.Unlink(p);
    }
    else
    {
      renderSim.Unlink(p);
    }
  }


  //
  // Simulate
  //
  void Simulate(F32 dt)
  {
    MSWRITEV(13, (0, 0, "Simulators: %6d", simulators.GetCount()));
    MSWRITEV(13, (1, 0, "Renderers : %6d", renderSim.GetCount() + renderInt.GetCount()));
    MSWRITEV(13, (2, 0, "TOTAL     : %6d", simulators.GetCount()+renderSim.GetCount()+renderInt.GetCount()));

    if (cineractiveMode)
    {
      if (GameTime::GetFastMode() && !GameTime::GetDisplayMode())
      {
        // Accumulate elapsed cineractive time when skipping
        fastModeElapsed += dt;
        return;
      }
    }
    else
    {
      if (fastModeElapsed > 0.0F)
      {
        dt += fastModeElapsed;
        fastModeElapsed = 0.0F;
      }
    }

    // Simulate all simulators
    for (NList<Particle>::Iterator i(&simulators); *i;)
    {
      Particle * particle = i++;

      // Check the proto - very rare bug - can sometimes be null??
      if (particle->proto)
      {
        particle->Simulate(dt);
      }
    }

    // Simulate all renderers
    for (NList<ParticleRender>::Iterator j(&renderSim); *j;)
    {
      ParticleRender * render = j++;

      // Check the proto - very rare bug - can sometimes be null??
      if (render->proto)
      {
        render->Simulate(dt);
      }
    }
  }


  //
  // SimulateInt
  //
  void SimulateInt(F32 dt)
  {
    // Simulate all interpolated renderers
    for (NList<ParticleRender>::Iterator j(&renderInt); *j;)
    {
      ParticleRender * render = j++;
      render->Simulate(dt);
    }
  }

  //
  // TRUE if cineractives are on and shroud is disabled
  //
  void SetCineractiveMode(Bool flag)
  {
    if (flag)
    {
      fastModeElapsed = 0.0F;
    }
    cineractiveMode = flag;
  }


  //
  // Render all 
  //
  void Render(Camera &)
  {
    if (drawParticles)
    {
      NList<ParticleRender>::Iterator i(&renderSim);
      while (ParticleRender * render = i++)
      {
        if (!render->particle || !render->particle->proto || render->particle->proto->defaultRender)
        {
          render->Render();
        }
      }
      NList<ParticleRender>::Iterator ii(&renderInt);
      while (ParticleRender * render = ii++)
      {
        if (!render->particle || !render->particle->proto || render->particle->proto->defaultRender)
        {
          render->Render();
        }
      }
    }
  }


  //
  // Read a D3D color value
  //
  void GetColor(FScope *parent, const char *name, ColorF32 & value, ColorF32 defval)
  {
		ColorF32 v = defval;
    FScope *fScope;

		// scale from [0..1] to [0..255]
		v.r *= 255.0f;
		v.g *= 255.0f;
		v.b *= 255.0f;
		v.a *= 255.0f;
    
    if ((fScope = parent->GetFunction(name)) != NULL)
    {
      v.r = F32(fScope->NextArgInteger());
      v.g = F32(fScope->NextArgInteger());
      v.b = F32(fScope->NextArgInteger());
      v.a = F32(fScope->NextArgInteger());
    }

		// scale from [0..255] to [0..1]
		v.r /= 255.0f;
		v.g /= 255.0f;
		v.b /= 255.0f;
		v.a /= 255.0f;

		// combine components
    value = v;
  }


  //
  // Vector transform
  //
  Vector VectorTransform(const Vector &v, const Matrix &M)
  {
    Vector Dst;
    Dst.x =  M.posit.x;
    Dst.y =  M.posit.y;
    Dst.z =  M.posit.z;
    Dst.x += v.x * M.right.x;
    Dst.y += v.x * M.right.y;
    Dst.z += v.x * M.right.z;
    Dst.x += v.y * M.up.x;
    Dst.y += v.y * M.up.y;
    Dst.z += v.y * M.up.z;
    Dst.x += v.z * M.front.x;
    Dst.y += v.z * M.front.y;
    Dst.z += v.z * M.front.z;

    return Dst;
  }


  //
  // Build a rotation matrix
  //
  Matrix BuildPositionRotationMatrix(const F32 pitch, const F32 yaw, const F32 roll, const F32 x, const F32 y, const F32 z)
  {
    // compute sine and cosine of each angle
    F32 rSin, rCos, pSin, pCos, ySin, yCos;

    rSin = F32(sin(roll));
    rCos = F32(cos(roll));
    pSin = F32(sin(pitch));
    pCos = F32(cos(pitch));
    ySin = F32(sin(yaw));
    yCos = F32(cos(yaw));

    // build [Roll] * [Pitch] * [Yaw] matrix
    Matrix M;
    M.right.x = rSin * ySin * pSin + rCos * yCos;
    M.right.y = rSin * pCos;
    M.right.z = rSin * yCos * pSin - rCos * ySin;
    M.rightw  = 0.0f;
    M.up.x    = rCos * ySin * pSin - rSin * yCos;
    M.up.y    = rCos * pCos;
    M.up.z    = rCos * yCos * pSin + rSin * ySin;
    M.upw     = 0.0f;
    M.front.x = ySin * pCos;
    M.front.y = -pSin;
    M.front.z = yCos * pCos;
    M.frontw  = 0.0f;
    M.posit.x = x;
    M.posit.y = y;
    M.posit.z = z;
    M.positw  = 1.0f;

    return M;
  }


  //
  // Apply angular velocity to a matrix
  //
  Matrix Spinner(Matrix &mat, const Vector &omg, const F32 dt)
  {
    Matrix M;
    M = BuildPositionRotationMatrix(-omg.x*dt, -omg.y*dt, -omg.z*dt, 0.0f, 0.0f, 0.0f);
    M = M * mat;
    return M;
  }


  //
  // Build a directional matrix
  //
  void BuildDirectionalMatrix(Matrix &M, const Vector &position, const Vector &direction)
  {
    F32 xz2;
  
    M.posit = position;
    M.positw = 1.0f;

    M.front = direction;
    M.front.Normalize();
    M.frontw = 0.0f;
  
    xz2 = M.front.x * M.front.x + M.front.z * M.front.z;

    if (xz2 < 0.02F)
    {
      M.right.x = 1.0f;
      M.right.y = 0.0f;
      M.right.z = 0.0f;
      M.rightw  = 0.0f;
    }
    else
    {
      M.up.x   = 0.0f;
      M.up.y   = 1.0f;
      M.up.z   = 0.0f;
      M.right  = M.up.Cross(M.front);
      M.right.Normalize();
      M.rightw = 0.0f;
    }
  
    M.up  = M.front.Cross(M.right);
    M.upw = 0.0f;
  }


  //
  // Console Command handler
  //
  void CmdHandler(U32 pathCrc)
  {
    ASSERT(sysInit);

    switch (pathCrc)
    {
      case 0xB6ED91C4: // "coregame.particle.listtypes"
      {
        char *s = NULL;
        Console::GetArgString(1, s);

        U32 len = 0;
        if (s)
        {
          len = strlen(s);
        }

        // Registered particle simulator types
        CON_DIAG(("[Particle Simulators]"))

        BinTree<ParticleClass>::Iterator i(&particleTypes);
        for (!i; *i; i++)
        {
          if (!s || !Utils::Strnicmp( (*i)->typeId.str, s, len))
	    	  {
            CON_DIAG(((*i)->typeId.str))
          }
        }

        // Registered particle renderer types
        CON_DIAG(("[Particle Renderers]"))

        for (BinTree<ParticleRenderClass>::Iterator j(&renderTypes); *j; j++)
        {
          if (!s || !Utils::Strnicmp( (*j)->typeId.str, s, len))
	    	  {
            CON_DIAG(((*j)->typeId.str))
          }
        }

        break;
      }

      case 0x9EB59E4C: // "coregame.particle.create"
      {
        const char *s;

        if (!Console::GetArgString(1, s))
        {
          break;
        }

        Vector pos;

        if (TerrainData::ScreenToTerrain(Input::MousePos().x, Input::MousePos().y, pos) && WorldCtrl::MetreOnMap(pos.x, pos.z))
        {
          // above the ground
          pos.y += 5.0F;

          //
          ParticleClass *p = FindType(Crc::CalcStr(s));

          if (p)
          {
				    Vector omg( 0.0f, 0.0f, 0.0f);
            Vector vel( 0.0F, 2.0F, 0.0F);

            Matrix m;
            m.ClearData();
            m.posit = pos;

            Particle *particle = New(p, m, vel, omg, Vector(50, 0, 50), 0.0F);

            if (particle)
            {
              lastParticle.Setup(particle);
            }
          }
        }
        break;
      }

      case 0x85B9299D: // "coregame.particle.delete"
      {
        if (lastParticle.Alive())
        {
          Particle *p = lastParticle.GetData();

          lastParticle.Clear();
          delete p;
        }
        break;
      }

      case 0x9EA8BAB3: // "coregame.particle.track"
      {
        if (lastParticle.Alive())
        {
          Vector pos;

          if (TerrainData::ScreenToTerrain(Input::MousePos().x, Input::MousePos().y, pos) && WorldCtrl::MetreOnMap(pos.x, pos.z))
          {
            // above the ground
            pos.y += 1.0F;
            lastParticle->Update(lastParticle->matrix, pos - lastParticle->matrix.posit);
          }
        }
        break;
      }
    }
  }

}
