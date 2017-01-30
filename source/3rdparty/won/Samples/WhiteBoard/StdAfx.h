// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C5231914_0C6C_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_STDAFX_H__C5231914_0C6C_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxtempl.h>		// MFC template classes
#include <afxext.h>         // MFC extensions

// Diable debug name truncation warning (STL bug)
#pragma warning (disable : 4786)

// Titan/WON includes
#include <Directory/DirectoryAPI.h>
#include <Factory/FactoryAPI.h>
#include <Routing/RoutingAPI.h>

#include <msg/ServerStatus.h>		// included for WONMsg namespace (Chat View.cpp)


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C5231914_0C6C_11D4_803B_0050046D274E__INCLUDED_)
