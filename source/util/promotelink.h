///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Run time promotion macros
//
// 07-DEC-98
//


#ifndef __PROMOTELINK_H
#define __PROMOTELINK_H


//
// Macro for base class
// 
// Usage: 
//
//   class Base
//   {
//     PROMOTE_BASE(0xD592CB38); // "UniqueName"
//
#define PROMOTE_BASE(base, id)                                              \
public:                                                                     \
  static U32 ClassId() { return (id); }                                     \
  static const char *ClassName() { return (#base); }                        \
  virtual Bool DerivedFrom(U32 classId)                                     \
  {                                                                         \
    return ((ClassId() == classId) ? TRUE : FALSE);                         \
  }                                                                         \
private:

//
// Macro for derived classes
// 
// Usage: 
//
//   class Derived : public Base
//   {
//     PROMOTE_LINK(Derived, Base, 0xD592CB38); // "UniqueName"
//
#define PROMOTE_LINK(cls, base, id)                                         \
public:                                                                     \
  static U32 ClassId() { return (id); }                                     \
  static const char *ClassName() { return (#cls); }                         \
  virtual Bool DerivedFrom(U32 classId)                                     \
  {                                                                         \
    ASSERT(base::ClassId());                                                \
    return (ClassId() == classId) ? TRUE : base::DerivedFrom(classId);      \
  }                                                                         \
private:

#endif
