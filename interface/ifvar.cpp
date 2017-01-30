/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 24-JUL-98
//


#include "ifvar.h"
#include "icontrol.h"
#include "debug_memory.h"
#include "multilanguage.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// IFaceVar implementation
//


// Temporary buffer for Babel::Sprintf
static CH babelBuf[MULTILANGUAGE_MAXDATA];


//
// IFaceVar::IFaceVar
//
// Initialise from a var name
//
IFaceVar::IFaceVar(IControl *ctrl, const char *name) : varPtr(NULL), varName(NULL), ctrl(ctrl), active(FALSE)
{
  ASSERT(name);

  VarSys::VarItem *varItem = VarSys::FindVarItem(name);

  if (varItem)
  {
    Init(varItem);
  }
  else
  {
    //LOG_DIAG(("Var[%s] Ctrl[%s] needs to be resolved", name, ctrl ? ctrl->Name() : "NULL"));
    varName = Utils::Strdup(name);
  }
}


//
// IFaceVar::IFaceVar
//
// Initialise from a var item
//
IFaceVar::IFaceVar(IControl *ctrl, VarSys::VarItem *item) : varPtr(NULL), varName(NULL), ctrl(ctrl), active(FALSE)
{
  ASSERT(item);

  Init(item);
}


//
// IFaceVar::~IFaceVar
//
IFaceVar::~IFaceVar()
{
  // Dispose of var name
  if (varName)
  {
    delete[] varName;
    varName = NULL;
  }

  // Release the variable memory
  Done();
}


//
// IFaceVar::Init
//
// Setup the var
//
void IFaceVar::Init(VarSys::VarItem *item)
{
  ASSERT(item);
  ASSERT(!varPtr);

  varType = item->type;

  switch (varType)
  {
    case VarSys::VI_INTEGER:
    {
      varInteger = new VarInteger;
      varInteger->item = item;
      varPtr = varInteger;
      break;
    }

    case VarSys::VI_FPOINT:
    {
      varFloat = new VarFloat;
      varFloat->item = item;
      varPtr = varFloat;
      break;
    }

    case VarSys::VI_STRING:
    {
      varString = new VarString;
      varString->item = item;
      varPtr = varString;
      break;
    }

    default:
    {
      ERR_FATAL(("Unsupported var type"));
      return;
    }
  }
}


//
// IFVar::Done
//
// Release the memory used by the var
//
void IFaceVar::Done()
{
  // Dipose of var pointer
  if (varPtr)
  {
    Deactivate();

    switch (varType)
    {
      case VarSys::VI_INTEGER:
      {
        delete varInteger;
        varInteger = NULL;
        break;
      }

      case VarSys::VI_FPOINT:
      {
        delete varFloat;
        varFloat = NULL;
        break;
      }

      case VarSys::VI_STRING:
      {
        delete varString;
        varString = NULL;
        break;
      }
    }

    varPtr = NULL;
  }
}

//
// IFaceVar::Activate
//
// Activate the var, resolve name if not already resolved
//
void IFaceVar::Activate()
{
  // The var died while we were inactive, reset it
  if (varPtr && !varPtr->Alive())
  {
    Done();
    varPtr = NULL;
  }

  // Setup the var
  if (varPtr == NULL)
  {
    if (varName == NULL)
    {
      ERR_FATAL(("Var for [%s] not setup", ctrl ? ctrl->Name() : "NULL"));
    }

    // Resolve the var
    VarSys::VarItem *item = VarSys::FindVarItem(ctrl ? ctrl->FindVarName(varName) : varName);

    if (item)
    {
      Init(item);
    }
    else
    {
      LOG_ERR(("Var [%s] not found", varName));
    }
  }

  if (varPtr && !active)
  {
    varPtr->NotifyMe(this);
    active = TRUE;
  }

  // First time notification
  if (ctrl)
  {
    ctrl->Notify(this);
  }
}


//
// IFaceVar::Deactivate
//
// Deactivate the var
//
void IFaceVar::Deactivate()
{
  if (varPtr && active)
  {
    VALIDATE(varPtr)

    varPtr->ForgetMe(this);
    active = FALSE;
  }
  ASSERT(!active);
}


//
// IFaceVar::Notify
//
// The var has changed value
//
void IFaceVar::Notify(VarNotify::Mode mode)
{
  ASSERT(varPtr);

  switch (mode)
  {
    case VarNotify::CHANGED:
    {
      if (ctrl)
      {
        VALIDATE(ctrl);

        // Tell the control that the value changed
        ctrl->Notify(this);
      }
      break;
    }

    case VarNotify::DELETED:
    {
      Done();
      break;
    }

    default:
    {
      ERR_FATAL(("Unknown VarNotify mode [%d]", U32(mode)));
      break;
    }
  }
}


//
// IFaceVar::SetValue
//
// Set the value of the variable from the string s
//
void IFaceVar::SetValue(const char *s)
{
  if (varPtr && varPtr->Alive())
  {
    if (!((*varPtr)->flags & VarSys::NOEDIT))
    {
      switch (varType)
      {
        case VarSys::VI_STRING:
        {
          ASSERT(varString)
          (*varString) = s;
          break;
        }

        case VarSys::VI_INTEGER:
        {
          ASSERT(varInteger);
          (*varInteger) = Utils::AtoI(s);
          break;
        }

        case VarSys::VI_FPOINT:
        {
          ASSERT(varFloat);
          (*varFloat) = Utils::AtoF(s);
          break;
        }

        default:
        {
          ERR_FATAL(("varType is not valid for control [%s]", ctrl ? ctrl->Name() : "NULL"));
          break;
        }
      }
    }
    else
    {
      LOG_WARN(("Attempt to edit a NOEDIT var"))
    }
  }
}


//
// IFaceVar::GetValue
//
// Retrieve a text representation of the value, with optional format string
//
void IFaceVar::GetValue(CH *buf, U32 size, const char *format, Bool translate)
{
  buf[0] = CH('\0');

  if (varPtr)
  {
    switch (varType)
    {
      case VarSys::VI_STRING:
      {
        if (format)
        {
          // FIXME: this is not very efficient
          Utils::Strmcpy(buf, TRANSLATE((format, 1, TRANSLATE((*(*varString))))), size);
        }
        else
        {
          if (translate)
          {
            Utils::Strmcpy(buf, TRANSLATE((*(*varString))), size);
          }
          else
          {
            // This is mainly for edit boxes
            Utils::Ansi2Unicode(babelBuf, MULTILANGUAGE_MAXDATA, *varString);
            Utils::Strmcpy(buf, babelBuf, size);
          }
        }
        break;
      }

      case VarSys::VI_INTEGER:
      {
        Utils::Strmcpy(buf, TRANSLATE((format ? format : "{1:d}", 1, *(*varInteger))), size);
        break;
      }

      case VarSys::VI_FPOINT:
      {
        Utils::Strmcpy(buf, TRANSLATE((format ? format : "{1:.1f}", 1, *(*varFloat))), size);
        break;
      }
    }
  }
}


//
// IFaveVar::GetValue
//
// Retrieve a text representation of the value, with optional format string
//
const CH *IFaceVar::GetValue(const char *format, Bool translate)
{
  static CH buffer[256];
  GetValue(buffer, 256, format, translate);
  return (buffer);
}


//
// IFaceVar::GetStringValue
//
// Return the value of the string, error if it isnt a string
//
const char *IFaceVar::GetStringValue()
{
  if (varPtr == NULL || varType != VarSys::VI_STRING)
  {
    ERR_FATAL(("Var is not a string"));
  }

  return **varString;
}


//
// IFaceVar::SetStringValue
//
void IFaceVar::SetStringValue(const char *val)
{
  if (varPtr == NULL || varType != VarSys::VI_STRING)
  {
    ERR_FATAL(("Var is not an string"));
  }

  if (((*varPtr)->flags & VarSys::NOEDIT))
  {
    LOG_WARN(("Var is non editable"))
  }
  else
  {
    *varString = val;
  }
}


//
// IFaceVar::GetIntegerValue
//
// Return the value of the integer var, error if it isnt a var
//
S32 IFaceVar::GetIntegerValue()
{
  if (varPtr == NULL || varType != VarSys::VI_INTEGER)
  {
    ERR_FATAL(("Var is not an integer"));
  }

  return **varInteger;
}


//
// IFaceVar::SetIntegerValue
//
void IFaceVar::SetIntegerValue(S32 val)
{
  if (varPtr == NULL || varType != VarSys::VI_INTEGER)
  {
    ERR_FATAL(("Var is not an integer"));
  }

  if (((*varPtr)->flags & VarSys::NOEDIT))
  {
    LOG_WARN(("Var is non editable"))
  }
  else
  {
    (*varInteger) = val;
  }
}


//
// IFaceVar::GetFloatValue
//
// Return the value of the float var, error if it isnt a var
//
F32 IFaceVar::GetFloatValue()
{
  if (varPtr == NULL || varType != VarSys::VI_FPOINT)
  {
    ERR_FATAL(("Var is not a float"));
  }

  return **varFloat;
}


//
// IFaceVar::SetFloatValue
//
// Set the value of the float var, error if it isnt a var
//
void IFaceVar::SetFloatValue(F32 val)
{
  if (varPtr == NULL || varType != VarSys::VI_FPOINT)
  {
    ERR_FATAL(("Var is not a float"));
  }

  if (((*varPtr)->flags & VarSys::NOEDIT))
  {
    LOG_WARN(("Var is non editable"))
  }
  else
  {
    (*varFloat) = val;
  }
}


//
// IFaceVar::GetItem
//
// Return a reference to the VarItem object of the var
//
VarSys::VarItem &IFaceVar::GetItem()
{
  if (!IsValid())
  {
    ERR_FATAL(("IFaceVar is not valid"));
  }

  return *(varPtr->item);
}


//
// IFaceVar::IsValid
//
// Test if the var is valid
//
Bool IFaceVar::IsValid()
{
  return (varPtr != NULL) ? TRUE : FALSE;
}
