///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls_propertylist.h"
#include "iface.h"
#include "multiplayer_private.h"
#include "multiplayer_cmd_private.h"
#include "multiplayer_host.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PropertyList
    //


    //
    // PropertyList::PropertyList
    //
    // Constructor
    //
    PropertyList::PropertyList(IControl *parent)
    : ICListBox(parent)
    {
    }

    struct SetPropertyData
    {
      ICListBox *listBox;
      const PropertyFilter *propertyFilter;
    };


    //
    // Activate
    //
    Bool PropertyList::Activate()
    {
      if (ICListBox::Activate())
      {
        ClearSelected(FALSE);

        SetPropertyData data;
        data.listBox = this;

        // Get the current set of properties and select those items
        if (Data::Get(&data.propertyFilter))
        {
          // Get all of the selected items and build up the current property list
          EnumNonSelected(SetPropertiesCallback, &data);
        }
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // HandleEvent
    //
    U32 PropertyList::HandleEvent(Event &e)
    {
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case PropertyListMsg::Download:
                // We should only try to send if we're the host, it'll get ignored anyhow though
                if (Cmd::isHost)
                {
                  PropertyFilter propertyFilter;
                  propertyFilter.numFilters = 0;
                  Utils::Memset(propertyFilter.filters, 0x00, maxFilters * sizeof (CRC));

                  // Get all of the selected items and build up the current property list
                  EnumSelected(GetPropertiesCallback, &propertyFilter);

                  // Send off the new property filters
                  Data::Store(propertyFilter);

                  // Invalidate host readyness
                  Host::InvalidateLaunchReadyness();
                }
                return (TRUE);
            }
          }
        }
      }
      return (ICListBox::HandleEvent(e));
    }


    //
    // SetPropertiesCallback
    //
    void PropertyList::SetPropertiesCallback(const char *key, const CH *, void *context)
    {
      SetPropertyData *data = reinterpret_cast<SetPropertyData *>(context);

      U32 property = Crc::CalcStr(key);

      for (U32 p = 0; p < data->propertyFilter->numFilters; p++)
      {
        if (property == data->propertyFilter->filters[p])
        {
          // Select this one!
          data->listBox->SetSelectedItem(key, FALSE, FALSE);
          return;
        }
      }
    }

    
    //
    // GetPropertiesCallback
    //
    void PropertyList::GetPropertiesCallback(const char *key, const CH *, void *context)
    {
      PropertyFilter *propertyFilter = reinterpret_cast<PropertyFilter *>(context);

      if (propertyFilter->numFilters < maxFilters)
      {
        // Add to the array of filtered properties
        propertyFilter->filters[(propertyFilter->numFilters)++] = Crc::CalcStr(key);
      }
      else
      {
        LOG_DIAG(("Ignored property '%s' since we have too many", key))
      }
    }

  }

}
