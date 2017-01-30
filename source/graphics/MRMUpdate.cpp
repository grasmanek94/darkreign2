#define WIN32_LEAN_AND_MEAN

#pragma pack(push, _save_mrmgen_h_)
#pragma warning(push,3)

#include <windows.h>
#include <fstream>
#include <assert.h>

#include "IMESH.h"
#include "IMESHUtil.h"
#include "MRMUpdate.h"

#pragma warning(pop)
#pragma pack(pop, _save_mrmgen_h_)

///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
unsigned int getSize(MRMUpdates *upd)
{
	unsigned int total = 0, size = 0;

	size = MRM_VERSION_LENGTH;
	size += sizeof(MRMUpdates);
	size += upd->maxNumVertices * sizeof(VertexUpdate);
	
	for (unsigned int v = 0; v < upd->maxNumVertices; v++)
		total += upd->vertexUpdates[v].numFaceUpdates;

	size += total * sizeof(FaceUpdate);

	return size+512; // a little extra in case a few things get added to the resd or write functions that they forget to add here.
}


///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
BOOL readMRMUpdates (MRMUpdates *mrmUpdates, InputStream *mrmStream)
{
	char version[MRM_VERSION_LENGTH];
	char errorString[512];
	int size;
	unsigned long totalNumUpdates = 0;
	FaceUpdate * faceUpdatePtr = NULL;
	VertexUpdate *vu = NULL;

	mrmStream->read (version, MRM_VERSION_LENGTH);

	if (!(strcmp((char *) &version, MRM_VERSION) == 0 || 
		strcmp((char *) &version, MRM_BETA_VERSION) == 0))
	{
		sprintf (errorString, "Error: Incompatible MRM versions:\n       %s%s\n       %s%s",
			"MRM reader is  ", MRM_VERSION,
			"MRM file is ", version);
		MessageBox (NULL, errorString, "readMRMUpdates() Error", MB_OK);
		return FALSE;
	}

	// Read the Header record:
	mrmStream->read ((char *) &mrmUpdates->maxNumVertices, sizeof (unsigned long));
	mrmStream->read ((char *) &mrmUpdates->maxNumFaces, sizeof (unsigned long));

	// Read all of the vertex update records:
	//int size = mrmUpdates->maxNumVertices * sizeof (VertexUpdate);
	mrmUpdates->vertexUpdates = new VertexUpdate[mrmUpdates->maxNumVertices];
	assert(mrmUpdates->vertexUpdates);

	size = sizeof (VertexUpdate) - sizeof (FaceUpdate *);
	for (U32 v = 0; v < mrmUpdates->maxNumVertices; v++)
	{
		vu = &mrmUpdates->vertexUpdates[v];
		mrmStream->read((char *) vu, size);
	}


	// Read the total number of face updates (In one giant clump):
	mrmStream->read ((char *) &totalNumUpdates, sizeof (unsigned long));
	faceUpdatePtr = new FaceUpdate[totalNumUpdates];
	assert(faceUpdatePtr);

	size = totalNumUpdates * sizeof (FaceUpdate);
	mrmStream->read ((char *) faceUpdatePtr, size);

	// Now fixup the faceUpdate pointers in the vertexUpdate records:
	for (v=0; v < mrmUpdates->maxNumVertices; v++)
	{
		mrmUpdates->vertexUpdates[v].faceUpdates = faceUpdatePtr;
		faceUpdatePtr += mrmUpdates->vertexUpdates[v].numFaceUpdates;
	}

	return TRUE;
}




///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
BOOL writeMRMUpdates (MRMUpdates *mrmUpdates, OutputStream *mrmStream)
{
	VertexUpdate *vu = NULL;

    // Write out the MRM mesh header:
	char version[MRM_VERSION_LENGTH];
	int size;

	sprintf (version, "%s", MRM_VERSION);
	
	mrmStream->write (version, MRM_VERSION_LENGTH);

	// Write out all the Header record:
	mrmStream->write ((char *) &mrmUpdates->maxNumVertices, sizeof (unsigned long));
	mrmStream->write ((char *) &mrmUpdates->maxNumFaces, sizeof (unsigned long));

	// Write out all of the vertex update records:
	//int size = mrmUpdates->maxNumVertices * sizeof (VertexUpdate);
	//mrmStream->write ((char *) mrmUpdates->vertexUpdates, size);
	size = sizeof (VertexUpdate) - sizeof (FaceUpdate *);
	for (U32 v = 0; v < mrmUpdates->maxNumVertices; v++)
	{
		vu = &mrmUpdates->vertexUpdates[v];
		mrmStream->write((char *) vu, size);
	}


	// Compute and write total number of face updates...for speed read:
	unsigned long totalNumUpdates = 0;
	for (v = 0; v < mrmUpdates->maxNumVertices; v++)
		totalNumUpdates += mrmUpdates->vertexUpdates[v].numFaceUpdates;
	mrmStream->write ((char *) &totalNumUpdates, sizeof (unsigned long));

	// Write clumps of face update records:
	for (v=0; v < mrmUpdates->maxNumVertices; v++)
	{
		size = mrmUpdates->vertexUpdates[v].numFaceUpdates * sizeof (FaceUpdate);
		mrmStream->write ((char *) mrmUpdates->vertexUpdates[v].faceUpdates, size);
	}

	return TRUE;
}



///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
BOOL MRMWrite (IMESH *imesh, MRMUpdates *mrmUpdates, char *fileName)
{
	char errorString[256];

	std::ofstream mrmStream(fileName, std::ios::out | std::ios::binary);

	if (!mrmStream)
	{
		sprintf (errorString, "Error: Could not open file %s.", fileName);
		return FALSE;
	}

	if (!writeMRMUpdates (mrmUpdates, &mrmStream))
	{
		mrmStream.close();
		return FALSE;
	}
	if (!writeIMESHVersion (&mrmStream))
	{
		mrmStream.close();
		return FALSE;
	}
	if (!writeIMESH (imesh, imesh->pMaterial, &mrmStream))
	{
		mrmStream.close();
		return FALSE;
	}

	mrmStream.close();

	return TRUE;
}




///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
BOOL MRMRead (IMESH **imesh, MRMUpdates **mrmUpdates, char *fileName)
{
	BOOL bRet = FALSE;

	std::ifstream mrmStream(fileName, std::ios::in | std::ios::binary);
	if (!mrmStream){
		return FALSE;
	}

	*imesh = new IMESH;
	assert(*imesh);

	initIMESH (*imesh);
	*mrmUpdates = new MRMUpdates;
	assert(*mrmUpdates);

	memset(*mrmUpdates, 0, sizeof(MRMUpdates));

	if (!readMRMUpdates (*mrmUpdates, &mrmStream)){
		goto cleanup;
	}
	
	if (!readIMESHVersion (&mrmStream)){
		goto cleanup;
	}
	
	if (!readIMESH (*imesh, (*imesh)->pMaterial, &mrmStream)){
		goto cleanup;
	}
	
	// Pull maxNumTexCoords and MaxNumNormals from imesh:
	(*mrmUpdates)->maxNumTexCoords = (*imesh)->numTexCoords;
	(*mrmUpdates)->maxNumNormals = (*imesh)->numNormals;

	bRet = TRUE;

cleanup:
	mrmStream.close();
	if (bRet == FALSE)
	{
		delete (*imesh);
		delete (*mrmUpdates);
		*imesh = NULL;
		*mrmUpdates = NULL;
	}
	return bRet;
}




///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
BOOL MRMFree (MRMUpdates *mrmUpdates)
{
	// Compute and write total number of face updates, less the last vertexUpdate:
	unsigned long totalNumUpdates = 0;
	int lastVert;
	FaceUpdate *firstFaceUpdate = NULL;
	FaceUpdate *lastFaceUpdate	= NULL;

	for (U32 v = 0; v < mrmUpdates->maxNumVertices; v++)
		totalNumUpdates += mrmUpdates->vertexUpdates[v].numFaceUpdates;

	// Were the FaceUpdate records allocated contiguously or individually:
	lastVert = mrmUpdates->maxNumVertices - 1;
	firstFaceUpdate = &mrmUpdates->vertexUpdates[0].faceUpdates[0];
	lastFaceUpdate = &mrmUpdates->vertexUpdates[lastVert].faceUpdates[mrmUpdates->vertexUpdates[lastVert].numFaceUpdates - 1];

	if ((firstFaceUpdate + totalNumUpdates - 1) == lastFaceUpdate)
	{
		// Delete contiguous allocation:
		delete []mrmUpdates->vertexUpdates[0].faceUpdates;
		mrmUpdates->vertexUpdates[0].faceUpdates = NULL;
	}
	else
	{
		// Delete individual constructions:
		for (v = 0; v < mrmUpdates->maxNumVertices; v++){
			delete mrmUpdates->vertexUpdates[v].faceUpdates;
			mrmUpdates->vertexUpdates[v].faceUpdates = NULL;
		}
	}

	// Delete contiguous allocation of vertex update records:
	delete []mrmUpdates->vertexUpdates;
	mrmUpdates->vertexUpdates = NULL;

	// Delete the parent update structure:
	delete mrmUpdates;
	mrmUpdates = NULL;

	return TRUE;
}



///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose: Run time resolution control code:
// Return Val:
///////////////////////////////////////////////////////
void MRMUpdateFaces(IMESH *imesh, VertexUpdate *vertexUpdate, Direction direction)
{
	// Grab the update list:
	FaceUpdate *faceUpdates	= vertexUpdate->faceUpdates;

	// Apply each update to the appropriate IFACE:
	for (U32 fu = 0; fu < vertexUpdate->numFaceUpdates; fu++)
	{
		FaceUpdate *update = &faceUpdates[fu];
		IFACE *face = &imesh->pFace[update->faceIndex];
		IFACE_ATTRIBUTE *faceAttrib= &imesh->pFaceAttrib[update->faceIndex];
		
		// Apply update to the appropriate attribute:
		switch (update->attribToken)
		{
		case VertexA:
			face->a = update->value[direction];
			break;
		case VertexB:
			face->b = update->value[direction];
			break;
		case VertexC:
			face->c = update->value[direction];
			break;
		case NormalA:
			faceAttrib->na = update->value[direction];
			break;
		case NormalB:
			faceAttrib->nb = update->value[direction];
			break;
		case NormalC:
			faceAttrib->nc = update->value[direction];
			break;
		case TexCoord1A:
			faceAttrib->ta = update->value[direction];
			break;
		case TexCoord1B:
			faceAttrib->tb = update->value[direction];
			break;
		case TexCoord1C:
			faceAttrib->tc = update->value[direction];
			break;
		}
	}

	// Update the face and vertex counts:
	if (direction == Increasing)
	{
		imesh->numFaces += vertexUpdate->numNewFaces;
		imesh->numNormals += vertexUpdate->numNewNormals;
		imesh->numTexCoords += vertexUpdate->numNewTexCoords;
		imesh->numVertices++;
	}
	else  // decreasing:
	{
		imesh->numVertices--;
		imesh->numFaces -= vertexUpdate->numNewFaces;
		imesh->numNormals -= vertexUpdate->numNewNormals;
		imesh->numTexCoords -= vertexUpdate->numNewTexCoords;
	}
}



///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
void MRMIncrVertices (MRMUpdates *mrmUpdates, IMESH *imesh, U32 r)
{ 
	for (U32 vu = 0; ((vu < r) && (imesh->numVertices != mrmUpdates->maxNumVertices)); vu++){
		MRMUpdateFaces (imesh, &mrmUpdates->vertexUpdates[imesh->numVertices], Increasing);
	}
}


///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
void MRMDecrVertices (MRMUpdates *mrmUpdates, IMESH *imesh, U32 r)
{
	for (U32 vu = 0; ((vu < r) && (imesh->numVertices >= 0)); vu++){
		MRMUpdateFaces (imesh, &mrmUpdates->vertexUpdates[imesh->numVertices - 1], Decreasing);
	}
}


///////////////////////////////////////////////////////
// Function:
// Parameters:
// Purpose:
// Return Val:
///////////////////////////////////////////////////////
void MRMSetNumVertices(MRMUpdates *mrmUpdates, IMESH *imesh, U32 r)
{	
	if(r > imesh->numVertices){
		MRMIncrVertices(mrmUpdates, imesh, r - imesh->numVertices);
	}
	else {
		if(r < imesh->numVertices){
			MRMDecrVertices(mrmUpdates, imesh, imesh->numVertices - r);
		}
	}
	return;
}