///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Godfile
//
// 21-MAY-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "godfile.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace God - God reading utility templates
//
namespace God
{

  //
  // Sphere
  //
  template<> void Load<Sphere>(GodFile &god, Sphere &sphere)
  {
    // matrix plus box size and radius
    Load(god, sphere.radius);
    Load(god, (Matrix &)sphere);
    Load(god, sphere.Width);
    Load(god, sphere.Height);
    Load(god, sphere.Breadth);

    // Setup radius
    sphere.radius2 = sphere.radius * sphere.radius;
  }

  template<> void Save<Sphere>(GodFile &god, const Sphere &sphere)
  {
    Save(god, sphere.radius);
    Save(god, (Matrix)sphere);
    Save(god, sphere.Width);
    Save(god, sphere.Height);
    Save(god, sphere.Breadth);
  }

  //
  // Bounds
  //
  template<> void Load<Bounds>(GodFile &god, Bounds &bounds)
  {
    F32 v0, v1, v2;
    Load(god, v0);
    bounds.SetRadius( v0);

    Load(god, v0);
    Load(god, v1);
    Load(god, v2);
    bounds.Set( v0, v1, v2);

    Vector offset;
    Load(god, offset);
    bounds.SetOffset( offset);
  }

  template<> void Save<Bounds>(GodFile &god, const Bounds &bounds)
  {
    Save(god, bounds.Radius());
    Save(god, bounds.Width());
    Save(god, bounds.Height());
    Save(god, bounds.Breadth());
    Save(god, bounds.Offset());
  }

  //
  // Animkey
  //
  template<> void Load<AnimKey>(GodFile &god, AnimKey &key)
  {
    key.ClearData();

    Load(god, key.frame);
    key.type = god.LoadU32();
    Load(god, key.quaternion);
    Load(god, key.position);

    if (god.version > 1)
    {
      // new versions
      //
      Load(god, key.scale);
    }
  }

  template<> void Save<AnimKey>(GodFile &god, const AnimKey &key)
  {
    Save(god, key.frame);
    Save(god, U32(key.type));
    Save(god, key.quaternion);
    Save(god, key.position);

    Save(god, key.scale);
  }

  //
  // FaceObj
  //
  template<> void Load<FaceObj>(GodFile &god, FaceObj &face)
  {
    face.ClearData();

    U16 bi;
    God::Load(god, bi);
    face.buckyIndex = (U8) bi;
    god.LoadData(face.verts, 3 * sizeof(U16));
    god.LoadData(face.norms, 3 * sizeof(U16));
    god.LoadData(face.uvs, 3 * sizeof(U16));
  }

  template<> void Save<FaceObj>(GodFile &god, const FaceObj &face)
  {
    God::Save(god, U16(face.buckyIndex));
    god.SaveData(&face.verts, 3 * sizeof(U16));
    god.SaveData(&face.norms, 3 * sizeof(U16));
    god.SaveData(&face.uvs, 3 * sizeof(U16));
  }

  //
  // VertGroup
  //
  template<> void Load<VertGroup>(GodFile &god, VertGroup &vertGroup)
  {
    vertGroup.ClearData();

    vertGroup.stateIndex = (U16)god.LoadU32();
    vertGroup.vertCount  = (U16)god.LoadU32();
    vertGroup.indexCount = (U16)god.LoadU32();
    vertGroup.planeIndex = (U16)god.LoadU32();

    if (god.Peek() == 0x9709513E) // "Material"
    {
      god.Skip(4);
      vertGroup.material = Material::Manager::Load(&god);
    }
    else
    {
      vertGroup.material = NULL;
    }

    if (god.Peek() == 0x7951FC0B) // "Texture"
    {
      god.Skip(4);
      vertGroup.texture  = Bitmap::Manager::Load( Bitmap::reduceHIGH, &god);
    }
    else
    {
      vertGroup.texture = NULL;
    }
  }

  template<> void Save<VertGroup>(GodFile &god, const VertGroup &vertGroup)
  {
    God::Save(god, U32(vertGroup.stateIndex));
    God::Save(god, U32(vertGroup.vertCount));
    God::Save(god, U32(vertGroup.indexCount));
    God::Save(god, U32(vertGroup.planeIndex));

    if (vertGroup.material)
    {
      God::Save(god, U32(0x9709513E)); // "Material"
      Material::Manager::Save( &god, *vertGroup.material);
    }
    if (vertGroup.texture)
    {
      God::Save(god, U32(0x7951FC0B)); // "Texture"
      Bitmap::Manager::Save( &god, *vertGroup.texture);
    }
  }

  //
  // VertIndex
  //
  template<> void Load<VertIndex>(GodFile &god, VertIndex &vertIndex)
  {
    U32 slack = 0;
    U32 count = god.LoadU32();

    if (count > MAXWEIGHTS)
    {
      slack = count - MAXWEIGHTS;
      count = MAXWEIGHTS;
    }

    vertIndex.count = count;

    for (U32 i = 0; i < count; i++)
    {
      God::Load(god, vertIndex.weight[i]);
      God::Load(god, vertIndex.index[i]);
    }

    if (slack)
    {
      god.Skip((sizeof(U16) + sizeof(F32)) * slack);
    }
  }

  template<> void Save<VertIndex>(GodFile &god, const VertIndex &vertIndex)
  {
    God::Save(god, U32(vertIndex.count));

    for (U32 i = 0; i < vertIndex.count; i++)
    {
      God::Save(god, F32(vertIndex.weight[i]));
      God::Save(god, U16(vertIndex.index[i]));
    }
  }

  //
  //  BucketLock
  //
  template<> void Load<BucketDesc>(GodFile &god, BucketDesc &bucky)
  {
    God::Load(god, bucky.flags0);
    God::Load(god, bucky.vertCount);
    God::Load(god, bucky.indexCount);

    if (god.Peek() == 0x9709513E) // "Material"
    {
      god.Skip(4);
      bucky.material = Material::Manager::Load(&god);
//      bucky.material->DoSetSpecular( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
      bucky.diff = bucky.material->Diffuse();
    }
    else
    {
      bucky.material = NULL;
    }

    bucky.flags1   = RS_BLEND_DECAL;

    if (god.version > 1)
    {
      // new versions
      //
      if (god.Peek() == 0xF644CB33) // "Texture0"
      {
        god.Skip(4);
        bucky.texture0  = Bitmap::Manager::Load( Bitmap::reduceHIGH, &god);
      }
      else
      {
        bucky.texture0  = NULL;
      }

      if (god.Peek() == 0xF285D684) // "Texture1"
      {
        god.Skip(4);
        bucky.texture1 = Bitmap::Manager::Load( Bitmap::reduceHIGH, &god);
        bucky.flags1   = RS_BLEND_DECAL;
      }
      else
      {
        bucky.texture1  = NULL;
        bucky.flags1    = RS_BLEND_DECAL;
      }

      bucky.teamColor = god.LoadU32();
      bucky.envMap = god.LoadU32();
      bucky.overlay = god.LoadU32();
    }
    else
    {
      if (god.Peek() == 0x7951FC0B) // "Texture"
      {
        god.Skip(4);
        bucky.texture0  = Bitmap::Manager::Load( Bitmap::reduceHIGH, &god);
      }
      else
      {
        bucky.texture0  = NULL;
      }

      bucky.texture1  = NULL;

      bucky.teamColor = FALSE;
      bucky.envMap    = FALSE;
      bucky.overlay   = FALSE;
    }

    if (god.version < 3)
    {
      if ((bucky.flags0 & RS_TEX_MASK) != RS_TEX_MODULATE)
      {
        bucky.flags0 &= ~RS_TEX_MASK;
        bucky.flags0 |= RS_TEX_MODULATE;
      }
    }
  }

  template<> void Save<BucketDesc>(GodFile &god, const BucketDesc &bucky)
  {
    God::Save(god, U32(bucky.flags0));
    God::Save(god, U32(bucky.vertCount));
    God::Save(god, U32(bucky.indexCount));

    if (bucky.material)
    {
      God::Save(god, U32(0x9709513E)); // "Material"
      Material::Manager::Save( &god, *bucky.material);
    }
    if (bucky.texture0)
    {
      God::Save(god, U32(0xF644CB33)); // "Texture0"
      Bitmap::Manager::Save( &god, *bucky.texture0);
    }
    if (bucky.texture1)
    {
      God::Save(god, U32(0xF285D684)); // "Texture1"
      Bitmap::Manager::Save( &god, *bucky.texture1);
    }
    God::Save(god, U32(bucky.teamColor));
    God::Save(god, U32(bucky.envMap));
    God::Save(god, U32(bucky.overlay));
  }

  template<> void Load<BucketLock>(GodFile &god, BucketLock &bucky)
  {
    Load<BucketDesc>( god, bucky);
  }

  template<> void Save<BucketLock>(GodFile &god, const BucketLock &bucky)
  {
    Save<BucketDesc>( god, bucky);
  }

}
