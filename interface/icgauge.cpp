/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-98 Activision Studios, Strategy Group
//
// Gauge control
//
// 06-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icgauge.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "ifvar.h"
#include "bitmap.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICGauge implementation
//


//
// ICGauge::ICGauge
//
ICGauge::ICGauge(IControl *parent) 
: IControl(parent), 
  gaugeVar(NULL)
{
	orientation  = HORIZONTAL;
	
	valueMin     = 0;
	valueMax     = 0;
	valueCur     = 0;
	
	// Create a var to handle the gauge value
	VarSys::CreateInteger(DynVarName("value"), 0, VarSys::DEFAULT);
}


//
// ICGauge::~ICGauge
//
ICGauge::~ICGauge()
{
	// Dispose of var
	if (gaugeVar)
	{
		delete gaugeVar;
		gaugeVar = NULL;
	}
}

//
// ICGauge::Notify
//
void ICGauge::Notify()
{
	GetGaugeValue();
}


//
// ICGauge::GetGaugeValue
//
void ICGauge::GetGaugeValue()
{
	VALIDATE(gaugeVar);
	
	// Read current var value
	if (gaugeVar->Type() == VarSys::VI_INTEGER)
	{
		valueCur = F32(gaugeVar->GetIntegerValue());
	}
	else
	{
		valueCur = F32(gaugeVar->GetFloatValue());
	}
}


//
// ICGauge::SetGaugeValue
//
void ICGauge::SetGaugeValue(F32 value)
{
	VALIDATE(gaugeVar);
	
	// Update internal value
	if (gaugeVar->Type() == VarSys::VI_INTEGER)
	{
		gaugeVar->SetIntegerValue(Utils::FtoL(value));
	}
	else
	{
		gaugeVar->SetFloatValue(value);
	}
}


//
// ICGauge::DrawSelf
//
void ICGauge::DrawSelf(PaintInfo &pi)
{
	VALIDATE(gaugeVar);
	
	// update the current value
	GetGaugeValue();
	
	// Draw control
	if (gaugeVar->Type() == VarSys::VI_INTEGER)
	{
		gaugeVar->GetIntegerValue();
	}
	else
	{
		gaugeVar->GetFloatValue();
	}
	
	// render the control background
	DrawCtrlBackground(pi, GetTexture());
  DrawCtrlFrame(pi);
	
	// start with control rectangle
	ClipRect r = pi.client;
	
	// if the control is horizontal
	if (orientation == HORIZONTAL)
	{
		// calculate pixel fill value
		S32 pixelCur = valueRange ? Utils::FastFtoL((valueCur - valueMin) * pixelRange / valueRange) : 0;
		
		// set right edge of rectangle
		r.p1.x = r.p0.x + pixelCur;
	}
	else
	{
		// calculate pixel fill value
		S32 pixelCur = valueRange ? Utils::FastFtoL((valueMax - valueCur) * pixelRange / valueRange) : 0;
		
		// set top edge of rectangle
		r.p0.y = r.p0.y + pixelCur;
	}
	
	// render filled portion
	IFace::RenderRectangle(r, pi.colors->fg[ColorIndex()]);
}


//
// ICGauge::Activate
//
Bool ICGauge::Activate()
{
	if (IControl::Activate())
	{
		// Check and setup the var
		ActivateVar(gaugeVar);
		
		if ((gaugeVar->Type() != VarSys::VI_INTEGER) && (gaugeVar->Type() != VarSys::VI_FPOINT))
		{
			ERR_FATAL(("Integer or Float var expected for slider [%s]", Name()));
		}
		
		// Calculate range of pixel movement of thumb
		pixelRange = (orientation == HORIZONTAL) 
			? paintInfo.client.Width() : paintInfo.client.Height();
		ASSERT(pixelRange > 0);
		
		// Initialise sliding range
		InitRange();
		
		// Get current values of the var
		GetGaugeValue();
		
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}
}

//
// ICGauge::InitRange
//
// Initialise range parameters, only to be done when active
//
void ICGauge::InitRange()
{
	ASSERT(gaugeVar);
	ASSERT(pixelRange > 0);
	
	// Set up ranges
	VarSys::VarItem &item = gaugeVar->GetItem();
	
	if (!(item.flags & VarSys::CLAMP))
	{
		valueMin = 0.0f;
		valueMax = 1.0f;
	}
	else
	{
		switch (gaugeVar->Type())
		{
  		case VarSys::VI_INTEGER:
			{
				valueMin = F32(item.integer.lo);
				valueMax = F32(item.integer.hi);
				break;
			}
			
	  	case VarSys::VI_FPOINT:
			{
				valueMin = F32(item.fpoint.lo);
				valueMax = F32(item.fpoint.hi);
				break;
			}
		}
	}
	
	// Incremental step
	valueRange = valueMax - valueMin;
}


//
// ICGauge::Setup
//
void ICGauge::Setup(FScope *fScope)
{
	switch (fScope->NameCrc())
	{
    case 0x742EA048: // "UseVar"
		{
			ConfigureVar(gaugeVar, fScope);
			break;
		}
		
    case 0xEE2D2689: // "Orientation"
		{
			SetOrientation(fScope->NextArgString());
			break;
		}
		
    default:
		{
			// Pass it to the previous level in the hierarchy
			IControl::Setup(fScope);
			break;
		}
	}
}


//
// ICGauge::SetOrientation
//
void ICGauge::SetOrientation(ICGauge::Orientation o)
{
	orientation = o;
}


//
// ICGauge::SetOrientation
//
void ICGauge::SetOrientation(const char *s)
{
	switch (Crc::CalcStr(s))
	{
    case 0x2942B3CD: // "HORIZONTAL"
		  SetOrientation(HORIZONTAL);
		  break;
		
    case 0xC8F18F06: // "VERTICAL"
		  SetOrientation(VERTICAL);
		  break;
		
    default:
		  ERR_FATAL(("Invalid orientation [%s]", s));
		  break;
	}
}
