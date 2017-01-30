/*
 *  MRMGen.h
 *
 *              INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of a license agreement or
 *  nondisclosure agreement with Intel Corporation and may not be copied
 *  or disclosed except in accordance with the terms of that agreement.
 *
 *      Copyright (c) 1998 Intel Corporation. All Rights Reserved.
 *
 *
 *  PURPOSE:
 * 
 *  This file protypes the COM interfaces for the MRMGen DLL.  At present
 *  2 interfaces are defined: IMRMGen2 and IMRMGen.  IMRMGen2 is the newest 
 *  and recommended interface.  IMRMGen is the older, "MRMGen 1" interface.
 *  We include it only for backward compatibility.   Both are COM compliant
 *  interfaces with definitions of QueryInterface(), AddRef(), and Release().  
 *
*/


#ifndef MRMGEN_DOT_H
#define MRMGEN_DOT_H

#pragma pack(push, _save_mrmgen_h_)
#pragma warning(push,3)

#ifdef _WIN32

#define COM_NO_WINDOWS_H

#include <objbase.h>

#else

#define IUnknown        void

#endif

// local includes go here
#include "IMESHutil.h"
#include "MRMUpdate.h"
#include "MRMGenParams.h"

#pragma warning(pop)
#pragma pack(pop, _save_mrmgen_h_)

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef _WIN32
/* GUID Definitions: */
/* {4604BE40-54E0-11d1-A762-00A0C908F8B9} */
DEFINE_GUID(CLSID_CMRMGEN, 0x4604be40, 0x54e0, 0x11d1, 0xa7, 0x62, 0x0, 0xa0, 0xc9, 0x8, 0xf8, 0xb9);
/* {4604BE41-54E0-11d1-A762-00A0C908F8B9} */
DEFINE_GUID(IID_IMRMGEN, 0x4604be41, 0x54e0, 0x11d1, 0xa7, 0x62, 0x0, 0xa0, 0xc9, 0x8, 0xf8, 0xb9);
/* {5BC10BC1-6846-11d2-A131-00A0C9240CB9} */
DEFINE_GUID(IID_IMRMGEN2, 0x5bc10bc1, 0x6846, 0x11d2, 0xa1, 0x31, 0x0, 0xa0, 0xc9, 0x24, 0xc, 0xb9);
#endif


/* The MRMGen2 Interface:
 * New and simplier interface to MRMGen. Also provides vertex and face 
 * re-mapping arrays. 
 */
#ifdef _WIN32
#undef INTERFACE

/*
 * The MRMResults structure will encapsulate all of the results generated
 * by the MRMGen2 Inteface for a particular mesh.  The pointers
 * pIMesh and pMrmUpdates point to data structures that define the generated 
 * multi-resolution mesh. pVertexMap and pFaceMap provide a 1:1 mapping of 
 * vertices and faces from the input IMESH to the IMESH pointed to by pIMesh. 
 * This is probably only needed by those wishing to write custom tools or 
 * authoring tool plug-ins.

 * The memory pointed to by this structure and the structure itself should 
 * be de-allocated by calling FreeMRMResults();
 */

typedef struct MRMResults_TAG
{
	int				iResultsTag;	/* MRMGen's Internal tag identifying results. Don't modify!	*/
	IMESH			*pIMesh;		/* Ptr to the re-ordered IMesh								*/
	MRMUpdates		*pMrmUpdates;	/* Ptr to the generated MRMUpdate records					*/
	unsigned long	*pVertexMap;	/* Ptr to a vertex re-map array.	(For tool developers)	*/
	unsigned long	*pFaceMap;		/* Ptr to a face re-map array.	(For tool developers)		*/
	unsigned long	undefIndexValue;/* Value of undefined indices in th re-map arrays.			*/
} MRMResults;


/* MRMGen2 Methods:																		
 * GenerateMRM(), FreeMRMResults() is to be used in place of the now antiquated			
 * IMRMGen call sequence of Load(), Generate(), GetResults(), FreeLoadMemory(),			
 * FreeGenerateMemory().																	
 */
#define INTERFACE IMRMGen2
DECLARE_INTERFACE_( IMRMGen2, IUnknown )
{
    /* IUnknown methods */
    STDMETHOD(QueryInterface)		(THIS_ REFIID riid, LPVOID FAR * ppvObj)	PURE;
    STDMETHOD_(ULONG,AddRef)		(THIS)										PURE;  
    STDMETHOD_(ULONG,Release)		(THIS)										PURE;
      
	/* MRMGen2 methods: */
	STDMETHOD(Kill)					(THIS_ )									PURE;
	STDMETHOD(GenerateMRM)			(THIS_ IMESH *pIMesh, MRMGenParams *pParams, 
											MRMResults **pMrmResults)			PURE;
	STDMETHOD(FreeMRMResults)		(THIS_ MRMResults *pMrmResults)				PURE;
};
#endif



#ifdef _WIN32
#undef INTERFACE

/* The MRMGen 1 Interface 
 * Now an anachronism included only for backward compatibility.  Please use the
 * new MRMGen2 interface instead.
 */
#define INTERFACE IMRMGen
DECLARE_INTERFACE_( IMRMGen, IUnknown )
{
    /* IUnknown methods */
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;  
    STDMETHOD_(ULONG,Release) (THIS) PURE;
      
	/* MRMGEN Methods */

    /* I/O methods */   
	STDMETHOD(Load)					(THIS_ IMESH *imesh, MRMGenParams *params)				PURE;
	STDMETHOD(Generate)				(THIS_ )												PURE;
	STDMETHOD(Kill)					(THIS_ )												PURE;
	STDMETHOD(GetResults)			(THIS_ int *resultsTag, IMESH **im, MRMUpdates **mu)	PURE;
	
	STDMETHOD(FreeLoadMemory)		(THIS_) PURE;
	STDMETHOD(FreeGenerateMemory)	(THIS_)	PURE;   
	STDMETHOD(FreeResultsMemory)	(THIS_ int resultsTag)	PURE;
};
#endif




#ifdef __cplusplus
};
#endif // extern "C"




#endif