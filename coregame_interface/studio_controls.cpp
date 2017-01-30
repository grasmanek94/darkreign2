///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Studio Controls
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_attachlist.h"
#include "studio_footeditor.h"
#include "studio_teameditor.h"
#include "studio_teamlist.h"
#include "studio_bookmarklist.h"
#include "studio_bookmarklistbox.h"
#include "studio_lighteditor.h"
#include "studio_regionlist.h"
#include "studio_taglist.h"
#include "studio_objecttweak.h"
#include "studio_weather.h"
#include "studio_mission.h"

#include "iface.h"
#include "coloreditor.h"
#include "colorbutton.h"



///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {
    // System initialized flag
    static Bool initialized = FALSE;


    //
    // CreateHandler
    //
    static IControl * CreateHandler(U32 crc, IControl *parent, U32)
    {
      ASSERT(initialized)

      IControl *ctrl = NULL;

      switch (crc)
      {
        case 0x68371F13: // "AttachList"
          ctrl = new AttachList(parent);
          break;

        case 0x2C74E521: // "FootEditor"
          ctrl = new FootEditor(parent);
          break;

        case 0xBB81F7D0: // "TeamEditor"
          ctrl = new TeamEditor(parent);
          break;

        case 0x18EB5DFA: // "TeamList"
          ctrl = new TeamList(parent);
          break;

        case 0x036FC2BE: // "MissionConfig"
          ctrl = new Mission(parent);
          break;

        case 0x0013DE4B: // "BookmarkList"
          ctrl = new BookmarkList(parent);
          break;

        case 0x1865D2EF: // "BookmarkListBox"
          ctrl = new BookmarkListBox(parent);
          break;

        case 0x1EB994CF: // "LightEditor"
          ctrl = new LightEditor(parent);
          break;

        case 0xE1181191: // "RegionListBox"
          ctrl = new RegionListBox(parent);
          break;

        case 0xB9F62A1C: // "TagListBox"
          ctrl = new TagListBox(parent);
          break;

        case 0x43A3F670: // "Weather"
          ctrl = new Weather( parent);
          break;

        case 0x8CC5AE1A: // "ObjectTweak::MapObj"
          ctrl = new ObjectTweak::MapObj(parent);
          break;

        case 0xC12DA4B3: // "ObjectTweak::UnitObj"
          ctrl = new ObjectTweak::UnitObj(parent);
          break;

        case 0x49D041A5: // "ObjectTweak::ResourceObj"
          ctrl = new ObjectTweak::ResourceObj(parent);
          break;

      }

      return (ctrl);
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      ASSERT(!initialized);

      // Register Controls
      IFace::RegisterControlClass("AttachList", CreateHandler);
      IFace::RegisterControlClass("FootEditor", CreateHandler);
      IFace::RegisterControlClass("MissionConfig", CreateHandler);
      IFace::RegisterControlClass("TeamEditor", CreateHandler);
      IFace::RegisterControlClass("TeamList", CreateHandler);
      IFace::RegisterControlClass("BookmarkList", CreateHandler);
      IFace::RegisterControlClass("BookmarkListBox", CreateHandler);
      IFace::RegisterControlClass("LightEditor", CreateHandler);
      IFace::RegisterControlClass("RegionListBox", CreateHandler);
      IFace::RegisterControlClass("TagListBox", CreateHandler);
      IFace::RegisterControlClass("Weather", CreateHandler);

      IFace::RegisterControlClass("ObjectTweak::MapObj", CreateHandler);
      IFace::RegisterControlClass("ObjectTweak::UnitObj", CreateHandler);
      IFace::RegisterControlClass("ObjectTweak::ResourceObj", CreateHandler);

      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      ASSERT(initialized);

      // Unregister controls
      IFace::UnregisterControlClass("AttachList");
      IFace::UnregisterControlClass("FootEditor");
      IFace::UnregisterControlClass("MissionConfig");
      IFace::UnregisterControlClass("TeamEditor");
      IFace::UnregisterControlClass("TeamList");
      IFace::UnregisterControlClass("BookmarkList");
      IFace::UnregisterControlClass("BookmarkListBox");
      IFace::UnregisterControlClass("LightEditor");
      IFace::UnregisterControlClass("RegionListBox");
      IFace::UnregisterControlClass("TagListBox");
      IFace::UnregisterControlClass("Weather");

      IFace::UnregisterControlClass("ObjectTweak::MapObj");
      IFace::UnregisterControlClass("ObjectTweak::UnitObj");
      IFace::UnregisterControlClass("ObjectTweak::ResourceObj");

      initialized = FALSE;
    }    
  }
}
