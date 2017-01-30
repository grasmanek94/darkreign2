// Stroke.cpp: implementation of the CStroke class.
//
//////////////////////////////////////////////////////////////////////

const unsigned char MSGTYPE_STROKE			= 4;

#include "stdafx.h"

#include "WhiteBoard.h"
#include "Stroke.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

void CStroke::DrawStroke( CDC* pDC )
{
	if (m_PointList.IsEmpty())
		return;


	// Create a temporary pen for drawing 
	// NOTE: Win2000 will not allow you to create a pen and select it
	// all in one statement
	// i.e.: CPen* pOldPen = pDC->SelectObject(&CPen(PS_SOLID,9,9)) // WONT WORK
	CPen thePen(PS_SOLID,mStrokeSize,mStrokeColor);

	// Select it into the DC 
	CPen* pOldPen = pDC->SelectObject(&thePen);	

	// Position ourselves at the beginning of the stroke
	CPoint* pPoint = m_PointList.GetHead();
	pDC->MoveTo(*pPoint);

	// Connect the dots on the stroke points
	for(POSITION pos = m_PointList.GetHeadPosition(); pos != NULL; )
	{
		// Get the next stroke, then draw a line to it
		pPoint = m_PointList.GetNext(pos);
		pDC->LineTo(*pPoint);
	}

	// Restore the old pen
	pDC->SelectObject(pOldPen);
}

void CStroke::DeleteContents()
{
	while(!m_PointList.IsEmpty())
	{
		delete m_PointList.RemoveHead();
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStroke::CStroke()
{
	mStrokeColor = 0;
	mStrokeSize = 0;
}


CStroke::CStroke(CPoint point)
{
	CPoint* pPoint = new CPoint(point);
	m_PointList.AddTail( pPoint );

	// Call the default constructor to initialize members
	CStroke::CStroke();
}

CStroke::~CStroke()
{}





/********************************************************************************
 * Calculate the size of the stroke												*
 ********************************************************************************/
int CStroke::GetSize()
{
	int nBufSize = 0;		// leave an extra byte for the message header

	nBufSize += sizeof(MSGTYPE_STROKE);	// 1 byte
	nBufSize += sizeof(mStrokeSize);	// 4 bytes
	nBufSize += sizeof(mStrokeColor);	// 4 bytes
	nBufSize += sizeof(int);			// 4 bytes
	nBufSize += m_PointList.GetCount() * sizeof(CPoint);
										// 8 bytes per point
	return nBufSize;
}





/********************************************************************************
 * Assemble the stroke data into a linear buffer for net transfer, returns size	*
 ********************************************************************************/
void CStroke::StoreToBuffer(unsigned char* pBuf)
{
	// Our btye place holder
	int byte_num = 0;

	// Insert message prefix
	pBuf[byte_num] = MSGTYPE_STROKE;
	byte_num += sizeof(MSGTYPE_STROKE);

	// Avoid trying to store empty strokes
	if (m_PointList.IsEmpty())
		return;

	// Store the stroke's size into the buffer
	memcpy( &pBuf[byte_num], &mStrokeSize, sizeof(mStrokeSize));
	byte_num += sizeof(mStrokeSize);

	// Store the stroke's color into the buffer
	memcpy( &pBuf[byte_num], &mStrokeColor, sizeof(mStrokeColor));
	byte_num += sizeof(mStrokeColor);

	// Store the buffer size into the buffer
	int buf_size = m_PointList.GetCount() * sizeof(CPoint);
	memcpy( &pBuf[byte_num], &buf_size, sizeof(buf_size));
	byte_num += sizeof(buf_size);

	/* Assemble the points into the linear buffer */
	CPoint* pPoint = m_PointList.GetHead();					// start at the top

	for(POSITION pos = m_PointList.GetHeadPosition(); pos != NULL;  )
	{
		// Retrieve the next point
		pPoint = m_PointList.GetNext(pos);
		
		// Store this point in the buffer
		memcpy(&pBuf[byte_num], pPoint, sizeof(CPoint));
		byte_num += sizeof(CPoint);
	}
	
}





/********************************************************************************
 * Load the stroke data from the linear buffer									*
 ********************************************************************************/
void CStroke::LoadFromBuffer(const unsigned char* pBuf)
{
	// Our place holder
	int byte_num = 1;								// skip the 1 byte message type

	// Load the stroke's size
	memcpy(&mStrokeSize, &pBuf[byte_num], sizeof(mStrokeSize));
	byte_num += sizeof(mStrokeSize);

	// Load the stroke's color
	memcpy(&mStrokeColor, &pBuf[byte_num], sizeof(mStrokeColor));
	byte_num += sizeof(mStrokeColor);

	// Load the points into CPoint objects
	CPoint* pPoint;					// temp storage
	int last_byte = 0;				// the last byte = header size + point buffer size
	
	// retrieve the point buffer size
	memcpy(&last_byte, &pBuf[byte_num], sizeof(last_byte));	
	last_byte += byte_num;
	byte_num += sizeof(last_byte);

	// For each segment of nStrokeSize bytes inthe buffer (i.e. each stroke)
	while (byte_num < last_byte)
	{
		// Create a new point and copy the data into it
		pPoint = new CPoint;
		
		memcpy(pPoint, &(pBuf[byte_num]), sizeof(CPoint));
		byte_num += sizeof(CPoint);	// increment our place holder

		// Add the point to the list
		m_PointList.AddTail(pPoint);
	}
}