///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2000 Pandemic Studios, Dark Reign II
//
// export.cpp
//
//
#include "xsiexp.h"
#include "bonesdef.h"
//----------------------------------------------------------------------------

Matrix3 oTopMatrix, topMatrix, correction( Point3(1,0,0), Point3(0,0,-1), Point3(0,1,0), Point3(0,0,0));   
AffineParts affine; 
//----------------------------------------------------------------------------

// dump some global information.
void XsiExp::ExportGlobalInfo()
{
	// header and format
	fprintf(pStream, "xsi 0101txt 0032\n\n");

	// coords
	fprintf(pStream, "SI_CoordinateSystem coord {\n\t1;\n\t0;\n\t1;\n\t0;\n\t2;\n\t5;\n}\n\n");

	// angle?
	fprintf(pStream, "SI_Angle {\n\t0;\n}\n\n");

  // camera
  fprintf(pStream, "SI_Camera  Camera1 {\n\t-4.681479; 4.736986; 8.858665;;\n\t-0.145911; 0.062833; 0.049857;;\n\t0.000000;\n\t44.024410;\n\t0.100000;\n\t32768.000000;\n}\n\n");

	// ambient light
  Color ambLight = ip->GetAmbient(GetStaticFrame(), FOREVER);
  fprintf(pStream, "SI_Ambience {\n\t%.6f; %.6f; %.6f;;\n}\n\n", ambLight.r, ambLight.g, ambLight.b);

  fprintf(pStream, "SI_Fog {\n\t0;\n\t0;\n\t0.157895; 0.225564; 0.270677;;\n\t20.000000;\n\t100.000000;\n}\n\n");
}
//----------------------------------------------------------------------------

void XsiExp::ExportMeshObject( INode * node, int indentLevel)
{
	Object * obj = node->EvalWorldState(GetStaticFrame()).obj;
	if (!obj || obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
  {
		return;
  }
	TSTR indent = GetIndent(indentLevel);
	
	ExportNodeHeader(node, "Frame", indentLevel);
	
	ExportNodeTM(node, indentLevel);
	
	ExportMesh(node, GetStaticFrame(), indentLevel);
}
//----------------------------------------------------------------------------

void XsiExp::ExportBoneObject( INode * node, int indentLevel)
{
	Object * obj = node->EvalWorldState(GetStaticFrame()).obj;
	if (!obj || obj->ClassID() != Class_ID(BONE_CLASS_ID, 0))
  {
    // reject non-bones
    return;
	}
  if (!node->GetParentNode() || node->GetParentNode()->IsRootNode())
  {
    // bone matrices get passed to children
    // drop root bone
    return;
  }
	TSTR indent = GetIndent(indentLevel);

  ExportNodeHeader(node, "Frame", indentLevel);

  // export parent as this bone
	ExportNodeTM(node->GetParentNode(), indentLevel);
}
//----------------------------------------------------------------------------

void XsiExp::ExportNodeHeader( INode * node, TCHAR * type, int indentLevel)
{
	TSTR indent = GetIndent(indentLevel);
	
	// node header: object type and name
	fprintf(pStream,"%s%s frm-%s {\n\n", indent.data(), type, FixupName(node->GetName()));
}
//----------------------------------------------------------------------------

void XsiExp::ExportNodeTM( INode * node, int indentLevel)
{
	// dump the full matrix
	Matrix3 matrix = node->GetNodeTM(GetStaticFrame());
	TSTR indent = GetIndent(indentLevel);
	
	fprintf(pStream,"%s\t%s {\n\n", indent.data(), "FrameTransformMatrix");

	Object * obj = node->EvalWorldState(0).obj;
  BOOL isBone = obj && obj->ClassID() == Class_ID(BONE_CLASS_ID, 0) ? TRUE : FALSE;

  if (node->GetParentNode() && node->GetParentNode()->IsRootNode())
  {
    // bone chains get grafted into the hierarchy tree
    //
	  if (!isBone)
    {
      // root mesh
      oTopMatrix = matrix;
      AffineParts ap;
      decomp_affine( matrix, &ap);
      topMatrix.Set( Point3( ap.k.x,0.0f,0.0f), Point3( 0.0f,ap.k.z,0.0f), Point3(0.0f,0.0f,ap.k.y), Point3(0,0,0));

      // root transform is controlled by the engine
      matrix.IdentityMatrix();
    }
  }
  else
  {
    matrix = matrix * Inverse(node->GetParentTM(GetStaticFrame()));
    if (!isBone)
    {
      matrix.SetRow( 3, topMatrix * matrix.GetRow(3));
    }
  }

  // write the matrix values
  DumpMatrix3( &matrix, indentLevel+2);

  // transform close brace
	fprintf(pStream,"%s\t}\n", indent.data());
}
//----------------------------------------------------------------------------

// is m negatively scaled; i.e. is one axis out of wack
BOOL XsiExp::TMNegParity( Matrix3 & m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2)) < 0.0) ? 1 : 0;
}
//----------------------------------------------------------------------------

void XsiExp::ExportMesh( INode * node, TimeValue t, int indentLevel)
{
	ObjectState os = node->EvalWorldState(t);
	if (!os.obj || os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID)
  {
		return; // Safety net. This shouldn't happen.
	}
	BOOL needDel;
	TriObject * tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri)
  {
    // no tri object
		return;
	}
  // prepare mesh
  Mesh * mesh = &tri->GetMesh();
  mesh->buildNormals();

  // object offset matrix; apply to verts
  // swap y and z; max to soft correction
  Matrix3 matrix(1);
  //  translate
  matrix.PreTranslate( Point3( node->GetObjOffsetPos().x, node->GetObjOffsetPos().z, -node->GetObjOffsetPos().y));

  // rotate
  AngAxis aa( node->GetObjOffsetRot());
  float temp = aa.axis.z;
  aa.axis.z = -aa.axis.y;
  aa.axis.y = temp;
  PreRotateMatrix(matrix, Quat( aa));

  // scale
  ScaleValue scale = node->GetObjOffsetScale();
  aa.Set( scale.q);
  temp = aa.axis.z;
  aa.axis.z = -aa.axis.y;
  aa.axis.y = temp;
  scale.q.Set( aa);
  temp = scale.s.z;
  scale.s.z = scale.s.y;
  scale.s.y = temp;
  ApplyScaling(matrix, scale);

  // apply root transform
  matrix = matrix * topMatrix;
  // only rotation for normals
  AffineParts ap;
  Matrix3 rotMatrix(1);
  decomp_affine( matrix, &ap);
  PreRotateMatrix( rotMatrix, ap.q);

  // set winding order
	int vx1 = 0, vx2 = 1, vx3 = 2;
	if (TMNegParity( node->GetNodeTM(GetStaticFrame())) != TMNegParity( matrix) )
  {
    // negative scaling; invert winding order and normal rotation
		vx1 = 2;	vx2 = 1;	vx3 = 0;
    rotMatrix = rotMatrix * Matrix3( Point3(-1,0,0), Point3(0,-1,0), Point3(0,0,-1), Point3(0,0,0));
	}

  // header
	TSTR indent = GetIndent(indentLevel+1);
	fprintf(pStream, "%s%s %s {\n",indent.data(), "Mesh", FixupName(node->GetName()));

  // write number of verts
  int numLoop = mesh->getNumVerts();
	fprintf(pStream, "%s\t%d;\n",indent.data(), numLoop);

  // write verts
	for (int i = 0; i < numLoop; i++)
  {
		Point3 v = mesh->verts[i];
		float temp = v.z;
    v.z = -v.y;
    v.y = temp;
		v = matrix * v;
		fprintf(pStream, "%s\t%.6f;%.6f;%.6f;%s\n", indent.data(), v.x, v.y, v.z, 
      i == numLoop - 1 ? ";\n" : ",");
	}
  // write number of faces
  numLoop = mesh->getNumFaces();
  fprintf(pStream, "%s\t%d;\n", indent.data(), numLoop);

  // write faces
	for (i = 0; i < numLoop; i++)
  {
		fprintf(pStream, "%s\t3;%d,%d,%d;%s\n",
			indent.data(),
			mesh->faces[i].v[vx1],
			mesh->faces[i].v[vx2],
			mesh->faces[i].v[vx3], 
      i == numLoop - 1 ? ";\n" : ",");
	}

  // face materials
	Mtl * nodeMtl = node->GetMtl();
  int numMtls = !nodeMtl || !nodeMtl->NumSubMtls() ? 1 : nodeMtl->NumSubMtls();

	// write face material list header	
	fprintf(pStream, "%s\tMeshMaterialList {\n", indent.data());
  // write number of materials
	fprintf(pStream, "%s\t\t%d;\n", indent.data(), numMtls);
  // write number of faces
  fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop);

  // write face material indices (1 for each face)
  for (i = 0; i < numLoop; i++)
  {
    int index = numMtls ? mesh->faces[i].getMatID() % numMtls : 0;
		fprintf(pStream,"%s\t\t%d%s\n",
			indent.data(),
      index,
      i == numLoop - 1 ? ";\n" : ",");
	}

  // write the materials
  ExportMaterial( node, indentLevel+2);

  // verts close brace
	fprintf(pStream, "%s\t}\n\n",indent.data());

  // write normals header
	fprintf(pStream, "%s\t%s {\n", indent.data(), "SI_MeshNormals");
	// write number of normals
  fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop * 3);

  // write normals (3 for each face)
	for (i = 0; i < numLoop; i++)
  {
		Face * f = &mesh->faces[i];
		int vert = f->getVert(vx1);

		Point3 vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
    float temp = vn.z;
    vn.z = -vn.y;
    vn.y = temp;
		vn = rotMatrix * vn;
		fprintf(pStream,"%s\t\t%.6f;%.6f;%.6f;,\n", indent.data(), vn.x, vn.y, vn.z);

		vert = f->getVert(vx2);
		vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
    temp = vn.z;
    vn.z = -vn.y;
    vn.y = temp;
		vn = rotMatrix * vn;
		fprintf(pStream,"%s\t\t%.6f;%.6f;%.6f;,\n", indent.data(), vn.x, vn.y, vn.z);
    
		vert = f->getVert(vx3);
		vn = GetVertexNormal(mesh, i, mesh->getRVertPtr(vert));
    temp = vn.z;
    vn.z = -vn.y;
    vn.y = temp;
		vn = rotMatrix * vn;
		fprintf(pStream,"%s\t\t%.6f;%.6f;%.6f;%s\n", indent.data(), vn.x, vn.y, vn.z,
      i == numLoop - 1 ? ";\n" : ",");
	}
  // write number of faces
  fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop);

  // write faces
  for (i = 0; i < numLoop; i++)
  {
	  fprintf(pStream, "%s\t\t%d;3;%d,%d,%d;%s\n",
		  indent.data(),
      i,
      i * 3 + vx1, i * 3 + vx2, i * 3 + vx3,
      i == numLoop - 1 ? ";\n" : ",");
  }
  // normals close brace
	fprintf(pStream, "%s\t}\n\n",indent.data());

	// texcoords
	if (nodeMtl && mesh && (nodeMtl->Requirements(-1) & MTLREQ_FACEMAP))
  {
    // facemapping
    numLoop = mesh->getNumFaces() * 3;

    // write texture coords header
    fprintf(pStream, "%s\tSI_MeshTextureCoords {\n", indent.data());
    // write number of texture coords
    fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop);

    // write texture coords
	  for (int i = 0; i < numLoop; i++)
    {
		  Point3 tv[3];
		  Face * f = &mesh->faces[i];
		  make_face_uv( f, tv);
		  fprintf(pStream, "%s\t\t%.6f;%.6f;,\n",  indent.data(), tv[0].x, tv[0].y);
		  fprintf(pStream, "%s\t\t%.6f;%.6f;,\n",  indent.data(), tv[1].x, tv[1].y);
		  fprintf(pStream, "%s\t\t%.6f;%.6f;%s\n", indent.data(), tv[2].x, tv[2].y,
        i == numLoop - 1 ? ";\n" : ",");
	  }
    // write number of faces
    numLoop = mesh->getNumFaces();
	  fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop);

    // write faces
	  for (i = 0; i < numLoop; i++)
    {
		  fprintf(pStream,"%s\t\t%d;3;%d,%d,%d;%s\n",
			  indent.data(),
			  i,
			  mesh->tvFace[i].t[vx1],
			  mesh->tvFace[i].t[vx2],
			  mesh->tvFace[i].t[vx3],
        i == numLoop - 1 ? ";\n" : ",");
	  }
    // texture coords close brace
	  fprintf(pStream, "%s\t}\n\n", indent.data());
  }
  else
  {
		numLoop = mesh->getNumTVerts();

		if (numLoop)
    {
      // write texture coords header
  		fprintf(pStream, "%s\tSI_MeshTextureCoords {\n", indent.data());
      // write number of texture coords
  		fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop);

      // write texture coords
			for (i = 0; i < numLoop; i++)
      {
				UVVert tv = mesh->tVerts[i];
				fprintf(pStream, "%s\t\t%.6f;%.6f;%s\n", indent.data(), tv.x, tv.y,
        i == numLoop - 1 ? ";\n" : ",");
			}
      // write number of faces
      numLoop = mesh->getNumFaces();
			fprintf(pStream, "%s\t\t%d;\n", indent.data(), numLoop);

      // write faces
			for (i = 0; i < numLoop; i++)
      {
				fprintf(pStream,"%s\t\t%d;3;%d,%d,%d;%s\n",
					indent.data(),
					i,
					mesh->tvFace[i].t[vx1],
					mesh->tvFace[i].t[vx2],
					mesh->tvFace[i].t[vx3],
          i == numLoop - 1 ? ";\n" : ",");
			}
      // texture coords close brace
			fprintf(pStream, "%s\t}\n\n", indent.data());
		}
  }

/*
	// Export color per vertex info
	if (GetIncludeVertexColors()) {
		int numCVx = mesh->numCVerts;

		fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMCVERTEX, numCVx);
		if (numCVx) {
			fprintf(pStream,"%s\t%s {\n",indent.data(), ID_MESH_CVERTLIST);
			for (i=0; i<numCVx; i++) {
				Point3 vc = mesh->vertCol[i];
				fprintf(pStream, "%s\t\t%s %d\t%s\n",indent.data(), ID_MESH_VERTCOL, i, Format(vc));
			}
			fprintf(pStream,"%s\t}\n",indent.data());
			
			fprintf(pStream, "%s\t%s %d\n",indent.data(), ID_MESH_NUMCVFACES, mesh->getNumFaces());

			fprintf(pStream, "%s\t%s {\n",indent.data(), ID_MESH_CFACELIST);
			for (i=0; i<mesh->getNumFaces(); i++) {
				fprintf(pStream,"%s\t\t%s %d\t%d\t%d\t%d\n",
					indent.data(),
					ID_MESH_CFACE, i,
					mesh->vcFace[i].t[vx1],
					mesh->vcFace[i].t[vx2],
					mesh->vcFace[i].t[vx3]);
			}
			fprintf(pStream, "%s\t}\n",indent.data());
		}
	}
*/

  // Mesh close brace
	fprintf(pStream, "%s}\n",indent.data());
  
  // dispose of tri object
  if (needDel)
  {
		delete tri;
	}
}
//----------------------------------------------------------------------------

// From the SDK
// How to calculate UV's for face mapped materials.
static Point3 basic_tva[3] = { 
	Point3(0.0,0.0,0.0),Point3(1.0,0.0,0.0),Point3(1.0,1.0,0.0)
};
static Point3 basic_tvb[3] = { 
	Point3(1.0,1.0,0.0),Point3(0.0,1.0,0.0),Point3(0.0,0.0,0.0)
};
static int nextpt[3] = {1,2,0};
static int prevpt[3] = {2,0,1};

void XsiExp::make_face_uv(Face *f, Point3 *tv)
{
	int na,nhid,i;
	Point3 *basetv;
	/* make the invisible edge be 2->0 */
	nhid = 2;
	if (!(f->flags&EDGE_A))  nhid=0;
	else if (!(f->flags&EDGE_B)) nhid = 1;
	else if (!(f->flags&EDGE_C)) nhid = 2;
	na = 2-nhid;
	basetv = (f->v[prevpt[nhid]]<f->v[nhid]) ? basic_tva : basic_tvb; 
	for (i=0; i<3; i++) {  
		tv[i] = basetv[na];
		na = nextpt[na];
	}
}
//----------------------------------------------------------------------------

void XsiExp::ExportMaterial( INode * node, int indentLevel)
{
	Mtl * mtl = node->GetMtl();
	
	TSTR indent = GetIndent(indentLevel+1);  
	
	// If the node does not have a material, export the wireframe color
	if (mtl)
  {
		int mtlID = mtlList.GetMtlID(mtl);
		if (mtlID >= 0)
    {
      DumpMaterial( mtl, mtlID, 0, indentLevel);
		}
	}
	else
  {
		DWORD c = node->GetWireColor();

		fprintf(pStream,"%sSI_Material {\n", indent.data());

		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;%.6f;;\n", 
      indent.data(),
			GetRValue(c)/255.0f, GetGValue(c)/255.0f, GetBValue(c)/255.0f, 1.0f); // diffuse
		fprintf(pStream,"%s\t%.6f;\n", indent.data(), 0.0f );                   // specular power
		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;;\n", 
      indent.data(), 0.0f, 0.0f, 0.0f);                                     // specular
		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;;\n", 
      indent.data(), 0.0f, 0.0f, 0.0f);                                     // emissive
		fprintf(pStream,"%s\t%.6f;\n", indent.data(), 0.0f );                   // ?
		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;;\n",                               // ambient
      indent.data(), 0.0f, 0.0f, 0.0f);

  	fprintf(pStream,"%s}\n", indent.data());
  }
}
//----------------------------------------------------------------------------

void XsiExp::DumpMaterial(Mtl * mtl, int mtlID, int subNo, int indentLevel)
{
	if (!mtl)
  {
    return;
  }
	TSTR indent = GetIndent(indentLevel+1);  
  
	if (mtl->NumSubMtls() > 0)
  {
  	for (int i = 0; i < mtl->NumSubMtls(); i++)
    {
			Mtl* subMtl = mtl->GetSubMtl(i);
			if (subMtl)
      {
				DumpMaterial( subMtl, 0, i, indentLevel);
			}
		}
	}
	else
  {
  	TimeValue t = GetStaticFrame();

		// Note about material colors:
		// This is only the color used by the interactive renderer in MAX.
		// To get the color used by the scanline renderer, we need to evaluate
		// the material using the mtl->Shade() method.
		// Since the materials are procedural there is no real "diffuse" color for a MAX material
		// but we can at least take the interactive color.

    fprintf(pStream,"%sSI_Material {\n", indent.data());

		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;%.6f;;\n", 
      indent.data(), mtl->GetDiffuse(t).r, mtl->GetDiffuse(t).g, mtl->GetDiffuse(t).b, 1.0 - mtl->GetXParency(t) );
		fprintf(pStream,"%s\t%.6f;\n", indent.data(), mtl->GetShinStr(t) );
		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;;\n", 
      indent.data(), mtl->GetSpecular(t).r, mtl->GetSpecular(t).g, mtl->GetSpecular(t).b );
		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;;\n", 
      indent.data(), 0.0f, 0.0f, 0.0f);         // emissive
		fprintf(pStream,"%s\t%.6f;\n", indent.data(), 0.0f);         // ?
		fprintf(pStream,"%s\t%.6f;%.6f;%.6f;;\n\n", 
      indent.data(), mtl->GetAmbient(t).r, mtl->GetAmbient(t).g, mtl->GetAmbient(t).b );

    for (int i = 0; i < mtl->NumSubTexmaps(); i++)
    {
		  Texmap * subTex = mtl->GetSubTexmap(i);
		  float amt = 1.0f;
		  if (subTex)
      {
			  // If it is a standard material we can see if the map is enabled.
			  if (mtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0))
        {
				  if (!((StdMat*)mtl)->MapEnabled(i))
          {
					  continue;
          }
				  amt = ((StdMat*)mtl)->GetTexmapAmt(i, 0);
        }
			  DumpTexture(subTex, mtl->ClassID(), i, amt, indentLevel+1);
		  }
	  }
  	fprintf(pStream,"%s}\n", indent.data());
  }
}
//----------------------------------------------------------------------------

void XsiExp::DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt, int indentLevel)
{
	if (!tex) return;
	
	TSTR indent = GetIndent(indentLevel+1);
	
	TSTR className;
	tex->GetClassName(className);

	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
  {
  	fprintf(pStream, "%sSI_Texture2D {\n", indent.data() );

		TSTR mapName = ((BitmapTex *)tex)->GetMapName();    
    fprintf(pStream, "%s\t\"%s\";\n", indent.data(), FixupName(mapName));
    fprintf(pStream, "%s\t3;\n", indent.data());
    fprintf(pStream, "%s\t0;0;\n", indent.data());
    fprintf(pStream, "%s\t0;12594;0;543;\n", indent.data());
    fprintf(pStream, "%s\t0;\n", indent.data());
    fprintf(pStream, "%s\t1;1;\n", indent.data());
    fprintf(pStream, "%s\t0;0;\n", indent.data());
    fprintf(pStream, "%s\t1.000000;1.000000;\n", indent.data());
    fprintf(pStream, "%s\t0.000000;0.000000;\n", indent.data());
    fprintf(pStream, "%s\t1.000000,0.000000,0.000000,0.000000,\n", indent.data());
    fprintf(pStream, "%s\t0.000000,1.000000,0.000000,0.000000,\n", indent.data());
    fprintf(pStream, "%s\t0.000000,0.000000,1.000000,0.000000,\n", indent.data());
    fprintf(pStream, "%s\t0.000000,0.000000,0.000000,1.000000;;\n", indent.data());
    fprintf(pStream, "%s\t3;\n", indent.data());
    fprintf(pStream, "%s\t1.000000;\n", indent.data());
    fprintf(pStream, "%s\t0.750000;\n", indent.data());
    fprintf(pStream, "%s\t1.000000;\n", indent.data());
    fprintf(pStream, "%s\t0.000000;\n", indent.data());
    fprintf(pStream, "%s\t0.000000;\n", indent.data());
    fprintf(pStream, "%s\t0.000000;\n", indent.data());
    fprintf(pStream, "%s\t0.000000;\n", indent.data());

  	fprintf(pStream, "%s}\n", indent.data());
  }

/*
	// If we include the subtexture ID, a parser could be smart enough to get
	// the class name of the parent texture/material and make it mean something.
	fprintf(pStream,"%s\t%s %d\n", indent.data(), ID_TEXSUBNO, subNo);
	
	fprintf(pStream,"%s\t%s %s\n", indent.data(), ID_TEXAMOUNT, Format(amt));
	
	// Is this a bitmap texture?
	// We know some extra bits 'n pieces about the bitmap texture
	if (tex->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00))
  {
		TSTR mapName = ((BitmapTex *)tex)->GetMapName();
		fprintf(pStream,"%s\t%s \"%s\"\n", indent.data(), ID_BITMAP, FixupName(mapName));
		
		StdUVGen* uvGen = ((BitmapTex *)tex)->GetUVGen();
		if (uvGen) {
			DumpUVGen(uvGen, indentLevel+1);
		}
		
		TextureOutput* texout = ((BitmapTex*)tex)->GetTexout();
		if (texout->GetInvert()) {
			fprintf(pStream,"%s\t%s\n", indent.data(), ID_TEX_INVERT);
		}
		
		fprintf(pStream,"%s\t%s ", indent.data(), ID_BMP_FILTER);
		switch(((BitmapTex*)tex)->GetFilterType()) {
		case FILTER_PYR:  fprintf(pStream,"%s\n", ID_BMP_FILT_PYR); break;
		case FILTER_SAT: fprintf(pStream,"%s\n", ID_BMP_FILT_SAT); break;
		default: fprintf(pStream,"%s\n", ID_BMP_FILT_NONE); break;
		}
	}
  
	for (int i = 0; i < tex->NumSubTexmaps(); i++)
  {
		DumpTexture(tex->GetSubTexmap(i), tex->ClassID(), i, 1.0f, indentLevel+1);
	}
	
	fprintf(pStream, "%s}\n", indent.data());
*/
}
//----------------------------------------------------------------------------

void XsiExp::DumpMatrix3( Matrix3 * m, int indentLevel)
{
	Point3 row;
	TSTR indent = GetIndent(indentLevel);

  // swap y and z; max to soft correction
  decomp_affine( *m, &affine);
  // translate
  float temp = affine.t.z;
  affine.t.z = -affine.t.y;
  affine.t.y = temp;

  // rotate
  AngAxis aa( affine.q);
  temp = aa.axis.z;
  aa.axis.z = -aa.axis.y;
  aa.axis.y = temp;
  affine.q.Set(aa);

  // scale
  aa.Set( affine.u);
  temp = aa.axis.z;
  aa.axis.z = -aa.axis.y;
  aa.axis.y = temp;
  affine.u.Set( aa);
  temp = affine.k.z;
  affine.k.z = affine.k.y;
  affine.k.y = temp;

  Matrix3 matrix(1);
  matrix.PreTranslate(affine.t);
  PreRotateMatrix(matrix, affine.q);
  
	row = matrix.GetRow(0);
	fprintf(pStream,"%s %.6f,%.6f,%.6f,0.000000,\n", indent.data(), row.x, row.y, row.z);
	row = matrix.GetRow(1);
	fprintf(pStream,"%s %.6f,%.6f,%.6f,0.000000,\n", indent.data(), row.x, row.y, row.z);
	row = matrix.GetRow(2);
	fprintf(pStream,"%s %.6f,%.6f,%.6f,0.000000,\n", indent.data(), row.x, row.y, row.z);
	row = matrix.GetRow(3);
	fprintf(pStream,"%s %.6f,%.6f,%.6f,1.000000;;\n", indent.data(), row.x, row.y, row.z);
}
//----------------------------------------------------------------------------

// Return an indentation string
TSTR XsiExp::GetIndent(int indentLevel)
{
	TSTR indentString = "";
	for (int i=0; i<indentLevel; i++)
  {
		indentString += "\t";
	}
	
	return indentString;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/****************************************************************************

  String manipulation functions
  
****************************************************************************/

#define CTL_CHARS  31
#define SINGLE_QUOTE 39

// Replace some characters we don't care for.
TCHAR * XsiExp::FixupName( TCHAR * name)
{
	static char buffer[256];

  _tcscpy(buffer, name);
	TCHAR * cPtr = buffer;
  TCHAR * last = *buffer == ' ' ? buffer : NULL;

  while(*cPtr)
  {
		if (*cPtr == '"')
    {
			*cPtr = SINGLE_QUOTE;
    }
    else if (*cPtr <= CTL_CHARS || *cPtr == ' ')
    {
      // replace ctrl chars and space with underscore
			*cPtr = _T('_');

      last = cPtr;
    }
    else 
    {
      last = NULL;
    }
    cPtr++;
  }
  if (last)
  {
    *last = '\0';
  }
  if (*buffer == '\0')
  {
    static int nameCounter = 0;

    sprintf( buffer, "obj%d", nameCounter);
    nameCounter++;
  }

	return buffer;
}
//----------------------------------------------------------------------------

// International settings in Windows could cause a number to be written
// with a "," instead of a ".".
// To compensate for this we need to convert all , to . in order to make the
// format consistent.
void XsiExp::CommaScan(TCHAR* buf)
{
    for(; *buf; buf++) if (*buf == ',') *buf = '.';
}
//----------------------------------------------------------------------------

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* XsiExp::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}
//----------------------------------------------------------------------------

Point3 XsiExp::GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv)
{
	Face* f = &mesh->faces[faceNo];
	DWORD smGroup = f->smGroup;
	int numNormals;
	Point3 vertexNormal;
	
	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL)
  {
		vertexNormal = rv->rn.getNormal();
	}
	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) && smGroup)
  {
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1)
    {
			vertexNormal = rv->rn.getNormal();
		}
		else
    {
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++)
      {
				if (rv->ern[i].getSmGroup() & smGroup)
        {
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else
  {
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = mesh->getFaceNormal(faceNo);
	}
	
	return vertexNormal;
}
//----------------------------------------------------------------------------
// skining export only works for scenes with a single mesh and single skin modifier 
// todo: enum all modifer's, need dirty flag in modifier data

static BonesDefMod * skinMod = NULL;
static INode * skinMesh = NULL;

// find the skinned mesh
//
class MeshEnum : public DependentEnumProc 
{
  virtual int proc(ReferenceMaker * rmaker)
  {
    TSTR name;
    rmaker->GetClassName( name);

    if (!strcmp(name, "Node"))
    {
	    ObjectState os = ((INode *) rmaker)->EvalWorldState(0); 

      // The obj member of ObjectState is the actual object we will export.
	    if (os.obj)
      {
        os.obj->GetClassName( name);

        if (!strcmp(name, "Editable Mesh"))
        {
          skinMesh = (INode *) rmaker;
          TCHAR * meshName = skinMesh->GetName();
          return DEP_ENUM_HALT; 
        }
      }
    }

    return DEP_ENUM_CONTINUE; 
  }
};
static MeshEnum meshEnum;
//----------------------------------------------------------------------------

// find the skin modifer
//
class SkinEnum : public DependentEnumProc 
{
  virtual int proc(ReferenceMaker * rmaker)
  {
    TSTR name;
    rmaker->GetClassName( name);

    if (!strcmp(name, "Skin"))
    {
      skinMod = (BonesDefMod *) rmaker;

      skinMesh = NULL;
      skinMod->EnumDependents( &meshEnum);

      return DEP_ENUM_HALT; 
    }

    return DEP_ENUM_CONTINUE; 
  }
};
static SkinEnum skinEnum;
//----------------------------------------------------------------------------

// find the skin modifer's data
//
static ModContext * skinMC = NULL;

class SkinMCEnum : public ModContextEnumProc
{
  virtual BOOL proc(ModContext * mc)
  {
    skinMC = mc;

    return TRUE;
  }
};
static SkinMCEnum skinMCEnum;
//----------------------------------------------------------------------------

// strait from bonesdef.cpp
//
float RetrieveNormalizedWeight( BoneModData * bmd, int vid, int bid)
{
  if (!skinMod)
  {
    return 0;
  }
  //need to reqeight based on remainder
  double tempdist=0.0f;
  double w;

  int bd = bmd->VertexData[vid]->d[bid].Bones;

  if (skinMod->BoneData[bd].Node == NULL)
  {
	  bmd->VertexData[vid]->d[bid].normalizedInfluences = 0.0f;
	  bmd->VertexData[vid]->d[bid].Influences = 0.0f;
	  return 0.0f;
  }

  if (bmd->VertexData[vid]->d[bid].normalizedInfluences != -1.0f)
  {
    return bmd->VertexData[vid]->d[bid].normalizedInfluences;
  }

  //if more than one bone use a weigthed system
  if (bmd->VertexData[vid]->d.Count() > 1) 
  {
	  double remainder = 0.0f; 
	  double offset =0.0f;
	  tempdist = 0.0f;
	  for (int j = 0; j < bmd->VertexData[vid]->d.Count(); j++) 
	  {
		  float infl = bmd->VertexData[vid]->d[j].Influences;
		  int bone = bmd->VertexData[vid]->d[j].Bones;
		  if (!(skinMod->BoneData[bone].flags & BONE_LOCK_FLAG))
      {
			  tempdist += infl;
      }
		  else 
      {
			  offset += infl;
      }
	  }
	  offset = 1.0f - offset;
	  double vinflu = bmd->VertexData[vid]->d[bid].Influences;
	  int bn = bmd->VertexData[vid]->d[bid].Bones;

	  if 	(!(skinMod->BoneData[bn].flags & BONE_LOCK_FLAG))
    {
		  w = ((bmd->VertexData[vid]->d[bid].Influences)/tempdist) * offset;
    }
	  else
    {
      w = bmd->VertexData[vid]->d[bid].Influences;
    }
  }
  else if (bmd->VertexData[vid]->d.Count() == 1) 
  {
    //if only one bone and absolute control set to it to max control
	  if ( (skinMod->BoneData[bmd->VertexData[vid]->d[0].Bones].flags & BONE_ABSOLUTE_FLAG) )
  //		&& !(bmd->VertexData[vid]->modified) )
	  {
  //		VertexData[vid].d[0].Influences = 1.0f;
		  w = 1.0f;
	  }
	  else
    {
      w = bmd->VertexData[vid]->d[0].Influences;
    }
	}
  return (float)w;
}
//----------------------------------------------------------------------------

BOOL XsiExp::ExportSkinData( INode * node)
{
  // find the skin modifer attached to this bone
  skinMod = NULL;
  node->EnumDependents( &skinEnum);

  // no skin modifer or skinned mesh
  if (!skinMod || !skinMesh)
  {
    return FALSE;
  }
  // get the skin modifer context (data)
  skinMod->EnumModContexts( &skinMCEnum);
  if (!skinMC)
  {
    return FALSE;
  }
	BoneModData * bmd = (BoneModData *) skinMC->localData;

  int bonesUsed = 0, boneCount = skinMod->BoneData.Count();
  for (int i = 0; i < boneCount; i++)
  {
    BoneDataClass & boneData = skinMod->BoneData[i];
    if (boneData.Node)
    {
      bonesUsed++;
    }
  }
  if (!bonesUsed)
  {
    return TRUE;
  }
  TCHAR * meshName = skinMesh->GetName();

  // skinning envelope list header
	fprintf(pStream, "SI_EnvelopeList {\n");
  fprintf(pStream, "\t%d;\n\n", bonesUsed);

  // export envelope data for each bone
  for (i = 0; i < boneCount; i++)
  {
//    fprintf(pStream, "\t\t\"frm-%s\";\n", skinMod->BoneData[i].name);
    BoneDataClass & boneData = skinMod->BoneData[i];
    if (!boneData.Node)
    {
      continue;
    }
    char * boneName = boneData.Node->GetName();

    // envelope header
    fprintf(pStream, "\tSI_Envelope {\n");
    fprintf(pStream, "\t\t\"frm-%s\";\n", meshName);
    fprintf(pStream, "\t\t\"frm-%s\";\n", boneName);

    assert( bmd->VertexDataCount == bmd->VertexData.Count() );

    // count verts for this bone
    int vertCount = 0;
    for (int k = 0; k < bmd->VertexDataCount; k++)
    {
      VertexListClass * vertList = bmd->VertexData[k];
      int boneRefs = vertList->d.Count();
      if (!boneRefs)
      {
        continue;
      }
      for (int l = 0; l < boneRefs; l++)
      {
        if (vertList->d[l].Bones == i)
        {
          vertCount++;
          break;
        }
      }
    }
    // vert count
    fprintf(pStream, "\t\t%d;\n", vertCount);

    // weight data for each vert
    for (k = 0; k < bmd->VertexDataCount; k++)
    {
      VertexListClass * vertList = bmd->VertexData[k];
      int boneRefs = vertList->d.Count();
      if (!boneRefs)
      {
        continue;
      }

      // find this bone's data
      for (int l = 0; l < boneRefs; l++)
      {
        if (vertList->d[l].Bones == i)
        {
          vertCount--;
          float weight = RetrieveNormalizedWeight( bmd, k, l);

          fprintf(pStream, "\t\t%d;%f;%s\n", k, weight * 100.0f,
            vertCount == 0 ? ";\n" : ",");
          break;
        }
      }
    }
    // envelope close
  	fprintf(pStream, "\t}\n\n");
  }
  // envelope list close
	fprintf(pStream, "}\n");

  return TRUE;
}
//----------------------------------------------------------------------------