///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 9-AUG-1999
//


#ifndef __COMMON_TYPELIST_H
#define __COMMON_TYPELIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "propertylist.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Editor
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TypeList
  //
  class TypeList : public ICListBox
  {
  protected:

    // Dynamic property
    IFaceVar *property;

    // Use type name as key
    IFaceVar *useKey;

    // The class id filter lists for both inclusion and exclusion
  	List<GameIdent> include, exclude;

    // Exclude properties
    PropertyList<16> excludeProperties;

  public:

    // Constructor
    TypeList(IControl *parent);
    ~TypeList();

    // Rebuild the list using the current filter
    void Rebuild();

    // Event handling function
    U32 HandleEvent(Event &e);

    // Function called when a var being watched changes value
    void Notify(IFaceVar *var);

    // Setup this control from one scope function
    void Setup(FScope *fScope);
  };
}

#endif
