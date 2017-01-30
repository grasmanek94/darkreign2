///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshmrm.cpp
//
// 07-JUL-1998
//

#pragma comment(lib, "ole32")

//#define INITGUID        // won't link?!, put INITGUID in the preprocessor includes for this file
#include "vid_private.h"
#include "mrmgen.h"
//----------------------------------------------------------------------------

Bool MeshRootToIMesh( MeshRoot &root, IMESH &imesh)
{
  imesh.numNormals        = root.normals.count;
	imesh.numTexCoords      = root.uvs.count;

  // allocate imesh
	imesh.numVertices       = root.vertices.count;
	imesh.numFaces          = root.faces.count;
	imesh.numMaterials      = root.buckys.count;

  imesh.mode.faceAttrib = TRUE;
  imesh.mode.genericVertex = TRUE;
  allocateMemoryIMESH( &imesh, TRUE);
  imesh.pGenericVertex->size = sizeof( VertIndex);
  imesh.pGenericVertex->num = imesh.numVertices;
  allocateGenericMemoryIMESH( &imesh);

  U32 i;
  for (i = 0; i < imesh.numMaterials; i++)
  {
    Material & material = *root.buckys[i].material;
    strcpy( imesh.pMaterial[i].matName, material.GetName());
    imesh.pMaterial[i].ambient  = *((ICOLOR *) &material.Ambient());
    imesh.pMaterial[i].diffuse  = *((ICOLOR *) &material.Diffuse());
    imesh.pMaterial[i].specular = *((ICOLOR *) &material.Specular());
    imesh.pMaterial[i].shininess   = material.SpecularPower() / 100.0f;

    *imesh.pMaterial[i].texturePath = '\0';
    imesh.pMaterial[i].textureFlag = TRUE;
  }

  {
    // setup verts
    for (i = 0; i < root.vertices.count; i++)
    {
      ((VertIndex *) imesh.pGenericVertex->pData)[i] = root.vertToState[i];
      *((Vector *) &imesh.pVertex[i]) = root.vertices[i];
    }
    for (i = 0; i < root.uvs.count; i++)
    {
      imesh.pTexCoord[i].u = root.uvs[i].u;
      imesh.pTexCoord[i].v = root.uvs[i].v;
    }

    // setup faces and normals
    U32 heapSize = root.normals.count;
    U8 * norms = (U8 *) Vid::Heap::Request( heapSize);
    memset( norms, 0xff, heapSize);

    for (i = 0; i < root.faces.count; i++)
    {
      FaceObj &face = root.faces[i];

      imesh.pFaceAttrib[i].smoothID = 0;

      imesh.pFaceAttrib[i].matID = face.buckyIndex;

      ASSERT( imesh.pFaceAttrib[i].matID < root.buckys.count);

      imesh.pFace[i].a = face.verts[0];
      imesh.pFace[i].b = face.verts[1];
      imesh.pFace[i].c = face.verts[2];

      imesh.pFaceAttrib[i].na = face.norms[0];
      imesh.pFaceAttrib[i].nb = face.norms[1];
      imesh.pFaceAttrib[i].nc = face.norms[2];

      imesh.pFaceAttrib[i].ta = face.uvs[0];
      imesh.pFaceAttrib[i].tb = face.uvs[1];
      imesh.pFaceAttrib[i].tc = face.uvs[2];

      U16 index;
      index = (U16) imesh.pFaceAttrib[i].na;
      if (norms[index] == 0xff)
      {
        *((Vector *) &imesh.pNormal[index]) = root.normals[index];
        norms[index] = 1;
      }
      index = (U16) imesh.pFaceAttrib[i].nb;
      if (norms[index] == 0xff)
      {
        *((Vector *) &imesh.pNormal[index]) = root.normals[index];
        norms[index] = 1;
      }
      index = (U16) imesh.pFaceAttrib[i].nc;
      if (norms[index] == 0xff)
      {
        *((Vector *) &imesh.pNormal[index]) = root.normals[index];
        norms[index] = 1;
      }
    }
    // norms
    Vid::Heap::Restore( heapSize);
  }

  return TRUE;
}
//----------------------------------------------------------------------------
MRMGenParams params;

Bool IMeshToMeshRoot( MeshRoot &root, IMESH &imesh, MRMResults &mrmResults)
{
  ASSERT( imesh.numVertices  <= Vid::renderState.maxVerts);
  ASSERT( imesh.numNormals   <= Vid::renderState.maxVerts);
  ASSERT( imesh.numTexCoords <= Vid::renderState.maxVerts);
  ASSERT( imesh.numFaces     <= Vid::renderState.maxTris);

  ASSERT( imesh.numMaterials == root.buckys.count);

  ASSERT( imesh.numVertices  <= root.vertices.count);
  ASSERT( imesh.numFaces     <= root.faces.count);


  if (imesh.numNormals > root.normals.count)
  {
    root.normals.Alloc( imesh.numNormals);
  }
  ASSERT( imesh.numTexCoords <= root.uvs.count);

  U32 oldCount = root.vertices.count;

  root.vertices.count = imesh.numVertices;
  root.normals.count  = imesh.numNormals;
  root.uvs.count      = imesh.numTexCoords;
  root.faces.count    = imesh.numFaces;

  if (root.colors.count)
  {
    root.colors.count = imesh.numVertices;
  }

  U32 i;
  for (i = 0; i < imesh.numVertices; i++)
  {
    root.vertToState[i] = ((VertIndex *) imesh.pGenericVertex->pData)[i];
    root.vertices[i] = *((Vector *)&imesh.pVertex[i]);
  }
  for (i = 0; i < imesh.numNormals; i++)
  {
    root.normals[i] = *((Vector *)&imesh.pNormal[i]);
  }
  for (i = 0; i < imesh.numTexCoords; i++)
  {
    root.uvs[i].u = imesh.pTexCoord[i].u;
    root.uvs[i].v = imesh.pTexCoord[i].v;
  }
  if (root.colors.count)
  {
    U32 heapSize = Max<U32>( root.colors.count, oldCount) * sizeof(Color);
    Color * colors = (Color *) Vid::Heap::Request( heapSize); 

    for (i = 0; i < oldCount; i++)
    {
      if (mrmResults.pVertexMap[i] != mrmResults.undefIndexValue)
      {
        colors[mrmResults.pVertexMap[i]] = root.colors[i];
      }
    }
    for (i = 0; i < root.vertices.count; i++)
    {
      root.colors[i] = colors[i];
    }

    Vid::Heap::Restore( heapSize);
  }

  for (i = 0; i < root.faces.count; i++)
  {
    FaceObj &face = root.faces[i];

    face.buckyIndex = (U8) imesh.pFaceAttrib[i].matID;

    face.verts[0] = (U16) imesh.pFace[i].a;
    face.verts[1] = (U16) imesh.pFace[i].b;
    face.verts[2] = (U16) imesh.pFace[i].c;

    if (params.normalsMode == PerVertex)
    {
      face.norms[0] = (U16) imesh.pFace[i].a;
      face.norms[1] = (U16) imesh.pFace[i].b;
      face.norms[2] = (U16) imesh.pFace[i].c;
    }
    else
    {
      face.norms[0] = (U16) imesh.pFaceAttrib[i].na;
      face.norms[1] = (U16) imesh.pFaceAttrib[i].nb;
      face.norms[2] = (U16) imesh.pFaceAttrib[i].nc;
    }

    face.uvs[0] = (U16) imesh.pFaceAttrib[i].ta;
    face.uvs[1] = (U16) imesh.pFaceAttrib[i].tb;
    face.uvs[2] = (U16) imesh.pFaceAttrib[i].tc;
  
//      ASSERT( imesh.pFaceAttrib[i].matID < root.buckys.count);
    if (imesh.pFaceAttrib[i].matID >= root.buckys.count)
    {
      LOG_WARN(("MrmGen: matId"));
      imesh.pFaceAttrib[i].matID = 0;
    }

    ASSERT( imesh.pFace[i].a < root.vertices.count);
    ASSERT( imesh.pFace[i].b < root.vertices.count);
    ASSERT( imesh.pFace[i].c < root.vertices.count);

    ASSERT( imesh.pFaceAttrib[i].na < root.normals.count);
    ASSERT( imesh.pFaceAttrib[i].nb < root.normals.count);
    ASSERT( imesh.pFaceAttrib[i].nc < root.normals.count);

    ASSERT( imesh.pFaceAttrib[i].ta < root.uvs.count);
    ASSERT( imesh.pFaceAttrib[i].tb < root.uvs.count);
    ASSERT( imesh.pFaceAttrib[i].tc < root.uvs.count);
  }
	return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshRoot::MRMGen( List<U16> * verts) // = NULL)
{
#ifndef DOMRMGEN
  verts;

  return FALSE;

#else

	// Initialize COM:
	CoInitialize(NULL);

	// Ok, Now create the mrmgen COM object:
	IMRMGen2 * s_lpMRMGen = NULL;
	HRESULT hr = CoCreateInstance(CLSID_CMRMGEN, NULL, CLSCTX_INPROC_SERVER, IID_IMRMGEN2,(void ** ) &s_lpMRMGen);

	// Did we get a valid interface:
  if( (FAILED(hr)) || (!(s_lpMRMGen)) ) 
	{
    LOG_DIAG( ("couldn't connect to mrmgen.dll; is it registered?") );
    return FALSE;
	}

  IMESH *imesh = new IMESH;
	initIMESH( imesh);

  MeshRootToIMesh( *this, *imesh);

//#define DOWRITEMRM
#ifdef DOWRITEMRM
	MRMWrite( imesh, NULL, "in.mrm");
#endif

	Utils::Memset( &params, 0, sizeof( params));
	params.size = sizeof(MRMGenParams);
	params.flags = 0;  // no parameters are valid
  params.flags |= MRMGP_MERGETHRESH | MRMGP_NORMALSMODE | MRMGP_CREASEANGLE;
  params.mergeThresh = Vid::Var::mrmMergeThresh;
  params.normalsCreaseAngle = Vid::Var::mrmNormalCrease;
 	params.normalsMode = Vid::Var::mrmMultiNormals ? PerFacePerVertex : PerVertex;

#define VERTTHRESH  0.1f

  // automatic shadow plane base verts
  //
  U32 baseCount = 0, heapSize = vertices.count << 2;
  U32 * baseIndices = (U32 *) Vid::Heap::Request( heapSize);

  if (shadowPlane)
  {
    Mesh *mesh = NULL;
    U32 i, j;
    for (i = 0; i < states.count; i++)
    {
    	if (!strnicmp(states[i].GetMeshFromRoot()->name.str, "sp-", 3))
      {
        mesh = states[i].GetMeshFromRoot();
        break;
      }
    }
    if (mesh && mesh->local)
    {
      for (i = 0; i < mesh->local->vertices.count; i++)
      {
        Vector v0;
        mesh->WorldMatrix().Transform( v0, mesh->local->vertices[i]);

        for (j = 0; j < vertices.count; j++)
        {
          Vector &v1 = vertices[j];
          if ((F32)fabs(v0.x - v1.x) < VERTTHRESH
           && (F32)fabs(v0.y - v1.y) < VERTTHRESH
           && (F32)fabs(v0.z - v1.z) < VERTTHRESH)
          {
            baseIndices[baseCount] = j;
            baseCount++;
          }
        }
      }
    }
  }

  // selected base verts
  //
  if (verts)
  {
    U32 startBaseCount = baseCount;

    List<U16>::Iterator vi( verts);
    U16 * index;
    while ((index = vi++) != NULL)
    {
      U32 i, hit = FALSE; 
      for (i = 0; i < startBaseCount; i++)
      {
        if (*index == baseIndices[i])
        {
          hit = TRUE;
          break;
        }
      }
      if (!hit)
      {
        baseIndices[baseCount] = *index;
        baseCount++;
      }
    }
  } 

  if (baseCount)
  {
    params.flags |= MRMGP_NUMBASEVERTICES;
    params.numBaseVertices = baseCount;
    params.baseVertices = baseIndices;
    LOG_DIAG( ("MeshRoot::MRMGen: baseCount = %d", baseCount) );
  }

  MRMResults *mrmResults;

  // Invoke the algorithm and collect the results:
	s_lpMRMGen->GenerateMRM(imesh, &params, &mrmResults);

  // baseIndices
  Vid::Heap::Restore( heapSize);

  MRMUpdates *mrmUpdates = mrmResults->pMrmUpdates;
  if (!mrmUpdates)
  {
    // Free memory associated with the results:
	  s_lpMRMGen->FreeMRMResults(mrmResults);

  	// Release memory:
	  freeIMESH( imesh);

  	hr = s_lpMRMGen->Release();

	// Shut down COM:
  	CoUninitialize();

    return FALSE;
  }
  IMESH *orderedImesh = mrmResults->pIMesh;

//  ASSERT( orderedImesh->numFaces == faces.count);

#ifdef DOWRITEMRM
	MRMWrite (orderedImesh, mrmUpdates, "out.mrm");
#endif

  // copy ordered geometry data, if its not empty
  //
  if ((orderedImesh->numVertices == 0 || orderedImesh->numFaces == 0) 
   || !IMeshToMeshRoot( *this, *orderedImesh, *mrmResults))
  {
    // Free memory associated with the results:
	  s_lpMRMGen->FreeMRMResults(mrmResults);

  	// Release memory:
	  freeIMESH( imesh);

  	hr = s_lpMRMGen->Release();

	// Shut down COM:
  	CoUninitialize();

    return FALSE;
  }

  if (verts)
  {
    List<U16>::Iterator vi( verts);
    U16 * index;
    while ((index = vi++) != NULL)
    {
      if (mrmResults->pVertexMap[*index] == mrmResults->undefIndexValue)
      {
        verts->Unlink( index);
      }
      else
      {
        *index = (U16) mrmResults->pVertexMap[*index];
      }
    }
  }

  // allocate mrm data
  if (mrm)
  {
    delete mrm;
    mrm = NULL;
  }

  mrm = new MRM;
  ASSERT( mrm);
  mrm->vertCount = (U16)orderedImesh->numVertices;
  mrm->maxVertCount = mrm->vertCount;
  mrm->minMinVertCount = U16(baseCount > 7 ? baseCount : 7);
  if (mrm->minMinVertCount > mrm->vertCount)
  {
    mrm->minMinVertCount = mrm->vertCount;
  }
  mrm->minVertCount = mrm->minMinVertCount;

  mrm->vertex = new MRM::Vertex[mrm->vertCount];
  ASSERT( mrm->vertex);

//#define DOWRITEDATA
#ifdef DOWRITEDATA
  FILE *fp;
  fp = fopen( "mrmdata.txt", "w");
#endif

  FaceUpdate *update;
  // count total number of faceupdates
  U32 i, j, l, k = 0;
  for (i = 0; i < mrm->vertCount; i++)
  {
    U16 numfu = mrmUpdates->vertexUpdates[i].numFaceUpdates;
#ifdef DOWRITEDATA
    fprintf( fp, "\nverts %d : newfaces %d : faceupdates %d\n", 
      i, mrmUpdates->vertexUpdates[i].numNewFaces, numfu);
#endif
    for (j = l = 0; j < numfu; j++)
    {
      update = &mrmUpdates->vertexUpdates[i].faceUpdates[j];

      // validate mrm data
	    switch (update->attribToken)
		  {
		  case VertexA:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : VertexA %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
        l++;
			  break;
		  case VertexB:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : VertexB %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
        l++;
			  break;
		  case VertexC:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : VertexC %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
        l++;
			  break;

		  case NormalA:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : NormalA %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
//        if (!indexed)
        {
          l++;
        }
			  break;
      case NormalB:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : NormalB %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
//        if (!indexed)
        {
          l++;
        }
			  break;
		  case NormalC:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : NormalC %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
//        if (!indexed)
        {
          l++;
        }
			  break;

		  case TexCoord1A:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : TexCrdA %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
//        if (!indexed)
        {
          l++;
        }
			  break;
		  case TexCoord1B:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : TexCrdB %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
//        if (!indexed)
        {
          l++;
        }
			  break;
		  case TexCoord1C:
#ifdef DOWRITEDATA
        fprintf( fp, "face %d : TexCrdC %d : %d\n", update->faceIndex, update->value[0], update->value[1]);
#endif
//        if (!indexed)
        {
          l++;
        }
			  break;
		  }
    }
    mrmUpdates->vertexUpdates[i].numFaceUpdates = (U16) l;
    k += l;
  }
#ifdef DOWRITEDATA
  if (fp)
  {
    fclose(fp);
  }
#endif

  if (k == 0)
  {
    // Free memory associated with the results:
	  s_lpMRMGen->FreeMRMResults(mrmResults);

  	// Release memory:
	  freeIMESH( imesh);

  	hr = s_lpMRMGen->Release();

	// Shut down COM:
  	CoUninitialize();


    delete [] mrm->vertex;
    delete mrm;
    mrm = NULL;

    return FALSE;
  }

  mrm->faceCount = (U16)k;
  MRM::Face *mrmFace = new MRM::Face[k];
  ASSERT( mrmFace);
  mrm->vertex[0].face = mrmFace;

  // copy mrm data
  for (i = 0; i < mrm->vertCount; i++)
  {
    U16 numfu = mrmUpdates->vertexUpdates[i].numFaceUpdates;
    mrm->vertex[i].faceCount = numfu;
//    mrmUpdates->vertexUpdates[i].numFaceUpdates = (U16) numfu;
    mrm->vertex[i].face = mrmFace;
    mrm->vertex[i].newFaceCount = mrmUpdates->vertexUpdates[i].numNewFaces;
    mrm->vertex[i].newNormCount = mrmUpdates->vertexUpdates[i].numNewNormals;
    mrm->vertex[i].newTextCount = mrmUpdates->vertexUpdates[i].numNewTexCoords;

    for (j = 0; j < numfu; j++)
    {
      // validate mrm data
      FaceUpdate &fupdate = mrmUpdates->vertexUpdates[i].faceUpdates[j];
	    switch (fupdate.attribToken)
		  {
		  case VertexA:
		  case VertexB:
		  case VertexC:
        ASSERT( fupdate.value[0] < vertices.count);
        ASSERT( fupdate.value[1] < vertices.count);

        mrmFace->token = (MRM::Face::Token) fupdate.attribToken;
        mrmFace->index[0] = (U16) fupdate.value[0];
        mrmFace->index[1] = (U16) fupdate.value[1];
        mrmFace->face = (U16) fupdate.faceIndex;
        mrmFace++;
			  break;
		  case NormalA:
		  case NormalB:
		  case NormalC:
//        if (!indexed)
        ASSERT( fupdate.value[0] < normals.count);
        ASSERT( fupdate.value[1] < normals.count);

        mrmFace->token = (MRM::Face::Token) fupdate.attribToken;
        mrmFace->index[0] = (U16) fupdate.value[0];
        mrmFace->index[1] = (U16) fupdate.value[1];
        mrmFace->face = (U16) fupdate.faceIndex;
        mrmFace++;
  		  break;
		  case TexCoord1A:
		  case TexCoord1B:
		  case TexCoord1C:
//        if (!indexed)
        ASSERT( fupdate.value[0] < uvs.count);
        ASSERT( fupdate.value[1] < uvs.count);

        mrmFace->token = (MRM::Face::Token) fupdate.attribToken;
        mrmFace->index[0] = (U16) fupdate.value[0];
        mrmFace->index[1] = (U16) fupdate.value[1];
        mrmFace->face = (U16) fupdate.faceIndex;
        mrmFace++;
			  break;
		  }
    }
  }

  // Free memory associated with the results:
	s_lpMRMGen->FreeMRMResults(mrmResults);

	// Release memory:
	freeIMESH( imesh);

	hr = s_lpMRMGen->Release();

	// Shut down COM:
	CoUninitialize();

  Setup();
	SetupPlanes();
	CalcBoundingSphere();
  SortFaces();

  return TRUE;
#endif
}
//----------------------------------------------------------------------------

void MRM::Alloc( U16 totalFaceCount, U16 vertCount)
{
  Free();

  vertex = new MRM::Vertex[vertCount];
  vertex[0].face = new MRM::Face[totalFaceCount];
}
//----------------------------------------------------------------------------

void MRM::Free()
{
  if (vertex)
  {
    delete [] vertex[0].face;
    delete [] vertex;
  }
  vertex = NULL;
}
//----------------------------------------------------------------------------
