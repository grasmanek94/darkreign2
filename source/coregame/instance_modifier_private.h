///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Instance modifiers
//
// 01-JUN-1999
//


#ifndef __INSTANCE_MODIFIER_PRIVATE_H
#define __INSTANCE_MODIFIER_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Class ApplyDamageModifier
//
class ApplyDamageModifier : public ApplyModifier
{
public:
  ApplyDamageModifier(FScope *fScope) : ApplyModifier(fScope) {}
  void Apply(MapObj *obj);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ApplySpeedModifier
//
class ApplySpeedModifier : public ApplyModifier
{
public:
  ApplySpeedModifier(FScope *fScope) : ApplyModifier(fScope) {}
  void Apply(MapObj *obj);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ApplyFireDelayModifier
//
class ApplyFireDelayModifier : public ApplyModifier
{
public:
  ApplyFireDelayModifier(FScope *fScope) : ApplyModifier(fScope) {}
  void Apply(MapObj *obj);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class ApplyFireDelayModifier
//
class ApplySightModifier : public ApplyModifier
{
public:
  ApplySightModifier(FScope *fScope) : ApplyModifier(fScope) {}
  void Apply(MapObj *obj);
};

#endif
