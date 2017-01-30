///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Particle system
//
// 22-OCT-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "geometryrender.h"
#include "meshent.h"
#include "terrain.h"
#include "common.h"



//
// Constructor
//
GeometryRenderClass::GeometryRenderClass()
: ParticleRenderClass()
{
  hasShadow = TRUE;
  noRotate = FALSE;
}


//
// Destructor
//
GeometryRenderClass::~GeometryRenderClass()
{
  colorKeys.Release();
}


//
// Configure the class
//
Bool GeometryRenderClass::Configure(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
  default:
    if (!data.Configure( fScope, colorKeys))
    {
      return ParticleRenderClass::Configure( fScope);
    }
    break;

//    case 0xC939D9B6: // "GeometryName"
//      meshId = StdLoad::TypeString( sScope);
//      break;
  case 0x930082A0: // "HasShadow"
    hasShadow = (U32) fScope->NextArgInteger();
    break;
  case 0x4D6C4ED6: // "NoRotate"
    noRotate = StdLoad::TypeU32( fScope);
    break;
  }
  return TRUE;
}


//
// Postload
//
void GeometryRenderClass::PostLoad()
{
  ParticleRenderClass::PostLoad();

  data.PostLoad( colorKeys);
}

//
// Build a new 
//
ParticleRender *GeometryRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new geometry renderer
	return new GeometryRender(this, particle, data);
}

//
// Constructor
//
GeometryScaleRenderClass::GeometryScaleRenderClass()
: GeometryRenderClass()
{
}

//
// Destructor
//
GeometryScaleRenderClass::~GeometryScaleRenderClass()
{
  scaleKeys.Release();
}

//
// Configure the class
//
Bool GeometryScaleRenderClass::Configure(FScope *fScope)
{
  if (!data.Configure( fScope, scaleKeys))
  {
    return GeometryRenderClass::Configure( fScope);
  }
  return TRUE;
}

//
// Postload
//
void GeometryScaleRenderClass::PostLoad()
{
  GeometryRenderClass::PostLoad();

  data.PostLoad( scaleKeys);
}

//
// Build a new 
//
ParticleRender *GeometryScaleRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new geometry renderer
	return new GeometryScaleRender(this, particle, data);
}



//
// Constructor
//
GeometryRender::GeometryRender( GeometryRenderClass *p, Particle *particle, void *data) // = NULL)
  : ParticleRender(p, particle, data)
{
  colorAnim.Setup( particle->proto->lifeTime, &p->colorKeys, &p->data, p->data.animFlags);

  matrix.ClearData();

  ent = NULL;

  MeshRoot * root = data ? (MeshRoot *) data : p->data.root;
  if (root)
  {
    if (!root->faces.count || !root->vertices.count)
    {
      LOG_WARN( ("chunk particle has empty root %s", root->xsiName.str));
    }
    else
    {
  	  // ChunkSimulate may pass in a chunk; 
      // otherwise data should be NULL and GeometryRenderClass->mesh is used
      ent = new MeshEnt( root);
      ASSERT( ent);

      ent->SetTeamColor( root->chunkColor);

      if (ent->curCycle)
      {
        // animate the full cycle in 'lifeTime'
        ent->fps = ent->curCycle->maxFrame * particle->proto->lifeTimeInv;
        // stop at the end
        ent->SetAnimType( anim2WAY);
        // start animation from the beginning
        ent->SetFrame( 0.0f);
      }

      ent->SetSimCurrent( particle->matrix);
    }
  }
  else
  {
    LOG_WARN(("GeometryRender particle with no mesh root!"))
  }
}


//
// Destructor
//
GeometryRender::~GeometryRender()
{
  if (ent)
  {
  	delete ent;
  }
}

//
// Renderer
//
void GeometryRender::Render()
{
  if (ent && Visible() != clipOUTSIDE)
  {
	  // get rendering class
	  GeometryRenderClass *pclass = (GeometryRenderClass *)proto;

    ent->RenderColor( colorAnim.Current().color);

    if (Vid::renderState.status.showShadows && pclass->hasShadow)
    {
      // FIXME: ent and shadow display are actually unrelated

//      TerrainData::RenderShadow( ent->WorldMatrixRender().Position(), ent->Root().shadowRadius);
      TerrainData::RenderShadowWithWater( *ent);
    }
  }
}

//
// Simulate renderer
//
void GeometryRender::Simulate( F32 dt)
{
	ParticleRender::Simulate( dt);

  GeometryRenderClass * pclass = (GeometryRenderClass *)proto;

  if (dt != 0)      // FIXME : why the pre-simulate with dt = 0?
  {
    colorAnim.Simulate( dt, pclass->data.animRate);

    if (ent)
    {
      ent->UpdateSim( dt);
      ent->SimulateInt( dt);

      ent->SetSimTarget( particle->matrix);
    }
  }
}

//
// Constructor
//
GeometryScaleRender::GeometryScaleRender( GeometryScaleRenderClass *p, Particle *particle, void *data) // = NULL)
  : GeometryRender(p, particle, data)
{
  matrix.ClearData();

  scaleAnim.Setup( particle->proto->lifeTime, &p->scaleKeys, NULL, p->data.animFlags);

  if (ent)
  {
    matrix.right.x = scaleAnim.Current().scale;
    matrix.up.y    = scaleAnim.Current().scale;
    matrix.front.z = scaleAnim.Current().scale;
  //  ent->SetWorldBoundsRadius( ent->ObjectBounds().Radius() * scaleAnim.Current().scale);

    ent->SetSimCurrent( particle->matrix);
  }
}


//
// Renderer
//
void GeometryScaleRender::Render()
{
  if (ent && Visible() != clipOUTSIDE)
  {
	  // get rendering class
	  GeometryRenderClass *pclass = (GeometryRenderClass *)proto;

    ent->RenderColor( colorAnim.Current().color);

    if (Vid::renderState.status.showShadows && pclass->hasShadow)
    {
      // FIXME: ent and shadow display are actually unrelated

//      TerrainData::RenderShadow( ent->WorldMatrixRender().Position(), ent->Root().shadowRadius * scale);
      TerrainData::RenderShadowWithWater( *ent);
    }
  }
}

//
// Simulate renderer
//
void GeometryScaleRender::Simulate( F32 dt)
{
  GeometryRender::Simulate( dt);

  scaleAnim.SetSlave( colorAnim.Current().frame);

  GeometryScaleRenderClass *p = (GeometryScaleRenderClass *)proto;

  if (p->noRotate)
  {
    matrix.right.x = scaleAnim.Current().scale;
    matrix.up.y    = scaleAnim.Current().scale;
    matrix.front.z = scaleAnim.Current().scale;
    matrix.posit = particle->matrix.posit;
  }
  else
  {
    matrix.ClearData();
    matrix.right.x = scaleAnim.Current().scale;
    matrix.up.y    = scaleAnim.Current().scale;
    matrix.front.z = scaleAnim.Current().scale;
    matrix = matrix * particle->matrix;
  }

  if (ent)
  {
    ent->UpdateSim( dt);
    ent->SimulateInt( dt);
    ent->SetSimTarget( matrix);
//  ent->SetWorldBoundsRadius( ent->ObjectBounds().Radius() * scaleAnim.Current().scale);
  }
}
