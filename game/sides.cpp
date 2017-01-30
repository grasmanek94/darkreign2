///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Side Management System
//
// 4-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "sides.h"
#include "ptree.h"
#include "stdload.h"
#include "filesys.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sides
//
namespace Sides
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized flag
  static Bool initialized = FALSE;

  // The path to the side configuation files
  static const char *configPath = "sides";

  // The name of the config resource stream
  static const char *configStream = "sideconfig";

  // The path to the interface resources
  static const char *resourcePath = "sides\\client";

  // The name of the interface resource stream
  static const char *resourceStream = "side";
  
  // The mask used to load side configuration files
  static const char *configMask = "*.cfg";

  // The current list of sides
  static NBinTree<Side> sides(&Side::node);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Side - Stores the configuration data for a single side
  //

  //
  // Constructor
  //
  Side::Side(const char *name) 
  : name(name), 
    description(name), 
    resource(name), 
    mappings(&Mapping::node)
  {
  }


  //
  // Destructor
  //
  Side::~Side()
  {
    mappings.DisposeAll();
  }


  //
  // AddMapping
  //
  // Add a single unit mapping
  //
  void Side::AddMapping(const char *generic, const char *specific)
  {
    // Create the new mapping
    Mapping *m = new Mapping(generic, specific);

    // Add to the tree
    mappings.Add(m->g.crc, m);
  }


  //
  // GetMapping
  //
  // Find an identifier mapping (NULL if none)
  //
  const char * Side::GetMapping(U32 crc)
  {
    // Try and find the mapping
    if (Mapping *mapping = mappings.Find(crc))
    {
      return (mapping->s.str);
    }
    
    return (NULL);
  }

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //


  //
  // ProcessCreateSide
  //
  // Process the given side creation scope
  //
  static void ProcessCreateSide(FScope *scope)
  {
    FScope *sScope;

    // The name of the side is the first argument
    Side *side = new Side(scope->NextArgString());

    // Process each function
    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x47CB37F2: // "Description"
          side->SetDescription(StdLoad::TypeString(sScope));
          break;

        case 0xEB8817E5: // "Mappings"
        {
          FScope *ssScope;

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x9F1D54D0: // "Add"
              {
                // Read mapping data first
                const char *generic = StdLoad::TypeString(ssScope);
                const char *specific = StdLoad::TypeString(ssScope);

                // Add the mapping to the side
                side->AddMapping(generic, specific);
                break;
              }
            }
          }
          break;
        }

        case 0x72724107: // "InterfaceResource"
          side->SetResource(sScope->NextArgString());
          break;
      }
    }

    // Add to the side list
    sides.Add(side->GetName().crc, side);
  }

  
  //
  // Load
  //
  // Load all side configurations from the given directory
  //
  static void Load(const char *dir)
  {
    Dir::Find find;
    PTree pTree;

    // Save the currently active stream
    const char * oldStream = FileSys::GetActiveStream();

    // Setup the info stream
    FileSys::AddSrcDir(configStream, dir);

    // And activate it
    FileSys::SetActiveStream(configStream);

    // Find each cfg file in this dir
    if (Dir::FindFirst(find, dir, configMask))
    {
      do
      {
        // Exclude directories
        if (!(find.finddata.attrib & File::Attrib::SUBDIR))
        {
          pTree.AddFile(find.finddata.name);
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);

    // Restore the old active stream
    FileSys::SetActiveStream(oldStream);

    // Process the configuration
    FScope *fScope = pTree.GetGlobalScope();
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1C949C6A: // "CreateSide"
          ProcessCreateSide(sScope);
          break;
      }
    }

    // There must be at least one side configured
    if (!sides.GetCount())
    {
      ERR_CONFIG(("Unable to load any side configs [%s]", dir));
    }
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)
    ASSERT(!sides.GetCount())

    // Load configs
    Load(configPath);

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    sides.DisposeAll();

    initialized = FALSE;
  }


  //
  // OpenResource
  //
  // Setup the interface resource for the given side
  //
  void OpenResource(Side &side)
  {
    ASSERT(initialized)
    ASSERT(sides.GetCount())

    // Delete any current setup
    FileSys::DeleteStream(resourceStream);

    // Add the resource
    FileSys::AddResource(resourceStream, resourcePath, side.GetResource().str, FALSE);
  }


  //
  // CloseResource
  //
  // Close the interface resource stream
  //
  void CloseResource()
  {
    ASSERT(initialized)
    ASSERT(sides.GetCount())

    // Delete any current stream
    FileSys::DeleteStream(resourceStream);
  }


  //
  // GetSide
  //
  // Get a given side, or the default side
  //
  Side & GetSide(const char *ident)
  {
    ASSERT(initialized)
    ASSERT(sides.GetCount())

    // If supplied, find the requested side
    Side *side = ident ? sides.Find(Crc::CalcStr(ident)) : NULL;

    // Otherwise use the default
    if (!side)
    {
      side = sides.GetFirst();
    }

    return (*side);
  }


  //
  // Get a random side
  //
  Side & GetRandomSide()
  {
    ASSERT(initialized)
    ASSERT(sides.GetCount())

    // Pick a side at random
    U32 side = Random::sync.Integer(GetSides().GetCount());

    NBinTree<Side>::Iterator s(&GetSides());
    while (side--)
    {
      ++s;
    }

    ASSERT(*s)
    return (**s);
  }


  //
  // GetSides
  //
  // Get the tree of all configured sides
  //
  const NBinTree<Side> & GetSides()
  {
    ASSERT(initialized)
    ASSERT(sides.GetCount())

    return (sides);
  }
}
