// Stroke.h: interface for the CStroke class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STROKE_H__438E49ED_0C81_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_STROKE_H__438E49ED_0C81_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStroke  
{
// Attributes
public:
	int mStrokeColor;
	int mStrokeSize;
	CTypedPtrList<CPtrList,CPoint*> m_PointList;
// Operations
public:
	void DeleteContents();
	CStroke();
	CStroke(CPoint pPoint);
	void DrawStroke( CDC* pDC );
	virtual ~CStroke();
	void StoreToBuffer(unsigned char*);
	void LoadFromBuffer(const unsigned char*);
	int GetSize();

};

#endif // !defined(AFX_STROKE_H__438E49ED_0C81_11D4_803B_0050046D274E__INCLUDED_)
