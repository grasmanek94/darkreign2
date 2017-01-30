/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-98 Activision Studios, Strategy Group
//
// Gauge control
//
// 15-JAN-1999
//


#ifndef __ICGAUGE_H
#define __ICGAUGE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICGauge
//
class ICGauge : public IControl
{
  PROMOTE_LINK(ICGauge, IControl, 0x291928BA); // "ICGauge"
  
public:

	// Gauge orientation
	enum Orientation
	{
		VERTICAL,
		HORIZONTAL,
	};
	
protected:

	// Current value
	IFaceVar *gaugeVar;

	// Gauge orientation
	Orientation orientation;
	
	// Value range
	F32 valueMin;
	F32 valueMax;
	F32 valueCur;
	F32 valueRange;

	// Pixel range
	S32 pixelRange;

protected:
	
	void Notify();
	
	// Get the gauge value
	virtual void GetGaugeValue();
	
	// Set the gauge value
	virtual void SetGaugeValue(F32 value);

	// Render the gauge control
	void DrawSelf(PaintInfo &pi);
	
	// Initialize range
	void InitRange();

public:
	// Gauge control constructor
	ICGauge(IControl *parents);
	
	// Gauge control destructor
	~ICGauge();
	
	// Set up the gauge control
	void Setup(FScope *fScope);
	
	// Activate the gauge control
	Bool Activate();
	
	// Set the orientation of the gauge
	void SetOrientation(ICGauge::Orientation o);
	
	// Set the orientation of the gauge using the string representation
	void SetOrientation(const char *s);
};


//
// Type definitions
//
typedef Reaper<ICGauge> ICGaugePtr;


#endif
