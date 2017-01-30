/*
 *  IMESH.h
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
 * The IMESH structures define a sample mesh data structure.  This mesh
 * data structure is used to load mesh data into MRMGen's multiresolution
 * algorithm.   IMESH is also used a sample export mesh strucutre
 * by The MRM Export Plugin and by the ITS Export Plugin and as a sample
 * runtime structure by the OpenGL Viewer sample provided.
 *
 *
 */

#ifndef IMESH_DOT_H
#define IMESH_DOT_H

#define IMESH_VERSION "IMESH 1.0                     "
#define IMESH_BETA_VERSION "IMESH 0.4                     "
#define IMESH_VERSION_LENGTH 32

#define IMESH_NAME_LENGTH 256
#define IMESH_PATH_LENGTH 512

typedef unsigned long IDWORD;


// Attribute present flag masks:
typedef union IMODE_FLAGS_TAG
{
	struct 
	{
		unsigned int faceAttrib			: 1;
		unsigned int triStripAttrib		: 1;
		unsigned int genericHeader		: 1;
		unsigned int genericVertex      : 1;
		unsigned int genericFace		: 1;
		unsigned int genericTriStrip	: 1;
		unsigned int intelReserved1		: 1;
		unsigned int intelReserved2		: 1;
		unsigned int perVertexNormals	: 1;
	};
	IDWORD allBits;	// ... extra bits for future additions

} IMODE_FLAGS;



// IGENERIC can be used to point to a user defined array of records
// stored contiguously in memory (as an array).   There should be
// IGENERIC.num records, each of length IGENERIC.size (in bytes) and
// pointed to by IGENERIC.pData.
typedef struct IGENERIC_TAG
{
	unsigned long num;
	unsigned long	size;
	char *pData;
} IGENERIC;


typedef struct IV2D_TAG 
{
	float u;
	float v;
} IV2D;

	
typedef struct IV3D_TAG 
{
	float x;
	float y;
	float z;
} IV3D;

typedef struct ICOLOR_TAG 
{
	float r;
	float g;
	float b;
	float a;
} ICOLOR;

typedef struct IMATERIAL_TAG
{
	char matName[IMESH_NAME_LENGTH];
	ICOLOR ambient;
	ICOLOR diffuse;
	ICOLOR specular;
	float shininess;
	int	 textureFlag;
	char texturePath[IMESH_PATH_LENGTH];
} IMATERIAL;


typedef struct IFACE_ATTRIBUTE_TAG
{
	unsigned long			matID;			// index into material list
	unsigned long			smoothID;		// Smoothing group.
	unsigned long			ta, tb, tc;		// Texture Coordinate indices
	unsigned long			na, nb, nc;		// Normal Indices.
								// user data
} IFACE_ATTRIBUTE;


typedef struct ITRISTRIP_ATTRIBUTE_TAG
{
	unsigned long		matID;				// index into material list
	unsigned long		*n,					// pointer to normal indices
						*t;					// pointer to tex coord indices
} ITRISTRIP_ATTRIBUTE;


typedef struct IFACE_TAG 
{
	unsigned long				a, b, c;		// Indices to vertices
} IFACE;	  


typedef struct ITRISTRIP_TAG
{
	unsigned long		numVertices;			// Num vertices in this tristrip.
	unsigned long		*v;						// Ptr to indices to vertices
} ITRISTRIP;


typedef struct IMESH_TAG 
{
	char				meshName[IMESH_NAME_LENGTH];
	unsigned long		numVertices;
	unsigned long		numFaces;
	unsigned long		numTriStrips;
	unsigned long		numMaterials;
	unsigned long		numTexCoords;
	unsigned long		numNormals;
	IMODE_FLAGS			mode;

	// User defined header (optional)
	IGENERIC			*pGenericHeader;

	IV3D				*pVertex;
	IFACE				*pFace;
	ITRISTRIP			*pTriStrip;
	IV2D				*pTexCoord;
	IV3D				*pNormal;

	IFACE_ATTRIBUTE		*pFaceAttrib;
	ITRISTRIP_ATTRIBUTE *pTriStripAttrib;
	
	IMATERIAL			*pMaterial;

	// User defined per face and per vertex data (optional):
	IGENERIC			*pGenericFace;
	IGENERIC			*pGenericVertex;
	IGENERIC			*pGenericTriStrip;
} IMESH;


typedef struct IMESHGROUP_TAG
{
	unsigned long		numMeshes;
	unsigned long		numMaterials;

	IMESH		*pMesh;
	IMATERIAL	*pMaterial;
} IMESHGROUP;


#endif
