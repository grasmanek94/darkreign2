///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// 
//
// 
//


#ifndef __EFFECTSUTILS_H
#define __EFFECTSUTILS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"
#include "utiltypes.h"
#include "list.h"

// forward references
//
class Bitmap;
class MeshRoot;
namespace Effects
{
  enum Flags
  {
    flagNONE     = 0x00,
    flagDESTROY  = 0x01,
    flagLOOP     = 0x02,
    flagCALLBACK = 0x04,
  };

  struct Data;
}
struct KeyFrame;
struct ColorKey;
struct ScaleKey;

///////////////////////////////////////////////////////////////////////////////
//
// KeyList template
//
// list of DATA as animation keys
// Effects::Data performs default configuration
//
// max frame is always 1.0f
//
// DATA must have 'frame' and 'Interpolate()' members
//
template <class DATA> class KeyList : public List<DATA>
{
protected:
  friend Effects::Data;

public:

  F32 data;   // for Effects::Data.Configure()

  KeyList<DATA>()
  {
    data = 0.0f;
  }

  ~KeyList<DATA>()
  {
    Release();
  }

  void Release()
  {
    DisposeAll();
  }

  void PostLoad()
  {
    F32 maxFrame = 0.0f;

    // find the maxframe
    //
    List<DATA>::Iterator key(this);
    F32 lastFrame = -1.0f;
    for (!key; *key; key++)
    {
      ASSERT( (*key)->frame > lastFrame );
      lastFrame = (*key)->frame;

      if ((*key)->frame > maxFrame)
      {
        maxFrame = (*key)->frame;
      }
    }

    // normalize frame values between 0 - 1
    //
    maxFrame = 1.0f / maxFrame;

    for (!key; *key; key++)
    {
      (*key)->frame *= maxFrame;
    }
  }

  U32 GetMem() const
  {
    return sizeof( *this) + count * sizeof(DATA);
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// KeyAnim template
//
// keyframing iterator for KeyList of DATA
// if list == NULL it just acts as a timer
// Effects::Data does default configuration
//
// max frame is always 1.0f
//
// DATA must have 'frame' and 'Interpolate()' members
//
template <class DATA> class KeyAnim
{
protected:
  friend Effects::Data;

  const KeyList<DATA> *list;
  List<DATA>::Iterator currKey, nextKey;

  DATA current;

  F32 oneOverLife;              // 'lifeTime' scaling factor
  F32 lifeTime;

  U32 flags;

public:

  KeyAnim<DATA>()
  {
    list = NULL;
  }
  KeyAnim<DATA>( F32 _lifeTime, const KeyList<DATA> * _list = NULL, U32 _flags = 0, F32 _frame = 0.0f)
  {
    Setup( _lifeTime, _list, _flags, _frame);
  }

  inline Bool IsLoop()
  {
    return flags & Effects::flagLOOP;
  }
  inline Bool IsDestroy()
  {
    return flags & Effects::flagDESTROY;
  }

  inline F32 OneOverLife() const
  {
    return oneOverLife;
  }
  inline F32 LifeTime() const
  {
    return lifeTime;
  }

  inline F32 CurTime() const
  {
    return current.frame * lifeTime;
  }

  inline const DATA & Current() const 
  {
    return current;
  }
  inline const DATA * CurrKey() const
  {
    return *currKey;
  }
  inline const DATA * NextKey() const
  {
    return *nextKey;
  }

  // current total anim parametric value
  //
  inline F32 Dt() const
  {
    return current.frame;
  }
  inline F32 CurFrame() const
  {
    return current.frame;
  }

  // current frame parametric value
  //
  F32 DtDf() const
  {
    if (list)
    {
      return (current.frame - CurrKey()->frame) / (NextKey()->frame - CurrKey()->frame);
    }
    return current.frame;
  }

  // 'dt' in Simulate calls will be scaled so maxFrame is reached at 'lifeTime'
  //
  void Setup( F32 _lifeTime, const KeyList<DATA> *_list = NULL, const Effects::Data * data = NULL, U32 _flags = Effects::flagDESTROY, F32 frame = 0.0f)
  {
//    ASSERT( _list.GetCount() >= 2);

    ASSERT( _lifeTime >= 0.0f);

    current.frame = 0.0f;
    lifeTime = _lifeTime;
    oneOverLife = lifeTime == 0.0f ? 1.0f : 1.0f / lifeTime;

    flags = _flags;

    if (_list)
    {
      list = _list;
      currKey.SetList( list);
      nextKey.SetList( list);
      nextKey++;
    }
    else if (data)
    {
      current.Setup( *data);
    }

    SetFrame( frame);
  }

  // clamps frame; returns FALSE if frame >= maxFrame
  //
  // expects pre-scaled frame value
  //
  Bool SetFrameScaled( F32 frame)
  {
    Bool retValue = TRUE;

    // clamp frame
    if (frame >= 1.0f)
    {
      if (flags & Effects::flagLOOP)
      {
        frame = (F32) fmod( frame, 1.0f);
      }
      else
      {
        frame = 1.0f;
      }

      retValue = FALSE;
    }

    SetSlave( frame);

    return retValue;
  }

  // doesn't do clamp check; doesn't scale frame
  //
  void SetSlave( F32 frame)
  {
    current.frame = frame;

    if (list)
    {
      if ((*currKey)->frame > current.frame)
      {
        // start at the beginning
        //
        !currKey;
        nextKey = currKey;
        nextKey++;
      }

      while ((*nextKey)->frame < current.frame)
      {
        currKey = nextKey;
        nextKey++;
      }

      Interpolate();
    }
  }

  // clamps frame; returns FALSE if frame >= maxFrame
  //
  // scales frame to normalized value (0.0f-1.0f)
  //
  inline Bool SetFrame( F32 frame)
  {
    return SetFrameScaled( frame * oneOverLife);
  }

  // call DATA's 'Interpolate()' member
  //
  inline void Interpolate()
  {
    current.Interpolate( *CurrKey(), *NextKey(), DtDf());
  }

  // returns FALSE if past maxFrame
  //
  inline Bool Simulate( F32 seconds, F32 modifier = 1)
  {
    // scale 'seconds' to normalized value
    //
    return SetFrameScaled( current.frame + seconds * oneOverLife * (IsLoop() ? modifier : 1));
  }
  inline Bool Process( F32 percentDone)
  {
    // scale 'seconds' to normalized value
    //
    return SetFrameScaled( percentDone);
  }
};

// basic timer object
//
typedef KeyAnim<KeyFrame> FrameAnim;


///////////////////////////////////////////////////////////////////////////////
//
// namespace Effects
//

namespace Effects
{

  namespace Blend
  {
    //
    // Convert a blend string to a D3D blend value
    //
    U32  GetValue( const char * blendStr, U32 defval);
    void GetString( U32 blend, GameIdent & string, Bool defs = TRUE);
  }

  enum DataFlags
  {
    dataNONE      = 0x0000,
    dataOBJECT    = 0x0001,
    dataTEXTURE   = 0x0002,
    dataBLEND     = 0x0004,
    dataCOLOR     = 0x0008,
    dataSCALE     = 0x0010,
    dataANIMTIME  = 0x0020,
    dataANIMRATE  = 0x0040,
    dataVECTOR    = 0x0080,
    dataHEIGHT    = 0x0100,
  };

  // basic configuration data for effects types
  // default configuration for KeyList types
  //
  struct Data
  {
    U32 flags;

    GameIdent objectId;
    MeshRoot * root;

    GameIdent textureId;
    Bitmap * texture;
  	U32 textReduce;

  	U32 blend;
    Color color;
  	F32 scale, height;

  	F32 animRate;
  	F32 lifeTime;
  	F32 texTime;

    F32 uvScale;
    F32 uvAnimRate;

    U32 noRestore   : 1;
    U32 noX         : 1;
    U32 noY         : 1;
    U32 noZ         : 1;
    U32 noZBuffer   : 1;
    U32 interpolate : 1;

    Vector vector;
    Vector offset;

    U32 animFlags;
    U32 sorting;

    F32 random;

    Data();

    Bool Configure( FScope *fScope, U32 counter = 1);
    Bool Configure( FScope *fScope, KeyList<ColorKey> &keys, U32 counter = 1);
    Bool Configure( FScope *fScope, KeyList<ScaleKey> &keys, U32 counter = 1);

    void PostLoad();
    void PostLoad( KeyList<ColorKey> &keys);
    void PostLoad( KeyList<ScaleKey> &keys);
  };
}

//
// key-framing base data
//
struct KeyFrame
{
  F32 frame;

  void Setup( const Effects::Data &data) 
  {
    data;
  } 
  void Interpolate( const KeyFrame &k1, const KeyFrame &k2, F32 dt)
  {
    k1;
    k2;
    dt;
  }
};

///////////////////////////////////////////////////////////////////////////////
//
// struct ColorKey
//

struct ColorKey : public KeyFrame
{
  Color color;

  ColorKey()
  {
    frame = 0.0f;
    color = 0xffffffff;
  }
  ColorKey( F32 f, U32 r, U32 g, U32 b, U32 a)
  {
    frame = f;
    color.Set( r, g, b, a);
  }
  ColorKey( F32 f, Color c)
  {
    frame = f;
    color = c;
  }
  ColorKey( F32 f, U32 c)
  {
    frame = f;
    color = c;
  }
  void Setup( const Effects::Data & data)
  {
    color = data.color;
  }

  void Interpolate( const ColorKey &k1, const ColorKey &k2, F32 dt)
  {
    color.Interpolate( k1.color, k2.color, dt);
  }
};

///////////////////////////////////////////////////////////////////////////////
//
// struct ScaleKey
//

struct ScaleKey : public KeyFrame
{
  F32 scale;

  ScaleKey()
  {
    frame = 0.0f;
    scale = 1.0f;
  }
  ScaleKey( F32 f, F32 s)
  {
    frame = f;
    scale = s;
  }
  void Setup( const Effects::Data & data)
  {
    scale = data.scale; 
  }

  void Interpolate( const ScaleKey &k1, const ScaleKey &k2, F32 dt)
  {
    scale = k1.scale + dt * (k2.scale - k1.scale);
  }
};


#endif
