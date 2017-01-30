// StrokeSizeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "StrokeSizeDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStrokeSizeDialog dialog


CStrokeSizeDialog::CStrokeSizeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CStrokeSizeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStrokeSizeDialog)
	m_StrokeSize = 0;
	//}}AFX_DATA_INIT
}


void CStrokeSizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStrokeSizeDialog)
	DDX_Text(pDX, IDC_EDIT2, m_StrokeSize);
	DDV_MinMaxInt(pDX, m_StrokeSize, 0, 999);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStrokeSizeDialog, CDialog)
	//{{AFX_MSG_MAP(CStrokeSizeDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStrokeSizeDialog message handlers

int CStrokeSizeDialog::DoModal(int nStrokeSize) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_StrokeSize = nStrokeSize;
	CDialog::DoModal();
	return m_StrokeSize;
}
