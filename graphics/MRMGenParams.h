/*
 *  MRMGenParams.h
 *
 *              INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of a license agreement or
 *  nondisclosure agreement with Intel Corporation and may not be copied
 *  or disclosed except in accordance with the terms of that agreement.
 *
 *      Copyright (c) 1998 Intel Corporation. All Rights Reserved.
 *
 *
 */
#ifndef MRMGENPARAMS_DOT_H
#define MRMGENPARAMS_DOT_H


// parameter valid flags
#define MRMGP_ALL               0xffffffff

#define MRMGP_MERGETHRESH       0x00000001
#define MRMGP_MERGEWITHIN       0x00000002
#define MRMGP_PROGRESSCALLBACK  0x00000004
#define MRMGP_PROGRESSFREQUENCY 0x00000008
#define MRMGP_NUMBASEVERTICES   0x00000010 // if this is valid then baseVertices is valid
#define MRMGP_METRIC            0x00000020
#define MRMGP_METRIC2			0x00000040 // if this is valid then metric2Start is valid
#define MRMGP_NORMALSMODE		0x00000080
#define MRMGP_CREASEANGLE		0x00000100

extern "C"
{

typedef enum _MRMG_METRIC { 
	HYBRID0,  
	HYBRID1,
	HYBRID2,
	HYBRID3
} MRMG_METRIC;


typedef void (* MRMG_PROGRESSCALLBACK) (int pairsRemaining);

typedef enum {PerVertex, PerFacePerVertex,} NormalsMode;
  
typedef struct MRMGENPARAMS_TAG
{
	unsigned long	size;				// size of MRMGenParams;
	unsigned long	flags;				// indicates which fields are valid.
	float			mergeThresh;		// Vertices within this distance of each other are allowed to merge even if not connected by edges.
	unsigned long	mergeWithin;		// This allows vertices within the same object to merge, default behavior is to only merge across objects.
	MRMG_PROGRESSCALLBACK progressCallback;// this will be called after progressFrequency 
	unsigned long	progressFrequency;	// number of edge removals per call back, first callback occurs at 0.
	unsigned long	numBaseVertices;	// number entries in the baseVertices array.
	unsigned long	*baseVertices;		// array of vertex indicies.  Indicates vertices that should be removed last.
	MRMG_METRIC		metric;				// this metric always starts with first vertex removed
	MRMG_METRIC		metric2;
	unsigned long	metric2Start;		// which vertex deletion should trigger switch to metric2.

	NormalsMode		normalsMode;		// Indicates how normals should be maintained and updated.
	float			normalsCreaseAngle; // If PerFacePerVertex normals are used, normalCreaseAngle indicates
	                                    // a angle (deg) threshold for sharing a vertex normal b/t two faces. 

} MRMGenParams;;

}
#endif