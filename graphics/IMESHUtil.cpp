/*
 *  IMESHUtil.cpp
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
 * This file contains the implementation for the IMESH Utility 
 * functions that manage IMESH memory, read and writing of IMESHes, etc.
 *
 *
 */
#include <windows.h>
#include <assert.h>

#include "IMESHUtil.h"

#pragma warning( disable : 4018) 

/////////////////////////////////////////////////////////
/*-----------------------------------------------------*/
// GLOBALS
static char				IMESHversion[IMESH_VERSION_LENGTH];
static char				IMESHBetaversion[IMESH_VERSION_LENGTH];

/*-----------------------------------------------------*/
/////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////
/*-----------------------------------------------------*/
// IMESH RELATED FUNCTIONS

void initIMESH(IMESH *imesh)
{
	ZeroMemory((char*)imesh->meshName,IMESH_NAME_LENGTH);
	sprintf (imesh->meshName , "IMESH No Name");
	imesh->numVertices			= 0;
	imesh->numFaces				= 0;
	imesh->numTriStrips			= 0;
	imesh->numMaterials			= 0;
	imesh->numTexCoords			= 0;
	imesh->numNormals			= 0;

	imesh->mode.allBits			= 0;

	imesh->pGenericHeader		= NULL;

	imesh->pVertex				= NULL;
	imesh->pFace				= NULL;
	imesh->pTriStrip			= NULL;

	imesh->pTexCoord			= NULL;
	imesh->pNormal				= NULL;
	imesh->pFaceAttrib			= NULL;
	imesh->pTriStripAttrib		= NULL;
	imesh->pMaterial			= NULL;

	imesh->pGenericFace			= NULL;
	imesh->pGenericVertex		= NULL;
	imesh->pGenericTriStrip		= NULL;
}


// this is only accurate for imesh's as they exist inside the
// max plug-in, doesn't take into account user attributes or tri-strips.
unsigned int getSize(IMESH *m)
{
   unsigned int size = 0;

   size = sizeof(IMESH);
   size += m->numVertices * sizeof(IV3D);
   size += m->numFaces * sizeof(IFACE);
   size += m->numFaces * sizeof(IFACE_ATTRIBUTE);
   size += m->numNormals * sizeof(IV3D);
   size += m->numTexCoords * sizeof(IV2D);
   size += m->numMaterials * sizeof(IMATERIAL);
  
   return size + 512;  // a little extra
}


BOOL writeIMESHVersion(OutputStream *outStream)
{
	sprintf (IMESHversion, "%s", IMESH_VERSION);
	outStream->write ((char *) IMESHversion, IMESH_VERSION_LENGTH);

	return TRUE;
}


BOOL readIMESHVersion(InputStream *inStream)
{
	char readVersion[IMESH_VERSION_LENGTH];
	char errorString[512];

	sprintf (IMESHversion, "%s", IMESH_VERSION);
	sprintf (IMESHBetaversion, "%s", IMESH_BETA_VERSION);

	inStream->read ((char *) readVersion, IMESH_VERSION_LENGTH);

	// Are versions consistent?:
	if ((strcmp(IMESHversion, readVersion) != 0) && (strcmp(IMESHBetaversion, readVersion) != 0))
	{

		sprintf (errorString, "Error: Incompatible IMESH versions.\n%s%s\n%s%s",
			         "IMESH reader is ", IMESHversion,
					 "IMESH file is   ", readVersion);
		MessageBox (NULL, errorString, "readIMESHVersion Error", MB_OK|MB_SYSTEMMODAL);
		return FALSE;
	}

	return TRUE;
}

BOOL writeIMESH (IMESH *imesh, IMATERIAL *iMat, OutputStream *outStream)
{
	int size = 0;

	// Write out the mesh header:
	outStream->write (imesh->meshName, IMESH_NAME_LENGTH);
   	outStream->write ((char *) &imesh->numVertices, sizeof (unsigned long));
	outStream->write ((char *) &imesh->numFaces, sizeof (unsigned long));
	outStream->write ((char *) &imesh->numMaterials, sizeof (unsigned long));
	outStream->write ((char *) &imesh->numTexCoords, sizeof (unsigned long));
	outStream->write ((char *) &imesh->numNormals, sizeof (unsigned long));
	outStream->write ((char *) &imesh->numTriStrips, sizeof (unsigned long));
	outStream->write ((char *) &imesh->mode, sizeof (IMODE_FLAGS));

	// Write any IGENERIC records present:
	if (imesh->mode.genericHeader)
	{
		outStream->write ((char *) imesh->pGenericHeader, sizeof(IGENERIC));
	}
	if (imesh->mode.genericFace)
	{
		outStream->write ((char *) imesh->pGenericFace, sizeof(IGENERIC));
	}
	if (imesh->mode.genericVertex)
	{
		outStream->write ((char *) imesh->pGenericVertex, sizeof(IGENERIC));
	}
	if (imesh->mode.genericTriStrip)
	{
		outStream->write ((char *) imesh->pGenericTriStrip, sizeof(IGENERIC));
	}

	// Write generic header data if present:
	if (imesh->mode.genericHeader && imesh->pGenericHeader)
	{
		size = imesh->pGenericHeader->size * imesh->pGenericHeader->num;
		outStream->write ((char *) imesh->pGenericHeader->pData, size);
	}

	if (imesh->numVertices > 0){
		outStream->write ((char *) imesh->pVertex, sizeof (IV3D) * imesh->numVertices);
	}
	if (imesh->numFaces > 0){
		outStream->write ((char *) imesh->pFace, sizeof (IFACE) * imesh->numFaces);
	}
	if (imesh->numMaterials > 0){
		outStream->write ((char *) imesh->pMaterial, sizeof (IMATERIAL) * imesh->numMaterials);	
	}
	if (imesh->numTexCoords > 0){
		outStream->write ((char *) imesh->pTexCoord, sizeof (IV2D) * imesh->numTexCoords);
	}
	if (imesh->numNormals > 0){
		outStream->write ((char *) imesh->pNormal, sizeof (IV3D) * imesh->numNormals);
	}	
	if (imesh->mode.faceAttrib){
		outStream->write ((char *) imesh->pFaceAttrib, sizeof (IFACE_ATTRIBUTE) * imesh->numFaces);
	}
	
	// Write generic attributes if present:
	if (imesh->mode.genericFace && imesh->pGenericFace)
	{
		size = imesh->pGenericFace->size * imesh->pGenericFace->num;
		outStream->write ((char *) imesh->pGenericFace->pData, size);
	}
	if (imesh->mode.genericVertex && imesh->pGenericVertex)
	{
		size = imesh->pGenericVertex->size * imesh->pGenericVertex->num;
		outStream->write ((char *) imesh->pGenericVertex->pData, size);
	}
	if (imesh->mode.genericTriStrip && imesh->pGenericTriStrip)
	{
		size = imesh->pGenericTriStrip->size * imesh->pGenericTriStrip->num;
		outStream->write ((char *) imesh->pGenericTriStrip->pData, size);
	}

	// Write Triangle Strips if they exist for this mesh:
	if (imesh->numTriStrips > 0)
	{
		for(int ts=0; ts < imesh->numTriStrips; ts++)
		{
			outStream->write ((char *) &imesh->pTriStrip[ts].numVertices, sizeof(unsigned long));
			outStream->write ((char *) imesh->pTriStrip[ts].v, imesh->pTriStrip[ts].numVertices *sizeof(unsigned long));
		}

		// If a NULL iMat pointer was passed, see if we have a local iMat ptr:
		if ((!iMat) && (imesh->pMaterial)){
			iMat = imesh->pMaterial;
		}

		// Write tristrip attributes if they exist:
		if (imesh->mode.triStripAttrib && iMat)
		{
			for(int tsa=0; tsa < imesh->numTriStrips; tsa++)
			{
				outStream->write ((char *) &imesh->pTriStripAttrib[tsa].matID, sizeof(unsigned long));
				outStream->write ((char *) imesh->pTriStripAttrib[tsa].n, imesh->pTriStrip[tsa].numVertices * sizeof(unsigned long));
				if(iMat[imesh->pTriStripAttrib[tsa].matID].textureFlag)
					outStream->write ((char *) imesh->pTriStripAttrib[tsa].t, imesh->pTriStrip[tsa].numVertices * sizeof(unsigned long));
					else imesh->pTriStripAttrib[tsa].t=NULL; // Reset it to null

			}
		}
	}

	return TRUE;
}

BOOL writeIMESHGROUP(IMESHGROUP *grpMesh, OutputStream *outStream)
{
	BOOL result;

	if (!writeIMESHVersion(outStream)){
		return FALSE;
	}

	outStream->write ((char *) &grpMesh->numMeshes, sizeof(unsigned long));
	outStream->write ((char *) &grpMesh->numMaterials, sizeof (unsigned long));

	if (grpMesh->numMaterials > 0){
		outStream->write ((char *) grpMesh->pMaterial, sizeof (IMATERIAL) * grpMesh->numMaterials);	
	}

	for(int g=0; g<grpMesh->numMeshes; g++)
	{
		grpMesh->pMesh[g].numMaterials = 0;
		result = writeIMESH (&grpMesh->pMesh[g], grpMesh->pMaterial, outStream);
		if (!result){
			return FALSE;
		}
	}

	return TRUE;
}

BOOL readIMESH (IMESH *imesh, IMATERIAL *iMat, InputStream *inStream)
{
	int size = 0;

	initIMESH(imesh);
	// Read in the mesh header:
	inStream->read ((char *) imesh->meshName, IMESH_NAME_LENGTH);
   	inStream->read ((char *) &imesh->numVertices, sizeof (unsigned long));
	inStream->read ((char *) &imesh->numFaces, sizeof (unsigned long));
	inStream->read ((char *) &imesh->numMaterials, sizeof (unsigned long));
	inStream->read ((char *) &imesh->numTexCoords, sizeof (unsigned long));
	inStream->read ((char *) &imesh->numNormals, sizeof (unsigned long));
	inStream->read ((char *) &imesh->numTriStrips, sizeof (unsigned long));
	inStream->read ((char *) &imesh->mode, sizeof (IMODE_FLAGS));

	allocateMemoryIMESH (imesh, TRUE);

	// Read any IGENERIC records present:
	if (imesh->mode.genericHeader)
	{
		inStream->read ((char *) imesh->pGenericHeader, sizeof(IGENERIC));
		imesh->pGenericHeader->pData = NULL;
	}
	if (imesh->mode.genericFace)
	{
		inStream->read ((char *) imesh->pGenericFace, sizeof(IGENERIC));
		imesh->pGenericFace->pData = NULL;
	}
	if (imesh->mode.genericVertex)
	{
		inStream->read ((char *) imesh->pGenericVertex, sizeof(IGENERIC));
		imesh->pGenericVertex->pData = NULL;
	}
	if (imesh->mode.genericTriStrip)
	{
		inStream->read ((char *) imesh->pGenericTriStrip, sizeof(IGENERIC));
		imesh->pGenericTriStrip->pData = NULL;
	}

	allocateGenericMemoryIMESH (imesh);

	// All IMESH memory should now be allocated:

	// Read generic header data if present:
	if (imesh->mode.genericHeader && imesh->pGenericHeader)
	{
		size = imesh->pGenericHeader->size * imesh->pGenericHeader->num;
		inStream->read ((char *) imesh->pGenericHeader->pData, size);
	}

	if (imesh->numVertices > 0)
	{
		size = sizeof (IV3D) * imesh->numVertices;
		inStream->read ((char *) imesh->pVertex, size);
	}
	if (imesh->numFaces > 0)
	{
		size = sizeof (IFACE) * imesh->numFaces;
		inStream->read ((char *) imesh->pFace, size);
	}
	
	if (imesh->numMaterials > 0)
	{
		size = sizeof (IMATERIAL) * imesh->numMaterials;
		inStream->read ((char *) imesh->pMaterial, size);
	}
	if (imesh->numTexCoords > 0)
	{
		size = sizeof (IV2D) * imesh->numTexCoords;
		inStream->read ((char *) imesh->pTexCoord, size);
	}
	if (imesh->numNormals > 0)
	{
		size = sizeof (IV3D) * imesh->numNormals;
		inStream->read ((char *) imesh->pNormal, size);
	}	
	if (imesh->mode.faceAttrib)
	{
		size = sizeof (IFACE_ATTRIBUTE) * imesh->numFaces;
		inStream->read ((char *) imesh->pFaceAttrib, size);
	}
	if (imesh->mode.genericFace && imesh->pGenericFace)
	{
		size = imesh->pGenericFace->size * imesh->pGenericFace->num;
		inStream->read ((char *) imesh->pGenericFace->pData, size);
	}
	if (imesh->mode.genericVertex && imesh->pGenericVertex)
	{
		size = imesh->pGenericVertex->size * imesh->pGenericVertex->num;
		inStream->read ((char *) imesh->pGenericVertex->pData, size);
	}
	if (imesh->mode.genericTriStrip && imesh->pGenericTriStrip)
	{
		size = imesh->pGenericTriStrip->size * imesh->pGenericTriStrip->num;
		inStream->read ((char *) imesh->pGenericTriStrip->pData, size);
	}
	
	// Triangle Strips
	if (imesh->numTriStrips > 0)
	{
		for(int ts=0; ts < imesh->numTriStrips; ts++)
		{
			// Allocate memory for each individual tri-strip array:
			inStream->read ((char *) &imesh->pTriStrip[ts].numVertices, sizeof(unsigned long));
			imesh->pTriStrip[ts].v = (unsigned long *) malloc(imesh->pTriStrip[ts].numVertices * sizeof(unsigned long));
			assert(imesh->pTriStrip[ts].v);
			inStream->read ((char *) imesh->pTriStrip[ts].v, imesh->pTriStrip[ts].numVertices * sizeof(unsigned long));	
		}

		// If a NULL iMat pointer was passed, see if we have a local iMat ptr:
		if ((!iMat) && (imesh->pMaterial)){
			iMat = imesh->pMaterial;
		}
		
		// Read tristrip attributes:
		if(imesh->mode.triStripAttrib && iMat)
		{
			for(int tsa=0; tsa < imesh->numTriStrips; tsa++)
			{
				inStream->read ((char *) &imesh->pTriStripAttrib[tsa].matID, sizeof(unsigned long));
				imesh->pTriStripAttrib[tsa].n = (unsigned long *) malloc(imesh->pTriStrip[tsa].numVertices * sizeof(unsigned long));
				assert(imesh->pTriStripAttrib[tsa].n);
				inStream->read ((char *) imesh->pTriStripAttrib[tsa].n, imesh->pTriStrip[tsa].numVertices * sizeof(unsigned long));

				//Check Material to see if a Texture is associated with it
				if(iMat[imesh->pTriStripAttrib[tsa].matID].textureFlag){
					imesh->pTriStripAttrib[tsa].t = (unsigned long *) malloc(imesh->pTriStrip[tsa].numVertices * sizeof(unsigned long));
					assert(imesh->pTriStripAttrib[tsa].t);
					inStream->read ((char *) imesh->pTriStripAttrib[tsa].t, imesh->pTriStrip[tsa].numVertices * sizeof(unsigned long));
				}
				else imesh->pTriStripAttrib[tsa].t=NULL; // reset to null

			}
		}
	}
	return TRUE;
}

BOOL readIMESHGROUP(IMESHGROUP *grpMesh, InputStream *inStream)
{
	int result;

	if (!readIMESHVersion(inStream)){
		return FALSE;
	}

	inStream->read ((char *) &grpMesh->numMeshes, sizeof(unsigned long));
	inStream->read ((char *) &grpMesh->numMaterials, sizeof (unsigned long));

	grpMesh->pMesh = (IMESH *) malloc(grpMesh->numMeshes * sizeof(IMESH));
	assert(grpMesh->pMesh);

	grpMesh->pMaterial = (IMATERIAL *) malloc(grpMesh->numMaterials * sizeof(IMATERIAL));
	assert(grpMesh->pMaterial);

	if (grpMesh->numMaterials > 0){
		inStream->read ((char *) grpMesh->pMaterial, sizeof (IMATERIAL) * grpMesh->numMaterials);	
	}

	for(int g=0; g<grpMesh->numMeshes; g++)
	{
		initIMESH (&grpMesh->pMesh[g]);
		__try{
			result = readIMESH (&grpMesh->pMesh[g], grpMesh->pMaterial, inStream);
		}
		__except(1){
			return FALSE;
		}
		if (!result){
			return FALSE;
		}
	}

	return TRUE;
}

BOOL writeIMESHGROUP(IMESHGROUP *imeshGroup, char *fileName)
{
	char errorString[256];
	std::ofstream imfFileStream (fileName, std::ios::out | std::ios::binary);

	if (!imfFileStream)
	{
		sprintf (errorString, "Error: Could not open file %s.", fileName);
		MessageBox (NULL, errorString, "writeIMESHGROUP() Error", MB_OK);
		return FALSE;
	}

	if (!writeIMESHGROUP (imeshGroup, &imfFileStream))
	{
		imfFileStream.close();
		return FALSE;
	}

	imfFileStream.close();

	return TRUE;
}


BOOL readIMESHGROUP(IMESHGROUP *imeshGroup, char *fileName)
{
	char errorString[256];
	std::ifstream imfFileStream (fileName, std::ios::in | std::ios::binary);

	if (!imfFileStream)
	{
		sprintf (errorString, "Error: Could not open file %s.", fileName);
		MessageBox (NULL, errorString, "readIMESHGROUP() Error", MB_OK);
		return FALSE;
	}

	if (!readIMESHGROUP (imeshGroup, &imfFileStream))
	{
		imfFileStream.close();
		return FALSE;
	}

	imfFileStream.close();

	return TRUE;

}

void allocateMemoryIMESH (IMESH *imesh, BOOL allocateMaterials)
{
//	int size = 0;
	
	// Allocate any needed generic attribute records:
	if (imesh->mode.genericHeader)
	{
//		imesh->pGenericHeader			= (IGENERIC *) malloc (sizeof(IGENERIC));
		imesh->pGenericHeader = new IGENERIC;
		assert(imesh->pGenericHeader);
		imesh->pGenericHeader->num		= 0;
		imesh->pGenericHeader->size		= 0;
		imesh->pGenericHeader->pData	= NULL;
	}
	if (imesh->mode.genericFace)
	{
//		imesh->pGenericFace				= (IGENERIC *) malloc (sizeof(IGENERIC));
		imesh->pGenericFace = new IGENERIC;
		assert(imesh->pGenericFace);
		imesh->pGenericFace->num		= 0;
		imesh->pGenericFace->size		= 0;
		imesh->pGenericFace->pData		= NULL;
	}
	if (imesh->mode.genericVertex)
	{
//		imesh->pGenericVertex			= (IGENERIC *) malloc (sizeof(IGENERIC));
		imesh->pGenericVertex = new IGENERIC;
		assert(imesh->pGenericVertex);
		imesh->pGenericVertex->num		= 0;
		imesh->pGenericVertex->size		= 0;
		imesh->pGenericVertex->pData	= NULL;
	}
	if (imesh->mode.genericTriStrip)
	{
//		imesh->pGenericTriStrip			= (IGENERIC *) malloc (sizeof(IGENERIC));
		imesh->pGenericTriStrip = new IGENERIC;
		assert(imesh->pGenericTriStrip);
		imesh->pGenericTriStrip->num	= 0;
		imesh->pGenericTriStrip->size	= 0;
		imesh->pGenericTriStrip->pData	= NULL;
	}

	if (imesh->numVertices > 0)
	{
//		size = sizeof (IV3D) * imesh->numVertices;
//		imesh->pVertex = (IV3D *) malloc (size);
		imesh->pVertex =  new IV3D[imesh->numVertices];
		assert(imesh->pVertex);
	}
	if (imesh->numFaces > 0)
	{
//		size = sizeof (IFACE) * imesh->numFaces;
//		imesh->pFace = (IFACE *) malloc (size);
		imesh->pFace = new IFACE[imesh->numFaces];
		assert(imesh->pFace);
	}
	if (imesh->numTriStrips > 0)
	{
//		size = sizeof(ITRISTRIP) * imesh->numTriStrips;
//		imesh->pTriStrip = (ITRISTRIP *) malloc(size);
		imesh->pTriStrip = new ITRISTRIP[imesh->numTriStrips];
		assert(imesh->pTriStrip);
	}
	
	if ((imesh->numMaterials > 0) && allocateMaterials)
	{
//		size = sizeof (IMATERIAL) * imesh->numMaterials;
//		imesh->pMaterial = (IMATERIAL *) malloc (size);
		imesh->pMaterial =  new IMATERIAL[imesh->numMaterials];
		assert(imesh->pMaterial);
	}
	if (imesh->numTexCoords > 0)
	{
//		size = sizeof (IV2D) * imesh->numTexCoords;
//		imesh->pTexCoord = (IV2D *) malloc (size);
		imesh->pTexCoord =  new IV2D[imesh->numTexCoords];
		assert(imesh->pTexCoord);
	}
	if (imesh->numNormals > 0)
	{
//		size = sizeof (IV3D) * imesh->numNormals;
//		imesh->pNormal = (IV3D *) malloc (size);
		imesh->pNormal = new IV3D[imesh->numNormals];
		assert(imesh->pNormal);
	}
	
	if (imesh->mode.faceAttrib)
	{
//		size = sizeof (IFACE_ATTRIBUTE) * imesh->numFaces;
//		imesh->pFaceAttrib = (IFACE_ATTRIBUTE *) malloc (size);
		imesh->pFaceAttrib = new IFACE_ATTRIBUTE[ imesh->numFaces];
    assert(imesh->pFaceAttrib);
	}
	if (imesh->mode.triStripAttrib)
	{
//		size = sizeof(ITRISTRIP_ATTRIBUTE) * imesh->numTriStrips;
//		imesh->pTriStripAttrib = (ITRISTRIP_ATTRIBUTE *) malloc(size);
		imesh->pTriStripAttrib = new ITRISTRIP_ATTRIBUTE[imesh->numTriStrips];
		assert(imesh->pTriStripAttrib);
	}
}

void allocateGenericMemoryIMESH (IMESH *imesh)
{
	int size;

	if (imesh->mode.genericHeader && imesh->pGenericHeader)
	{
		size = imesh->pGenericHeader->size * imesh->pGenericHeader->num;
//		imesh->pGenericHeader->pData = (char *) malloc (size);
		imesh->pGenericHeader->pData = new char[size];
		assert(imesh->pGenericHeader->pData);
	}
	if (imesh->mode.genericVertex && imesh->pGenericVertex)
	{
		size = imesh->pGenericVertex->size * imesh->pGenericVertex->num;
//		imesh->pGenericVertex->pData = (char *) malloc (size);
		imesh->pGenericVertex->pData = new char[size];
		assert(imesh->pGenericVertex->pData); 
	}
	if (imesh->mode.genericFace && imesh->pGenericFace)
	{
		size = imesh->pGenericFace->size * imesh->pGenericFace->num;
//		imesh->pGenericFace->pData = (char *) malloc (size);
		imesh->pGenericFace->pData = new char[size];
		assert(imesh->pGenericFace->pData);
	}
	if (imesh->mode.genericTriStrip && imesh->pGenericTriStrip)
	{
		size = imesh->pGenericTriStrip->size * imesh->pGenericTriStrip->num;
//		imesh->pGenericTriStrip->pData = (char *) malloc (size);
		imesh->pGenericTriStrip->pData = new char[size];
		assert(imesh->pGenericTriStrip->pData);
	}
}


IMESH * copyIMESH (IMESH *imesh1)
{
	int size;
	IMESH *imesh2 = NULL;

//	imesh2 = (IMESH *) malloc (sizeof (IMESH));
	imesh2 = new IMESH;
	assert(imesh2);

	initIMESH (imesh2);
	memcpy(imesh2->meshName , imesh1->meshName, IMESH_NAME_LENGTH);

	imesh2->numVertices  = imesh1->numVertices;
	imesh2->numFaces     = imesh1->numFaces;
	imesh2->numTriStrips = imesh1->numTriStrips;
	imesh2->numMaterials = imesh1->numMaterials;
	imesh2->numTexCoords = imesh1->numTexCoords;
	imesh2->numNormals   = imesh1->numNormals;
	imesh2->mode.allBits = imesh1->mode.allBits;

	allocateMemoryIMESH (imesh2, TRUE);

	// Copy data:
	if(imesh2->pVertex){
		memcpy (imesh2->pVertex, imesh1->pVertex, imesh2->numVertices * sizeof (IV3D));
	}
	if(imesh2->pFace){
		memcpy (imesh2->pFace, imesh1->pFace, imesh2->numFaces * sizeof (IFACE));
	}
	if(imesh2->pNormal){
		memcpy (imesh2->pNormal, imesh1->pNormal, imesh2->numNormals * sizeof (IV3D));
	}
	if(imesh2->pTexCoord){
		memcpy (imesh2->pTexCoord, imesh1->pTexCoord, imesh2->numTexCoords * sizeof (IV2D));
	}
	if(imesh2->pTriStrip){
		memcpy (imesh2->pTriStrip, imesh1->pTriStrip, imesh2->numTriStrips * sizeof (ITRISTRIP));
	}
	if (imesh2->mode.faceAttrib){
		memcpy (imesh2->pFaceAttrib, imesh1->pFaceAttrib, imesh2->numFaces * sizeof (IFACE_ATTRIBUTE));
	}
	if(imesh2->pMaterial){
		memcpy (imesh2->pMaterial, imesh1->pMaterial, imesh2->numMaterials * sizeof (IMATERIAL));
	}

	// Copy generic records if they exist:
	if (imesh1->mode.genericHeader){
		memcpy (imesh2->pGenericHeader, imesh1->pGenericHeader, sizeof (IGENERIC));
	}
	if (imesh1->mode.genericVertex){
		memcpy (imesh2->pGenericVertex, imesh1->pGenericVertex, sizeof (IGENERIC));
	}
	if (imesh1->mode.genericFace){
		memcpy (imesh2->pGenericFace, imesh1->pGenericFace, sizeof (IGENERIC));
	}
	if (imesh1->mode.genericTriStrip){
		memcpy (imesh2->pGenericTriStrip, imesh1->pGenericTriStrip, sizeof (IGENERIC));
	}

	// Copy generic data if it exists:

	allocateGenericMemoryIMESH (imesh2);
	if (imesh2->mode.genericHeader && imesh2->pGenericHeader)
	{
		size = imesh2->pGenericHeader->size * imesh2->pGenericHeader->num;
		memcpy (imesh2->pGenericHeader->pData, imesh1->pGenericHeader->pData, size);
	}
	if (imesh2->mode.genericVertex && imesh2->pGenericVertex)
	{
		size = imesh2->pGenericVertex->size * imesh2->pGenericVertex->num;
		memcpy (imesh2->pGenericVertex->pData, imesh1->pGenericVertex->pData, size);
	}
	if (imesh2->mode.genericFace && imesh2->pGenericFace)
	{
		size = imesh2->pGenericFace->size * imesh2->pGenericFace->num;
		memcpy (imesh2->pGenericFace->pData, imesh1->pGenericFace->pData, size);
	}
	if (imesh2->mode.genericTriStrip && imesh2->pGenericTriStrip)
	{
		size = imesh2->pGenericTriStrip->size * imesh2->pGenericTriStrip->num;
		memcpy (imesh2->pGenericTriStrip->pData, imesh1->pGenericTriStrip->pData, size);
	}

	return imesh2;
}

int sortIMESHFaces(IMESH *imesh, int numMaterials){
	IMESH *tmpIMESH = NULL;
	unsigned long indexFace = 0;

	if((numMaterials <=0) || (imesh == NULL)){
		return FALSE;
	}

//	tmpIMESH = (IMESH *) malloc(sizeof(IMESH));
	tmpIMESH = new IMESH;
	assert(tmpIMESH);

//	tmpIMESH->pFace = (IFACE *) malloc(imesh->numFaces * sizeof(IFACE));
	tmpIMESH->pFace = new IFACE[imesh->numFaces];
	assert(tmpIMESH->pFace);

//	tmpIMESH->pFaceAttrib = (IFACE_ATTRIBUTE *) malloc(imesh->numFaces * sizeof(IFACE_ATTRIBUTE));
	tmpIMESH->pFaceAttrib = new IFACE_ATTRIBUTE[imesh->numFaces];
	assert(tmpIMESH->pFaceAttrib);

	for(int mat=0; mat<numMaterials; mat++){
		for(int f=0; f<imesh->numFaces; f++){
			if(imesh->pFaceAttrib[f].matID == mat){
				memcpy(&tmpIMESH->pFace[indexFace], &imesh->pFace[f], sizeof(IFACE));
				memcpy(&tmpIMESH->pFaceAttrib[indexFace], &imesh->pFaceAttrib[f], sizeof(IFACE_ATTRIBUTE));
				indexFace++;
			}
		}
	}

//	free(imesh->pFace);
  delete [] imesh->pFace;
	imesh->pFace = NULL;

//	free(imesh->pFaceAttrib);
  delete [] imesh->pFaceAttrib;
	imesh->pFaceAttrib = NULL;

	imesh->pFace = tmpIMESH->pFace;
	imesh->pFaceAttrib = tmpIMESH->pFaceAttrib;

//	free(tmpIMESH);
  delete tmpIMESH;
	tmpIMESH = NULL;

	return TRUE;
}


int sortIMESHTriStrips(IMESH *imesh, int numMaterials){
	IMESH *tmpIMESH = NULL;
	int indexTS = 0;
	int size;

	if((numMaterials <=0) || (imesh == NULL)){
		return FALSE;
	}

//	tmpIMESH = (IMESH *) malloc(sizeof(IMESH));
	tmpIMESH = new IMESH;
	assert(tmpIMESH);

//	tmpIMESH->pTriStrip = (ITRISTRIP *) malloc(imesh->numTriStrips * sizeof(ITRISTRIP));
	tmpIMESH->pTriStrip = new ITRISTRIP[imesh->numTriStrips];
	assert(tmpIMESH->pTriStrip);

//	tmpIMESH->pTriStripAttrib = (ITRISTRIP_ATTRIBUTE *) malloc(imesh->numTriStrips * sizeof(ITRISTRIP_ATTRIBUTE));
	tmpIMESH->pTriStripAttrib = new ITRISTRIP_ATTRIBUTE[imesh->numTriStrips];
	assert(tmpIMESH->pTriStripAttrib);

	for(int mat=0; mat<numMaterials; mat++){
		for(int ts=0; ts<imesh->numTriStrips; ts++){
			if(imesh->pTriStripAttrib[ts].matID == mat){
				tmpIMESH->pTriStrip[indexTS].numVertices = imesh->pTriStrip[ts].numVertices;
				tmpIMESH->pTriStripAttrib[indexTS].matID = mat;

				size = imesh->pTriStrip[ts].numVertices * sizeof(unsigned long);
				tmpIMESH->pTriStrip[indexTS].v = (unsigned long *) malloc(size);
				assert(tmpIMESH->pTriStrip[indexTS].v);
				memcpy (tmpIMESH->pTriStrip[indexTS].v, imesh->pTriStrip[ts].v, size);

				tmpIMESH->pTriStripAttrib[indexTS].n = (unsigned long *) malloc(size);
				assert(tmpIMESH->pTriStripAttrib[indexTS].n);
				memcpy (tmpIMESH->pTriStripAttrib[indexTS].n, imesh->pTriStripAttrib[ts].n, size);

				if(imesh->pTriStripAttrib[ts].t != NULL){
					tmpIMESH->pTriStripAttrib[indexTS].t = (unsigned long *) malloc(size);
					assert(tmpIMESH->pTriStripAttrib[indexTS].t);
					memcpy (tmpIMESH->pTriStripAttrib[indexTS].t, imesh->pTriStripAttrib[ts].t, size);
				}
				indexTS++;
			}
		}
	}

	//--------------------------------------------------
	//free imesh tri-strips
	for(int ts=0; ts < imesh->numTriStrips; ts++){
//		free(imesh->pTriStrip[ts].v);
		delete [] imesh->pTriStrip[ts].v;
		imesh->pTriStrip[ts].v = NULL;
	}
//	free(imesh->pTriStrip);
	delete [] imesh->pTriStrip;
	imesh->pTriStrip = NULL;
	
	for(int tsa=0; tsa < imesh->numTriStrips; tsa++){
//		free(imesh->pTriStripAttrib[tsa].n);
		delete [] imesh->pTriStripAttrib[tsa].n;
		imesh->pTriStripAttrib[tsa].n = NULL;

		if(imesh->pTriStripAttrib[tsa].t != NULL){
//			free(imesh->pTriStripAttrib[tsa].t);
			delete [] imesh->pTriStripAttrib[tsa].t;
      imesh->pTriStripAttrib[tsa].t = NULL;
		}
	}
//	free(imesh->pTriStripAttrib);
	delete [] imesh->pTriStripAttrib;
	imesh->pTriStripAttrib = NULL;
	
	imesh->pTriStrip = tmpIMESH->pTriStrip;
	imesh->pTriStripAttrib = tmpIMESH->pTriStripAttrib;

//	free(tmpIMESH);
  delete tmpIMESH;
	tmpIMESH = NULL;

	return TRUE;
}

void freeIMESH(IMESH *imesh, IMATERIAL *iMat, BOOL inAGroup)
{
	if(imesh->numVertices > 0){
//		free(imesh->pVertex);
		delete [] imesh->pVertex;
		imesh->pVertex = NULL;
	}
	if(imesh->numFaces > 0){
//		free(imesh->pFace);
		delete [] imesh->pFace;
		imesh->pFace = NULL;
	}
	if(imesh->numMaterials > 0){
//		free(imesh->pMaterial);
		delete [] imesh->pMaterial;
		imesh->pMaterial = NULL;
	}
	if(imesh->numTexCoords > 0){
//		free(imesh->pTexCoord);
		delete [] imesh->pTexCoord;
		imesh->pTexCoord = NULL;
	}
	if(imesh->numNormals > 0){
//		free(imesh->pNormal);
		delete [] imesh->pNormal;
		imesh->pNormal = NULL;
	}
	if (imesh->mode.faceAttrib){
//		free(imesh->pFaceAttrib);
		delete [] imesh->pFaceAttrib;
		imesh->pFaceAttrib = NULL;
	}
	if (imesh->numTriStrips > 0){
		for(int ts=0; ts < imesh->numTriStrips; ts++){
//			free(imesh->pTriStrip[ts].v);
			delete [] imesh->pTriStrip[ts].v;
			imesh->pTriStrip[ts].v = NULL;
		}
//		free(imesh->pTriStrip);
		delete [] imesh->pTriStrip;
		imesh->pTriStrip = NULL;
	}
	if (imesh->mode.triStripAttrib){
		for(int tsa=0; tsa < imesh->numTriStrips; tsa++){
//			free(imesh->pTriStripAttrib[tsa].n);
			delete [] imesh->pTriStripAttrib[tsa].n;
			imesh->pTriStripAttrib[tsa].n = NULL;

			if(iMat[imesh->pTriStripAttrib[tsa].matID].textureFlag){
//				free(imesh->pTriStripAttrib[tsa].t);
				delete [] imesh->pTriStripAttrib[tsa].t;
				imesh->pTriStripAttrib[tsa].t = NULL; 
			}
		}
//		free(imesh->pTriStripAttrib);
		delete [] imesh->pTriStripAttrib;
		imesh->pTriStripAttrib = NULL;
	}

	// Free all used generic attributes:
	if (imesh->pGenericHeader)
	{
		if (imesh->pGenericHeader->pData){
//			free (imesh->pGenericHeader->pData);
			delete [] imesh->pGenericHeader->pData;
			imesh->pGenericHeader->pData = NULL;
		}
//		free (imesh->pGenericHeader);
		delete imesh->pGenericHeader;
		imesh->pGenericHeader = NULL;
	}
	if (imesh->pGenericFace)
	{
		if (imesh->pGenericFace->pData){
//			free (imesh->pGenericFace->pData);
			delete [] imesh->pGenericFace->pData;
			imesh->pGenericFace->pData = NULL;
		}
//		free (imesh->pGenericFace);
		delete imesh->pGenericFace;
		imesh->pGenericFace = NULL;
	}
	if (imesh->pGenericVertex)
	{
		if (imesh->pGenericVertex->pData){
//			free (imesh->pGenericVertex->pData);
			delete [] imesh->pGenericVertex->pData;
			imesh->pGenericVertex->pData = NULL;
		}
//		free (imesh->pGenericVertex);
		delete imesh->pGenericVertex;
		imesh->pGenericVertex = NULL;
	}
	if (imesh->pGenericTriStrip)
	{
		if (imesh->pGenericVertex->pData){
//			free (imesh->pGenericVertex->pData);
			delete [] imesh->pGenericVertex->pData;
			imesh->pGenericVertex->pData = NULL;
		}
//		free (imesh->pGenericTriStrip);
		delete imesh->pGenericTriStrip;
		imesh->pGenericTriStrip = NULL;
	}

	if (!inAGroup){
//		free (imesh);
		delete imesh;
		imesh = NULL;
	}
}


void freeIMESHGROUP(IMESHGROUP *grpMesh)
{
	int im;

	for(im=0; im < grpMesh->numMeshes; im++){
		freeIMESH(&grpMesh->pMesh[im], grpMesh->pMaterial, TRUE);
	}

//	free(grpMesh->pMesh);
	delete [] grpMesh->pMesh;
	grpMesh->pMesh = NULL;

	if(grpMesh->numMaterials > 0){
//		free(grpMesh->pMaterial);
		delete [] grpMesh->pMaterial;
		grpMesh->pMaterial = NULL;
	}
	memset(grpMesh, 0, sizeof(IMESHGROUP));
}

/*-----------------------------------------------------*/
/////////////////////////////////////////////////////////
