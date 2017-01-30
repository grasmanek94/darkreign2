///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// anim.h
//
// 13-JUL-1998
//

#ifndef __ANIM_H
#define __ANIM_H

#include "family.h"
//----------------------------------------------------------------------------

extern Bool OptimizeKeys( AnimKey * src, U32 &srcCount, AnimKeyEnum type);
extern Bool CombineKeys( AnimKey * dst, U32 &dstCount, const AnimKey * src, U32 srcCount);

class Animation
{
//protected:
public:
  F32                     maxFrame;     // the highest key number

  Array<AnimKey>          keys;

  U32                     index;        // which mesh in a group 

public:
  Animation()
  {
    ClearData();
  }
  ~Animation()
  {
    Release();
  }

  void ClearData()
  {
    maxFrame = 0.0f;
    index = 0;
  }
  void Release()
  {
    keys.Release();
    ClearData();
  }

  U32 GetMem() const;

  Bool SetKeys( const AnimKey *inKeys, U32 count);

  void SetFrame( F32 frame, AnimKey & key) const;
  void SetFrameOverlay( F32 frame, AnimKey & key, F32 controlFrame) const;

  static void Set( F32 frame, const AnimKey & lastKey, const AnimKey & thisKey, AnimKey & key);    

  inline void SetFrameObject( F32 frame, Array<FamilyState> & stateArray) const
  {
    FamilyState & state = stateArray[index];

    SetFrame( frame, state);
    state.SetObject();
    state.SetObjectScale();
  }
  inline void SetFrame( F32 frame, Array<FamilyState> & stateArray) const
  {
    SetFrame( frame, stateArray[index]);
  }
  inline void SetFrame( F32 frame, Array<AnimKey> & keyArray) const
  {
    SetFrame( frame, keyArray[index]);
  }

  inline void SetFrameOverlay( F32 frame, Array<FamilyState> & stateArray, F32 controlFrame) const
  {
    SetFrameOverlay( frame, stateArray[index], controlFrame);
  }
  inline void SetFrameOverlay( F32 frame, Array<AnimKey> & keyArray, F32 controlFrame) const
  {
    SetFrameOverlay( frame, keyArray[index], controlFrame);
  }

  // local interpolation 
  //
  inline void Setup( U32 _index = 0, U32 count = 2)
  {
    maxFrame = 1.0f;
    index = _index;
    keys.Alloc( count);
  }
  inline void SetInterpolate( F32 frame, FamilyState * states) const 
  {
    ASSERT( keys.count == 2);
    Set( frame, keys[0], keys[1], states[index]);
  }

  inline U32 Type() const 
  {
    return keys[0].type;
  }
  inline Bool IsValid() const
  {
    return keys[0].frame < keys[1].frame; 
  }

};
//----------------------------------------------------------------------------

enum AnimType
{
  animLOOP,
  anim2WAY,
  anim1WAY,
  animCONTROL,
};

class AnimList : public List<Animation>
{
public:
  NBinTree<AnimList>::Node cycleNode;

  F32                     maxFrame;     // the highest key number
  F32                     endFrame;     // last non-loop frame
  Array<AnimKey>          states;       // starting states for this cycle

  AnimType                type;

  GameIdent               name;

  F32                     animSpeed, framesPerMeter;
  F32                     controlFrame;  // usually middle

  Bounds                  bounds;        // bounding bounds for this cycle

  void ClearData()
  {
    maxFrame = endFrame = 0.0f;
    type = animLOOP;
    animSpeed = 40.0f;
    framesPerMeter = 0.0f;
    controlFrame = 0.0f;

    bounds.ClearData();
  }

  AnimList() : List<Animation>()
  {
    ClearData();
  }
  void Release()
  {
    states.Release();
    DisposeAll();
  }
  ~AnimList()
  {
    Release();
  }

  void SetType( AnimType _type)
  {
    type = _type;
  }

  void SetupStates( const Array<FamilyState> & stateArray);
  void SetupMaxFrame();

  void SetFrame( F32 frame, Array<FamilyState> & stateArray) const;
  void SetFrameObject( F32 frame, Array<FamilyState> & stateArray) const;
  void SetBlend( F32 frame, AnimKey * keys0, AnimKey * keys1, Array<FamilyState> & stateArray) const;

  void SetFrame( F32 frame, Array<AnimKey> & keyArray) const;
  void SetBlend( F32 frame, AnimKey * keys0, AnimKey * keys1, Array<AnimKey> & keyArray) const;

  void SetFrameOverlay( F32 frame, Array<FamilyState> & stateArray) const;
  void SetFrameOverlay( F32 frame, Array<AnimKey> & keyArray) const;

  U32 GetMem() const;
};
//----------------------------------------------------------------------------

class AnimCycles : public NBinTree<AnimList>
{
public:
  U32 GetMem() const;
};
//----------------------------------------------------------------------------

#endif  // __ANIM_H
