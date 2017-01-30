///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// anim.cpp
//
// 14-JUL-1998
//


//
// Includes
//
#include "vid.h"
#include "array.h"
#include "anim.h"
//----------------------------------------------------------------------------

#if 0
    MonoBufWriteV
    (
      buf, (row++, 0, 
      "state:%-12s moveState:%-12s boardState:%-12s slow:%c ",
      state.GetName(), moveState.GetName(), boardState.GetName(), slowingDown ? 'Y' : 'N')
    );
#endif

// remove useless keys
//
Bool OptimizeKeys( AnimKey *src, U32 &srcCount, AnimKeyEnum type)
{
  if (srcCount == 0)
  {
    return TRUE;
  }

  const F32 ROTKEYDOTTHRESH = 0.99999f;

  const F32 POSKEYDOTTHRESH = 0.001f;
  const F32 SCLKEYDOTTHRESH = 0.001f;
  S32 i;

  if (type & animPOSITION)
  {
    for (i = 0; i < S32(srcCount-2); i++)
    {
      AnimKey &k1 = src[i+0];
      AnimKey &k2 = src[i+1];
      AnimKey &k3 = src[i+2];

      if (fabs(k1.position.x - k2.position.x) < POSKEYDOTTHRESH
       && fabs(k1.position.y - k2.position.y) < POSKEYDOTTHRESH
       && fabs(k1.position.z - k2.position.z) < POSKEYDOTTHRESH)
      {
        if (fabs(k2.position.x - k3.position.x) < POSKEYDOTTHRESH
         && fabs(k2.position.y - k3.position.y) < POSKEYDOTTHRESH
         && fabs(k2.position.z - k3.position.z) < POSKEYDOTTHRESH)
        {
          if (i < S32(srcCount - 2))
          {
            Utils::Memcpy( (void *)&k2, (void *)&k3, (srcCount - i - 1) * sizeof(AnimKey));
            i--;
          }
          srcCount--;
        }
      }
    }
  }
  else if (type & animQUATERNION)
  {
    for (i = 0; i < S32(srcCount-2); i++)
    {
      AnimKey &k1 = src[i+0];
      AnimKey &k2 = src[i+1];
      AnimKey &k3 = src[i+2];

      if (k1.quaternion.Dot(k2.quaternion) > ROTKEYDOTTHRESH)
      {
        if (k2.quaternion.Dot(k3.quaternion) > ROTKEYDOTTHRESH)
        {
          if (i < S32(srcCount - 2))
          {
            Utils::Memcpy( (void *)&k2, (void *)&k3, (srcCount - i - 1) * sizeof(AnimKey));
            i--;
          }
          srcCount--;
          continue;
        }
      }
      ASSERT( k1.quaternion.Dot( k2.quaternion) >= 0);
    }
  }
  else if (type & animSCALE)
  {
    for (i = 0; i < S32(srcCount-2); i++)
    {
      AnimKey &k1 = src[i+0];
      AnimKey &k2 = src[i+1];
      AnimKey &k3 = src[i+2];

      if (fabs(k1.scale.x - k2.scale.x) < SCLKEYDOTTHRESH
       && fabs(k1.scale.y - k2.scale.y) < SCLKEYDOTTHRESH
       && fabs(k1.scale.z - k2.scale.z) < SCLKEYDOTTHRESH)
      {
        if (fabs(k2.scale.x - k3.scale.x) < SCLKEYDOTTHRESH
         && fabs(k2.scale.y - k3.scale.y) < SCLKEYDOTTHRESH
         && fabs(k2.scale.z - k3.scale.z) < SCLKEYDOTTHRESH)
        {
          if (i < S32(srcCount - 2))
          {
            Utils::Memcpy( (void *)&k2, (void *)&k3, (srcCount - i - 1) * sizeof(AnimKey));
            i--;
          }
          srcCount--;
        }
      }
    }
    if (srcCount == 2 && src[0].scale == src[1].scale && src[1].scale == Vector(1.0f, 1.0f, 1.0f))
    {
      srcCount = 0;
    }
  }
  return TRUE;
}
//----------------------------------------------------------------------------

Bool CombineKeys( AnimKey *dst, U32 &dstCount, const AnimKey *src, U32 srcCount)
{
  if (srcCount < 2 || (srcCount == 2 
    && src[0].quaternion  == src[1].quaternion 
    && src[0].position  == src[1].position
    && src[0].scale == src[1].scale))
  {
    // no keys
    return TRUE;
  }

  // build a new combined key list in dst
  U32 i, j;
  for (i = j = 0; i < srcCount; )
  {
    if (j >= dstCount)
    {
      dst[j] = src[i];
      dst[j].type = src[i].type;
      if (j > 0)
      {
        if ((dst[j - 1].type & animQUATERNION) && !(dst[j].type & animQUATERNION))
        {
          dst[j].quaternion = dst[j - 1].quaternion;
          dst[j].type |= animQUATERNION;
        }
        if ((dst[j - 1].type & animPOSITION) && !(dst[j].type & animPOSITION))
        {
          dst[j].position = dst[j - 1].position;
          dst[j].type |= animPOSITION;
        }
        if ((dst[j - 1].type & animSCALE) && !(dst[j].type & animSCALE))
        {
          dst[j].scale = dst[j - 1].scale;
          dst[j].quaternion  = dst[j - 1].quaternion;
          dst[j].type |= animSCALE;
        }
      }
      dstCount++;
      i++;
      j++;
    }
    else if (src[i].frame == dst[j].frame)
    {
      if (src[i].type & animQUATERNION)
      {
        dst[j].quaternion = src[i].quaternion;
        dst[j].type |= animQUATERNION;
      }
      if (src[i].type & animPOSITION)
      {
        dst[j].position = src[i].position;
        dst[j].type |= animPOSITION;
      }
      if (src[i].type & animSCALE)
      {
        dst[j].scale = src[i].scale;
        dst[j].type |= animSCALE;
      }
      j++;
      i++;
    }
    else if (src[i].frame < dst[j].frame)
    {
      memmove( &dst[j + 1], &dst[j], (dstCount - j) * sizeof( AnimKey));
      dst[j] = src[i];
      dstCount++;
      j++;
      i++;
    }
    else
    {
      j++;
      if (j >= dstCount && i >= srcCount)
      {
        ERR_FATAL(("Unexpected error in anim (CombineKeys)"));
      }
    }
  }

  // interpolate and fill in missing key values
  AnimKey extra = dst[dstCount - 1];
  extra.frame += 1.0f;
  extra.type |= animQUATERNION | animPOSITION | animSCALE;

  AnimKey *lastKey = &dst[0];

  if (lastKey->type & animSCALE)
  {
    ASSERT( !(lastKey->quaternion.s == 0.0f
      && lastKey->quaternion.v.x == 0.0f
      && lastKey->quaternion.v.y == 0.0f
      && lastKey->quaternion.v.y == 0.0f));
  }

  AnimKey *rot0 = lastKey;
  for (j = 1; j < dstCount; j++)
  {
    AnimKey *thisKey = &dst[j];

    if (!(thisKey->type & animQUATERNION) && (lastKey->type & animQUATERNION))
    {
      // interpolate thisKey's ROT key
      AnimKey *thisRot = NULL;
      // find the next rot
      for (i = j + 1; i < dstCount; i++)
      {
        thisRot = &dst[i];
        if (thisRot->type & animQUATERNION)
        {
          break;
        }
      }
      if (i == dstCount)
      {
        thisRot = &extra;
      }
      // determine the current fraction to this cycle
      F32 dk = thisRot->frame - lastKey->frame;
      F32 dfdk  = (thisKey->frame - lastKey->frame) / dk;

      // do a parametric interpolation of the quaternion using dfdk
      Quaternion q = thisRot->quaternion;
      q -= lastKey->quaternion;
      q *= dfdk;
      q += lastKey->quaternion;

      thisKey->quaternion = q;
      thisKey->type |= animQUATERNION;

      rot0 = thisKey;
    }
    if (!(thisKey->type & animPOSITION) && (lastKey->type & animPOSITION))
    {
      // interpolate thisKey's POS key
      AnimKey *thisPos = NULL;
      // find the next rot
      for (i = j + 1; i < dstCount; i++)
      {
        thisPos = &dst[i];
        if (thisPos->type & animPOSITION)
        {
          break;
        }
      }
      if (i == dstCount)
      {
        thisPos = &extra;
      }
      // determine the current fraction to this cycle
      F32 dk = thisPos->frame - lastKey->frame;
      F32 dfdk  = (thisKey->frame - lastKey->frame) / dk;

      // do a parametric interpolation of the quaternion using dfdk
      Vector v = thisPos->position;
      v -= lastKey->position;
      v *= dfdk;
      v += lastKey->position;

      thisKey->position = v;
      thisKey->type |= animPOSITION;
    }
    if (!(thisKey->type & animSCALE) && (lastKey->type & animSCALE))
    {
      // interpolate thisKey's SCALE key
      AnimKey *thisScale = NULL;
      // find the next scale
      for (i = j + 1; i < dstCount; i++)
      {
        thisScale = &dst[i];
        if (thisScale->type & animSCALE)
        {
          break;
        }
      }
      if (i == dstCount)
      {
        thisScale = &extra;
      }
      // determine the current fraction to this cycle
      F32 dk = thisScale->frame - lastKey->frame;
      F32 dfdk  = (thisKey->frame - lastKey->frame) / dk;

      // do a parametric interpolation of the quaternion using dfdk
      Vector v = thisScale->scale;
      v -= lastKey->scale;
      v *= dfdk;
      v += lastKey->scale;

      thisKey->scale = v;
      thisKey->type |= animSCALE;
    }
    if (thisKey->type & animSCALE)
    {
      if (!(thisKey->type & animQUATERNION))
      {
        // scale keys need a valid quaternion

        // interpolate thisKey's ROT key
        AnimKey *thisRot = NULL;
        // find the next rot
        for (i = j + 1; i < dstCount; i++)
        {
          thisRot = &dst[i];
          if (thisRot->type & animQUATERNION)
          {
            break;
          }
        }
        if (i == dstCount)
        {
          thisRot = &extra;
        }
        // determine the current fraction to this cycle
        F32 dk = thisRot->frame - rot0->frame;
        F32 dfdk  = (thisKey->frame - rot0->frame) / dk;

        // do a parametric interpolation of the quaternion using dfdk
        Quaternion q = thisRot->quaternion;
        q -= rot0->quaternion;
        q *= dfdk;
        q += rot0->quaternion;

        thisKey->quaternion = q;
        thisKey->type |= animQUATERNION;

        rot0 = thisKey;
      }

      ASSERT( !(thisKey->quaternion.s == 0.0f
        && thisKey->quaternion.v.x == 0.0f
        && thisKey->quaternion.v.y == 0.0f
        && thisKey->quaternion.v.y == 0.0f));
    }

    lastKey = thisKey;
  }

  return TRUE;
}
//----------------------------------------------------------------------------

Bool Animation::SetKeys( const AnimKey * inKeys, U32 count)
{
  if (count < 2 || (count == 2 
    && inKeys[0].quaternion  == inKeys[1].quaternion 
    && inKeys[0].position  == inKeys[1].position
    && inKeys[0].scale == inKeys[1].scale))
  {
    // no keys
    return TRUE;
  }
  // delete old keys
  keys.Release();

  if (!keys.Alloc( count + 1))
  {
    return FALSE;
  }
  F32 offset = inKeys[0].frame;

  // assign the new keys
  U32 i;
  for (i = 0; i < count; i++)
  {
    keys[i] = inKeys[i];

    // force animation to start at frame zero
    keys[i].frame -= offset;
  }

  // copy the first key to the end for a cycle
  // FIXME
  //
  keys[count] = inKeys[0];
  keys[count].frame = keys[count - 1].frame + 1;
  maxFrame = keys[count].frame;

  AnimKey & key0 = keys[count-1];
  AnimKey & key1 = keys[count];
  F32 dot = key0.quaternion.Dot( key1.quaternion);
  if (dot < 0)
  {
    key1.quaternion *= -1.0f;
  }

  AnimKey & key2 = keys[keys.count-1];

  dot = key1.quaternion.Dot( key2.quaternion);

  if ((key1.type == animQUATERNION) && (key2.type == animQUATERNION) && key1.quaternion == key2.quaternion)
  {
    Array<AnimKey> kkeys(keys.count-1);
    for (U32 i = 0; i < keys.count-1; i++)
    {
      kkeys[i] = keys[i];
    }
    kkeys.Swap( keys);
  }
  
  dot = keys[keys.count-2].quaternion.Dot( keys[keys.count-1].quaternion);
  if (dot < 0)
  {
    keys[keys.count-1].quaternion *= -1.0f;
  }

  maxFrame = keys[keys.count-1].frame;

  return TRUE;
}
//----------------------------------------------------------------------------

void Animation::SetFrame( F32 frame, AnimKey &state) const
{
  U32 i;
  for (i = 1; i < keys.count; i++)
  {
    AnimKey &lastKey = keys[i - 1];
    AnimKey &thisKey = keys[i];

    if (frame <= thisKey.frame)
    {
//      LOG_DIAG( ("%f ; %f ; %f", lastKey.frame, frame, thisKey.frame) );

      ASSERT( lastKey.frame < thisKey.frame);

      Set( frame, lastKey, thisKey, state);
      break;
    }
  }
}
//----------------------------------------------------------------------------

void AnimList::SetupStates( const Array<FamilyState> & stateArray)
{
  states.Alloc( stateArray.count);

  U32 i;
  for (i = 0; i < stateArray.count; i++)
  {
    states[i] = stateArray[i];
  }
}
//----------------------------------------------------------------------------

void AnimList::SetupMaxFrame()
{
  List<Animation>::Iterator li(this); 

  maxFrame = 1;
  for (!li; *li; li++)
  {
    Animation &anim = *(*li);

    if (anim.maxFrame > maxFrame)
    {
      maxFrame = anim.maxFrame;
    }
  }
  for (!li; *li; li++)
  {
    Animation &anim = *(*li);

    anim.maxFrame = maxFrame;
  }
  endFrame = maxFrame - 1.0f;
}
//----------------------------------------------------------------------------

void AnimList::SetFrameOverlay( F32 frame, Array<FamilyState> & stateArray) const
{
  List<Animation>::Iterator li(this); 

  for (!li; *li; li++)
  {
    Animation &anim = *(*li);
    anim.SetFrameOverlay( frame, stateArray, controlFrame);
  }
}
//----------------------------------------------------------------------------

void AnimList::SetFrameOverlay( F32 frame, Array<AnimKey> & keyArray) const
{
  List<Animation>::Iterator li(this); 

  for (!li; *li; li++)
  {
    Animation &anim = *(*li);
    anim.SetFrameOverlay( frame, keyArray, controlFrame);
  }
}
//----------------------------------------------------------------------------
U32 statQuatErrorIndex;

void AnimList::SetFrame( F32 frame, Array<FamilyState> & stateArray) const
{
  List<Animation>::Iterator li(this); 

  for (!li; *li; li++)
  {
    Animation &anim = *(*li);

    anim.SetFrame( frame, stateArray);

    statQuatErrorIndex = anim.index;  
  }
}
//----------------------------------------------------------------------------

void AnimList::SetFrameObject( F32 frame, Array<FamilyState> & stateArray) const
{
  List<Animation>::Iterator li(this); 

  for (!li; *li; li++)
  {
    Animation &anim = *(*li);

    anim.SetFrameObject( frame, stateArray);

    statQuatErrorIndex = anim.index;  
  }
}
//----------------------------------------------------------------------------

void AnimList::SetFrame( F32 frame, Array<AnimKey> & keyArray) const
{
  List<Animation>::Iterator li(this); 

  for (!li; *li; li++)
  {
    Animation &anim = *(*li);

    anim.SetFrame( frame, keyArray);

    statQuatErrorIndex = anim.index;  
  }
}
//----------------------------------------------------------------------------

void AnimList::SetBlend( F32 frame, AnimKey * keys0, AnimKey * keys1, Array<FamilyState> & stateArray) const
{
  U32 i;
  for (i = 1; i < stateArray.count; i++)
  {
    ASSERT( keys0[i].frame < keys1[i].frame);

    if (keys0[i].type & animNOANIM)
    {
      continue;
    }

    Animation::Set( frame, keys0[i], keys1[i], stateArray[i]);
  }
}
//----------------------------------------------------------------------------

void AnimList::SetBlend( F32 frame, AnimKey * keys0, AnimKey * keys1, Array<AnimKey> & keyArray) const
{
  U32 i;
  for (i = 1; i < keyArray.count; i++)
  {
    ASSERT( keys0[i].frame < keys1[i].frame);

    if (keys0[i].type & animNOANIM)
    {
      continue;
    }

    Animation::Set( frame, keys0[i], keys1[i], keyArray[i]);
  }
}
//----------------------------------------------------------------------------

U32 Animation::GetMem() const
{
  U32 mem = sizeof(*this);
  mem += keys.size;

  return mem;
}
//----------------------------------------------------------------------------

U32 AnimList::GetMem() const
{
  U32 mem = sizeof(*this);

  mem += states.size;

  List<Animation>::Iterator li(this); 
  for (!li; *li; li++)
  {
    Animation *anim = (*li);

    mem += anim->GetMem();
  }
  return mem;
}
//----------------------------------------------------------------------------

U32 AnimCycles::GetMem() const
{
  U32 mem = sizeof(*this);

  NBinTree<AnimList>::Iterator li(this); 
  for (!li; *li; li++)
  {
    AnimList *animList = (*li);

    mem += animList->GetMem();
  }
  return mem;
}
//----------------------------------------------------------------------------

Bool statQuatError = FALSE;

void Animation::Set( F32 frame, const AnimKey &lastKey, const AnimKey &thisKey, AnimKey &state)
{
  // determine the current fraction to this cycle
  F32 dk = thisKey.frame - lastKey.frame;
  ASSERT( dk > 0);
  F32 dfdk  = (frame - lastKey.frame) / dk;

//  state.type = 0;

  if (lastKey.type & animSCALE)
  {
    // do a parametric interpolation of the quaternion using dfdk
    Quaternion q = (thisKey.quaternion - lastKey.quaternion) * dfdk + lastKey.quaternion;
    
    // do a parametric interpolation of the quaternion using dfdk
    if (q.Dot( q) <= 0)
    {
      LOG_WARN( ("Quat::Set q.Dot(q) <= 0: %f %f %f %f", q.s, q.v.x, q.v.y, q.v.z) );
      LOG_WARN( ("lastKey.quat: %f %f %f %f", lastKey.quaternion.s, lastKey.quaternion.v.x, lastKey.quaternion.v.y, lastKey.quaternion.v.z) );
      LOG_WARN( ("thisKey.quat: %f %f %f %f", thisKey.quaternion.s, thisKey.quaternion.v.x, thisKey.quaternion.v.y, thisKey.quaternion.v.z) );
      LOG_WARN( ("dfdk: %f; lastKey.frame %f; thisKey.frame %f; frame %f", dfdk, lastKey.frame, thisKey.frame, frame) );

      statQuatError = TRUE;

      state.Set(thisKey.quaternion);
    }
    else
    {
      // set the matrix
      state.Set( q);
    }
    // do a parametric interpolation of the scale using dfdk
    Vector v = (thisKey.scale - lastKey.scale) * dfdk + lastKey.scale;
    // set the matrix
    state.SetScale( v);

//    state.type |= animSCALE;
  }
  else if (lastKey.type & animQUATERNION)
  {
    Quaternion q = (thisKey.quaternion - lastKey.quaternion) * dfdk + lastKey.quaternion;

    // do a parametric interpolation of the quaternion using dfdk
    if (q.Dot( q) <= 0)
    {
      LOG_WARN( ("Quat::Set q.Dot(q) <= 0: %f %f %f %f", q.s, q.v.x, q.v.y, q.v.z) );
      LOG_WARN( ("lastKey.quat: %f %f %f %f", lastKey.quaternion.s, lastKey.quaternion.v.x, lastKey.quaternion.v.y, lastKey.quaternion.v.z) );
      LOG_WARN( ("thisKey.quat: %f %f %f %f", thisKey.quaternion.s, thisKey.quaternion.v.x, thisKey.quaternion.v.y, thisKey.quaternion.v.z) );
      LOG_WARN( ("dfdk: %f; lastKey.frame %f; thisKey.frame %f; frame %f", dfdk, lastKey.frame, thisKey.frame, frame) );

      statQuatError = TRUE;

      state.Set(thisKey.quaternion);
    }
    else
    {
      // set the matrix
      state.Set( q);
    }
//    state.type |= animQUATERNION;
  }

  if (lastKey.type & animPOSITION)
  {
    // do a parametric interpolation of the pos using dfdk
    Vector v = (thisKey.position - lastKey.position) * dfdk + lastKey.position;
    // set the matrix
    state.Set( v);

//    state.type |= animPOSITION;
  }
}
//----------------------------------------------------------------------------

void Animation::SetFrameOverlay( F32 frame, AnimKey &state, F32 controlFrame) const
{
  controlFrame;

  U32 i;
#if 0
  Quaternion conq;
  conq.ClearData();
  for (i = 1; i < keys.count; i++)
  {
    AnimKey &lastKey = keys[i - 1];
    AnimKey &thisKey = keys[i];

    if (controlFrame <= thisKey.frame)
    {
      // determine the current fraction to this cycle
      F32 dk = thisKey.frame - lastKey.frame;
      ASSERT( dk > 0);
      F32 dfdk  = (controlFrame - lastKey.frame) / dk;

      if (lastKey.type & animQUATERNION)
      {
        // do a parametric interpolation of the quaternion using dfdk
        conq = (thisKey.quaternion - lastKey.quaternion) * dfdk + lastKey.quaternion;
      }
      break;
    }
  }
#endif
//         0;  4; 0.923880, 0.382683, 0.0, 0.0;;,
//         0;  4; 0.898028, 0.439939, 0.0, 0.0;;,
//         0;  4; 0.868631, 0.495459, 0.0, 0.0;;,
//         0;  4; 0.770513, 0.637424, 0.0, 0.0;;,

  for (i = 1; i < keys.count; i++)
  {
    AnimKey &lastKey = keys[i - 1];
    AnimKey &thisKey = keys[i];

    if (frame <= thisKey.frame)
    {
//      LOG_DIAG( ("%f ; %f ; %f", lastKey.frame, frame, thisKey.frame) );

      ASSERT( lastKey.frame < thisKey.frame);

      // determine the current fraction to this cycle
      F32 dk = thisKey.frame - lastKey.frame;
      ASSERT( dk > 0);
      F32 dfdk  = (frame - lastKey.frame) / dk;

      if (lastKey.type & animQUATERNION)
      {
        // do a parametric interpolation of the quaternion using dfdk
        Quaternion q = (thisKey.quaternion - lastKey.quaternion) * dfdk + lastKey.quaternion;

    #if 1
        if (q.Dot( q) <= 0)
        {
          LOG_WARN( ("Quat::Set q.Dot(q) <= 0: %f %f %f %f", q.s, q.v.x, q.v.y, q.v.z) );
          LOG_WARN( ("lastKey.quat: %f %f %f %f", lastKey.quaternion.s, lastKey.quaternion.v.x, lastKey.quaternion.v.y, lastKey.quaternion.v.z) );
          LOG_WARN( ("thisKey.quat: %f %f %f %f", thisKey.quaternion.s, thisKey.quaternion.v.x, thisKey.quaternion.v.y, thisKey.quaternion.v.z) );
          LOG_WARN( ("dfdk: %f; lastKey.frame %f; thisKey.frame; frame", dfdk, lastKey.frame, thisKey.frame, frame) );

          statQuatError = TRUE;
        }
    #else
        ASSERT( q.Dot( q) > 0);
    #endif

        // blend in the quat
        //
//        q -= conq;
//        q = state.quaternion * q;
//        q.Set( PI * 0.11f, Matrix::I.right);
//        q = state.quaternion * q;
        state.Set( q );
      }
      break;
    }
  }
}
//----------------------------------------------------------------------------
