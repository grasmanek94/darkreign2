/*
 *  IMESHUtil.h
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
 * This file contains the prototypes for the IMESH Utility functions that
 * manage IMESH memory, read and writing of IMESHes, etc.
 *
 *
 */

/*
#ifndef IMESHUTIL_DOT_H
#define IMESHUTIL_DOT_H

#include <iostream>
#include "IMESH.h"
#include <fstream>

#define BOGUS_TEXCOORD_INDEX 9999999
#define BOGUS_NORMAL_INDEX 9999999

// Some useful types:
#ifndef FALSE
#define FALSE 0
#define TRUE 1
typedef int BOOL;
#endif // !FALSE

typedef std::ostream OutputStream;
typedef std::istream InputStream;

void initIMESH(IMESH *imesh);
void allocateGenericMemoryIMESH (IMESH *imesh);
void allocateMemoryIMESH (IMESH *imesh, BOOL allocateMaterials = FALSE);
void freeIMESH(IMESH *imesh, IMATERIAL *iMat = NULL, BOOL inAGroup = FALSE);
void freeIMESHGROUP(IMESHGROUP *grpMesh);
int sortIMESHFaces(IMESH *imesh, int numMaterials);
int sortIMESHTriStrips(IMESH *imesh1, int numMaterials);
IMESH * copyIMESH (IMESH *imesh1);

BOOL writeIMESHVersion	(OutputStream *outStream);
BOOL readIMESHVersion	(InputStream *inStream);

BOOL readIMESH  (IMESH *imesh, IMATERIAL *iMat, InputStream *inStream);
BOOL writeIMESH (IMESH *imesh, IMATERIAL *iMat, OutputStream *outStream);

BOOL writeIMESHGROUP(IMESHGROUP *grpMesh, OutputStream *outStream);
BOOL readIMESHGROUP	(IMESHGROUP *grpMesh, InputStream *inStream);

BOOL writeIMESHGROUP(IMESHGROUP *grpMesh, char *fileName);
BOOL readIMESHGROUP(IMESHGROUP *grpMesh, char *fileName);

#endif
*/

#ifndef IMESHUTIL_DOT_H
#define IMESHUTIL_DOT_H

#pragma pack(push, _save_mrmgen_h_)
#pragma warning(push,3)

#include <iostream>
#include <fstream>
#include "IMESH.h"

#pragma warning(pop)
#pragma pack(pop, _save_mrmgen_h_)

// USEFUL TYPES
#ifndef FALSE
#define FALSE	0
#define TRUE	1
typedef int BOOL;
#endif // !FALSE


/////////////////////////////////////////////////////////
/*-----------------------------------------------------*/
// INDICE DEFINES
#define BOGUS_TEXCOORD_INDEX		9999999
#define BOGUS_NORMAL_INDEX			9999999
/*-----------------------------------------------------*/
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
/*-----------------------------------------------------*/
// DEFAULT IMESH VALUE DEFINES		
#define DEF_ICOLOR_ALPHA			1.0f
#define	DEF_IMATERIAL_AMB_R			0.0f
#define	DEF_IMATERIAL_AMB_G			0.7f
#define	DEF_IMATERIAL_AMB_B			0.3f
#define	DEF_IMATERIAL_AMB_A			1.0f
#define	DEF_IMATERIAL_DIF_R			0.0f
#define	DEF_IMATERIAL_DIF_G			0.7f
#define	DEF_IMATERIAL_DIF_B			0.3f
#define	DEF_IMATERIAL_DIF_A			1.0f

#define	DEF_IMATERIAL_SPEC_R		0.0f
#define	DEF_IMATERIAL_SPEC_G		0.7f
#define	DEF_IMATERIAL_SPEC_B		0.3f
#define	DEF_IMATERIAL_SPEC_A		1.0f

#define DEF_IMATERIAL_SHINY			10.0f
/*-----------------------------------------------------*/
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
/*-----------------------------------------------------*/
// TYPEDEFS
typedef std::ostream OutputStream;
typedef std::istream InputStream;
/*-----------------------------------------------------*/
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
/*-----------------------------------------------------*/
// PROTOTYPES
void initIMESH(IMESH *imesh);

unsigned int getSize(IMESH *m);

void allocateGenericMemoryIMESH (IMESH *imesh);
void allocateMemoryIMESH (IMESH *imesh, BOOL allocateMaterials = FALSE);

void freeIMESH(IMESH *imesh, IMATERIAL *iMat = NULL, BOOL inAGroup = FALSE);
void freeIMESHGROUP(IMESHGROUP *grpMesh);

int sortIMESHFaces(IMESH *imesh, int numMaterials);
int sortIMESHTriStrips(IMESH *imesh1, int numMaterials);

IMESH * copyIMESH (IMESH *imesh1);

BOOL writeIMESHVersion	(OutputStream *outStream);
BOOL writeIMESH (IMESH *imesh, IMATERIAL *iMat, OutputStream *outStream);
BOOL writeIMESHGROUP(IMESHGROUP *grpMesh, OutputStream *outStream);
BOOL writeIMESHGROUP(IMESHGROUP *grpMesh, char *fileName);

BOOL readIMESH  (IMESH *imesh, IMATERIAL *iMat, InputStream *inStream);
BOOL readIMESHVersion	(InputStream *inStream);
BOOL readIMESHGROUP	(IMESHGROUP *grpMesh, InputStream *inStream);
BOOL readIMESHGROUP(IMESHGROUP *grpMesh, char *fileName);
/*-----------------------------------------------------*/
/////////////////////////////////////////////////////////

#endif
