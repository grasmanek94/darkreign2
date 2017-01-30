///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_attachlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TeamList
  //

  //
  // Constructor
  //
  AttachList::AttachList(IControl *parent) 
  : ICListBox(parent)
  {
  }


  //
  // Destructor
  //
  AttachList::~AttachList()
  {
  }

  //
  // AttachList::BuildList
  //
  void AttachList::BuildList(MapObj *obj)
  {
    ASSERT(obj)

    // Clear the list
    DeleteAllItems();

    // Get the mesh ent
    MeshEnt &ent = obj->Mesh();

    // Get the names
    BuffString names[MAXMESHPERGROUP];
    U32 count = ent.GetHierarchy(names, TRUE);
    U32 i;
    for (i = 0; i < count; i++)
    {
      char *p = names[i].str;
      while (*p == ' ')
      {
        p++;
      }

      // Find this object
      MeshObj *hp = ent.FamilyNode::FindMeshObj(p, TRUE);

      // Must not have anything else attached
      if (hp && !hp->HasAttachment())
      {
        AddTextItem(p, Utils::Ansi2Unicode(names[i].str));
      }
    }
  }
}
