// AdSampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdSample.h"
#include "AdSampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace WONAPI;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdSampleDlg dialog

CAdSampleDlg::CAdSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAdSampleDlg::IDD, pParent),
	m_bPaused(0),
	m_AdViewBMP(true),
	m_TestInstall(true)
{
	//{{AFX_DATA_INIT(CAdSampleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hHandCursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_HAND_CURSOR));
}

void CAdSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdSampleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAdSampleDlg, CDialog)
	//{{AFX_MSG_MAP(CAdSampleDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BMPIMG_RAD, OnBmpImgRad)
	ON_BN_CLICKED(IDC_RGBIMG_RAD, OnRGBImgRad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdSampleDlg message handlers

BOOL CAdSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
		
	((CButton*)GetDlgItem(IDC_BMPIMG_RAD))->SetCheck(true);
	m_AdViewBMP = true;

	// Initilize add API
	AdApi::AdResult result = m_AdApi.Init(AdApi::AdProvider_24x7, "Test Product", NULL, NULL, 0);
	
	if (result == AdApi::AdResult_Success)
	{
		result = m_AdApi.SetPaintCallback(Paint_Callback, this);
		if (result != AdApi::AdResult_Success)
		{
			MessageBox("Error Initilizing Ad display", "Error", MB_ICONSTOP);
		}
	}
	else
	{
		MessageBox("Error Initilizing Ad display", "Error", MB_ICONSTOP);
	}

	// Make sure ad fits on dialog box
	RECT dlgRect;
	GetWindowRect(&dlgRect);
	SetWindowPos(NULL, 0, 0, 468+25, dlgRect.bottom - dlgRect.top, SWP_NOMOVE | SWP_NOZORDER);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAdSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAdSampleDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else // paint the ad
	{
		if (m_AdViewBMP) // Bitmap
		{
			HBITMAP hBmp = NULL;
				
			// create a memory device context
			HDC hWindowDC = ::GetDC(m_hWnd);
			HDC hDC = CreateCompatibleDC(hWindowDC);

			AdApi::AdResult result = m_AdApi.GetDisplayImage(hWindowDC, hBmp);	// Get Bitmap
			
			if (result == AdApi::AdResult_Success)
			{
				SelectObject(hDC, hBmp);									// select Bitmap into memory DC
				BitBlt(dc, 10, 10, 478, 70, hDC, 0, 0, SRCCOPY);			// blt the Bitmap onto the screen
				m_AdApi.ReleaseDisplayImage(hBmp);
			}
			//else
			//{
			//	MessageBox("Error getting ad image", "Error", MB_ICONSTOP);
			//}
	
			// delete the memory device context
			::ReleaseDC(m_hWnd, hWindowDC); 
			DeleteDC(hDC);
		}
		else // RGB
		{
			BYTE* RGBBuffer;
			int SizeX, SizeY;
			AdApi::AdResult result = m_AdApi.GetDisplayImage(RGBBuffer, SizeX, SizeY);	// Get Raw RGB pointer
			
			if (result == AdApi::AdResult_Success)
			{
				if (RGBBuffer)
				{
					// Display image
					int i, j, r, g, b;
					int ix = 0;

					for (j=0; j<SizeY; j++)
					{
						for (i=0; i<SizeX; i++)	
						{
							r = RGBBuffer[ix++];
							g = RGBBuffer[ix++];
							b = RGBBuffer[ix++];
							dc.SetPixel(10 + i, 10 + j, RGB(r, g, b));
						}
					}
				}
			}
			//else
			//{
			//	MessageBox("Error getting ad image", "Error", MB_ICONSTOP);
			//}
			m_AdApi.ReleaseDisplayImage(RGBBuffer);	
		}

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAdSampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CAdSampleDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	AdApi::AdResult result = m_AdApi.Shutdown();
	if (result != AdApi::AdResult_Success)
	{
		MessageBox("Error on shutdown", "Error", MB_ICONSTOP);
	}
}

void CAdSampleDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect ad_rect(CPoint(10, 10), CSize(468, 60));
	if (ad_rect.PtInRect(point))
	{
		AdApi::AdResult result = m_AdApi.AdClicked();
		if (result != AdApi::AdResult_Success)
		{
			MessageBox("Error launching browser", "Error", MB_ICONSTOP);
		}
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CAdSampleDlg::OnBmpImgRad() 
{
	m_AdViewBMP = true;
}

void CAdSampleDlg::OnRGBImgRad() 
{
	m_AdViewBMP = false;
}

BOOL CAdSampleDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CRect ad_rect(CPoint(10, 10), CSize(468, 60));
	CPoint point;

	GetCursorPos(&point);
	ScreenToClient(&point);

	if (ad_rect.PtInRect(point)) {
		SetCursor(m_hHandCursor);
		return TRUE;
	} else {
		return CDialog::OnSetCursor(pWnd, nHitTest, message);
	}
}

// Callbacks
void __stdcall Paint_Callback(void *data)
{
	CAdSampleDlg *dlg = reinterpret_cast<CAdSampleDlg*>(data);
	CRect ad_rect(CPoint(10, 10), CSize(468, 60));
	dlg->InvalidateRect(&ad_rect, FALSE);
}












