///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Console viewer
//
// 16-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "console_viewer.h"
#include "console.h"
#include "stdload.h"


static Bool ConsoleHook(const CH *text, U32 &type, void *context);


///////////////////////////////////////////////////////////////////////////////
//
// Class ConsoleViewer
//

// Control names
static const char *ConsoleItemCtrlName = "[ConsoleItem]";


//
// Constructor
//
ConsoleViewer::ConsoleViewer(IControl *parent) 
: ICListBox(parent)
{
  listBoxStyle |= STYLE_WRAP;
}


//
// Destructor
//
ConsoleViewer::~ConsoleViewer()
{
  filters.DisposeAll();
}


//
// ConsoleViewer::Setup
//
void ConsoleViewer::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x5C5656F8: // "Filters"
    {
      List<GameIdent> idents;
      StdLoad::TypeStrCrcList(fScope, idents);
      for (List<GameIdent>::Iterator i(&idents); *i; i++)
      {
        filters.Add((*i)->crc);
      }
      idents.DisposeAll();
      break;
    }

    default:
      ICListBox::Setup(fScope);
      break;
  }
}


//
// ConsoleViewer::ClearFilters
//
// Clear filters
//
void ConsoleViewer::ClearFilters()
{
  filters.DisposeAll();
}


//
// ConsoleViewer::AddFilter
//
// Add filter
//
void ConsoleViewer::AddFilter(U32 filter)
{
  filters.Add(filter);
}


//
// ConsoleViewer::DelFilter
//
// Del filter
//
void ConsoleViewer::DelFilter(U32 filter)
{
  filters.Dispose(filter);
}


//
// ConsoleViewer::Rebuild
//
// Rebuild the display
//
void ConsoleViewer::Rebuild()
{
  BinTree<U32> *pf = filters.GetCount() ? &filters : NULL;

  // Empty the list
  DeleteAllItems();

  // Rebuild list
  Console::EnumStrings(pf, ConsoleHook, this);
}


//
// ConsoleViewer::Activate
//
Bool ConsoleViewer::Activate()
{
  if (CanActivate())
  {
    // Install message hook
    BinTree<U32> *pf = filters.GetCount() ? &filters : NULL;
    Console::RegisterMsgHook(pf, ConsoleHook, this);
  }

  return (ICListBox::Activate());
}


//
// ConsoleViewer::Deactivate
//
Bool ConsoleViewer::Deactivate()
{
  if (ICListBox::Deactivate())
  {
    // Uninstall message hook
    Console::UnregisterMsgHook(ConsoleHook, this);

    // Delete all items
    DeleteAllItems();

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// ConsoleViewer::HandleEvent
//
// Event handling function
//
U32 ConsoleViewer::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
        switch (e.iface.p1)
        {
          case ConsoleViewerMsg::Rebuild:
          case ICListBoxMsg::Rebuild:
          {
            Rebuild();
            return (TRUE);
          }

          case ConsoleViewerMsg::Clear:
          {
            // Clear the console
            Console::ClearScrollBack();

            // Change the message to a delete all message so that it is cleared
            e.iface.p1 = ICListBoxMsg::DeleteAll;
            break;
          }

          case ConsoleViewerMsg::ClearFilters:
          {
            ClearFilters();
            return (TRUE);
          }

          case ConsoleViewerMsg::AddFilter:
          {
            AddFilter(e.iface.p2);
            return (TRUE);
          }

          case ConsoleViewerMsg::DelFilter:
          {
            DelFilter(e.iface.p2);
            return (TRUE);
          }
        }
        break;
    }
  }

  return (ICListBox::HandleEvent(e));
}


//
// ConsoleHook
//
// Message hook callback for ConsoleViewer objects
//
static Bool ConsoleHook(const CH *text, U32 &type, void *context)
{
  ConsoleViewer *cv = (ConsoleViewer *)context;

  // Construct a new text item
  cv->AddTextItem(ConsoleItemCtrlName, text, &IFace::GetConsoleColor(type));

  return (TRUE);
}
