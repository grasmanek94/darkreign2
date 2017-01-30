///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// trailrender.cpp
//
// 22-OCT-98
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "trailrender.h"
#include "random.h"
#include "statistics.h"



//
// TrailRenderClass::TrailRenderClass
//
TrailRenderClass::TrailRenderClass() : ParticleRenderClass()
{
	data.uvScale = 0.1f;
  decay = 0;
  hasDecay = FALSE;
}


//
// Configure the class
//
Bool TrailRenderClass::Configure(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
  default:
    if (!data.Configure( fScope, colorKeys)
     && !data.Configure( fScope, scaleKeys))
    {
      return ParticleRenderClass::Configure( fScope);
    }
    break;

  case 0x3800F7F0: // "Decay"
    decay = StdLoad::TypeF32(fScope, decay);
    hasDecay = TRUE;
    break;
  }
  return TRUE;
}


//
// Postload
//
void TrailRenderClass::PostLoad()
{
  ParticleRenderClass::PostLoad();

  data.PostLoad( colorKeys);
  data.PostLoad( scaleKeys);
}


//
// Build a new renderer
//
ParticleRender *TrailRenderClass::Build( Particle *particle, void *data) // = NULL)
{
	// return the new sprite renderer
	return new TrailRender(this, particle, data);
}


//
// TrailRender::TrailRender
//
TrailRender::TrailRender( TrailRenderClass *p, Particle *particle, void *data) // = NULL)
 : ParticleRender(p, particle, data),  trail(&TrailSegment::node)
{
//  colorAnim.Setup( p->data.animRate, &p->colorKeys, &p->data, p->data.animFlags);
//  scaleAnim.Setup( p->data.animRate, &p->scaleKeys, &p->data, p->data.animFlags);

  F32 lifeTime = particle->proto->lifeTime;

  colorAnim.Setup( lifeTime, &p->colorKeys, &p->data, p->data.animFlags);
  scaleAnim.Setup( lifeTime, &p->scaleKeys, &p->data, p->data.animFlags);

	// initial frame
	frame = 0.0f;

	// initial u value
  u = Random::nonSync.Float(); // 0.0f

	// trail decay rate
  decay = p->hasDecay ? p->decay : p->data.animRate == 0 ? 0 : particle->proto->lifeTime / p->data.animRate;

	// store starting position
	lastpos = particle->matrix.posit;

  firstTime = TRUE;
}


//
// TrailRender::~TrailRender
//
TrailRender::~TrailRender()
{
  // Delete all segments
  trail.DisposeAll();
}


//
// Detach trail renderer
//
void TrailRender::Detach(Particle *p)
{
  p;

	// make sure this is the right matrix
	ASSERT(particle == p);

	// clear the matrix
	particle = NULL;
}


//
// Simulate trail renderer
//
void TrailRender::Simulate( F32 dt)
{
	ParticleRender::Simulate(dt);

	// get class data
	TrailRenderClass *pclass = (TrailRenderClass *)proto;

	// for each trail segment...
  TrailSegment *segment;

  NList<TrailSegment>::Iterator i(&trail);

	while ((segment = i++) != NULL)
	{
    if (!colorAnim.SetFrameScaled( segment->frame + dt * colorAnim.OneOverLife() * pclass->data.animRate))
    {
    	// if the timer has expired...
      // erase this segment
			trail.Dispose(segment);
		}
    else
    {
      segment->frame = colorAnim.Current().frame;
      segment->color = colorAnim.Current().color;

      scaleAnim.SetSlave( colorAnim.Current().frame);

      segment->scale = scaleAnim.Current().scale;
    }
	}

	// update starting frame
	frame += decay * dt;
  colorAnim.SetFrame( frame);
  scaleAnim.SetSlave( colorAnim.Current().frame);

	if (particle)
	{
    if (!firstTime && Visible( &particle->matrix.posit) != clipOUTSIDE) // start on 2nd Simulate call
    {
		  // generate direction vector
		  Vector dir = particle->matrix.posit - lastpos;

		  // store matrix position
		  lastpos = particle->matrix.posit;

		  // if the trail has a texture...
		  if (pclass->data.texture)
		  {
			  // update current u value
			  u += pclass->data.uvScale * dir.Magnitude();
		  }

		  // create a new segment
		  TrailSegment *newSeg = new TrailSegment( frame * colorAnim.OneOverLife(), colorAnim.Current().color, scaleAnim.Current().scale, u);
		  ParticleSystem::BuildDirectionalMatrix(newSeg->matrix, particle->matrix.posit, dir);

		  // push the new segment onto the back
		  trail.Append(newSeg);
    }
	}
	else
	{
		// if the trail is empty...
		if (trail.GetCount() == 0)
		{
			// delete this renderer
			delete this;
			return;
		}
	}
  firstTime = FALSE;
}

#if 1

//
// Render trail renderer
//
void TrailRender::Render()
{
  Camera & camera = Vid::CurCamera();

	// get class data
	TrailRenderClass *pclass = (TrailRenderClass *)proto;

	// get the length of the trail
	int length = trail.GetCount();

	// if the trail is long enough...
	if (length > 1)
	{
		// get sprite depth in camera coordinates
    Vector &origin = trail.GetTailNode()->GetData()->matrix.posit;
		F32 depth = camera.ViewMatrix().posit.z + origin.Dot(camera.ViewMatrix().front);

		// set bucket depth
		Vid::SetTranBucketZ(depth);

    U32 heapSize = length * sizeof( Vector) + length * sizeof( Color) + length * sizeof( F32);
    Vector * points = (Vector *) Vid::Heap::Request( heapSize), * p = points;
    Color  * colors = (Color *) (points + length), * c = colors;
    F32    * rads   = (F32 *) (colors + length), * r = rads;

		// for each streamer segment in the trail...
    NList<TrailSegment>::Iterator i(&trail);
		while (TrailSegment * segment = i++)
		{
			*p++ = segment->matrix.posit;
			*c++ = segment->color;
      *r++ = segment->scale;
    }

    Vid::RenderBeam( TRUE, points, length, 1, depth, pclass->data.texture, pclass->data.color, pclass->data.blend, Vid::sortNORMAL0, trail.GetHead()->u, pclass->data.uvScale, FALSE, colors, rads);

    // restore temp memory
    Vid::Heap::Restore( heapSize);
	}
}


#else

//
// Render trail renderer
//
void TrailRender::Render()
{
  Camera & camera = Vid::CurCamera();

	// get class data
	TrailRenderClass *pclass = (TrailRenderClass *)proto;

	// get the length of the trail
	int length = trail.GetCount();

	// if the trail is long enough...
	if (length > 1)
	{
		// set the primitive description
		Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
			DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | pclass->data.blend);

		// set material
    Vid::SetBucketMaterial( Vid::defMaterial);
		Vid::SetBucketTexture( pclass->data.texture, TRUE, 0, pclass->data.blend);

  	// set the world transform matrix
		Vid::SetWorldTransform(Matrix::I);

		// get sprite depth in camera coordinates
    Vector &origin = trail.GetTailNode()->GetData()->matrix.posit;
		F32 depth = camera.ViewMatrix().posit.z + origin.Dot(camera.ViewMatrix().front);

		// set bucket depth
		Vid::SetTranBucketZ(depth);

		// lock primitive memory
    VertexTL * point;
    U16 * index;
    U32 heapSize = Vid::Heap::ReqVertex( &point, length * 4, &index, (length - 1) * 12);

		// start with first vertices
		VertexTL * ppoint = point;
		U16 * pindex = index;
		U16 offset = 0;

		// for each streamer segment in the trail...
		for (NList<TrailSegment>::Iterator i(&trail); *i; i++)
		{
			// get the streamer segment
			TrailSegment &segment = **i;

			// get the transform matrix
			Matrix &M = segment.matrix;

			// top vertex
			ppoint->vv       = M.posit + (M.up * segment.scale);
			ppoint->diffuse  = segment.color;
			ppoint->specular = 0xFF000000;
			ppoint->u        = segment.u;
			ppoint->v        = 0.0f;
			ppoint++;

			// bottom vertex
			ppoint->vv       = M.posit - (M.up * segment.scale);
			ppoint->diffuse  = segment.color;
			ppoint->specular = 0xFF000000;
			ppoint->u        = segment.u;
			ppoint->v        = 1.0f;
			ppoint++;

			// right vertex
			ppoint->vv       = M.posit + (M.right * segment.scale);
			ppoint->diffuse  = segment.color;
			ppoint->specular = 0xFF000000;
			ppoint->u        = segment.u + 0.5f;
			ppoint->v        = 0.0f;
			ppoint++;

			// left vertex
			ppoint->vv       = M.posit - (M.right * segment.scale);
			ppoint->diffuse  = segment.color;
			ppoint->specular = 0xFF000000;
			ppoint->u        = segment.u + 0.5f;
			ppoint->v        = 1.0f;
			ppoint++;
		}

		// generate index list
		for (int l = 1; l < length; l++)
		{
			// vertical triangles
			*pindex++ = U16(offset);
			*pindex++ = U16(offset + 1);
			*pindex++ = U16(offset + 4);
			*pindex++ = U16(offset + 5);
			*pindex++ = U16(offset + 4);
			*pindex++ = U16(offset + 1);

			// horizontal triangles
			*pindex++ = U16(offset + 2);
			*pindex++ = U16(offset + 3);
			*pindex++ = U16(offset + 6);
			*pindex++ = U16(offset + 7);
			*pindex++ = U16(offset + 6);
			*pindex++ = U16(offset + 3);
	
			// move to the next offset
			offset += 4;
		}

#ifdef DOSTATISTICS
    Statistics::tempTris = 0;
#endif

		// submit the primitives
		Vid::ProjectClip( point, ppoint - point, index, pindex - index);

    // restore temp memory
    Vid::Heap::Restore( heapSize);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + Statistics::tempTris;
#endif

	}
}
#endif