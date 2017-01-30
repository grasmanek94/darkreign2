///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshread.cpp     mesh xfile loading
//
// 07-JUL-1998
//

#include "vid_private.h"
#include "lex.h"
#include "console.h"

//#define DOAPPMESH
//#define DONOANIMS

#define MAXREADVERTS      (Vid::renderState.maxVerts   * 3)
#define MAXREADINDICES    (Vid::renderState.maxIndices * 3)
#define MAXREADTRIS       (Vid::renderState.maxTris    * 3)
#define MAXREADBUCKYS     64
//----------------------------------------------------------------------------

enum XFileToken
{
	_OPEN_BRACE,
	_CLOSE_BRACE,
	_HEADER,
	_TEMPLATE,
	_FRAME,
	_MESH,
	_MESHMATERIALLIST,
	_MATERIAL,
	_TEXTUREFILENAME,
	_MESHNORMALS,
	_MESHTEXTURECOORDS,
	_FRAMETRANSFORMATIONMATRIX,
	_ANIMATIONSET,
	_ANIMATION,
	_ANIMATIONKEY,
	_TRANSITION_ANIMATIONKEY,
	
	_XSICOORDINATESYSTEM,     // xsi stuff
	_XSITEXTURECOORDS,
	_XSIMESHCOLORS,
	_XSIMATERIAL,
	
	_SI_MESHNORMALS,
	_SI_ANIMATIONPARAMKEY,
  _SI_TEXTURE2D,
  
	_SI_BASEPOSEMATRIX,
	_SI_ENVELOPELIST,
	_SI_ENVELOPE,
	
	_END_OF_FILE
};

struct X_KEYWORDS
{
	char                        *Token_Name;
	XFileToken                  token;
};

static X_KEYWORDS XKeyTable[] =
{
	{ "}",                      _CLOSE_BRACE                },
	{ "{",                      _OPEN_BRACE                 },
	{ "Header",                 _HEADER                     },
	{ "template",               _TEMPLATE                   },
	{ "Frame",                  _FRAME                      },
	{ "Mesh",                   _MESH                       },
	{ "MeshMaterialList",       _MESHMATERIALLIST           },
	{ "Material",               _MATERIAL                   },
	{ "TextureFilename",        _TEXTUREFILENAME            },
	{ "MeshNormals",            _MESHNORMALS                },
	{ "MeshTextureCoords",      _MESHTEXTURECOORDS          },
	{ "FrameTransformMatrix",   _FRAMETRANSFORMATIONMATRIX  },
	{ "AnimationSet",           _ANIMATIONSET               },
	{ "Animation",              _ANIMATION                  },
	{ "AnimationKey",           _ANIMATIONKEY               },
	{ "Transition_Key_Frames",  _TRANSITION_ANIMATIONKEY    },
	
	// xsi stuff revision 0
	{ "CoordinateSystem",       _XSICOORDINATESYSTEM        },
	{ "XSIMaterial",            _XSIMATERIAL                },
	{ "XSIMeshTextureCoords",   _XSITEXTURECOORDS           },
	{ "XSIMeshVertexColors",    _XSIMESHCOLORS              },
	{ "XSIMeshNormals",         _MESHNORMALS                },
	{ "XSIAnimationKey",        _ANIMATIONKEY               },
	
	// xsi stuff revision 1
	{ "SI_CoordinateSystem",    _XSICOORDINATESYSTEM        },
	{ "SI_Material",            _XSIMATERIAL                },
	{ "SI_Texture2D",           _SI_TEXTURE2D               },
  { "SI_MeshTextureCoords",   _XSITEXTURECOORDS           },
	{ "SI_MeshVertexColors",    _XSIMESHCOLORS              },
	{ "SI_MeshNormals",         _SI_MESHNORMALS             },
	{ "SI_AnimationKey",        _ANIMATIONKEY               },
	{ "SI_AnimationParamKey",   _SI_ANIMATIONPARAMKEY       },
	
	{ "SI_FrameBasePoseMatrix", _SI_BASEPOSEMATRIX          },
	{ "SI_EnvelopeList",        _SI_ENVELOPELIST            },
	{ "SI_Envelope",            _SI_ENVELOPE                },
	
	{  NULL,                    _END_OF_FILE},
};

static LexFile <XFileToken> XFileLex((void *) XKeyTable);
//----------------------------------------------------------------------------

// temp mesh data loading structure
//
struct MeshFeature
{
	U32             objCount, objCountLast;
	
	U32             triCount, triCountLast;
	U16 *           tris;
	
	U32             faceCount, faceCountLast;
	U16 *           faceIndex;
	U16 *           faceTris;
	
	MeshFeature()
	{
		objCount = triCount = faceCount = 0;
		objCountLast = triCountLast = faceCountLast = 0;

 	  tris      = new U16[MAXREADTRIS * 3];
  	faceIndex = new U16[MAXREADTRIS];
 	  faceTris  = new U16[MAXREADTRIS];
	}
  ~MeshFeature()
  {
    delete [] tris;
    delete [] faceIndex;
    delete [] faceTris;
  }

	void SaveCounts()
	{
		objCountLast = objCount;
		triCountLast = triCount;
		faceCountLast = faceCount;
	}
	void RestoreCounts()
	{
		objCount = objCountLast;
		triCount = triCountLast;
		faceCount = faceCountLast;
	}
};

struct VectFeature : public MeshFeature
{
  Vector *     vects;

  VectFeature()
  {
 	  vects = new Vector[MAXREADVERTS];
  }
  ~VectFeature()
  {
    delete [] vects;
  }
};

struct ColorFeature : public MeshFeature
{
	Color *     colors;

  ColorFeature()
  {
	  colors = new Color[MAXREADVERTS];
  }
  ~ColorFeature()
  {
    delete [] colors;
  }
};

struct TextFeature : public MeshFeature
{
	UVPair *    uvs;

  TextFeature()
  {
	  uvs = new UVPair[MAXREADVERTS];
  }
  ~TextFeature()
  {
    delete [] uvs;
  }
};

#define MAXOVERLAYS 1

struct MatFeature : public MeshFeature
{
  Material::Wrap *  materials[MAXMATERIALS + 1];
	Bitmap *          textures[MAXMATERIALS + 1][MAXOVERLAYS + 2];

	MatFeature()
	{

		textures[0][0]  = NULL;
  }

};
//----------------------------------------------------------------------------

static Bool sFlipZ = TRUE;
static Bool sFlipX = TRUE;
static Bool sIsXSI = FALSE;
static F32  sScaleFactor;
static char *sCurrFileName = NULL;
static char *istreambuf = NULL;
static sMatList[MAXMATERIALS + 1];

const char *XFILEVERSION    = "xof 0302txt ";
const U32 XFILEVERSIONLEN   = 12;
const char *XSIFILEVERSION_0  = "xsi 0100txt ";
const char *XSIFILEVERSION_1  = "xsi 0101txt ";
const U32 XSIFILEVERSIONLEN = 12;
//----------------------------------------------------------------------------


static istrstream *Open(const char *fileName, XFileToken &token) 
{
	// open the model file
	FileSys::DataFile *file = FileSys::Open(fileName);

	if (!file) 
	{
		//LOG_ERR(("MeshMan::Open: can't open file %s", fileName));
		return NULL;
	}
	
	// get the file size
	U32 size = file->Size();

	// allocate a string buffer
	istreambuf = new char [size+1];
	file->Read(istreambuf, size);
	istreambuf[size] = '\0';

  // finished with the file
  FileSys::Close( file);

	// create a string stream
	istrstream *iss = new istrstream(istreambuf, size+1);

	// record file name
	sCurrFileName = (char *)fileName;
	
	// read the version string
	char buffer[MAX_BUFFLEN];
	iss->read(buffer, XFILEVERSIONLEN); 
	buffer[XFILEVERSIONLEN] = '\0';

	// check the version string
	sIsXSI = FALSE;
	if (strnicmp(buffer, XFILEVERSION,   XFILEVERSIONLEN))
	{
		sIsXSI = TRUE;
		
		if (strnicmp(buffer, XSIFILEVERSION_0, XSIFILEVERSIONLEN) &&
			strnicmp(buffer, XSIFILEVERSION_1, XSIFILEVERSIONLEN))
		{
      delete istreambuf;
			delete iss;
			LOG_ERR(("MeshMan::Open: bad xfile %s", fileName));
			return NULL;
		}
	}
	
	// skip template section
	token = XFileLex.GetToken(iss);
	while (token == _TEMPLATE)
	{
		while (token != _CLOSE_BRACE)
		{
			token = XFileLex.GetToken(iss);
		}
		token = XFileLex.GetToken(iss);
	}
	
	// skip anything before real data
	do
	{
		if (token == _XSICOORDINATESYSTEM)
		{
			token = XFileLex.GetToken(iss);
			char c;
			S32 i[6];
			*iss >> i[0] >> c >> i[1] >> c >> i[2] >> c >> i[3] >> c >> i[4] >> c >> i[5] >> c;
			sFlipZ = i[0] == 1 || i[5] == 5 ? FALSE : TRUE;
			sFlipX = i[3] == 0 ? TRUE : FALSE;
		}
		token = XFileLex.GetToken(iss);
		
		if (token == _END_OF_FILE)
		{
      delete istreambuf;
      delete iss;
			LOG_ERR(("MeshMan::Open: no geometry in %s", fileName));
			return NULL;
		}
		
	} while (token != _MATERIAL && token != _FRAME);
	
	return iss;
}
//----------------------------------------------------------------------------

static Material::Wrap * ReadMaterial( istrstream *iss, Bitmap **texture, Bool readXSI)
{
	readXSI;
	
	char buffer[MAX_BUFFLEN];
	// get the material name
	char *matName = buffer;
	*matName = '\0';
	XFileLex.GetString(iss, matName);
	
	// open brace
	XFileToken token = XFileLex.GetToken(iss);
	
	// read the material's attributes
	ColorF32 diffuse, specular, emissive, ambient;
  ColorF32 white( 1, 1, 1, 1);

	F32 power;
	char c;
	specular.a = 1.0f;
	emissive.a = 1.0f;
	ambient.r = ambient.g = ambient.b = 1.0f;
	ambient.a  = 1.0f;
	
	// handle comma or semicolon delimted values
	*iss >> diffuse.r >> c >> diffuse.g >> c >> diffuse.b >> c >> diffuse.a >> c >> c;
	*iss >> power >> c;
	*iss >> specular.r >> c >> specular.g >> c >> specular.b >> c >> c;
	*iss >> emissive.r >> c >> emissive.g >> c >> emissive.b >> c >> c;

  Bool teamColor = diffuse.r != diffuse.g 
                || diffuse.r != diffuse.b 
                || diffuse.g != diffuse.b ? TRUE : FALSE;

  Bool envMap    = specular.r != 0 || specular.g != 0 || specular.b != 0 ? TRUE : FALSE;
  Bool overlay = FALSE;
  U32  blend = RS_BLEND_DEF;

  if (Vid::Var::doGenericMat)
  {
    diffuse.r = diffuse.g = diffuse.b = Material::Manager::diffuseVal;
//    specular.r = specular.g = specular.b = 0.0f;
//    diffuse.a = 1.0f;
  }
  if (specular.r == 0.0f && specular.g == 0.0f && specular.b == 0.0f)
  {
    power = 0.0f;
  }
  if (emissive.r != 0 || emissive.g != 0 || emissive.b != 0)
  {
#if 0
    blend = RS_BLEND_DECAL;
#else
    if (emissive.r >= 3.0f)
    {
      blend = RS_BLEND_GLOW;

    }
    else if (emissive.r >= 2.0f)
    {
      blend = RS_BLEND_ADD;
    }
    else
    {
      blend = RS_BLEND_DECAL;
    }
#endif
  }

  // clear out 2 slots
  //
  *texture = NULL;
  *(texture + 1) = NULL;

	char texName[MAX_BUFFLEN];
	*texName = '\0';
	token = XFileLex.GetToken(iss);
	if (token == _TEXTUREFILENAME)
	{
		// open brace
		token = XFileLex.GetToken(iss);
		
		texName[0] = '\0';
    XFileLex.GetString( iss, texName);
/*
    char c;
    *iss >> c;
    iss->getline( texName, MAX_BUFFLEN, '\"');
*/	
		// name close brace
		token = XFileLex.GetToken(iss);
		
		// material close brace
		token = XFileLex.GetToken(iss);

    if (*texName)
    {
      *texture = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, texName, Vid::Var::mipCount);
      texture++;
    }
	}
  else if (token == _SI_TEXTURE2D)
  {
    U32 texCount = 0;
    do
    {
		  // open brace
		  token = XFileLex.GetToken(iss);
		  
      char tname[MAX_BUFFLEN];
		  tname[0] = '\0';
      XFileLex.GetString( iss, tname);

      if (texCount < MAXOVERLAYS + 1)
      {
        strcpy( texName, Utils::ChopPath( tname));
        char *dot = Utils::FindExt( tname);
        if (!dot)
        {
          strcat( texName, ".pic");
        }
        if (*texName)
        {
          *texture = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, texName, Vid::Var::mipCount, bitmapTEXTURE, texCount);

          if (!*texture)
          {
            WARN_CON_DIAG(("Can't find texture %s", texName));
          }

          if (texCount > 0)
          {
            if (!*texture)
            {
              WARN_CON_DIAG(("Missing overlay texture in %s", sCurrFileName));
            }
            overlay = TRUE;
          }

          texture++;
          texCount++;
        }
      }

      // texture2D close brace
		  token = XFileLex.GetToken(iss);
		  
		  // material close brace
		  token = XFileLex.GetToken(iss);

    } while (token == _SI_TEXTURE2D);
  }

  // terminating NULL
  //
  *texture = NULL;
  
	// find the material and set it up
  Material::Wrap * wrap = Material::Manager::FindCreateWrap( 
    diffuse, specular, power, white, white, 
    blend, teamColor, envMap, overlay);
	
	if (!wrap)
	{
		LOG_ERR(("MeshMan::ReadMaterial: out of memory reading %s", sCurrFileName));
		return NULL;
	}

	wrap->SetDiffuse(  diffuse.r, diffuse.g, diffuse.b, diffuse.a);
//	wrap->SetSpecular( specular.r, specular.g, specular.b, specular.a, power);
//	mat->DoSetEmissive(emissive.r, emissive.g, emissive.b, emissive.a);
  wrap->SetStatus( teamColor, envMap, overlay);
  wrap->SetBlendFlags( blend);

	return wrap;
}
//----------------------------------------------------------------------------

static void ReadFaces(istrstream *iss, MeshFeature & feature, Bool readTexXSI)
{
	char c;
	
	// read the face info
	U32 count;
	*iss >> count >> c;
	if (count + feature.triCount > MAXREADTRIS)
	{
		ERR_FATAL(("Too many tris in: %s", sCurrFileName));
	}
	U32 facecount = count;
	
	U32 i, j, fcount = feature.triCount;
	for (i = 0; i < facecount; i++)
	{
		U32 indexCount;
		U32 index[256];
		
		// read the index count
		if (readTexXSI)
		{
			*iss >> indexCount >> c;
		}
		*iss >> indexCount >> c;

    ASSERT( indexCount <= 256);

		if (indexCount > 0)
		{
			// if something is flipped...
			if (sFlipZ ^ sFlipX)
			{
				// read indices in reverse order
				for (j = 0; j < indexCount; j++)
				{
					*iss >> index[indexCount-j-1] >> c;
//					index[indexCount-j-1] += feature.objCount;
					index[indexCount-j-1] += feature.objCountLast;
				}
			}
			else
			{
				// read indices in forward order
				for (j = 0; j < indexCount; j++)
				{
					*iss >> index[j] >> c;
//					index[j] += feature.objCount;
					index[j] += feature.objCountLast;
				}
			}

			if (indexCount == 1)
			{
				// clone point into a triangle
				indexCount = 3;
				index[1] = index[2] = index[0];
			}
			else if (indexCount == 2)
			{
				// clone line into a triangle
				indexCount = 3;
				index[2] = index[1];
			}
			
			// save face index and triangle count
			feature.faceIndex[i + feature.faceCount] = (U16) fcount;
			feature.faceTris[ i + feature.faceCount] = (U16) (indexCount - 2);
			
      U32 icount = fcount * 3;
			for (j = 2; j < indexCount; j++, fcount++)
			{
				// add to the triangle count
				if (fcount > MAXREADTRIS)
				{
					ERR_FATAL(("Too many tris in: %s", sCurrFileName));
				}

				ASSERT( index[0] < feature.objCount && index[j-1] < feature.objCount && index[j] < feature.objCount);

				// add a new triangle
				feature.tris[icount++] = (U16) index[0];
				feature.tris[icount++] = (U16) index[j-1];
				feature.tris[icount++] = (U16) index[j];
			}
		}
		
		// get the comma
		*iss >> c;
	}

	feature.triCount  = fcount;
	feature.faceCount += facecount;
}
//----------------------------------------------------------------------------

static void ReadVertices( istrstream * iss, VectFeature & feature)
{
	char c;
	
	// open brace
	XFileLex.GetToken(iss);
	
	// read the vertCount
	U32 count;
	*iss >> count >> c;
	if (count + feature.objCount > MAXREADVERTS)
	{
		ERR_FATAL(("Too many verts in: %s", sCurrFileName));
	}
	
	Vector * vects = feature.vects;
	U32 i;
	for (i = 0; i < count; i ++)
	{
		U32 j = i + feature.objCount;
		
		*iss >> vects[j].x >> c >> vects[j].y >> c >> vects[j].z >> c >> c;
		
		if (sFlipZ)
		{
			vects[j].z *= -1.0f;
		}
		if (sFlipX)
		{
			vects[j].x *= -1.0f;
		}
	}
	feature.objCount += count;

	ReadFaces(iss, feature, FALSE);
}
//----------------------------------------------------------------------------

static void ReadSINormals( istrstream * iss, VectFeature & feature)
{
	char c;
	
	// open brace
	XFileLex.GetToken(iss);
	
	// read the vertCount
	U32 count;
	*iss >> count >> c;
	if (count + feature.objCount > MAXREADVERTS)
	{
		ERR_FATAL(("Too many verts in: %s", sCurrFileName));
	}
	
	Vector * vects = feature.vects;
	U32 i;
	for (i = 0; i < count; i++)
	{
		U32 j = i + feature.objCount;
		
		*iss >> vects[j].x >> c >> vects[j].y >> c >> vects[j].z >> c >> c;
		
		if (sFlipZ)
		{
			vects[j].z *= -1.0f;
		}
		if (sFlipX)
		{
			vects[j].x *= -1.0f;
		}
	}
	feature.objCount += count;

	ReadFaces(iss, feature, TRUE);
}
//----------------------------------------------------------------------------

static void ReadColors( istrstream * iss, ColorFeature & feature)
{
	char c;
	
	// open brace
	XFileLex.GetToken(iss);
	
	// read the vertCount
	U32 count;
	*iss >> count >> c;
	if (count + feature.objCount > MAXREADVERTS)
	{
		ERR_FATAL(("Too many verts in: %s", sCurrFileName));
	}
	
	Color *colors = feature.colors;
	U32 i;
	for (i = 0; i < count; i++)
	{
		U32 j = i + feature.objCount;

    ColorF32 color;
		*iss >> color.r >> c >> color.g >> c >> color.b >> c >> color.a >> c >> c;

    colors[j].Set( color.r, color.g, color.b, color.a);
	}
	feature.objCount += count;
  
  ReadFaces(iss, feature, TRUE);
}
//----------------------------------------------------------------------------

static void ReadTexCoords( istrstream * iss, TextFeature & feature, Bool readXSI)
{
	char c;
	
	// open brace
	XFileToken token = XFileLex.GetToken(iss);
	
	// read the vertCount
	U32 count;
	*iss >> count >> c;
	if (count + feature.objCount > MAXREADVERTS)
	{
		ERR_FATAL(("Too many verts in: %s", sCurrFileName));
	}
	
	UVPairS *uvs = feature.uvs;
	U32 i;
	
	if (readXSI)
	{
		F32 Max_V = 0.0f;
		for (i = 0; i < count; i ++)
		{
			U32 j = i + feature.objCount;
			
			*iss >> uvs[j].u >> c >> uvs[j].v >> c >> c;

			if (Max_V < uvs[j].v)
			{
				Max_V = uvs[j].v;
			}
		}

    // invert uvs for pics:  FIXME
    //    
		Max_V = (F32) ceil (Max_V);
		for (i = 0; i < count; i ++)
		{
			U32 j = i + feature.objCount;
			
			uvs[j].v = Max_V - uvs[j].v;
		}
	
  	feature.objCount += count;
  
		ReadFaces(iss, feature, TRUE);
	}
	else
	{
		for (i = 0; i < count; i ++)
		{
			U32 j = i + feature.objCount;
			
			*iss >> uvs[j].u >> c >> uvs[j].v >> c >> c;
		}
  	feature.objCount += count;
	}
	// open brace
	token = XFileLex.GetToken(iss);
}
//----------------------------------------------------------------------------

static void ReadMaterialList( istrstream * iss, MatFeature & feature)
{
	char c;
	
	// open brace
	XFileToken token = XFileLex.GetToken(iss);
	
	U32 matCount, faceCount;
	*iss >> matCount >> c;
	*iss >> faceCount >> c;
	
	ASSERT(feature.objCount + matCount < MAXMATERIALS);
	
	U32 i, index;
	for (i = 0; i < faceCount; i++)
	{
		*iss >> index >> c;
		
		index += feature.objCount;
		feature.tris[(i + feature.triCount) * 3] = (U16) index;
	}
	// get the last semicolon   JDC!!!
	//*iss >> c;

	// read the _MATERIAL token or open brace
	token = XFileLex.GetToken(iss);
	if (token == _MATERIAL || token == _XSIMATERIAL)
	{
    LOG_DIAG((""));

		for (i = 0; i < matCount; i++)
		{
			U32 count = i + feature.objCount;
			feature.materials[count] = ReadMaterial(iss, &(feature.textures[count][0]), sIsXSI);

#if 0
      LOG_DIAG(("%d: tex0 : %s  tex1 : %s", count, 
        feature.textures[count][0] ? feature.textures[count][0]->GetName() : "null",
        feature.textures[count][1] ? feature.textures[count][1]->GetName() : "null"
        ));
#endif

			// read the next _MATERIAL token or close brace
			token = XFileLex.GetToken(iss);
		}    
	}
	else
	{
		for (i = 0; i < matCount; i++)
		{
			U32 count = i + feature.objCount;
			
			// read the material name
		  NameString buffer;
			XFileLex.GetString(iss, buffer.str);
			
			feature.materials[count] = NULL;  // Material::Manager::FindCreateWrap( buffer.str);  FIXME
			feature.textures[count][0] = NULL;
			feature.textures[count][1] = NULL;
			if (feature.materials[count])
			{
//				feature.textures[count][0] = feature.materials[count]->GetTexture();    FIXME
			}
			// read the close brace
			token = XFileLex.GetToken(iss);
			
			// read the next open brace
			token = XFileLex.GetToken(iss);
		}
	}
	
	feature.triCount += faceCount;
	feature.objCount += matCount;
}
//----------------------------------------------------------------------------
static MeshRoot *sRoot;
static VectFeature *vertfeat, *normfeat;
static TextFeature *textfeat;
static ColorFeature *colorfeat;
static MatFeature *matfeat;
static VertIndex *verttostate;
static U16 *texttostate;
static U16 *meshvertindex;
static U32 meshStateCount;
static Matrix *stateMatrix;

static Bool ReadMesh(istrstream *iss, XFileToken &token, Mesh *mesh)
{
	vertfeat->SaveCounts();
	normfeat->SaveCounts();
	textfeat->SaveCounts();
	colorfeat->SaveCounts();
	matfeat->SaveCounts();
	
	// get the mesh name
	char buffer[MAX_BUFFLEN];
	XFileLex.GetString(iss, buffer);
	
	Bool ishardpoint   = !strnicmp(buffer, "hp-", 3) || !strnicmp(buffer, "op-", 3) ? TRUE : FALSE;
	Bool isShadowPlane = !strnicmp(buffer, "sp-", 3) ? TRUE : FALSE;	

  if (mesh && !strnicmp(buffer, "tread", 5))
  {
    mesh->isTread = TRUE;
    sRoot->hasTread = TRUE;
  }

  if (mesh && !strnicmp(buffer, "cp-", 3))
  {
    mesh->isControl = TRUE;
    sRoot->hasControl = TRUE;
  }

  if (ishardpoint && mesh && mesh->Parent() && mesh->Parent()->Parent())
  {
//    LOG_CON_DIAG( ("Hardpoint %s below 2nd generation in %s", buffer, sCurrFileName) );
  }

	ReadVertices(iss, *vertfeat);
    
	while((token = XFileLex.GetToken(iss)) != _CLOSE_BRACE)
	{
		switch (token)
		{
		case _MESHMATERIALLIST:
			ReadMaterialList(iss, *matfeat);
			break;
		case _MESHNORMALS:
			ReadVertices(iss, *normfeat);
			// close brace
			token = XFileLex.GetToken(iss);
			break;
			
		case _SI_MESHNORMALS:
			ReadSINormals(iss, *normfeat);
			// close brace
			token = XFileLex.GetToken(iss);
			break;
			
		case _MESHTEXTURECOORDS:
			ReadTexCoords(iss, *textfeat, FALSE);
			textfeat->triCount = vertfeat->triCount;
			memcpy(textfeat->tris, vertfeat->tris, vertfeat->triCount * 3 * sizeof(U16));
			break;
		case _XSITEXTURECOORDS:
			ReadTexCoords(iss, *textfeat, TRUE);
			//token = XFileLex.GetToken(iss);
      break;
		case _XSIMESHCOLORS:
			ReadColors(iss, *colorfeat);
			token = XFileLex.GetToken(iss);
      break;
		}
	}
	// check for a null object (all verts equal)
	Bool isnull = TRUE;
	for (U32 i = vertfeat->objCountLast + 1; i < vertfeat->objCount; i++)
	{
		if (vertfeat->vects[i] != vertfeat->vects[vertfeat->objCountLast])
		{
			isnull = FALSE;
			break;
		}
	}
  if (ishardpoint || isnull)
	{
		// not real geometry
		vertfeat->RestoreCounts();
		normfeat->RestoreCounts();
		textfeat->RestoreCounts();
		colorfeat->RestoreCounts();
		matfeat->RestoreCounts();
		return TRUE;
	}

  for (i = vertfeat->objCountLast; i < vertfeat->objCount; i++)
	{
		vertfeat->vects[i] *= sScaleFactor;
		verttostate[i].count = 1;
		verttostate[i].index[0] = (U16) meshStateCount;
		verttostate[i].weight[0] = 1.0f;
	}

  for (i = textfeat->objCountLast; i < textfeat->objCount; i++)
	{
		texttostate[i] = (U16) meshStateCount;
	}

//  Bool local->geometry = (mesh && mesh->RenderFlags() & RS_HIDDEN) || isShadowPlane;

	// force identical tricounts
	if (normfeat->triCount < vertfeat->triCount)
	{
		for (i = normfeat->triCount; i < vertfeat->triCount; i++)
		{
			normfeat->tris[i * 3 + 0] = normfeat->tris[i * 3 + 1] = normfeat->tris[i * 3 + 2] = 0;
		}
		normfeat->triCount = vertfeat->triCount;
	}
	if (textfeat->triCount < vertfeat->triCount)
	{
		for (i = textfeat->triCount; i < vertfeat->triCount; i++)
		{
			textfeat->tris[i * 3 + 0] = textfeat->tris[i * 3 + 1] = textfeat->tris[i * 3 + 2] = 0;
		}
		textfeat->triCount = vertfeat->triCount;
	}
//	if (!local->geometry && colorfeat->objCount && colorfeat->triCount < vertfeat->triCount)  FIXME
  if (colorfeat->objCount && colorfeat->triCount < vertfeat->triCount)
	{
		colorfeat->colors[colorfeat->objCount] = 0xFFFFFFFF;
		for (i = colorfeat->triCount; i < vertfeat->triCount; i++)
		{
			colorfeat->tris[i * 3 + 1] = colorfeat->tris[i * 3 + 1] = colorfeat->tris[i * 3 + 2] = 
				(U16)colorfeat->objCount;
		}
		colorfeat->objCount++;
		colorfeat->triCount = vertfeat->triCount;
	}
  if (isShadowPlane)
  {
  	sRoot->shadowPlane = TRUE;
  }

  if (mesh && ((mesh->RenderFlags() & RS_HIDDEN) || isShadowPlane))
  {
    Bool hasColors = colorfeat->objCount != colorfeat->objCountLast;

	  // build a vertex list
    Vertex * vertices = new Vertex[ MAXREADINDICES];
    Color  * colors   = new Color[  MAXREADINDICES];
    U16    * indices  = new U16[    MAXREADINDICES];

    U32 icount, lasti = vertfeat->triCountLast * 3, endi = vertfeat->triCount * 3;
	  for (icount = 0; lasti < endi; icount++, lasti++)
	  {
			Vertex & v = vertices[icount];

      ASSERT( vertfeat->tris[lasti] < vertfeat->objCount);
			v.vv = vertfeat->vects[vertfeat->tris[lasti]];

      ASSERT( normfeat->tris[lasti] < normfeat->objCount);
			if (isShadowPlane)
      {
        v.nv.ClearData();
  		  v.u = v.v = 0.0f;
      }
      else
      { 
        v.nv = normfeat->vects[normfeat->tris[lasti]];

        if (textfeat->objCount)
			  {
          ASSERT( textfeat->tris[lasti] < textfeat->objCount);
				  v.u  = textfeat->uvs[textfeat->tris[lasti]].u;
				  v.v  = textfeat->uvs[textfeat->tris[lasti]].v;
			  }
			  else
			  {
				  v.u = v.v = 0.0f;
			  }
      }
			if (hasColors)
			{
        ASSERT( colorfeat->tris[lasti] < colorfeat->objCount);
				colors[icount] = colorfeat->colors[colorfeat->tris[lasti]];
			}
		}

	  // remove all redundancy in geometry
    Utils::Memset(indices, 0xff, icount << 1);
	  U32 vertexCount = 0;
	  for (i = 0; i < icount; i++)
	  {
		  if (indices[i] == 0xffff)
		  {
			  for (U32 j = i + 1; j < icount; j++)
			  {
				  Bool test = 
            (fabs (vertices[i].vv.x - vertices[j].vv.x) < Vid::Var::vertexThresh) &&
					  (fabs (vertices[i].vv.y - vertices[j].vv.y) < Vid::Var::vertexThresh) &&
					  (fabs (vertices[i].vv.z - vertices[j].vv.z) < Vid::Var::vertexThresh) &&
					  (fabs (vertices[i].nv.x - vertices[j].nv.x) < Vid::Var::normalThresh) &&
					  (fabs (vertices[i].nv.y - vertices[j].nv.y) < Vid::Var::normalThresh) &&
					  (fabs (vertices[i].nv.z - vertices[j].nv.z) < Vid::Var::normalThresh) &&
					  (fabs (vertices[i].u - vertices[j].u)       < Vid::Var::tcoordThresh) &&
            (fabs (vertices[i].v - vertices[j].v)       < Vid::Var::tcoordThresh) &&
            (!hasColors || isShadowPlane || colors[i] == colors[j]);

				  if (test)
				  {
					  indices[j] = (U16) vertexCount;
				  }
			  }
			  vertices[vertexCount] = vertices[i];
			  if (hasColors)
			  {
				  colors[vertexCount] = colors[i];
			  }
        indices[i] = (U16) vertexCount;
			  vertexCount++;
		  }
	  }
	  if (vertexCount < 3)
	  {
		  // not real geometry
		  vertfeat->RestoreCounts();
		  normfeat->RestoreCounts();
		  textfeat->RestoreCounts();
		  colorfeat->RestoreCounts();
		  matfeat->RestoreCounts();

      delete [] vertices;
      delete [] colors;
      delete [] indices;

		  return TRUE;
	  }

    mesh->isShadowPlane = isShadowPlane;
    if (isShadowPlane)
    {
      mesh->AllocLocal( vertexCount, icount, 0, 0, hasColors ? vertexCount : 0);
    }
    else
    {
      mesh->AllocLocal( vertexCount, icount, vertexCount, vertexCount, hasColors ? vertexCount : 0);
    }
	  for (i = 0; i < mesh->local->indices.count; i++)
	  {
		  U32 vIndex = indices[i];

      mesh->local->indices[i] = (U16) vIndex;
		  mesh->local->vertices[vIndex]  = vertices[vIndex].vv;
      if (hasColors)
      {
        mesh->local->colors[vIndex]  = colors[vIndex];
      }
      if (!isShadowPlane)
      {
    	  mesh->local->normals[vIndex] = vertices[vIndex].nv;
		    mesh->local->uvs[vIndex]     = vertices[vIndex].uv;
      }
	  }
	  // one material per mesh for now
	  mesh->local->groups.Alloc(1);
	  mesh->local->groups[0].vertCount  = (U16) mesh->local->vertices.count;
	  mesh->local->groups[0].indexCount = (U16) mesh->local->indices.count;
	  mesh->local->groups[0].planeIndex = (U16) 0;
	  mesh->local->groups[0].stateIndex = (U16) meshStateCount;

    Material::Wrap * wrap = matfeat->materials[matfeat->tris[matfeat->triCountLast * 3 + 0]];
	  mesh->local->groups[0].material = wrap->material;
	  mesh->local->groups[0].texture  = matfeat->textures[ matfeat->tris[matfeat->triCountLast * 3 + 0]][0];

    vertfeat->RestoreCounts();
    normfeat->RestoreCounts();
    textfeat->RestoreCounts();
    colorfeat->RestoreCounts();
    matfeat->RestoreCounts();

    delete [] vertices;
    delete [] colors;
    delete [] indices;

    if (isShadowPlane)
    {
      U32 vCount = mesh->local->vertices.count;
      U32 cCount = mesh->local->colors.count;

      // validate the shadow plane 
      //
      if (vCount < 4)
      {
        WARN_CON_DIAG( ("Shadow layer %s : Invalid vert count (%d) in %s", 
          buffer, vCount, sCurrFileName));
      }

      // If there are any colors, must be same number as verts
      if (cCount && cCount != vCount)
      {
        WARN_CON_DIAG( ("Shadow layer %s: Invalid color count (%d/%d) in %s", 
          buffer, vCount, cCount, sCurrFileName));
      }

      Vector * verts = &mesh->local->vertices[0];

      // Set initial min and max
      Vector min = verts[0];
      Vector max = verts[0];

      // Do a first pass to find extents
      for (U32 i = 0; i < vCount; i++)
      {
        // Get this vertex
        const Vector &v = verts[i];

        // Is this the most negative vert
        if (v.x <= min.x && v.z <= min.z)
        {
          min = v;
        }
      
        // Is this the most positive vert
        if (v.x >= max.x && v.z >= max.z)
        {
          max = v;
        }

#if 1
        S32 modx = Utils::FtoL( v.x);
        S32 modz = Utils::FtoL( v.z);
        if ((F32) modx != v.x || (F32) modz != v.z)
        {
          WARN_CON_DIAG( ("Shadow layer %s : Invalid vert pos (%f,%f,%f) in %s", 
            buffer, v.x, v.y, v.z, sCurrFileName));
        }
#endif

      }

      // Get the difference between max and min verts
      Vector delta = max - min;

      // Save sizes in verts
      Point<S32> size( 
        (S32)((delta.x + 4.0f) / 8.0f) + 1, 
        (S32)((delta.z + 4.0f) / 8.0f) + 1);

      // Ensure at least one cell wide
      if (size.x < 2 || size.z < 2)
      {
        WARN_CON_DIAG( ("Shadow layer %s : Invalid extents (%d, %d) in %s", 
          buffer, size.x, size.z, sCurrFileName));
      }
    }
  }
  else if (mesh)
  {
    mesh->isNull = FALSE;
  }
  
	// vertCount sanity checks
	if (vertfeat->objCount > normfeat->objCount)
	{
		ERR_FATAL(("vertfeat->objCount > normfeat->objCount: %s", sCurrFileName));
	}
	
	// triCount sanity checks
	if (vertfeat->triCount != normfeat->triCount)
	{
		ERR_FATAL(("vertfeat->faceCount != normfeat->triCount: %s", sCurrFileName));
	}
	
	meshvertindex[meshStateCount] = (U16) vertfeat->objCountLast;
	
	return TRUE;
}
//----------------------------------------------------------------------------

static void ReadMatrix(istrstream *iss, Matrix &m)
{
	// open brace
	XFileToken token = XFileLex.GetToken(iss);
	
	// read values
	char c;
	*iss >> m.right.x >> c >> m.right.y >> c >> m.right.z >> c >> m.rightw >> c;
	*iss >> m.up.x    >> c >> m.up.y    >> c >> m.up.z    >> c >> m.upw    >> c;
	*iss >> m.front.x >> c >> m.front.y >> c >> m.front.z >> c >> m.frontw >> c;
	*iss >> m.posit.x >> c >> m.posit.y >> c >> m.posit.z >> c >> m.positw >> c >> c;

  // ignore tiny offsets
  if (fabs(m.posit.x) < 0.01)
  {
    m.posit.x = 0.0f;
  }
  if (fabs(m.posit.y) < 0.01)
  {
    m.posit.y = 0.0f;
  }
  if (fabs(m.posit.z) < 0.01)
  {
    m.posit.z = 0.0f;
  }

	// flip z if necessary
	if (sFlipZ)
	{
		Quaternion q;
		q.Set(m);
		q.v.x *= -1.0f;
		q.v.y *= -1.0f;
		m.Set(q);
		m.posit.z *= -1.0f;
	}
	
	// flip x if necessary
	if (sFlipX)
	{
		Quaternion q;
		q.Set(m);
		q.v.y *= -1.0f;
		q.v.z *= -1.0f;
		m.Set(q);
		m.posit.x *= -1.0f;
	}
	
	// apply scale factor
	m.posit *= sScaleFactor;
	
	// close brace
	token = XFileLex.GetToken(iss);
}
//----------------------------------------------------------------------------

static Bool ReadMeshGroup(istrstream *iss, XFileToken &token, Mesh *mesh, FamilyState *meshStates, Mesh *parent, Bool isPrimary = TRUE) 
{
	char frameName[MAX_BUFFLEN];
	char *namep = frameName;
	
	// get the frame name
	XFileLex.GetString(iss, namep);
	
	// strip off the "frm-" that the exporter prepends to the name
	if (!strnicmp("frm-", namep, 4))
	{
		namep += 4;
	}
	
	// open brace
	token = XFileLex.GetToken(iss);

  Matrix matrix;

	// _TRANSFORM keyword
	token = XFileLex.GetToken(iss);
	ReadMatrix(iss, matrix);
  if (mesh)
  {
    mesh->SetState( namep, meshStates[meshStateCount], meshStateCount);
    mesh->ClearState();
    mesh->SetObjectMatrix( matrix);
    if (parent)
    {
      parent->Attach( *mesh, isPrimary);
    }
  }
	stateMatrix[meshStateCount] = matrix;
	
	token = XFileLex.GetToken(iss);
	if (token == _SI_BASEPOSEMATRIX)
	{
    if (mesh)
    {
  		ReadMatrix(iss, matrix);
      mesh->SetObjectMatrix( matrix);
    }
    else
    {
      Matrix dummyMatrix;
  		ReadMatrix(iss, dummyMatrix);
    }
		token = XFileLex.GetToken(iss);
	}
	
//	meshStates[meshStateCount].quaternion.Set(matrix);
//	meshStates[meshStateCount].position = matrix.posit;
	
	if (token == _MESH)
	{
		ReadMesh(iss, token, mesh);
		token = XFileLex.GetToken(iss);  
	}
    
	// increment the state count for children
	meshStateCount++;
	
	if (token == _FRAME)
	{
    // child frame
    Mesh *newMesh = NULL;
    if (mesh)
    {
      newMesh = new Mesh;
    }
    ReadMeshGroup(iss, token, newMesh, meshStates, mesh, TRUE);
	}
	
	if (token == _CLOSE_BRACE)
	{
		token = XFileLex.GetToken(iss);  
		if (token == _FRAME)
		{
      // sibling frame
  	  Mesh *newMesh = NULL;
      Mesh *curPar  = NULL;
      if (mesh)
      {
        if (!mesh->Parent())
        {
          ERR_FATAL( ("sibling mesh without parent in %s!", sCurrFileName) );
        }
        // sibling attach
        newMesh = new Mesh;
        curPar  = (Mesh *) mesh->Parent();
      }
			ReadMeshGroup(iss, token, newMesh, meshStates, curPar, FALSE);
		}
	}
	return TRUE;
}
//----------------------------------------------------------------------------

static U32 ReadRotations(istrstream *iss, AnimKey *keys, U32 &count)
{
	char c;
	*iss >> count >> c;
	if (count > MAXKEYFRAMES)
	{
		ERR_FATAL(("Too many rotation keys in anim: %s", sCurrFileName));
	}
	
	U32 i, datacount;
  S32 frame;
	for (i = 0; i < count; i++)
	{
		// clear out the key
		Utils::Memset( &keys[i], 0, sizeof( AnimKey));
		
		// read key data
		*iss >>
			frame >> c >>
			datacount >> c >>
			keys[i].quaternion.s >> c >>
			keys[i].quaternion.v.x >> c >>
			keys[i].quaternion.v.y >> c >>
			keys[i].quaternion.v.z >> c >> c >>
			c;
		
		// flip z if necessary
		if (sFlipZ)
		{
			keys[i].quaternion.v.x *= -1.0f;
			keys[i].quaternion.v.y *= -1.0f;
		}
		
		// flip x if necessary
		if (sFlipX)
		{
			keys[i].quaternion.v.y *= -1.0f;
			keys[i].quaternion.v.z *= -1.0f;
		}
		
		// record animation type
		keys[i].type = animQUATERNION;
		
		// record animation frame
		keys[i].frame = (F32) frame;
		
		// force rotations beyond 180 degrees to take the correct anim path
		if (i > 0)
		{
			F32 dot = keys[i - 1].quaternion.Dot(keys[i].quaternion);
			if (dot < 0)
			{
				keys[i].quaternion *= -1.0f;
			}
		}
	}
	
	return count;
}
//----------------------------------------------------------------------------

static U32 ReadPositions(istrstream *iss, AnimKey *keys, U32 &count)
{
	char c;
	*iss >> count >> c;
	if (count > MAXKEYFRAMES)
	{
		ERR_FATAL(("Too many position keys in anim: %s", sCurrFileName));
	}
	
	U32 i, datacount;
  S32 frame;
	for (i = 0; i < count; i++)
	{
		// clear out the key
		Utils::Memset( &keys[i], 0, sizeof( AnimKey));
		
		// read key data
		*iss >>
			frame >> c >>
			datacount >> c >>
			keys[i].position.x >> c >>
			keys[i].position.y >> c >>
			keys[i].position.z >> c >> c >>
			c;

    // ignore tiny offsets
    if (fabs(keys[i].position.x) < 0.01)
    {
      keys[i].position.x = 0.0f;
    }
    if (fabs(keys[i].position.y) < 0.01)
    {
      keys[i].position.y = 0.0f;
    }
    if (fabs(keys[i].position.z) < 0.01)
    {
      keys[i].position.z = 0.0f;
    }
    
		// flip z if necessary
		if (sFlipZ)
		{
			keys[i].position.z *= -1.0f;
		}
		
		// flip x if necessary
		if (sFlipX)
		{
			keys[i].position.x *= -1.0f;
		}
		
		// apply scale factor
		keys[i].position *= sScaleFactor;
		
		// record animation type
		keys[i].type = animPOSITION;
		
		// record animation number
		keys[i].frame = (F32) frame;
	}
	
	return count;
}
//----------------------------------------------------------------------------

static U32 ReadScales(istrstream *iss, AnimKey *keys, U32 &count)
{
	char c;
	*iss >> count >> c;
	if (count > MAXKEYFRAMES)
	{
		ERR_FATAL(("Too many scale keys in anim: %s", sCurrFileName));
	}
	
	U32 i, datacount;
  S32 frame;
	for (i = 0; i < count; i++)
	{
		// clear out the key
		Utils::Memset( &keys[i], 0, sizeof( AnimKey));
    keys[i].scale.Set( 1.0f, 1.0f, 1.0f);
    
		// read key data
		*iss >>
			frame >> c >>
			datacount >> c >>
			keys[i].scale.x >> c >>
			keys[i].scale.y >> c >>
			keys[i].scale.z >> c >> c >>
			c;
		
		// record animation type
		keys[i].type = animSCALE;
		
		// record animation frame
		keys[i].frame = (F32) frame;
	}
	
	return count;
}
//----------------------------------------------------------------------------

static Bool ReadAnimList(istrstream *iss, AnimList &animList, MeshRoot &root)
{
	// _ANIMATIONKEY 
	XFileToken token = XFileLex.GetToken(iss);
	if (token != _ANIMATION)
	{
		return FALSE;
	}
	
	AnimKey posKeys[MAXKEYFRAMES];
	AnimKey rotKeys[MAXKEYFRAMES];
	AnimKey scaleKeys[MAXKEYFRAMES];
  U32 posCount, rotCount, scaleCount;

	Animation *anim = NULL;
	do
	{
		if (!anim)
		{
			anim = new Animation;
		}
		
		// reset counters
		posCount = rotCount = scaleCount = 0;
		
		// ignore anim name
		token = XFileLex.GetToken(iss);
		// open brace
		token = XFileLex.GetToken(iss);
		
		char c;
		char buffer[MAX_BUFFLEN];
		char *namep = buffer;
		// animation's frame name
		XFileLex.GetString(iss, namep);
		
		// strip off the "frm-" that the exporter prepends to the name
		if (!strnicmp("frm-", namep, 4))
		{
			namep += 4;
		}

#if 0
    // strip off any renderstate name stuff
	  char *s;
    for (s = namep; s[0] != '\0'; s++)
    {
      if (s[0] == '_' && s[1] == '_')
      {
        s[0] = '\0';
        break;
      }
    }
#endif

		// close brace of the frame name
		token = XFileLex.GetToken(iss);
		
		// _ANIMATIONKEY
		token = XFileLex.GetToken(iss);
		while (token == _ANIMATIONKEY)
		{
			token = XFileLex.GetToken(iss);
			U32 type;
			*iss >> type >> c;
			switch (type)
			{
			case 0:     // rotation (quaternion)
				ReadRotations(iss, rotKeys, rotCount);
				break;
				
			case 1:     // scale (vector)
				ReadScales(iss, scaleKeys, scaleCount);
				break;
				
			case 2:     // position (vector)
				ReadPositions(iss, posKeys, posCount);
				break;
			}
			// _ANIMATIONKEY close brace
			token = XFileLex.GetToken(iss);
			
			// next _ANIMATIONKEY  or _ANIMATION close brace
			token = XFileLex.GetToken(iss);
		}
		while (token == _SI_ANIMATIONPARAMKEY)
		{
			// open brace
			token = XFileLex.GetToken(iss);
			
			// open brace of name
			token = XFileLex.GetToken(iss);
			char buffer1[MAX_BUFFLEN];
			XFileLex.GetString(iss, buffer1);
			// close brace of name
			token = XFileLex.GetToken(iss);
			
			// close brace of _SI_ANIMATIONPARAMKEY
			token = XFileLex.GetToken(iss);
			
			// next _SI_ANIMATIONPARAMKEY  or _ANIMATION close brace
			token = XFileLex.GetToken(iss);
		}
		
		// setup anim to node index
		Mesh * meshnode = (Mesh *) root.FindMesh( namep);
		if (meshnode)
		{
      anim->index = meshnode->GetIndex();

//      LOG_DIAG( ("MeshRoot::FindRead: anim: %s", namep) );

			// setup node's matrix to reflect the first key
			//    AnimKey &state = mesh.states[anim->index];
			AnimKey &state = animList.states[anim->index];
			
			state.frame = 0.0f;
			state.quaternion.Set(meshnode->ObjectMatrix());
			state.position = meshnode->ObjectMatrix().Position();
//    state.quaternion.Set(stateMatrix[ident.index]);
//	state.position = stateMatrix[ident.index].Position();
			
			state.type = 0;
			if (rotCount)
			{
        state.type |= animQUATERNION;
				state.quaternion = rotKeys[0].quaternion;

        OptimizeKeys( rotKeys, rotCount, animQUATERNION);
			}
			
			if (posCount)
			{
        state.type |= animPOSITION;
				state.position = posKeys[0].position;

        OptimizeKeys( posKeys, posCount, animPOSITION);
			}
#define DOSCALEKEYS
#ifdef DOSCALEKEYS
			if (scaleCount)
      {
        OptimizeKeys( scaleKeys, scaleCount, animSCALE);
        
        if (scaleCount)
        {
          state.type |= animSCALE;
				  state.scale = scaleKeys[0].scale;

          // scale keys need a valid quat
          scaleKeys[0].quaternion = state.quaternion;
          scaleKeys[0].type |= animQUATERNION;
          ASSERT( !(state.quaternion.s == 0.0f
            && state.quaternion.v.x == 0.0f
            && state.quaternion.v.y == 0.0f
            && state.quaternion.v.y == 0.0f));
        }
      }
#endif
//			if (!mesh.animCycles.GetCount())
			{
				// if this is the default cycle then setup the node's stateArray
//				root.states[anim->index] = state;
				meshnode->SetObjectMatrix( state.quaternion, state.position);
			}
			
			// add the keys to the animation
			CombineKeys(rotKeys, rotCount, posKeys, posCount);
#ifdef DOSCALEKEYS
			CombineKeys(rotKeys, rotCount, scaleKeys, scaleCount);
#endif
			anim->SetKeys(rotKeys, rotCount);

      if ((rotKeys || rotCount) && !Utils::Strnicmp( meshnode->GetName(), "cp-", 3))
      {
			  LOG_CON_DIAG( ("Control point %s in %s has conflictin animation.", 
          meshnode->GetName(), sCurrFileName) );
      }
		}
		else
		{
			LOG_DIAG(("Can't find mesh for anim: %s", namep));
		}
		
		// add the anim to the list and setup for a new one only if it wasn't emtpy
		if (anim->maxFrame != 0)
		{
			animList.Append(anim);
			anim = NULL;
		}
		token = XFileLex.GetToken(iss);
		
	} while (token == _ANIMATION);
	
	if (anim && anim->maxFrame == 0)
	{
		// get rid of the last anim if it's empty
		delete anim;
	}
	// in the event that the animation set is a hierarchy
	// and not a list then the following line will stop
	ASSERT (token == _CLOSE_BRACE);
	
	return TRUE;
}
//----------------------------------------------------------------------------

static AnimList * ReadAnim(istrstream *iss, XFileToken &token, MeshRoot &mesh, Bool useEmpty = FALSE)
{
	while (token != _SI_ENVELOPELIST && token != _END_OF_FILE && token != _ANIMATIONSET)
	{
		token = XFileLex.GetToken(iss); 
	}
	if (token != _ANIMATIONSET && !useEmpty)
	{
		return NULL;
	}
	AnimList * animList = new AnimList;

	// create this cycle's initial state array
	animList->SetupStates( mesh.states);
	
	// open brace
	token = XFileLex.GetToken(iss);
	
  ReadAnimList(iss, *animList, mesh); 

	if ((animList->GetCount() == 0 && !useEmpty))
	{
		delete animList;
		animList = NULL;
	}
	else 
	{
		animList->SetupMaxFrame();
		
		if (animList->maxFrame == 0)
		{
			// if nobody has any animations get rid of the base group
			delete animList;
			animList = NULL;
		}
	}
	return animList;
}
//----------------------------------------------------------------------------
U32 oldVertexCount;

// large vertex index buffer
struct BigVertIndex
{
	U32 count;
	U16 index[16];
	F32 weight[16];
};

Bool ReadEnvelopes(istrstream *iss, XFileToken &token, MeshRoot &root)
{
//  root.LogHierarchy();

  char c;
	
	while (token != _END_OF_FILE && token != _SI_ENVELOPELIST)
	{
		token = XFileLex.GetToken(iss); 
	}
	if (token != _SI_ENVELOPELIST)
	{
		return FALSE;
	}
	
	U32 i, j;
  BigVertIndex * vertindex = new BigVertIndex[MAXREADVERTS];

	// copy the normal vertex index buffer
	for (i = 0; i < oldVertexCount; i++)
	{
		// copy the weight count
		vertindex[i].count = verttostate[i].count;

    ASSERT( vertindex[i].count < 22);

		// for each weight count...
		for (j = 0; j < verttostate[i].count; j++)
		{
			// copy the index and weight
			vertindex[i].index[j] = verttostate[i].index[j];
			vertindex[i].weight[j] = verttostate[i].weight[j];
		}
	}

	// open brace
	token = XFileLex.GetToken(iss);
	
	// get the number of envelopes
	U32 envCount;
	*iss >> envCount >> c;

	for (; envCount > 0; envCount--)
	{
		// _SI_ENVELOPE
		token = XFileLex.GetToken(iss);
		
		if (token != _SI_ENVELOPE)
		{
			ERR_FATAL(("Missing envelope in: %s", sCurrFileName));
			break;
		}
		// open brace
		token = XFileLex.GetToken(iss);
		
		char buffer1[MAX_BUFFLEN];
		char *name1 = buffer1;

		// animation's frame name
		XFileLex.GetString(iss, name1);
		
		// strip off the "frm-" that the exporter prepends to the name
		if (!strnicmp("frm-", name1, 4))
		{
			name1 += 4;
		}

		// get the semicolon
		*iss >> c;
		
		char buffer2[MAX_BUFFLEN];
		char *name2 = buffer2;

		// animation's frame name
		XFileLex.GetString(iss, name2);
		
		// strip off the "frm-" that the exporter prepends to the name
		if (!strnicmp("frm-", name2, 4))
		{
			name2 += 4;
		}

		// get the semicolon
		*iss >> c;
		
		const Mesh * meshnode1 = root.FindMesh( name1);
		if (!meshnode1)
		{
			ERR_FATAL(("Can't find mesh %s in: %s", name1, sCurrFileName));
		}

    // ignore envelope data for local meshes
    if (!meshnode1->local)
    {
		  const Mesh * meshnode2 = root.FindMesh( name2);
		  if (!meshnode2)
		  {
			  ERR_FATAL(("Can't find node %s in: %s", name2, sCurrFileName));
		  }
		  
		  // get the vertex count
		  U32 count;
		  *iss >> count >> c;
		  
		  for (; count > 0; count--)
		  {
			  U32 index;
			  F32 weight;
			  
			  *iss >> index >> c >> weight >> c >> c;
			  
			  // scale percentage to [0..1]
			  weight *= 0.01f;
			  
			  // add in offset to first vert index
        U32 nodeIndex = meshnode1->GetIndex();
        ASSERT( nodeIndex < meshStateCount);

			  index += meshvertindex[nodeIndex];
        ASSERT( index < oldVertexCount);

			  // get the weight count
			  U32 &wc = vertindex[index].count;
        ASSERT( wc < 22);

			  // subtract weight from skin space component
			  vertindex[index].weight[0] -= weight;

			  // add new weight component
			  vertindex[index].index[wc]  = (U16) meshnode2->GetIndex();
			  vertindex[index].weight[wc] = weight;

			  // increment the weight count
			  wc++;
		  }
    }		

		// close brace
		token = XFileLex.GetToken(iss);
	}
	
	// sort vertex weights for each vertex
	for (i = 0; i < oldVertexCount; i++)
	{
		// get the source weight list
		BigVertIndex &srcVI = vertindex[i];

		// get the destination weight list
		VertIndex &dstVI = verttostate[i];

		// limit the number of weights to the maximum
		dstVI.count = min(srcVI.count, MAXWEIGHTS);

		// start with extra weight of one
		F32 extra = 1.0f;

		// sort the weights in descending order
		for (j = 0; j < dstVI.count; j++)
		{
			// start with the jth weight in the list
			U32 max = j;

			// search the list for a larger weight
			for (U32 k = j+1; k < srcVI.count; k++)
			{
				if (srcVI.weight[max] < srcVI.weight[k])
				{
					max = k;
				}
			}

			// get the values of the maximum
			U16 index = srcVI.index[max];
			F32 weight = srcVI.weight[max];

			// if the maximum is out of place...
			if (max != j)
			{
				// shift intervening elements up by one
				memmove(&srcVI.index[j+1], &srcVI.index[j], (max-j)*sizeof(U16));
				memmove(&srcVI.weight[j+1], &srcVI.weight[j], (max-j)*sizeof(F32));

				// place the maximum in the correct place
				srcVI.index[j] = index;
				srcVI.weight[j] = weight;
			}

			// if the weight is small...
			if (weight < 0.01f)
			{
				// truncate the list
				dstVI.count = j;
				break;
			}

			// save the maximum in the output array
			dstVI.index[j] = index;
			dstVI.weight[j] = weight;

			// deduct weight from extra
			extra -= weight;
		}

		// divide extra by weight count
		extra /= dstVI.count;

		// distribute extra weight among weights
		for (j = 0; j < dstVI.count; j++)
		{
			dstVI.weight[j] += extra;
		}
	}

  delete [] vertindex;

	// close brace
	token = XFileLex.GetToken(iss);
	
	ASSERT(token == _CLOSE_BRACE);
	
	return TRUE;
}
//----------------------------------------------------------------------------

// compare function for sorting faces via qsort
//
static int _cdecl CompareFaces(const void *e1, const void *e2)
{
	FaceObj *f1 = (FaceObj *) e1;
	FaceObj *f2 = (FaceObj *) e2;
	
	// primary sort by state
	if (sRoot->vertToState[f1->verts[0]].index[0] > sRoot->vertToState[f2->verts[0]].index[0])
	{
		return -1;
	}
	if (sRoot->vertToState[f1->verts[0]].index[0] < sRoot->vertToState[f2->verts[0]].index[0])
	{
		return 1;
	}
	// secondary sort by material
	if (f1->buckyIndex > f2->buckyIndex)
	{
		return -1;
	}
	if (f1->buckyIndex < f2->buckyIndex)
	{
		return 1;
	}
	
	return 0;
}
//----------------------------------------------------------------------------

MeshRoot *Mesh::Manager::FindRead(const char *meshName, F32 scale, Bool mrmGen, const char *fileName) // = NULL
{
	if (!meshName)
	{
		// return the global null root
		return nullRoot;
	}

	// save changable mesh load control values
	F32  scal = Vid::Var::scaleFactor;
	Bool mgen = Vid::Var::doMrmGen;
	
	// set load controls
	Vid::Var::scaleFactor   = scale;
	Vid::Var::doMrmGen      = mrmGen;

  MeshRoot *root = NULL;
#if 1
  #pragma message("God file loading enabled")

  if (Vid::Var::doLoadGod)
  {
    root = FindLoadScale( meshName);
  }
#endif

  if (!root)
  {
  	// load the mesh
	  root = FindRead(meshName, fileName);
  }
	
	// reset control values
	Vid::Var::scaleFactor  = scal;
	Vid::Var::doMrmGen     = mgen;

	return root;
}
//----------------------------------------------------------------------------

MeshRoot *Mesh::Manager::FindRead(const char *meshName, const char *fileName) // = NULL
{
  readErrors = FALSE;

	// static module wide copies of load control values
	sScaleFactor = Vid::Var::scaleFactor;
	
	//  doOldPipe = TRUE;
	
	// local copies
	Bool domrmgen  = Vid::Var::doMrmGen;
	
	if (!meshName || !*meshName)
	{
		// return the global null root
		return nullRoot;
	}

  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;
  Dir::PathExpand( meshName, drive, dir, name, ext);

  BuffString buff;
  if (fileName)
  {
    FileName fname;
    Dir::PathExpand( fileName, drive, dir, fname, ext);
    buff = fname.str;
  }
  else
  {
    Mesh::Manager::MakeName( buff, name.str, Vid::Var::scaleFactor);
  }

  // check if it's already loaded
  MeshRoot *root = Find( name.str);
  if (root)
  {
    return root;
  }

  // check for scale version
  root = Find( buff.str);
  if (root)
  {
    return root;
  }
  
	// load it up
	sFlipZ = TRUE;
	sFlipX = FALSE;
	XFileToken token;
	istrstream *iss = Open(meshName, token);
	if (!iss) 
	{
		return NULL;
	}

	// setup temp mesh data pointers
	FamilyState meshStates[MAXREADMESH];
	meshStateCount = 0;
	
	Matrix statemats[MAXREADMESH];
	stateMatrix = statemats;
	
	U16 meshverts[MAXREADMESH];
	meshvertindex = meshverts;
  memset( meshvertindex, 0, MAXREADMESH * sizeof(U16));

	verttostate = new VertIndex[ MAXREADVERTS];
	texttostate = new U16[MAXREADVERTS];

	// clear out verttostate
	U32 i, j;
	for (i = 0; i < MAXREADVERTS; i++)
	{
		verttostate[i].count = 0;

    for (j = 0; j < MAXWEIGHTS; j++)
		{
			verttostate[i].index[j] = 0xffff;
			verttostate[i].weight[j] = j ? 0.0f : 1.0f;
		}
	}
	
  U16 *mapNewToOldIndex = new U16[ MAXREADINDICES];

	vertfeat  = new VectFeature;
	normfeat  = new VectFeature;
	textfeat  = new TextFeature;
	colorfeat = new ColorFeature;
	matfeat   = new MatFeature;

  Utils::Memset( matfeat->tris, 0xff, sizeof(matfeat->tris));
  Utils::Memset( matfeat->materials, 0, sizeof(matfeat->materials));
  Utils::Memset( matfeat->textures,  0, sizeof(matfeat->textures));

	// read materials
	while (token == _MATERIAL)
	{
		// materials
		matfeat->materials[matfeat->objCount] = ReadMaterial(iss, &matfeat->textures[matfeat->objCount][0], sIsXSI);
    matfeat->objCount++;
    
		// get the next token
		token = XFileLex.GetToken(iss);
	}
	
	root = new MeshRoot();
	if (!root)
	{
		ERR_FATAL(("Can't create meshroot in %s", sCurrFileName));
	}
  root->read = new MeshRead;  

  sRoot = root;
  root->xsiName.Set( name.str);
  root->fileName.Set( buff.str);
    
	// geometry
	switch (token)
	{
	case _MESH:
		ReadMesh(iss, token, root);
		break;
	case _FRAME:
		ReadMeshGroup(iss, token, root, meshStates, NULL);
		break;
	}
	
	// validate tri data
	ASSERT(normfeat->triCount  == vertfeat->triCount);
	ASSERT(textfeat->objCount  == 0 || textfeat->triCount  == vertfeat->triCount);
  ASSERT(colorfeat->objCount == 0 || colorfeat->triCount == vertfeat->triCount);

	// must setup mesh states before loading animation
	root->SetupStates( meshStates, meshStateCount, stateMatrix);

  root->states[0].SetObjectMatrix( Matrix::I.posit);

	// set world matrices relative to 0,0,0
	root->SetWorldAll(Matrix::I);

  Vector * vects = new Vector[MAXREADVERTS];

  // save baseMats for CompressStates later
  //
  root->read->baseMats.Alloc(meshStateCount);
    
	// create root space to state space transforms
  //
	for (i = 0; i < meshStateCount; i++)
	{
    root->states[i].SetState( root->states[i]);
    root->read->baseMats[i] = root->states[i].ObjectMatrixPriv();

		root->stateMats[i].SetInverse(root->states[i].WorldMatrix());
	}
	
	// transform verts into root space; setup index mapper
	for (i = 0; i < vertfeat->objCount; i++)
	{
    Vector v( vertfeat->vects[i]);
		root->states[verttostate[i].index[0]].WorldMatrix().Transform(v);
    vertfeat->vects[i] = v;
	}
	
	// rotate normals into root space
	for (i = 0; i < normfeat->triCount * 3; i++)
	{
		U32 index = normfeat->tris[i];
		root->states[verttostate[vertfeat->tris[i]].index[0]].WorldMatrix().Rotate(vects[index], normfeat->vects[index]);
	}

	// intialize index mapper
	for (i = 0; i < vertfeat->objCount; i++)
	{
		mapNewToOldIndex[i] = (U16) i;
	}
	
	oldVertexCount = vertfeat->objCount;
	
	// report unoptimized counts
	LOG_DIAG(("Mesh::Manager::FindRead: %s", buff.str));
	LOG_DIAG(("verts %d, norms %d, uvs %d, faces %d", 
		vertfeat->objCount,
		normfeat->objCount,
		textfeat->objCount,
		vertfeat->triCount));
	
  U16 *mapper = new U16[MAXREADVERTS];

  if (Vid::Var::doOptimize)
  {
	  // optimize the mesh (remove redundancy)
  
	  // optimize verts
	  memset(mapper, 0xff, vertfeat->objCount * sizeof(U16));
	  U32 count2, counter = 0;
	  Bool hit = FALSE;
	  for (i = 0; i < vertfeat->objCount; i++)
	  {
		  if (mapper[i] == 0xffff)
		  {
			  mapper[i] = (U16) i;
			  counter++;
			  
			  for (j = i + 1; j < vertfeat->objCount; j++)
			  {
				  if (verttostate[i].index[0] == verttostate[j].index[0] && 
             (fabs (vertfeat->vects[i].x - vertfeat->vects[j].x) <= Vid::Var::vertexThresh) &&
					   (fabs (vertfeat->vects[i].y - vertfeat->vects[j].y) <= Vid::Var::vertexThresh) &&
					   (fabs (vertfeat->vects[i].z - vertfeat->vects[j].z) <= Vid::Var::vertexThresh))
				  {
					  mapper[j] = (U16) i;
					  hit = TRUE;
				  }
			  }
		  }
	  }
	  count2 = 0;
	  if (hit)
	  {
		  for (i = 0; i < vertfeat->objCount; i++)
		  {
			  if (mapper[i] == i)
			  {
				  vertfeat->vects[count2] = vertfeat->vects[i];
				  mapper[i] = (U16) count2;
				  mapNewToOldIndex[count2] = (U16) i;
				  count2++;
			  }
			  else
			  {
				  mapper[i] = mapper[mapper[i]];
				  mapNewToOldIndex[mapper[i]] = (U16) i;
			  }
		  }
		  vertfeat->objCount = counter;
		  
		  for (i = 0; i < vertfeat->triCount * 3; i++)
		  {
				U32 index = vertfeat->tris[i];
				
				ASSERT(mapper[index] < counter);
				
				vertfeat->tris[i] = mapper[index];
		  }
	  }
	  
	  // optimize normals
	  memset(mapper, 0xff, normfeat->objCount * sizeof(U16));
	  counter = 0;
	  hit = FALSE;
	  for (i = 0; i < normfeat->objCount; i++)
	  {
		  if (mapper[i] == 0xffff)
		  {
			  mapper[i] = (U16) i;
			  counter++;
			  
			  for (j = i + 1; j < normfeat->objCount; j++)
			  {
          if ((fabs (vects[i].x - vects[j].x) <= Vid::Var::normalThresh) &&
 					    (fabs (vects[i].y - vects[j].y) <= Vid::Var::normalThresh) &&
 					    (fabs (vects[i].z - vects[j].z) <= Vid::Var::normalThresh))
          {
					  mapper[j] = (U16) i;
					  hit = TRUE;
				  }
			  }
		  }
	  }
	  count2 = 0;
	  if (hit)
	  {
		  for (i = 0; i < normfeat->objCount; i++)
		  {
			  if (mapper[i] == i)
			  {
				  normfeat->vects[count2] = vects[i];
				  mapper[i] = (U16) count2;
				  count2++;
			  }
			  else
			  {
				  mapper[i] = mapper[mapper[i]];
			  }
		  }
		  normfeat->objCount = counter;
		  
		  for (i = 0; i < normfeat->triCount * 3; i++)
		  {

				U32 index = normfeat->tris[i];
				
				ASSERT(mapper[index] < counter);
				
				normfeat->tris[i] = mapper[index];
		  }
	  }
	  
	  // optimize texture coords
	  memset(mapper, 0xff, textfeat->objCount * sizeof(U16));
	  counter = 0;
	  hit = FALSE;
	  for (i = 0; i < textfeat->objCount; i++)
	  {
		  if (mapper[i] == 0xffff)
		  {
			  mapper[i] = (U16) i;
			  counter++;
			  
			  for (j = i + 1; j < textfeat->objCount; j++)
			  {
          if ((fabs (textfeat->uvs[i].u - textfeat->uvs[j].u) <= Vid::Var::tcoordThresh)
					 && (fabs (textfeat->uvs[i].v - textfeat->uvs[j].v) <= Vid::Var::tcoordThresh)
           && (texttostate[i] == texttostate[j] || (!meshStates[texttostate[i]].IsTread() && !meshStates[texttostate[j]].IsTread()))
          )
				  {
					  mapper[j] = (U16) i;
					  hit = TRUE;
				  }
			  }
		  }
	  }
	  
	  count2 = 0;
	  if (hit)
	  {
		  for (i = 0; i < textfeat->objCount; i++)
		  {
			  if (mapper[i] == i)
			  {
				  textfeat->uvs[count2] = textfeat->uvs[i];
				  mapper[i] = (U16) count2;
				  count2++;
			  }
			  else
			  {
				  mapper[i] = mapper[mapper[i]];
			  }
		  }
		  textfeat->objCount = counter;
		  
		  for (i = 0; i < textfeat->triCount * 3; i++)
		  {
				U32 index = textfeat->tris[i];
				textfeat->tris[i] = mapper[index];
		  }
	  }

  #if 1
    // optimize colors
	  memset(mapper, 0xff, colorfeat->objCount * sizeof(U16));
	  counter = 0;
	  hit = FALSE;
	  for (i = 0; i < colorfeat->objCount; i++)
	  {
		  if (mapper[i] == 0xffff)
		  {
			  mapper[i] = (U16) i;
			  counter++;
			  
			  for (j = i + 1; j < colorfeat->objCount; j++)
			  {
				  if (colorfeat->colors[i] == colorfeat->colors[j])
				  {
					  mapper[j] = (U16) i;
					  hit = TRUE;
				  }
			  }
		  }
	  }
	  
	  count2 = 0;
	  if (hit)
	  {
		  for (i = 0; i < colorfeat->objCount; i++)
		  {
			  if (mapper[i] == i)
			  {
				  colorfeat->colors[count2] = colorfeat->colors[i];
				  mapper[i] = (U16) count2;
				  count2++;
			  }
			  else
			  {
				  mapper[i] = mapper[mapper[i]];
			  }
		  }
		  colorfeat->objCount = counter;
		  
		  for (i = 0; i < colorfeat->triCount * 3; i++)
		  {
				U32 index = colorfeat->tris[i];
				colorfeat->tris[i] = mapper[index];
		  }
	  }
  #endif

    delete vects;
  
#if 1
	  // optimize faces
	  hit = FALSE;
	  for (i = 0; i < vertfeat->triCount; i++)
	  {
#if 0
      if (vertfeat->vects[vertfeat->tris[i * 3 + 0]] == vertfeat->vects[vertfeat->tris[i * 3 + 1]] 
       && vertfeat->vects[vertfeat->tris[i * 3 + 0]] == vertfeat->vects[vertfeat->tris[i * 3 + 2]])
      {
			  Utils::Memcpy(  &vertfeat->tris[i * 3 + 0],  &vertfeat->tris[(i + 1) * 3 + 0],  (vertfeat->triCount - i)  * 3 * sizeof(U16));
			  Utils::Memcpy(  &normfeat->tris[i * 3 + 0],  &normfeat->tris[(i + 1) * 3 + 0],  (normfeat->triCount - i)  * 3 * sizeof(U16));
			  Utils::Memcpy(  &textfeat->tris[i * 3 + 0],  &textfeat->tris[(i + 1) * 3 + 0],  (textfeat->triCount - i)  * 3 * sizeof(U16));
			  Utils::Memcpy( &colorfeat->tris[i * 3 + 0], &colorfeat->tris[(i + 1) * 3 + 0], (colorfeat->triCount - i)  * 3 * sizeof(U16));
      
			  vertfeat->triCount--;
			  normfeat->triCount--;
			  textfeat->triCount--;
			  colorfeat->triCount--;

			  U32 k;
			  for (k = 0; k < matfeat->triCount; k++)
			  {
				  if (vertfeat->faceIndex[k] == i)
				  {
					  matfeat->triCount--;
					  k--;
				  }
				  if (vertfeat->faceIndex[k] >= i)
				  {
					  vertfeat->faceIndex[k]--;
				  }
			  }
			  
			  i--;
			  if (!hit)
			  {
				  hit = TRUE;
			  }
        continue;
      }
#endif
		  for (j = i + 1; j < vertfeat->triCount; j++)
		  {
			  if (vertfeat->tris[i * 3 + 0] == vertfeat->tris[j * 3 + 0]
			   && vertfeat->tris[i * 3 + 1] == vertfeat->tris[j * 3 + 1]
			   && vertfeat->tris[i * 3 + 2] == vertfeat->tris[j * 3 + 2]
				  
			   && normfeat->tris[i * 3 + 0] == normfeat->tris[j * 3 + 0]
			   && normfeat->tris[i * 3 + 1] == normfeat->tris[j * 3 + 1]
			   && normfeat->tris[i * 3 + 2] == normfeat->tris[j * 3 + 2]       
				  
			   && textfeat->tris[i * 3 + 0] == textfeat->tris[j * 3 + 0]
			   && textfeat->tris[i * 3 + 1] == textfeat->tris[j * 3 + 1]
			   && textfeat->tris[i * 3 + 2] == textfeat->tris[j * 3 + 2]
       
         && (!colorfeat->objCount
         || (colorfeat->tris[i * 3 + 0] == colorfeat->tris[j * 3 + 0]
			   &&  colorfeat->tris[i * 3 + 1] == colorfeat->tris[j * 3 + 1]
			   &&  colorfeat->tris[i * 3 + 2] == colorfeat->tris[j * 3 + 2])))
			  {
          if (!hit)
          {
            WARN_CON_DIAG( ("duplicate faces in %s", sCurrFileName) );
            hit = TRUE;
          }

#if 0
          U32 k;
          S32 mi = -1, mj = -1;
          for (k = 0; k < matfeat->triCount; k++)
          {
            if (vertfeat->faceIndex[k] == i)
            {
              mi = k;
            }
            if (vertfeat->faceIndex[k] == j)
            {
              mj = j;
            }
          }

				  Utils::Memcpy( &vertfeat->tris[j * 3 + 0],  &vertfeat->tris[(j + 1) * 3 + 0],  (vertfeat->triCount - j)  * 3 * sizeof(U16));
				  Utils::Memcpy( &normfeat->tris[j * 3 + 0],  &normfeat->tris[(j + 1) * 3 + 0],  (normfeat->triCount - j)  * 3 * sizeof(U16));
				  Utils::Memcpy( &textfeat->tris[j * 3 + 0],  &textfeat->tris[(j + 1) * 3 + 0],  (textfeat->triCount - j)  * 3 * sizeof(U16));

          if (colorfeat->triCount)
          {
  				  Utils::Memcpy( &colorfeat->tris[j * 3 + 0], &colorfeat->tris[(j + 1) * 3 + 0], (colorfeat->triCount - j) * 3 * sizeof(U16));
				    colorfeat->triCount--;
          }
        
				  vertfeat->triCount--;
				  normfeat->triCount--;
				  textfeat->triCount--;

				  for (k = 0; k < matfeat->triCount; k++)
				  {
					  if (vertfeat->faceIndex[k] == j)
					  {
						  matfeat->triCount--;
						  k--;
					  }
					  if (vertfeat->faceIndex[k] >= j)
					  {
						  vertfeat->faceIndex[k]--;
					  }
				  }
				  
				  j--;
#endif
			  }
		  }
	  }
#endif
  }	

	if (!Vid::Var::doBasePose)
	{
		// setup initial state matrices (previous matrices were for base pose)
    //
    Bool hit = FALSE;
		for (i = 1; i < meshStateCount; i++)
		{
      if (root->states[i].ObjectMatrixPriv() == stateMatrix[i])
      {
        hit = TRUE;
      }
			root->states[i].SetObjectMatrix( stateMatrix[i]);
		}
		root->hasAnim = hit;

		// set world matrices relative to 0,0,0
    //
		root->SetWorldAll(Matrix::I);
	}
	
	// allocate and initialize faces
	root->faces.Alloc(vertfeat->triCount);
	U32 k, buckycount = 0;
	for (i = 0; i < vertfeat->triCount; i++)
	{
		FaceObj &face = root->faces[i];
		face.buckyIndex = 0xffff;
	}
	
	// count buckys necessary; setup face->buckyIndex
	BucketLock buckys[MAXREADBUCKYS];
  root->envMap = FALSE;
	for (i = 0; i < matfeat->triCount; i++)
	{
    // search current buckys
    //
		U32 index = matfeat->tris[i * 3 + 0];

		for (j = 0; j < buckycount; j++)
		{
      BucketLock & bucky = buckys[j];

      // does it match an existing bucky
			if (bucky.material  == matfeat->materials[index]->material
       && bucky.teamColor == matfeat->materials[index]->status.teamColor
       && bucky.envMap    == matfeat->materials[index]->status.envMap
       && bucky.overlay   == matfeat->materials[index]->status.overlay
			 && bucky.texture0  == matfeat->textures[index][0]
       && bucky.texture1  == matfeat->textures[index][1])
			{
        U16 faceIndex = vertfeat->faceIndex[i];
				FaceObj * face = &root->faces[faceIndex];

        U32 meshFlags = root->states[verttostate[vertfeat->tris[faceIndex * 3 + 0]].index[0]].GetMeshFromRoot()->RenderFlags() & RS_MESH_MASK;
        if ((buckys[j].flags0 & RS_MESH_MASK) == meshFlags)
        {
				  for (k = 0; k < vertfeat->faceTris[i]; k++, face++)
				  {
					  face->buckyIndex = (U8) j;
				  }
				  break;
        }
			}
		}

  	if (j == buckycount)
		{
      // new bucky
      //
      BucketLock &bucky = buckys[buckycount];
      bucky.ClearData();

			FaceObj *face = &root->faces[vertfeat->faceIndex[i]];
      U32 triindex = vertfeat->tris[vertfeat->faceIndex[i] * 3 + 0];

      Material::Wrap * wrap = matfeat->materials[index];
      bucky.material = wrap->material;

      bucky.diff = bucky.material->Diffuse();
      bucky.teamColor = wrap->status.teamColor;
      bucky.envMap    = wrap->status.envMap;
      bucky.overlay   = wrap->status.overlay;

      if (bucky.envMap)
      {
        root->envMap = TRUE;
      }

			bucky.texture0  = matfeat->textures[index][0];
			bucky.texture1  = matfeat->textures[index][1];
      bucky.flags1 = RS_BLEND_DECAL;

      bucky.flags0 = wrap->blendFlags | (root->states[verttostate[triindex].index[0]].GetMeshFromRoot()->RenderFlags() & RS_MESH_MASK);
//      bucky.flags0 = root->states[verttostate[mapNewToOldIndex[triindex]].index[0]].GetMeshFromRoot()->renderFlags;

			bucky.vertCount = 0;
			bucky.indexCount = 0;
			
			for (k = 0; k < vertfeat->faceTris[i]; k++, face++)
			{
				face->buckyIndex = (U8) j;
			}
			buckycount++;

			ASSERT(buckycount < MAXREADBUCKYS);
		}
	}
	
	// possible extra bucky necessary for blank faces (default material)
	Bool doExtraBucketLock = FALSE;
	buckys[buckycount].vertCount = 0;
	buckys[buckycount].indexCount = 0;

  ASSERT( buckycount < MAXREADBUCKYS - 1);
        
	// setup faces and bucky vert counts
	for (i = 0; i < vertfeat->triCount; i++)
	{
		FaceObj &face = root->faces[i];
    face.index = U16(i);

		if (face.buckyIndex == 0xffff)
		{
			// blank face
			face.buckyIndex = (U8) buckycount;
			doExtraBucketLock = TRUE;
		}
		else
		{
			ASSERT(face.buckyIndex < buckycount);
		}
		
		buckys[face.buckyIndex].vertCount += 3;       // !!! FIXME when the pipline is final
		buckys[face.buckyIndex].indexCount += 3;
		
		if (textfeat->objCount && i >= textfeat->triCount)
		{
			textfeat->tris[i * 3 + 0] = textfeat->tris[i * 3 + 1] = textfeat->tris[i * 3 + 2] = 0;
			textfeat->triCount++;
		}
		for (j = 0; j < 3; j++)
		{
			face.verts[j] = vertfeat->tris[i * 3 + j];
			face.norms[j] = normfeat->tris[i * 3 + j];
			face.uvs[j]   = textfeat->tris[i * 3 + j];
			
			ASSERT(face.verts[j] < vertfeat->objCount);
			ASSERT(face.norms[j] < normfeat->objCount);
			if (textfeat->objCount)
			{
				ASSERT(face.uvs[j]   < textfeat->objCount);
			}
		}
	}
	if (doExtraBucketLock)
	{
    buckys[buckycount].teamColor = FALSE;
    buckys[buckycount].envMap    = FALSE;
    buckys[buckycount].overlay   = FALSE;

		// add the default material for blank faces
		buckys[buckycount].material = Vid::defMaterial;
		buckys[buckycount].texture0  = NULL;
		buckycount++;
	}
	root->buckys.Setup(buckycount, buckys);

  U8 check[MAXBUCKYS];
  Utils::Memset( check, 0, sizeof(check));
  for (i = 0; i < root->faces.count; i++)
  {
    check[root->faces[i].buckyIndex]++;
  }  
  for (i = 0; i < buckycount; i++)
  {
    ASSERT( buckys[i].vertCount > 0);
    ASSERT( check[i] > 0);
  }

	// animation
	AnimList *animList = NULL;

	if (!Vid::Var::doBasePose)
	{
		animList = ReadAnim(iss, token, *root);
	}

	// toss away root object matrix 
  root->states[0].ClearData();

	// setup anims
	if (animList)
	{
		root->SetupAnimCycle(*animList);
	}
	
	// set world matrices relative to 0,0,0
	root->SetWorldAll(Matrix::I);
	
	ReadEnvelopes(iss, token, *root);
	
	U32 vertexCount = vertfeat->objCount;

	if (vertexCount == 0 && root->children.GetCount() == 0)
	{
		// empty object
    delete istreambuf;
    delete iss;
		delete root;

    delete mapNewToOldIndex;
	  delete verttostate;
	  delete texttostate;
	  delete vertfeat;
	  delete normfeat;
	  delete textfeat;
	  delete colorfeat;
	  delete matfeat;
    delete mapper;

		LOG_ERR(("MeshMan::Load: error reading %s", meshName));
		return NULL;
	}

	// must be done before SetupRoot
	root->vertToState.Alloc(vertexCount);

  for (i = 0; i < vertexCount; i++)
	{
  	ASSERT(verttostate[mapNewToOldIndex[i]].index[0] < meshStateCount);
		
		root->vertToState[i] = verttostate[mapNewToOldIndex[i]];
  }
 
	// allocate meshroot data lists and copy data in 
	root->vertices.Alloc(vertexCount);
	for (i = 0; i < vertfeat->objCount; i++)
	{
		root->vertices[i] = vertfeat->vects[i];
	}
	root->normals.Alloc(normfeat->objCount);
	for (i = 0; i < normfeat->objCount; i++)
	{
		root->normals[i] = normfeat->vects[i];
	}

  if (!textfeat->objCount)
  {
    textfeat->objCount = 1;
    textfeat->uvs[0].u = textfeat->uvs[0].v = 0.0f;
  }

	root->uvs.Alloc(textfeat->objCount);
	for (i = 0; i < textfeat->objCount; i++)
	{
		root->uvs[i] = textfeat->uvs[i];
	}

  if (colorfeat->objCount)
  {
    ASSERT( colorfeat->triCount == vertfeat->triCount);

    root->colors.Alloc( vertexCount);
    memset(mapper, 0xff, vertexCount * 2);
    for (i = 0; i < colorfeat->triCount; i++)
    {
      for (j = 0; j < 3; j++)
      {
        U16 vindex = vertfeat->tris[i * 3 + j];
        if (mapper[vindex] == 0xffff)
        {
          root->colors[vindex] = colorfeat->colors[colorfeat->tris[i * 3 + j]];
          mapper[vindex] = vindex;
        }
      }
    }
  }

	// done with the file
  delete istreambuf;
  delete iss;

  // validate weighting
  for (i = 0; i < root->vertToState.count; i++)
  {
    ASSERT( root->vertToState[i].weight[0] >= root->vertToState[i].weight[1]);
  }
        
	// set world matrices relative to 0,0,0
	root->SetWorldAll(Matrix::I);

	if (domrmgen && root->uvs.count)
	{
    AnimList *animList = NULL;
    if (Vid::Var::doFrogPose)
    {
      // attempt to load frog pose anim data for mrmgen
      BuffString buff1;
      buff1 = name.str;
      Utils::Strcat( buff1.str, ext.str);

      char *c = Utils::Strchr(buff1.str, '-');
      if (c)
      {
        *c = '\0';
      }
      else
      {
        for (c = buff1.str + Utils::Strlen(buff1.str) - 1; c > buff1.str; c--)
        {
          if (*c == '.')
          {
            *c = '\0';
            break;
          }
        }
        if (c <= buff1.str)
        {
          c = buff1.str + + Utils::Strlen(buff1.str);
        }
      }
      Utils::Strcpy( c, "-frog.xsi");
      
	    iss = Open(buff1.str, token);
    
	    if (iss) 
	    {
        animList = ReadAnim( iss, token, *root);
    	  // done with the file
        delete istreambuf;
        delete iss;

        if (animList)
        {
          animList->SetFrame( 0.0f, root->states);
      	  // set world matrices relative to 0,0,0
      	  root->SetWorldAll(Matrix::I);
        }
  	  }
    }

		root->MRMGen();
		if (!root->mrm)
		{
			LOG_DIAG(("Can't generate mrm data for mesh: %s", sCurrFileName));
		}

    if (animList)
    {
      // delete frog pose data
      delete animList;

      animList = root->animCycles.GetFirst();
      if (animList)
      {
        root->SetupStates( animList->states);
//        animList->SetFrame( 0.0f, root->states);
      	// set world matrices relative to 0,0,0
      	root->SetWorldAll(Matrix::I);
      }
    }
	}

  // general setup
  //
	root->scale = sScaleFactor;
  root->Setup();

	// setup up face plane equations
  //
	root->SetupPlanes();
	root->CalcBoundingSphere();

	LOG_DIAG(("verts %d, norms %d, uvs %d, faces %d", 
		root->vertices.count,
		root->normals.count,
		root->uvs.count,
		root->faces.count));

  delete mapNewToOldIndex;
  delete verttostate;
  delete texttostate;
	delete vertfeat;
	delete normfeat;
	delete textfeat;
	delete colorfeat;
	delete matfeat;
  delete mapper;

	// tag root mesh and add it to the search tree
	if (!SetupRoot(*root, buff.str))
  {
    delete root;
    root = NULL;
  }
  root->Check( MAXBUCKYS, *Vid::Var::checkMaxVerts, *Vid::Var::checkMaxTris * 3);
  root->shadowRadius = root->ObjectBounds().Radius();

//  root->Rebuild();

	return root;
}
//----------------------------------------------------------------------------

AnimList * Mesh::Manager::ReadAnimCycle(const char *fileName, MeshRoot &root, const char *cycleName)
{
#ifdef DONOANIMS
  return NULL;
#endif

  if (Vid::Var::doBasePose)
  {
    return NULL;
  }

  // check if its already loaded
  AnimList *animList = root.animCycles.Find( Crc::CalcStr( cycleName));
  if (animList)
  {
    return animList;
  }

	// load it up
	sFlipZ = TRUE;
	sFlipX = FALSE;
	XFileToken token;
	istrstream *iss = Open(fileName, token);
	if (!iss)
	{
		return FALSE;
	}

  if (!root.FindAnimCycle( DEFCYCLENAME))
  {
    // create a default anim cycle before loading others
    AnimList *animList = new AnimList;

	  // create this cycle's initial state array
	  animList->SetupStates(root.states);
	  animList->SetupMaxFrame();

		root.SetupAnimCycle(*animList);
  }
  
	sScaleFactor = root.scale;
	Vid::Var::scaleFactor = 1.0f;

	// setup temp mesh data pointers
	FamilyState meshStates[MAXREADMESH];
	meshStateCount = 0;
	
	Matrix statemats[MAXREADMESH];
	stateMatrix = statemats;
	
	U16 meshverts[MAXREADMESH];
	meshvertindex = meshverts;
  memset( meshvertindex, 0, MAXREADMESH * sizeof(U16));

	verttostate = new VertIndex[MAXREADVERTS];
  texttostate = new U16[MAXREADVERTS];

	vertfeat  = new VectFeature;
	normfeat  = new VectFeature;
	textfeat  = new TextFeature;
	colorfeat = new ColorFeature;
	matfeat   = new MatFeature;

	while (token != _MESH && token != _FRAME)
	{
		// get the next token
		token = XFileLex.GetToken(iss);
	}

	// geometry
	switch (token)
	{
	case _MESH:
		ReadMesh(iss, token, NULL);
		break;
	case _FRAME:
		ReadMeshGroup(iss, token, NULL, meshStates, NULL);
		break;
	}
	
	// validate tri data
	ASSERT(normfeat->triCount  == vertfeat->triCount);
	ASSERT(textfeat->objCount  == 0 || textfeat->triCount  == vertfeat->triCount);
  ASSERT(colorfeat->objCount == 0 || colorfeat->triCount == vertfeat->triCount);

  delete verttostate;
  delete texttostate;
	delete vertfeat;
	delete normfeat;
	delete textfeat;
	delete colorfeat;
	delete matfeat;

  if (root.states.count != meshStateCount)
  {
    WARN_CON_DIAG( ("ReadAnimCycle: anim's hierarchy differs from base: %s", sCurrFileName) );
    delete istreambuf;  
	  delete iss;
    return NULL;
  }

  // eliminate root position and rotation offsets
  //
  root.states[0].ClearData();

  U32 i;
  for (i = 1; i < meshStateCount; i++)
  {
	  root.states[i].SetObjectMatrix( stateMatrix[i]);
//		root.states[i].SetObjectMatrixScale( meshStates[i].GetScale());
  }

	animList = ReadAnim(iss, token, root, TRUE);

  delete istreambuf;  
	delete iss;
	
	if (animList) 
	{
		root.SetupAnimCycle(*animList, cycleName);

  	root.CalcBoundingSphere();

  	LOG_DIAG(("Mesh::Manager::ReadAnimCycle: %s", cycleName));
	}
  else
  {
    LOG_CON_DIAG( ("No animation data in file %s", sCurrFileName) );
  }

	return animList;
}
//----------------------------------------------------------------------------
