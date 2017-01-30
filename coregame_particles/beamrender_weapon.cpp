///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// beamrender_weapon.cpp
//
// 05-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"
#include "light_priv.h"
#include "mesh.h"
#include "beamrender_weapon.h"
#include "random.h"
#include "environment_light.h"


BeamRenderWeaponClass::BeamRenderWeaponClass() : BeamRenderBaseClass()
{
}

//
// Build a new renderer
//
ParticleRender * BeamRenderWeaponClass::Build( Particle * particle, void * data) // = NULL
{
	return new BeamRenderWeapon( this, particle, data);
}


//
// BeamRenderWeapon::BeamRenderWeapon
//
BeamRenderWeapon::BeamRenderWeapon( BeamRenderWeaponClass * proto, Particle *particle, void *data) // = NULL
: BeamRenderBase( proto, particle, data)
{
  flareColor = colorAnim.Current().color;
  uvRotate = 0.0f;

  light = NULL;
  if (proto->light)
  {
    light = Vid::Light::Create( "beamlight", Vid::Light::lightPOINT, particle->proto->priority);

    // Vid::Light::Create filters light new via performance and priority
    //
    if (light)
    {
      light->SetRange( 16.0f);
    }
  }

  // vertices and indices
  //
  U32 pcount = proto->oriented ? proto->pointCount : 3;
  vcount = (pcount - 1) * 2;
  icount = (vcount - 1) * 6;

  points.Alloc( pcount * 2);
  randoms.Alloc( pcount);

  F32 random = proto->wiggle;
  if (random)
  {
    random0.SetMemory();
    random0.SetSpeed( proto->wiggleSpeed);
    random0.Set( Vector( 
      Random::nonSync.Float() * 2.0f * random - random,
      Random::nonSync.Float() * 2.0f * random - random,
      Random::nonSync.Float() * 2.0f * random - random));

    random1.SetMemory();
    random1.SetSpeed( proto->wiggleSpeed);
    random1.Set( Vector( 
      Random::nonSync.Float() * 2.0f * random - random,
      Random::nonSync.Float() * 2.0f * random - random,
      Random::nonSync.Float() * 2.0f * random - random));
  }

  // setup slave beam spiral quaternions
  rotation.ClearData();
  rotate.ClearData();
  twist.ClearData();
  beamTwist.ClearData();

  rotation.Set( Random::nonSync.Float() * 2 * PI, Matrix::I.front);
  rotate.Set( proto->rotate, Matrix::I.front);
  twist.Set( 2.0f * PI * proto->twist / pcount, Matrix::I.front);
  if (proto->beamCount > 1)
  {
    // space beams equally all or only slave
    beamTwist.Set( 2.0f * PI / (proto->beamCount - (proto->slave ? 1 : 0)), Matrix::I.front);
  }

  // calculate the points
  SetPoints();
}

//
// BeamRenderWeapon::~BeamRenderWeapon
//
BeamRenderWeapon::~BeamRenderWeapon()
{
  if (light)
  {
    delete light;
  }
  randoms.Release();
}

//
// draw a line along a set of points
//
void BeamRenderWeapon::Render()
{
  // get beam rendering class
  BeamRenderWeaponClass * pclass = (BeamRenderWeaponClass *)proto;
  U32 pcount = pclass->oriented ? pclass->pointCount : 3;

  if (!points.count || Visible( &points[0], &points[pcount-1]) == clipOUTSIDE)
  {
    return;
  }

  // render
  Camera & cam = Vid::CurCamera();
  Bitmap * texture = pclass->data.texture;
  U32 blend = pclass->data.blend;
  F32 radius = pclass->data.scale;

  Vector spos, & pos0 = points[0];
  Vid::TransformFromWorld( spos, pos0);

  F32 df  = (1.0f - *Vid::Var::perfs[2]);
  F32 dfp = df * particle->proto->priority;

  if (pclass->flares && dfp < .5555f)
  {
    // random flare rotation
    Vector rotate( uvRotate, (F32) sqrt( 1.0f - uvRotate * uvRotate), 0);

    // source flare
    if (spos.z >= cam.NearPlane())
    {
      Vid::RenderFlareSprite( TRUE, pos0, 2.0f * radius + 3.0f * uvRotate, 
        Vid::Light::flareTex0, flareColor, blend, Vid::sortLIGHT0, rotate);
    }

    // target flare
    Vector epos, &pos1 = points[pcount-1];
    Vid::TransformFromWorld( epos, pos1);
    if (epos.z >= cam.NearPlane())
    {
      Vid::RenderFlareSprite( TRUE, pos1, 2.0f * radius + 4.0f * uvRotate, 
        Vid::Light::flareTex0, flareColor, blend, Vid::sortLIGHT0, rotate);
    }
  }
  Color color = colorAnim.Current().color;

  if (pclass->slave)
  {
    if (pclass->noface)
    {
      // draw the main beam
      Vid::RenderBeamOriented( TRUE, 
        points.data, pcount, pclass->orient, spos.z, 
        texture, color, blend, U16(pclass->data.sorting),
        uvCurrent, pclass->data.uvScale, pclass->tapers);
    }
    else
    {
      // draw the main beam
      Vid::RenderBeam( TRUE, 
        points.data, pcount, radius, spos.z, 
        texture, color, blend, U16(pclass->data.sorting),
        uvCurrent, pclass->data.uvScale, pclass->tapers);
    }

    // draw the slave beams
    // do the slave beams have random behavior
    Bool random = pclass->distance * pclass->randomFactor != 0;
    color.Set( (U32)color.g, (U32)color.r, (U32)color.b, (U32)color.a >> 1);
    Quaternion rr = rotation;
    U32 i;
    for (i = 1; i < pclass->beamCount; i++, rr *= beamTwist)
    {
      Vector * p, * pe = &points[pcount * 2 - 1], *psrc = &points[0];
      p = &points[pcount];
      *p++ = *psrc++;

      Quaternion r = rr;
      F32 * ran = &randoms[0];
      for ( ; p < pe; p++, psrc++, r *= twist, ran++)
      {
        // slave beams spiral around the main beam
        Vector db = (*psrc - *(psrc - 1));
        db.Normalize();
        Quaternion q = Quaternion( 0, db) * r;
        *p = *psrc + Vector( q.GetRight() * (random ? *ran : pclass->distance));
      }
      *p = *psrc++;

      if (pclass->noface)
      {
        Vid::RenderBeamOriented( TRUE, psrc, pcount, pclass->orient * 0.3f, spos.z, 
          texture, color, blend, U16(pclass->data.sorting),
          uvCurrent, pclass->data.uvScale * 0.45f, pclass->tapers);
      }
      else
      {
        Vid::RenderBeam( TRUE, psrc, pcount, radius * 0.3f, spos.z, 
          texture, color, blend, U16(pclass->data.sorting),
          uvCurrent, pclass->data.uvScale * 0.45f, pclass->tapers);
      }
    }
  }
  else
  {
    Bool random = pclass->distance * pclass->randomFactor != 0;
    Quaternion rr = rotation;
    U32 i;
    for (i = 0; i < pclass->beamCount; i++, rr *= beamTwist)
    {
      Vector * p = &points[pcount], * pe = &points[pcount * 2 - 1], *psrc = &points[0];
      *p++ = *psrc++;

      for (F32 * ran = &randoms[0]; p < pe; p++, psrc++, ran++)
      {
        *p = *psrc + Vector( rr.GetRight() * (random ? *ran : pclass->distance));
      }
      *p = *psrc++;

      if (pclass->noface)
      {
        Vid::RenderBeamOriented( TRUE, psrc, pcount, pclass->orient, spos.z, 
          texture, color, blend, U16(pclass->data.sorting),
          uvCurrent, pclass->data.uvScale, pclass->tapers);
      }
      else
      {
        Vid::RenderBeam( TRUE, psrc, pcount, radius, spos.z, 
          texture, color, blend, U16(pclass->data.sorting),
          uvCurrent, pclass->data.uvScale, pclass->tapers);
      }
    }
    // all main beams
  }
}

// generate points in a nice arc between source and target
//
void BeamRenderWeapon::SetPoints()
{
  // get beam rendering class
  BeamRenderWeaponClass * pclass = (BeamRenderWeaponClass *)proto;
  U32 pcount = pclass->oriented ? pclass->pointCount : 3;

  // setup source and target points
  points[0] = particle->matrix.posit;
  Vector * p = &points[pcount-1];
  *p = particle->matrix.posit + particle->length;

  distance = particle->length.Magnitude();

  // figure out the curve's target tangent
  Vector t1, front;
  F32 dist, rand1 = pclass->wiggle;
  if (pclass->curvature != 0)
  {
    // oriented beam
    //
    front = particle->matrix.front;

    if (particle->length.Magnitude2() < .2f)
    {
      // FIXME: for 0 length beams
      t1 = front;
      dist = 10.0f;
    }
    else
    {
      t1 = *p - points[0];
      dist = distance;
      t1 *= 1.2f / dist;
      t1 = t1 - front;
      t1.Normalize();
    }
    dist *= pclass->curvature;

    // wiggle the beam endpoint tangents
    if (rand1 && pclass->curvature != 0)
    {
      front += random0.Update();
      if (random0.Current() - random0.Target() < Vector(.001f,.001f,.001f))
      {
        random0.Set( Vector(
          Random::nonSync.Float() * 2.0f * rand1 - rand1,
          Random::nonSync.Float() * 2.0f * rand1 - rand1,
          Random::nonSync.Float() * 2.0f * rand1 - rand1));
      }
      front.Normalize();

      // wiggle
      t1 += random1.Update();
      if (random1.Current() - random1.Target() < Vector(.001f,.001f,.001f))
      {
        random1.Set( Vector(
          Random::nonSync.Float() * 2.0f * rand1 - rand1,
          Random::nonSync.Float() * 2.0f * rand1 - rand1,
          Random::nonSync.Float() * 2.0f * rand1 - rand1));
      }
      t1.Normalize();
    }
    t1    *= dist;
    front *= dist;

    // Setup the curve
  //  spline.Setup( points[0], front, *p, t1);
    spline.SetupHermite( points[0], front, *p, t1);

    F32 random = pclass->distance * pclass->randomFactor;  
    F32 t, dt = 1.0f / pcount, * ran = &randoms[0];
    Vector * pe = &points[pcount-1];
    p = &points[1]; 
    for (t = dt; p < pe; p++, t += dt, ran++)
    {
      *ran = Random::nonSync.Float() * random;    // random for slaves
      *p = spline.Step(t);

      if (rand1 && pclass->curvature == 0)
      {
        // random behavior for the main beam
        //
        *p += Vector( 
          Random::nonSync.Float() * 2.0f * rand1 - rand1,
          Random::nonSync.Float() * 2.0f * rand1 - rand1,
          Random::nonSync.Float() * 2.0f * rand1 - rand1);
      }
    }
  }
  else
  {
    // strait point-to-point beam (curvature = 0)
    //
    front = *p - points[0];
    front /= (F32)(pcount - 1);

    U32 i;
    for (i = 1; i < pcount - 1; i++)
    {
      points[i] = points[i - 1] + front;
    }
    F32 random = pclass->distance * pclass->randomFactor;  
    if (random)
    {
      F32 * ran = &randoms[0];
      Vector * pe = &points[pcount-1];
      p = &points[1]; 
      for ( ; p < pe; p++, ran++)
      {
        if (pclass->beamCount == 1)
        {
          *p += Vector( 
            Random::nonSync.Float() * 2.0f * random - random,
            Random::nonSync.Float() * 2.0f * random - random,
            Random::nonSync.Float() * 2.0f * random - random);
        }
        else
        {
          *ran = Random::nonSync.Float() * random;
        }
      }
    }
  }

  // distance per poly
//  distance /= F32(pcount - 1);

  // rotate the slave beams
  rotation *= rotate;

  if (light)
  {
    Color color = colorAnim.Current().color;
    F32 v = (Random::nonSync.Float() * 0.7f + 0.3f) * U8toNormF32;
    light->SetColor( color.r * v, color.g * v, color.b * v);
    light->SetPosition( points[(pcount >> 1)] );
  }
}

// simulate beam renderer
void BeamRenderWeapon::Simulate( F32 dt)
{
  BeamRenderBase::Simulate( dt);
  
  SetPoints();

  flareColor = colorAnim.Current().color;
  flareColor.Darken( (U32) (244.0f * Random::nonSync.Float()));

  uvRotate = Random::nonSync.Float();
}
