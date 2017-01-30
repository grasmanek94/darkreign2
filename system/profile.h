///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Profiling
//
// 30-MAR-1998
//

#ifndef __PROFILE_H
#define __PROFILE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "system.h"
#include "str.h"

#pragma pack(push, _save_profile_h_)
#include <commctrl.h>
#pragma pack(pop, _save_profile_h_)


///////////////////////////////////////////////////////////////////////////////
//
// ProfileNode Class
//

class ProfileNode
{
public:
  enum Type
  {
    Root,
    Module,
    Library,
    Object,
    Function,
    Offset
  };

private:

  Type    type;
  String  name;
  U32     samples;

public:

  BinTree<ProfileNode> children;
  F32                  cutParent;
  F32                  cutTotal;

  //
  // ProfileNode
  //
  // Constructor
  //
  ProfileNode(Type t, String n) : type(t), name(n), samples(1), cutParent(0), cutTotal(0)
  {
  }

  //
  // ~ProfileNode
  //
  ~ProfileNode()
  {
    children.DisposeAll();
  }

  //
  // GetType
  //
  // Retrieves the type of the node
  //
  Type GetType() 
  { 
    return (type); 
  }

  //
  // GetName
  //
  // Retrieves the name of the node
  //
  String GetName() 
  { 
    return (name); 
  }

  //
  // GetSamples
  //
  // Retrieves the number of samples this node contains
  //
  U32 GetSamples()
  {
    return (samples);
  }

  //
  // IncSamples
  //
  // Adds one to the samples
  //
  void IncSamples()
  {
    samples++;
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Profile Class
//

class Profile
{
private:

  LOGDEC

  static U32             *samples;
  static U32             *samplePtr;
  static U32             samplesMax;
  static U32             samplesNum;
  static U32             interval;
  static System::Thread  *profileThread;
  static HANDLE          watchThread;
  static System::Event   shutdown;
  static System::Event   dead;

public:
  static void Init(U32 max, U32 interval);
  static void Done();
  static void Reset();
  static void Start();
  static void Stop();
  static void Report();


  static void AddItemToTree(HWND tree, HTREEITEM item, ProfileNode *node);
  static Bool CALLBACK ProgressDlgProc(HWND, UINT, WPARAM, LPARAM);
  static Bool CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
  static U32 STDCALL Process(void *);

};


#endif