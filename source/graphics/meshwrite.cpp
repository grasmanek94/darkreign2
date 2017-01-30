///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshwrite.cpp     mesh xsi fp save
//
// 07-JUL-1998
//

#include "vid_public.h"
//----------------------------------------------------------------------------

// reflip the model's z coords
//#define DOZFLIP
#define DOXFLIP
#define DOXSITEX

static void SaveMeshRoot( FILE *fp, MeshRoot *mesh, char *name, char *margin) 
{
  fprintf (fp, "%sFrame %s {\n\n", margin, name);

  fprintf (fp, "%s   FrameTransformMatrix {\n", margin);

  Matrix matrix = mesh->ObjectMatrix();

#ifdef DOXFLIP
  Quaternion q;
  q.Set( matrix);
  q.v.y *= -1.0f;
  q.v.z *= -1.0f;
  matrix.Set( q);
  matrix.posit.x *= -1.0f;
#else
#ifdef DOZFLIP
  Quaternion q;
  q.Set( matrix);
  q.v.x *= -1.0f;
  q.v.y *= -1.0f;
  matrix.Set( q);
  matrix.posit.z *= -1.0f;
#endif
#endif

  fprintf (fp, "%s      %lf,%lf,%lf,%lf,\n", margin,
    matrix.right.x,
    matrix.right.y,
    matrix.right.z,
    matrix.rightw);

  fprintf (fp, "%s      %lf,%lf,%lf,%lf,\n", margin,
    matrix.up.x,
    matrix.up.y,
    matrix.up.z,
    matrix.upw);

  fprintf (fp, "%s      %lf,%lf,%lf,%lf,\n", margin,
    matrix.front.x,
    matrix.front.y,
    matrix.front.z,
    matrix.frontw);

  fprintf (fp, "%s      %lf,%lf,%lf,%lf;;\n", margin,
    matrix.posit.x,
    matrix.posit.y,
    matrix.posit.z,
    matrix.positw);

  fprintf (fp, "%s   }\n\n", margin);

	U32 vCount = mesh->vertices.count;
	U32 fCount = mesh->faces.count;

	// dump polygon mesh header
	fprintf (fp, "%s   Mesh %s {\n", margin, name);
	
	// dump polygon vertices
	fprintf ( fp, "%s      %d;\n", margin, vCount);
  U32 i, j;
	for (i = 0; i < vCount; i++)
  {
    Vector &vert = mesh->vertices[i];

    if (i != 0)
    {
  		fprintf ( fp, ",\n");
    }
		fprintf ( fp, "%s      %f;%f;%f;", margin,
#ifdef DOXFLIP
      -vert.x, vert.y, vert.z);
#else
#ifdef DOZFLIP
      vert.x, vert.y, -vert.z);
#else
      vert.x, vert.y,  vert.z);
#endif
#endif
	}
	fprintf ( fp, ";\n\n");
	
	// dump polygon indices
	fprintf (fp, "%s      %d;\n", margin, fCount);
	if (fCount > 0)
	{
		for (i = 0; i < fCount; i++)
		{
      FaceObj &face = mesh->faces[i];

      if (i != 0)
      {
  			fprintf (fp, ",\n");
      }
			fprintf (fp, "%s      %d;%d,%d,%d;", margin, 3, 
#if (defined DOXFLIP) || (defined DOZFLIP)
				face.verts[2], face.verts[1], face.verts[0]);
#else
				face.verts[0], face.verts[1],	face.verts[2]);
#endif
		}
		fprintf (fp, ";\n\n");
	}

  // count materials
  U32 mCount = 0;
  Material *mats[MAXMATERIALS];
  Bitmap *  texs[MAXMATERIALS];

  U16 * indexList;
  U32 heapSize = Vid::Heap::ReqU16( &indexList, fCount);

  for (i = 0; i < fCount; i++)
  {
    BucketLock &bucky = mesh->buckys[mesh->faces[i].buckyIndex];

    for (j = 0; j < mCount; j++)
    {
      if (bucky.material == mats[j] && bucky.texture0 == texs[j])
      {
        // fall out of loop early
        break;
      }
    }
    if (j == mCount)
    {
      mats[mCount] = bucky.material;
      texs[mCount] = bucky.texture0;
      mCount++;
    }
    indexList[i] = (U16) j;
  }

	// dump material list header
	fprintf (fp, "%s      MeshMaterialList {\n", margin);
	fprintf (fp, "%s         %d;\n", margin, mCount); 
	fprintf (fp, "%s         %d;\n", margin, fCount); 

	// dump polygon material indices
  for (i = 0; i < fCount; i++)
  {
    if (i != 0)
    {
  		fprintf (fp, ",\n"); 
    }
		fprintf (fp, "%s         %d", margin, indexList[i]); 
  }
	fprintf (fp, ";\n\n"); 

  Vid::Heap::Restore( heapSize);

	// dump material list
	Bool textureFlag = FALSE;
	for (i = 0; i < mCount; i++)
	{
		// get the material
		Material *material = mats[i];

		ASSERT( material);

		// write out material data
		fprintf (fp, "%s         SI_Material {\n", margin);
		fprintf (fp, "%s            %f;%f;%f;%f;;\n", margin, 
			material->GetDesc().diffuse.r,
			material->GetDesc().diffuse.g,
			material->GetDesc().diffuse.b,
			material->GetDesc().diffuse.a);
		fprintf (fp, "%s            %f;\n", margin,
			material->GetDesc().power);
		fprintf (fp, "%s            %f;%f;%f;;\n", margin,
			material->GetDesc().specular.r,
			material->GetDesc().specular.g,
			material->GetDesc().specular.b);
		fprintf (fp, "%s            %f;%f;%f;;\n", margin,
			material->GetDesc().emissive.r,
			material->GetDesc().emissive.g,
			material->GetDesc().emissive.b);
		fprintf (fp, "%s            %d;\n", margin, 2);
		fprintf (fp, "%s            %f;%f;%f;;\n", margin, 0.7f, 0.7f, 0.7f);

    if (texs[i])
    {
  		fprintf (fp, "%s            TextureFilename {\n", margin);
	  	fprintf (fp, "%s               \"%s\";\n", margin, texs[i]->GetName());
  		fprintf (fp, "%s            }\n", margin);

      textureFlag = TRUE;
    }
		fprintf (fp, "%s         }\n", margin);
	}
	fprintf (fp, "%s      }\n\n", margin);

  U32 nCount = fCount * 3;
	// dump mesh normals header
	fprintf (fp, "%s      SI_MeshNormals {\n", margin);
	fprintf (fp, "%s         %d;\n", margin, nCount);
	
	// dump polygon normals
  Bool hit = FALSE;
	for (i = 0; i < fCount; i++)
	{
    for (j = 0; j < 3; j++)
    {
      Vector &norm = mesh->normals[mesh->faces[i].norms[j]];

      if (hit)
      {
  		  fprintf ( fp, ",\n");
      }
		  fprintf ( fp, "%s         %f;%f;%f;", margin, 
#ifdef DOXFLIP
        -norm.x, norm.y, norm.z);
#else
#ifdef DOZFLIP
        norm.x, norm.y, -norm.z);
#else
        norm.x, norm.y,  norm.z);
#endif
#endif
      hit = TRUE;
    }
	}
	fprintf ( fp, ";\n\n");

	// output polygon normal indices
	fprintf (fp, "%s         %d;\n", margin, fCount);
 	for (i = j = 0; i < fCount; i++, j += 3)
	{
    if (i != 0)
    {
  		fprintf ( fp, ",\n");
    }
		fprintf (fp, "%s         %d;%d;%d,%d,%d;", 
      margin, i, 3,
#if (defined DOXFLIP) || (defined DOZFLIP)
      j + 2, j + 1, j + 0);
#else
      j + 0, j + 1, j + 2);
#endif
	}
	fprintf ( fp, ";\n");

  fprintf (fp, "%s      }\n\n", margin);
	
	if (textureFlag)
  {
  	// dump texture map coordinates header
    fprintf (fp,  "%s      SI_MeshTextureCoords {\n", margin);

		// dump polygon texture map coordinates
		fprintf ( fp, "%s         %d;\n", margin, nCount);
    hit = FALSE;
		for (i = 0; i < fCount; i++)
		{
		  for (j = 0; j < 3; j++)
		  {
        UVPair &uv = mesh->uvs[mesh->faces[i].uvs[j]];

        if (hit)
        {
  		    fprintf ( fp, ",\n");
        }
#ifdef DOXSITEX     // FIXME
			  fprintf ( fp, "%s         %f;%f;", margin,  uv.u, 1.0f - uv.v);
#else
			  fprintf ( fp, "%s         %f;%f;", margin,  uv.u, uv.v);
#endif

        hit = TRUE;
		  }
    }
 		fprintf ( fp, ";\n\n");

		// dump polygon texture map indices
		fprintf (fp, "%s         %d;\n", margin, fCount);
		for (i = j = 0; i < fCount; i++, j += 3)
		{
      if (i != 0)
      {
  		  fprintf ( fp, ",\n");
      }
		  fprintf (fp, "%s         %d;%d;%d,%d,%d;", 
        margin, i, 3,
#if (defined DOXFLIP) || (defined DOZFLIP)
      j + 2, j + 1, j + 0);
#else
      j + 0, j + 1, j + 2);
#endif
	  }
	  fprintf ( fp, ";\n");

		fprintf (fp, "%s      }\n", margin);
	}
	
	fprintf (fp, "%s   }\n", margin);

  fprintf (fp, "%s}\n", margin);
}
//----------------------------------------------------------------------------

Bool MeshRoot::WriteXSI( const char *name) 
{
  FILE *fp = fopen (name, "wb");
  if (!fp)
  {
   ASSERT (fp);
   return FALSE;
  }

  fprintf (fp, "xsi 0101txt 0032\n\n");

  fprintf (fp, "SI_CoordinateSystem coord {\n");                 
  fprintf (fp, "   1;\n");                                    
  fprintf (fp, "   0;\n");                                    
  fprintf (fp, "   1;\n");                                    
  fprintf (fp, "   0;\n");                                    
  fprintf (fp, "   2;\n");                                    
  fprintf (fp, "   5;\n");                                    
  fprintf (fp, "}\n\n");                                        

  fprintf (fp, "SI_Camera  cam_int1 {\n");                       
  fprintf (fp, "   -594.508301; 79.008514; 893.325684;;\n");  
  fprintf (fp, "   -589.973083; 1.795721; 750.634949;;\n");   
  fprintf (fp, "   0;0.000000;;\n");                          
  fprintf (fp, "   0;41.539440;;\n");                         
  fprintf (fp, "   0.100000;\n");                             
  fprintf (fp, "   32768.000000;\n");                         
  fprintf (fp, "}\n\n");                                        

  fprintf (fp, "SI_Ambience {\n");                               
  fprintf (fp, "   0.300000; 0.300000; 0.300000;;\n");        
  fprintf (fp, "}\n\n");                                        

  char margin[256];
	margin[0] = '\0';

  SaveMeshRoot( fp, this, (char *) this->GetName(), margin);

  fclose (fp);

  return TRUE;
}
//----------------------------------------------------------------------------

#ifdef SAVEIT

// reflip the model's z coords
//#define DOZFLIP

static void SaveMesh( FILE *fp, Mesh *mesh, char *name, char *margin) 
{
  margin;
  name;
  mesh;
  fp;
  
#if 0
	U32 vCount = mesh->vertices.count;
	U32 fCount = mesh->indices.count / 3;
	U32 mCount = mesh->primitives.count;

	// dump polygon mesh header
	fprintf (fp, "%s   Mesh %s {\n", margin, name);
	
	// dump polygon vertices
	fprintf ( fp, "%s      %d;\n", margin, vCount);
  U32 i, j, k;
	for (i = 0; i < vCount-1; i++)
  {
		fprintf ( fp, "%s      %f;%f;%f;,\n", margin,
			mesh->VertVectorX(i), 
			mesh->VertVectorY(i), 
#ifdef DOZFLIP
			-mesh->VertVectorZ(i));
#else
			mesh->VertVectorZ(i));
#endif
	}
	fprintf ( fp, "%s      %f;%f;%f;;\n\n", margin,
		mesh->VertVectorX(i), 
		mesh->VertVectorY(i), 
#ifdef DOZFLIP
			-mesh->VertVectorZ(i));
#else
			mesh->VertVectorZ(i));
#endif
	
	// dump polygon indices
	fprintf (fp, "%s      %d;\n", margin, fCount);
	if (fCount > 0)
	{
#ifdef DOZFLIP
		for (i = 0; i < fCount - 1; i ++)
		{
			fprintf (fp, "%s      %d;%d,%d,%d;,\n", margin, 3, 
				mesh->indices[i*3+2],
				mesh->indices[i*3+1],
				mesh->indices[i*3+0]);
		}
		fprintf (fp, "%s      %d;%d,%d,%d;;\n\n", margin, 3,
			mesh->indices[i*3+2],
			mesh->indices[i*3+1],
			mesh->indices[i*3+0]);
#else
		for (i = 0; i < fCount - 1; i ++)
		{
			fprintf (fp, "%s      %d;%d,%d,%d;,\n", margin, 3, 
				mesh->indices[i*3+0],
				mesh->indices[i*3+1],
				mesh->indices[i*3+2]);
		}
		fprintf (fp, "%s      %d;%d,%d,%d;;\n\n", margin, 3,
			mesh->indices[i*3+0],
			mesh->indices[i*3+1],
			mesh->indices[i*3+2]);
#endif
	}
	
	// dump material list header
	fprintf (fp, "%s      MeshMaterialList {\n", margin);
	fprintf (fp, "%s         %d;\n", margin, mCount); 
	fprintf (fp, "%s         %d;\n", margin, fCount); 

	// dump polygon material indices
	k = fCount;
//	for (i = 0; i < mCount; i++)
//	{
	i = 0;
//		for (j = 0; j < mesh->primitives[i].Face_Count; j++)
		for (j = 0; j < fCount-1; j++)
		{
			fprintf (fp, "%s         %d%,\n", margin, i); 
		}
		fprintf (fp, "%s         %d%;\n\n", margin, i); 
//	}

	// dump material list
	Bool Texture_Flag = TRUE;
	for (i = 0; i < mCount; i++)
	{
		// get the material
		Material *material = mesh->primitives[i].material;

		if (material)
		{
			// change texture name extension
      char MatName[32];
			if (material->texName[0] == '\0')
      {
				strcpy (MatName, "engine_default.pic");
			}
      else
      {
				strcpy (MatName, material->texName);
				strtok (MatName, ".");
				strcat (MatName, ".pic");
			}
//			strcpy (MatName, material->texName);
//			strtok (MatName, ".");
//			strcat (MatName, ".tga");
//			strcat (MatName, ".bmp");


			// write out material data
			fprintf (fp, "%s         SI_Material {\n", margin);
			fprintf (fp, "%s            %f;%f;%f;%f;;\n", margin, 
				material->GetDesc().diffuse.r,
				material->GetDesc().diffuse.g,
				material->GetDesc().diffuse.b,
				material->GetDesc().diffuse.a);
			fprintf (fp, "%s            %f;\n", margin,
				material->GetDesc().power);
			fprintf (fp, "%s            %f;%f;%f;;\n", margin,
				material->GetDesc().specular.r,
				material->GetDesc().specular.g,
				material->GetDesc().specular.b);
			fprintf (fp, "%s            %f;%f;%f;;\n", margin,
				material->GetDesc().emissive.r,
				material->GetDesc().emissive.g,
				material->GetDesc().emissive.b);
			fprintf (fp, "%s            %d;\n", margin, 2);
			fprintf (fp, "%s            %f;%f;%f;;\n", margin, 0.7f, 0.7f, 0.7f);
			fprintf (fp, "%s            TextureFilename {\n", margin);
			fprintf (fp, "%s               \"%s\";\n", margin, MatName);
			fprintf (fp, "%s            }\n", margin);
			fprintf (fp, "%s         }\n", margin);
			fprintf (fp, "%s      }\n", margin);
		}
		else
		{
			// generate a default material
			Texture_Flag = FALSE;
			fprintf (fp, "%s         SI_Material {\n", margin);
			fprintf (fp, "%s            %f;%f;%f;%f;;\n", margin, 0.7f, 0.7f, 0.7f, 1.0f);
			fprintf (fp, "%s            %f;\n", margin, 50.0f);
			fprintf (fp, "%s            %f;%f;%f;;\n", margin, 1.0f, 1.0f, 1.0f);
			fprintf (fp, "%s            %f;%f;%f;;\n", margin, 0.0f, 0.0f, 0.0f);
			fprintf (fp, "%s            %d;\n", margin, 2);
			fprintf (fp, "%s            %f;%f;%f;;\n", margin, 0.7f, 0.7f, 0.7f);
			fprintf (fp, "%s         }\n", margin);
			fprintf (fp, "%s      }\n", margin);
		}
	}

	// dump mesh normals header
	fprintf (fp, "%s      SI_MeshNormals {\n", margin);
	fprintf (fp, "%s         %d;\n", margin, fCount * 3);
	
	// output polygon normals
	for (j = 0; j < fCount * 3 - 1; j ++)
	{
		fprintf ( fp, "%s         %f;%f;%f;,\n", margin, 
      mesh->VertNormalX(mesh->indices[j]), 
      mesh->VertNormalY(mesh->indices[j]), 
#ifdef DOZFLIP
      -mesh->VertNormalZ(mesh->indices[j]));
#else
      mesh->VertNormalZ(mesh->indices[j]));
#endif
	}
	fprintf ( fp, "%s         %f;%f;%f;;\n", margin, 
    mesh->VertNormalX(mesh->indices[j]), 
    mesh->VertNormalY(mesh->indices[j]), 
#ifdef DOZFLIP
    -mesh->VertNormalZ(mesh->indices[j]));
#else
    mesh->VertNormalZ(mesh->indices[j]));
#endif
	
	// output polygon normal indices
	fprintf (fp, "%s         %d;\n", margin, fCount);
	k = 0;
	for (j = 0; j < fCount - 1; j ++)
	{
#ifdef DOZFLIP
		fprintf (fp, "%s         %d;%d;%d,%d,%d;,\n", margin, j, 3, k+2, k+1, k+0);
#else
		fprintf (fp, "%s         %d;%d;%d,%d,%d;,\n", margin, j, 3, k+0, k+1, k+2);
#endif
		k += 3;
	}
#ifdef DOZFLIP
    fprintf (fp, "%s         %d;%d;%d,%d,%d;;\n", margin, j, 3, k+2, k+1, k+0);
#else
    fprintf (fp, "%s         %d;%d;%d,%d,%d;;\n", margin, j, 3, k+0, k+1, k+2);
#endif

  fprintf (fp, "%s      }\n\n\n", margin);
	
	if (Texture_Flag)
  {
  	// dump texture map coordinates header
    fprintf (fp,  "%s      SI_MeshTextureCoords {\n", margin);

		// dump polygon texture map coordinates
		fprintf ( fp, "%s         %d;\n", margin, fCount * 3);
		for (j = 0; j < fCount * 3 - 1; j ++)
		{
			fprintf ( fp, "%s         %f;%f;,\n", margin, 
        mesh->VertU(mesh->indices[j]),
        mesh->VertV(mesh->indices[j]));
		}
		fprintf ( fp, "%s         %f;%f;;\n", margin, 
      mesh->VertU(mesh->indices[j]),
      mesh->VertV(mesh->indices[j]));

		// dump polygon texture map indices
		fprintf (fp, "%s         %d;\n", margin, fCount);
		k = 0;
		for (j = 0; j < fCount - 1; j ++)
		{
#ifdef DOZFLIP
			fprintf (fp, "%s         %d%;%d;%d,%d,%d;,\n", margin, j, 3, k+2, k+1, k+0);
#else
			fprintf (fp, "%s         %d%;%d;%d,%d,%d;,\n", margin, j, 3, k+0, k+1, k+2);
#endif
			k += 3;
		}
#ifdef DOZFLIP
		fprintf (fp, "%s         %d;%d;%d,%d,%d;;\n", margin, j, 3, k+2, k+1, k+0);
#else
		fprintf (fp, "%s         %d;%d;%d,%d,%d;;\n", margin, j, 3, k+0, k+1, k+2);
#endif
		fprintf (fp, "%s      }\n", margin);
	}
	
	fprintf (fp, "%s   }\n", margin);

#endif
}
//----------------------------------------------------------------------------

static void SaveMeshGroup( FILE *fp, Mesh *mesh, U32 level)
{
  fp;
  mesh;
  level;

#if 0
  while (mesh)
	{
  	static char Indent[] = "   ";
	  char margin[256];
		margin[0] = '\0';
    U32 i;
		for (i = 0; i < level; i ++)
    {
			strcat( margin, Indent);
    }

    char name[80];
		strcpy(name, "frm-");
    Utils::Strncpy (name + 4, mesh->name, 16);
		
    fprintf (fp, "%sFrame %s {\n\n", margin, name);

    fprintf (fp, "%s   FrameTransformMatrix {\n", margin);

    Matrix matrix = mesh->objectMatrixSim;
#ifdef DOZFLIP
    Quaternion q;
    q.Set( matrix);
    q.v.x *= -1.0f;
    q.v.y *= -1.0f;
    matrix.Set( q);
    matrix.posit.z *= -1.0f;
#endif

    fprintf (fp, "%s      %lf,%lf,%lf,%lf,\n", margin,
      matrix.right.x,
      matrix.right.y,
      matrix.right.z,
      matrix.rightw);

    fprintf (fp, "%s      %lf,%lf,%lf,%lf,\n", margin,
      matrix.up.x,
      matrix.up.y,
      matrix.up.z,
      matrix.upw);

    fprintf (fp, "%s      %lf,%lf,%lf,%lf,\n", margin,
      matrix.front.x,
      matrix.front.y,
      matrix.front.z,
      matrix.frontw);

    fprintf (fp, "%s      %lf,%lf,%lf,%lf;;\n", margin,
      matrix.posit.x,
      matrix.posit.y,
      matrix.posit.z,
      matrix.positw);

    fprintf (fp, "%s   }\n\n", margin);
	
		if (mesh->vertices.count)
    {
			SaveMesh(fp, mesh, name + 4, margin);
		}
		
    if (mesh->Child())
		{
			fprintf (fp, "\n");
      SaveMeshGroup( fp, (Mesh *)mesh->Child(), level + 1);
    }

    fprintf (fp, "%s}\n", margin);

    mesh = (Mesh *)mesh->Sibling();
  }
#endif
}
//----------------------------------------------------------------------------

Bool MeshRoot::Save( char *name) 
{
  name;

#if 0
  FILE *fp = fopen (name, "wb");
  if (!fp)
  {
   ASSERT (fp);
   return FALSE;
  }

  fprintf (fp, "xsi 0101txt 0032\n\n");

  fprintf (fp, "SI_CoordinateSystem coord {\n");                 
  fprintf (fp, "   1;\n");                                    
  fprintf (fp, "   0;\n");                                    
  fprintf (fp, "   1;\n");                                    
  fprintf (fp, "   0;\n");                                    
  fprintf (fp, "   2;\n");                                    
  fprintf (fp, "   5;\n");                                    
  fprintf (fp, "}\n\n");                                        

  fprintf (fp, "SI_Camera  cam_int1 {\n");                       
  fprintf (fp, "   -594.508301; 79.008514; 893.325684;;\n");  
  fprintf (fp, "   -589.973083; 1.795721; 750.634949;;\n");   
  fprintf (fp, "   0;0.000000;;\n");                          
  fprintf (fp, "   0;41.539440;;\n");                         
  fprintf (fp, "   0.100000;\n");                             
  fprintf (fp, "   32768.000000;\n");                         
  fprintf (fp, "}\n\n");                                        

  fprintf (fp, "SI_Ambience {\n");                               
  fprintf (fp, "   0.300000; 0.300000; 0.300000;;\n");        
  fprintf (fp, "}\n\n");                                        

  SaveMeshGroup( fp, this, 0);

  fclose (fp);

#endif

  return TRUE;
}
//----------------------------------------------------------------------------

#endif
