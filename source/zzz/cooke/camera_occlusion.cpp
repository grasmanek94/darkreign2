//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// camera_occulsion.cpp
//
//
//

//#include <crtdbg.h>
#include "vid_priv.h"
#include "meshent.h"
#include "camera.h"
#include "iftypes.h"
#include "iface_util.h"
//-----------------------------------------------------------------------------

//#define DEBUG_OCCLUSION
#ifdef DEBUG_OCCLUSION
#define SAVE_OCCLUSION_BITMAP
#endif

#define MAX_VERTEX				1024


#define BUFFER_RESOLUTION_BITS	1		

struct SPAN_EDGE
{
	Float2Int  ix;
	float x;
	float dx;
	int index;
	int next;
};

static Vector Vertex_Pool[MAX_VERTEX];
static Vector NewPoly[40];
//-----------------------------------------------------------------------------


OcclusionClass::OcclusionClass(Camera *Cam)
{
//	Enable = FALSE;
	Enable = TRUE;
	MyCamera = Cam;

	Width = 0;
	Height = 0;
	Size = 0;
	AllocatedBuffer = NULL;    
	Buffer = NULL;
	Occlusion_Plane = OCCLUSION_PLANE_DIST;
	GeometryIsNear = FALSE;
	GeometryClipPlane = FALSE;
	LargeSphere	= FALSE;

}

OcclusionClass::~OcclusionClass()
{
	if (AllocatedBuffer) {
		delete AllocatedBuffer;
	}
}

void OcclusionClass::ClearOcclusionBuffer()
{
#ifdef SAVE_OCCLUSION_BITMAP
	static int Count = 0;
	char Name[32];
	if ((Count > 30) && (Count < 40)) {
		sprintf (Name, "OccMask%d.bmp", Count);
		Bitmap Mask;
		Mask.Create(Width * 8, Height, TRUE);
		Mask.ClearData(RGBA_MAKE (0, 0, 0, 0));
		int i;
		int j;
		int k;
		unsigned Color;
		unsigned char *Ptr;
		Ptr = Buffer;
		for (i = 0; i < (int)Height; i ++) {
			for (j = 0; j < (int)Width; j ++) {
				Color =  Ptr[j];
				for (k = 0; k < 8; k ++) {
					if (!(Color & 0x80)) {
						 Mask.PutPixel(j * 8 + k, i, RGBA_MAKE (255, 255, 255, 0));
					}
					Color <<= 1;
				}

			}
			Ptr += Width;
		}
		Mask.WriteBMP (Name);
		IFace::ScreenDump();
	}
	Count ++;


#endif

	memset (Buffer, 0xff, Size);


}


Bool OcclusionClass::SetOcclusion (Bool OnOff)
{
	Bool tmp = Enable;

	Enable = OnOff;
	return tmp;
}


void OcclusionClass::Setup ()
{
	unsigned w;
	unsigned h;
	unsigned Bytes;
	
	// Call to base class setup
//	Ret = Camera::Setup();

	if (!Enable) {
		return;
	}
	
	// now do my thing
	w = ((MyCamera->rect.Width() >> (3 + BUFFER_RESOLUTION_BITS)) + 8) & -3;
	h = MyCamera->rect.Height() >> BUFFER_RESOLUTION_BITS;

	Bytes = w * h;
	
	if (!AllocatedBuffer) {
		// Create buffer for the first time
		AllocatedBuffer  = new unsigned char [Bytes + w * 8];
	} else if (Bytes > Size) {
		// the new buffer is bigger the the old one 
		// i need to reallocate a new one
		delete AllocatedBuffer;
		AllocatedBuffer  = new unsigned char [Bytes + w * 8];
	}
	
	Width = w;
	Height = h;
	Size = Bytes;
	
	Buffer = AllocatedBuffer + w * 4;
	
    Occlusion_Plane = MyCamera->farPlane * 0.2f;
	nearPlane = MyCamera->nearPlane;

	if (Occlusion_Plane < OCCLUSION_PLANE_DIST) {
		Occlusion_Plane = OCCLUSION_PLANE_DIST;
	}

	ClearOcclusionBuffer();

}


// returns clip flag bits found in rendconst.h
unsigned OcclusionClass::SphereTest(
	const Sphere &Src_sphere, 
	unsigned flag) 
{
	float R;
//	unsigned flag;
	
	
	Sphere sphere(Src_sphere);
	
	Src_sphere.Transform (sphere, MyCamera->viewMatrix);
	
//	flag = Camera::SphereTest (sphere.Origin(), sphere.Radius());
//	if (flag == clipOUTSIDE) {
//		return flag;
//	}
	
	// set geometry visibility flag to FALSE
	GeometryIsNear = TRUE;
	R = sphere.posit.z - sphere.radius;
	if (R > Occlusion_Plane) {
		GeometryIsNear = FALSE;
		if (!Test_bSphere (sphere)) {
			flag = clipOUTSIDE;
			
#ifdef DEBUG_OCCLUSION
			// this sphere is hidden show sphere
//			Src_sphere.Render();
#endif
			
		}
		return flag;
	}
	
	GeometryClipPlane = TRUE;
	R = sphere.posit.z + sphere.radius;
	if (R < Occlusion_Plane) {
		GeometryClipPlane = FALSE;
	}
	
	
	LargeSphere	= TRUE;
	// check if the sphere is larger than 2 pixels in screen space
	if (sphere.radius * Width < 8.0f * sphere.posit.z) {
		LargeSphere	= FALSE;
	}
	
	return flag;
}

Bool OcclusionClass::Test_bSphere (const Sphere &sphere)
{
	int i;
	unsigned char Bits;
	unsigned char ClipBits[8];
	float Test;

	
	static unsigned short Indices[3][2][4] = 
	{
		{{1, 2, 6, 5}, {0, 4, 7, 3}},
		{{3, 7, 6, 2}, {0, 1, 5, 4}},
		{{4, 5, 6, 7}, {3, 2, 1, 0}}, 
	};
	
	Vertex_Pool[0].x = -sphere.Width;
	Vertex_Pool[0].y = -sphere.Height;
	Vertex_Pool[0].z = -sphere.Breadth;
	
	Vertex_Pool[1].x =  sphere.Width;
	Vertex_Pool[1].y = -sphere.Height;
	Vertex_Pool[1].z = -sphere.Breadth;
	
	Vertex_Pool[2].x =  sphere.Width;
	Vertex_Pool[2].y =  sphere.Height;
	Vertex_Pool[2].z = -sphere.Breadth;
	
	Vertex_Pool[3].x = -sphere.Width;
	Vertex_Pool[3].y =  sphere.Height;
	Vertex_Pool[3].z = -sphere.Breadth;
	
	Vertex_Pool[4].x = -sphere.Width;
	Vertex_Pool[4].y = -sphere.Height;
	Vertex_Pool[4].z =  sphere.Breadth;
	
	Vertex_Pool[5].x =  sphere.Width;
	Vertex_Pool[5].y = -sphere.Height;
	Vertex_Pool[5].z =  sphere.Breadth;
	
	Vertex_Pool[6].x = sphere.Width;
	Vertex_Pool[6].y = sphere.Height;
	Vertex_Pool[6].z = sphere.Breadth;
	
	Vertex_Pool[7].x = -sphere.Width;
	Vertex_Pool[7].y =  sphere.Height;
	Vertex_Pool[7].z =  sphere.Breadth;
	
	for (i = 0; i < 8; i ++) {
		sphere.Transform (Vertex_Pool[i], Vertex_Pool[i]);
		Vid::ProjectFromCamera(Vertex_Pool[i]);
		
		Bits = 0;
		if (Vertex_Pool[i].x < MyCamera->rect.p0.x) {
			Bits  |= 1;
		}
		
		if (Vertex_Pool[i].y < MyCamera->rect.p0.y) {
			Bits  |= 2;
		}
		
		if (Vertex_Pool[i].x > (MyCamera->rect.p1.x - 1)) {
			Bits  |= 4;
		}
		
		if (Vertex_Pool[i].y > (MyCamera->rect.p1.y - 1)) {
			Bits |= 8;
		}
		ClipBits[i] = Bits;
	}

  U16 Rounding = Utils::FP::SetRoundUpMode();
	
	Test = sphere.right.Dot(sphere.posit);
	if ((sphere.Width + Test) < 0) {
		if (Test_Box_Poly(&Indices[0][0][0], ClipBits)) {
			Utils::FP::RestoreMode(Rounding);
			return TRUE;
		}
	}
	
	if ((sphere.Width - Test) < 0) {
		if (Test_Box_Poly(&Indices[0][1][0], ClipBits)) {
			Utils::FP::RestoreMode(Rounding);
			return TRUE;
		}
	}
	
	
	Test = sphere.up.Dot(sphere.posit);
	if ((sphere.Height + Test) < 0) {
		if (Test_Box_Poly(&Indices[1][0][0], ClipBits)) {
			Utils::FP::RestoreMode(Rounding);
			return TRUE;
		}
	}
	
	if ((sphere.Height - Test) < 0) {
		if (Test_Box_Poly(&Indices[1][1][0], ClipBits)) {
			Utils::FP::RestoreMode(Rounding);
			return TRUE;
		}
	}
	
	Test = sphere.front.Dot(sphere.posit);
	if ((sphere.Breadth + Test) < 0) {
		if (Test_Box_Poly(&Indices[2][0][0], ClipBits)) {
			Utils::FP::RestoreMode(Rounding);
			return TRUE;
		}
	}
	
	if ((sphere.Breadth - Test) < 0) {
		if (Test_Box_Poly(&Indices[2][1][0], ClipBits)) {
			Utils::FP::RestoreMode(Rounding);
			return TRUE;
		}
	}
	
	Utils::FP::RestoreMode(Rounding);
	return FALSE;
}
//-----------------------------------------------------------------------------


Bool OcclusionClass::Test_Box_Poly( unsigned short *IndexList, unsigned char *ClipBits)
{
	int i;
	int Or;
	int And;
	int index;
	
	And = 0xffff;
	Or = 0;
	for (i = 0; i < 4; i ++) {
		index = IndexList[i];
		NewPoly[i] = Vertex_Pool[index];
		And &= ClipBits[index];
		Or |= ClipBits[index];
	}
	if (And) {
		return FALSE;
	}
	
	return ClipXY_And_Draw (NewPoly, 4, Or, TEST_POLY);
}


void OcclusionClass::RenderToOcclusion(OCCLUSION_PLANE_DATA *Context)
{
	int i;
	int i0;
	int i1;
	int i2;
	int Index_Count;
	int Vertex_Count;
	int Vertex_Stride;
	
	float *Src;
	unsigned short *Index_List; 

	if (!Enable) {
		return;
	}
	
	if (!GeometryIsNear) 
		return;
	
	if (!LargeSphere & GeometryClipPlane) 
		return;
	
	//   _ASSERTE (Context->vertex_count < MAX_VERTEX);
	
	// Transform all vertex
	Vertex_Count = Context->vertex_count;
	Vertex_Stride = Context->vertex_stride>>2;
	Src = (float*)Context->vertex_ptr;
	
	for (i = 0; i < Vertex_Count; i ++) {
		Vertex_Pool[i].x = Src[0];
		Vertex_Pool[i].y = Src[1];
		Vertex_Pool[i].z = Src[2];
		Src += Vertex_Stride;
	}
	
	// loop trough every triangle
	Index_Count = Context->index_count; 
	Index_List =  Context->index_list; 

  U16 Rounding = Utils::FP::SetRoundUpMode();

	for (i = 0; i < Index_Count; i += 3) {
		i0 = Index_List[0];
		i1 = Index_List[1];
		i2 = Index_List[2];
		
		Index_List += 3;
		
		Draw_OcclusionPoly (i0, i1, i2);
	}
	Utils::FP::RestoreMode(Rounding);

}


void OcclusionClass::RenderToOcclusion(MeshEnt *Ent)
{
	if (!Enable) {
		return;
	}

	if (!GeometryIsNear) {
		return;
	}
	
	if (!LargeSphere & GeometryClipPlane) {
		return;
	}

	Ent = NULL;

	// Get John Cook to find out how to render and object

//	Matrix Mat;
//	Vector Eye;
//	Ent->GetIntWorldMatrix().Transform(Mat, viewMatrix);
//
//	Eye.x = -Mat.posit.Dot(Mat.right);
//	Eye.y = -Mat.posit.Dot(Mat.up);
//	Eye.z = -Mat.posit.Dot(Mat.front);
//
//	unsigned i;
//	unsigned vertIndex;
//	unsigned indexIndex;
//	unsigned vcount;
//	unsigned icount;
//	MeshRoot	*Root;
//
//	vertIndex = 0;
//	indexIndex = 0;
//
//	Root = &Ent->Root();
//	for (i = 0; i < Root->groups.count; i++)
//	{
//		VertGroup &group = Root->groups[i];
//
//		vcount = group.vertCount;
//		icount = group.indexCount;
//
//		//	    DxLight::Manager::Light( vertmem, &planes[group.planeIndex], 
//		//      &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], vcount, 
//		//      indexmem, &indices[indexIndex], icount);
//
//		vertIndex  += group.vertCount;
//		indexIndex += group.indexCount;
//	}
//
}


void OcclusionClass::ShowPoly(	Vector *Ptr, unsigned Count, unsigned color)
{
	unsigned i1;
	unsigned i0;
	float Scale;
	VertexTL points[2];
	
	Scale = 1.0f;
#if BUFFER_RESOLUTION_BITS > 0
	Scale *= (1<<BUFFER_RESOLUTION_BITS);
#endif
	
	
	
	i0 = Count - 1;
	for (i1 = 0; i1 < Count; i1++ ) {
		// starting points
		points[0].vv.x     = Ptr[i0].x * Scale;
		points[0].vv.y     = Ptr[i0].y * Scale;
		points[0].vv.z     = 0.0;
		points[0].rhw	  =	1.0;
		points[0].diffuse  = color;
		points[0].specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
		points[0].u        = 0.0f;
		points[0].v        = 0.0f;
		
		// ending points
		points[1].vv.x     = Ptr[i1].x * Scale;
		points[1].vv.y     = Ptr[i1].y * Scale;
		points[1].vv.z     = 0.0;
		points[1].rhw	  =	1.0;
		points[1].diffuse  = color;
		points[1].specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
		points[1].u        = 0.0f;
		points[1].v        = 0.0f;
		
		// draw the line
		Vid::SetMaterial(NULL);
		Vid::DrawPrimitive(PT_LINELIST, FVF_TLVERTEX, points, 2, 
			DP_DONOTUPDATEEXTENTS | RS_BLEND_DEF);
		i0 = i1;
	}
}
//-----------------------------------------------------------------------------


void OcclusionClass::Draw_OcclusionPoly( unsigned i0, unsigned i1, unsigned i2)
{
	int i;
	int Or;
	int And;
	int Code;
	int Count;
	float dx1;
	float dy1;
	float dx2;
	float dy2;
	int *Src;
	int Index[16];
	Float2Int::Sign Test0;
	Float2Int::Sign Test1;
	Vector *Dst_Vertex;
	Vector *Src_Vertex0;
	
	Test0.f = nearPlane - Vertex_Pool[i0].z;
	And = Test0.i;
	Or = Test0.i;
	
	Test0.f = nearPlane - Vertex_Pool[i1].z;
	And &= Test0.i;
	Or |= Test0.i;
	
	Test0.f = nearPlane - Vertex_Pool[i2].z;
	And &= Test0.i;
	Or |= Test0.i;
	
	if (Or >= 0) {
		return;
	}
	
	Index[0] = i0;
	Index[1] = i1;
	Index[2] = i2;
	Count = 3;
	Src_Vertex0 = Vertex_Pool;
	
	if ((And ^ Or) < 0) {
		Src = Index;
		
		Count = 0;
		i0 = Index[2];
		Test0.f = Vertex_Pool[i0].z - nearPlane;
		for (i = 0; i < 3; i ++) {
			i1 = *Src;
			Test1.f = Vertex_Pool[i1].z - nearPlane;
			
			if (Test1.i >= 0) {
				if (Test0.i >= 0) {
					NewPoly[Count] = Vertex_Pool[i1];
					Count ++;
					
					
				} else {
					NewPoly[Count + 1] = Vertex_Pool[i1];
					
					dx1 = Test0.f / (Vertex_Pool[i0].z - Vertex_Pool[i1].z);
					
					NewPoly[Count].z = nearPlane;
					NewPoly[Count].x = Vertex_Pool[i0].x + (Vertex_Pool[i1].x - Vertex_Pool[i0].x) * dx1;
					NewPoly[Count].y = Vertex_Pool[i0].y + (Vertex_Pool[i1].y - Vertex_Pool[i0].y) * dx1;
					Count += 2;
				}
				
			} else {
				if (Test0.i >= 0) {
					
					dx1 = Test0.f / (Vertex_Pool[i0].z - Vertex_Pool[i1].z);
					
					NewPoly[Count].z = nearPlane;
					NewPoly[Count].x = Vertex_Pool[i0].x + (Vertex_Pool[i1].x - Vertex_Pool[i0].x) * dx1;
					NewPoly[Count].y = Vertex_Pool[i0].y + (Vertex_Pool[i1].y - Vertex_Pool[i0].y) * dx1;
					Count ++;
					
				}	
			}
			Test0 = Test1;
			i0 = i1;
			Src ++;
		}
		
		if (!Count) {
			return;
		}
		Index[0] = 0;
		Index[1] = 1;
		Index[2] = 2;
		Index[3] = 3;
		Src_Vertex0 = NewPoly;
	}
	
	if (LargeSphere & GeometryClipPlane) {
		// clip aganst Oclusion plane
		
		Dst_Vertex = NewPoly + Count;
		Src = Index;
		
		i0 = Index[Count-1];
		Code = 0;
		Test0.f = Src_Vertex0[i0].z - Occlusion_Plane;
		for (i = 0; i < Count; i ++) {
			i1 = *Src;
			Test1.f = Src_Vertex0[i1].z - Occlusion_Plane;
			
			if (Test1.i <= 0) {
				if (Test0.i <= 0) {
					Dst_Vertex[Code] = Src_Vertex0[i1];
					Code ++;
					
					
				} else {
					Dst_Vertex[Code + 1] = Src_Vertex0[i1];
					
					dx1 = Test0.f / (Src_Vertex0[i0].z - Src_Vertex0[i1].z);
					
					Dst_Vertex[Code].z = Occlusion_Plane;
					Dst_Vertex[Code].x = Src_Vertex0[i0].x + (Src_Vertex0[i1].x - Src_Vertex0[i0].x) * dx1;
					Dst_Vertex[Code].y = Src_Vertex0[i0].y + (Src_Vertex0[i1].y - Src_Vertex0[i0].y) * dx1;
					Code += 2;
				}
				
			} else {
				if (Test0.i <= 0) {
					
					dx1 = Test0.f / (Src_Vertex0[i0].z - Src_Vertex0[i1].z);
					
					Dst_Vertex[Code].z = Occlusion_Plane;
					Dst_Vertex[Code].x = Src_Vertex0[i0].x + (Src_Vertex0[i1].x - Src_Vertex0[i0].x) * dx1;
					Dst_Vertex[Code].y = Src_Vertex0[i0].y + (Src_Vertex0[i1].y - Src_Vertex0[i0].y) * dx1;
					Code ++;
				}	
			}
			Test0 = Test1;
			i0 = i1;
			Src ++;
		}
		
		if (!Code) {
			return;
		}
		Index[0] = 0;
		Index[1] = 1;
		Index[2] = 2;
		Index[3] = 3;
		Index[4] = 4;
		Index[4] = 5;
		Count = Code;
		Src_Vertex0 = Dst_Vertex;
	}
	
	
	And = -1;
	Or = 0;
	for (i = 0; i < Count; i ++) {
		Code = 0;
		i0 = Index[i];
		Vid::ProjectFromCamera(NewPoly[i], Src_Vertex0[i0]);
		
		if (NewPoly[i].x < MyCamera->rect.p0.x) {
			Code |= 1;
		}
		if (NewPoly[i].y < MyCamera->rect.p0.y) {
			Code |= 2;
		}
		if (NewPoly[i].x > (MyCamera->rect.p1.x - 1)) {
			Code |= 4;
		}
		if (NewPoly[i].y > (MyCamera->rect.p1.y - 1)) {
			Code |= 8;
		}
		
		And &= Code;
		Or |= Code;
	}
	
	if (And) {
		return;
	}
	
	dx2 = NewPoly[2].x - NewPoly[0].x;
	dy2 = NewPoly[2].y - NewPoly[0].y;
	dx1 = NewPoly[1].x - NewPoly[0].x;
	dy1 = NewPoly[1].y - NewPoly[0].y;
	Test0.f = dx2 * dy1 - dy2 * dx1;
	if (Test0.i >= 0) {
		return;
	}

//	if (Test0.i >= 0) {
//		return;
//	}

	
	ClipXY_And_Draw (NewPoly, Count, Or, DRAW_POLY);
}

Bool OcclusionClass::ClipXY_And_Draw (
	Vector *Src_Vertex1,
	unsigned	Count,
	unsigned Bits,
	DrawOperation Oper)
{
	unsigned i;
	unsigned NewCount;
	Float2Int::Sign Test0;
	Float2Int::Sign Test1;
	Vector *Dst_Vertex;
	Vector *Src_Vertex0;
	
	Src_Vertex1 = NewPoly;
	if (Bits & 1) {
		NewCount = 0;
		Dst_Vertex = Src_Vertex1 + Count;
		Src_Vertex0	= Dst_Vertex - 1;
		Test0.f = Src_Vertex0->x - MyCamera->rect.p0.x;
		for (i = 0; i < Count; i ++) {
			
			Test1.f = Src_Vertex1->x - MyCamera->rect.p0.x;
			if (Test1.i >= 0) {
				if (Test0.i >= 0) {
					Dst_Vertex->x = Src_Vertex1->x;
					Dst_Vertex->y = Src_Vertex1->y;
					Dst_Vertex ++;
					NewCount ++;
					
				} else {
					Dst_Vertex[1].x = Src_Vertex1->x;
					Dst_Vertex[1].y = Src_Vertex1->y;
					
					Dst_Vertex[0].x = (float)MyCamera->rect.p0.x;
					Dst_Vertex[0].y = Src_Vertex0->y + Test0.f * (Src_Vertex1->y - Src_Vertex0->y) / (Src_Vertex0->x - Src_Vertex1->x);
					Dst_Vertex += 2;
					NewCount += 2;
				}
			} else {
				if (Test0.i >= 0) {
					Dst_Vertex[0].x = (float)MyCamera->rect.p0.x;
					Dst_Vertex[0].y = Src_Vertex0->y + Test0.f * (Src_Vertex1->y - Src_Vertex0->y) / (Src_Vertex0->x - Src_Vertex1->x);
					Dst_Vertex ++;
					NewCount ++;
				}	
			}
			Src_Vertex0 = Src_Vertex1;
			Src_Vertex1	++;
			Test0 = Test1;
		}
		
		if (!NewCount) {
			return FALSE;
		}
		Count = NewCount;
	}
	
	
	if (Bits & 2) {
		NewCount = 0;
		Dst_Vertex = Src_Vertex1 + Count;
		Src_Vertex0	= Dst_Vertex - 1;
		Test0.f = Src_Vertex0->y - MyCamera->rect.p0.y;
		for (i = 0; i < Count; i ++) {
			
			Test1.f = Src_Vertex1->y - MyCamera->rect.p0.y;
			if (Test1.i >= 0) {
				if (Test0.i >= 0) {
					Dst_Vertex->x = Src_Vertex1->x;
					Dst_Vertex->y = Src_Vertex1->y;
					Dst_Vertex ++;
					NewCount ++;
					
				} else {
					Dst_Vertex[1].x = Src_Vertex1->x;
					Dst_Vertex[1].y = Src_Vertex1->y;
					
					Dst_Vertex[0].y = (float)MyCamera->rect.p0.y;
					Dst_Vertex[0].x = Src_Vertex0->x + Test0.f * (Src_Vertex1->x - Src_Vertex0->x) / (Src_Vertex0->y - Src_Vertex1->y);
					Dst_Vertex += 2;
					NewCount += 2;
				}
			} else {
				if (Test0.i >= 0) {
					Dst_Vertex[0].y = (float)MyCamera->rect.p0.y;
					Dst_Vertex[0].x = Src_Vertex0->x + Test0.f * (Src_Vertex1->x - Src_Vertex0->x) / (Src_Vertex0->y - Src_Vertex1->y);
					Dst_Vertex ++;
					NewCount ++;
				}	
			}
			Src_Vertex0 = Src_Vertex1;
			Src_Vertex1	++;
			Test0 = Test1;
		}
		
		if (!NewCount) {
			return FALSE;
		}
		Count = NewCount;
	}
	
	if (Bits & 4) {
		NewCount = 0;
		Dst_Vertex = Src_Vertex1 + Count;
		Src_Vertex0	= Dst_Vertex - 1;
		Test0.f = Src_Vertex0->x - (MyCamera->rect.p1.x - 1);
		for (i = 0; i < Count; i ++) {
			
			Test1.f = Src_Vertex1->x - (MyCamera->rect.p1.x - 1);
			if (Test1.i <= 0) {
				if (Test0.i <= 0) {
					Dst_Vertex->x = Src_Vertex1->x;
					Dst_Vertex->y = Src_Vertex1->y;
					Dst_Vertex ++;
					NewCount ++;
					
				} else {
					Dst_Vertex[1].x = Src_Vertex1->x;
					Dst_Vertex[1].y = Src_Vertex1->y;
					
					Dst_Vertex[0].x = (float)(MyCamera->rect.p1.x - 1);
					Dst_Vertex[0].y = Src_Vertex0->y + Test0.f * (Src_Vertex1->y - Src_Vertex0->y) / (Src_Vertex0->x - Src_Vertex1->x);
					Dst_Vertex += 2;
					NewCount += 2;
				}
			} else {
				if (Test0.i <= 0) {
					Dst_Vertex[0].x = (float)(MyCamera->rect.p1.x - 1);
					Dst_Vertex[0].y = Src_Vertex0->y + Test0.f * (Src_Vertex1->y - Src_Vertex0->y) / (Src_Vertex0->x - Src_Vertex1->x);
					Dst_Vertex ++;
					NewCount ++;
				}	
			}
			Src_Vertex0 = Src_Vertex1;
			Src_Vertex1	++;
			Test0 = Test1;
		}
		
		if (!NewCount) {
			return FALSE;
		}
		Count = NewCount;
	}
	
	if (Bits & 8) {
		NewCount = 0;
		Dst_Vertex = Src_Vertex1 + Count;
		Src_Vertex0	= Dst_Vertex - 1;
		Test0.f = Src_Vertex0->y - (MyCamera->rect.p1.y - 1);
		for (i = 0; i < Count; i ++) {
			
			Test1.f = Src_Vertex1->y - (MyCamera->rect.p1.y - 1);
			if (Test1.i <= 0) {
				if (Test0.i <= 0) {
					Dst_Vertex->x = Src_Vertex1->x;
					Dst_Vertex->y = Src_Vertex1->y;
					Dst_Vertex ++;
					NewCount ++;
					
				} else {
					Dst_Vertex[1].x = Src_Vertex1->x;
					Dst_Vertex[1].y = Src_Vertex1->y;
					
					Dst_Vertex[0].y = (float)(MyCamera->rect.p1.y - 1);
					Dst_Vertex[0].x = Src_Vertex0->x + Test0.f * (Src_Vertex1->x - Src_Vertex0->x) / (Src_Vertex0->y - Src_Vertex1->y);
					Dst_Vertex += 2;
					NewCount += 2;
				}
			} else {
				if (Test0.i <= 0) {
					Dst_Vertex[0].y = (float)(MyCamera->rect.p1.y - 1);
					Dst_Vertex[0].x = Src_Vertex0->x + Test0.f * (Src_Vertex1->x - Src_Vertex0->x) / (Src_Vertex0->y - Src_Vertex1->y);
					Dst_Vertex ++;
					NewCount ++;
				}	
			}
			Src_Vertex0 = Src_Vertex1;
			Src_Vertex1	++;
			Test0 = Test1;
		}
		
		if (!NewCount) {
			return FALSE;
		}
		Count = NewCount;
	}
	
#if BUFFER_RESOLUTION_BITS > 0
	for (i = 0; i < Count ; i ++) {
		Src_Vertex1[i].x *= (1.0f/(1<<BUFFER_RESOLUTION_BITS));
		Src_Vertex1[i].y *= (1.0f/(1<<BUFFER_RESOLUTION_BITS));
	}
#endif
	
	
	if (Oper == DRAW_POLY) {
		Draw_Poly (Src_Vertex1, Count);
		return TRUE;
	}
	return Test_Poly (Src_Vertex1, Count);
}






void OcclusionClass::Draw_Poly (Vector *Poly, unsigned Vertices)
{
	unsigned i;
	int x0;
	int x1;
    int Min_Index;
	int Pixels_Run;
	unsigned Mask0;
	unsigned Mask1;
	unsigned *DwordPtr;
	unsigned char *BytesPtr;
	unsigned char *BuffPtr;
    Float2Int::Sign r0;
    Float2Int::Sign r1;
    Float2Int y;
    Float2Int yn;
    SPAN_EDGE Right;
    SPAN_EDGE Left;	
    int Down_dir[32]; 
    int Up_dir[32];   
	
#ifdef DEBUG_OCCLUSION	
	ShowPoly (Poly, Vertices, RGBA_MAKE(255, 255, 0, 0xFF));
#endif
	
    Min_Index = 0;
    Down_dir[0] = Vertices - 1;
    Down_dir[1] = 0;
	
    r0.f = Poly[0].y;
    for (i = 1; i < Vertices; i ++ )  {
		r1.f = Poly[i].y;
		Up_dir[i-1] = i;
		Down_dir[i+1] = i;
		if ( r1.i <= r0.i ) {
			Min_Index = i;
			r0.i = r1.i;
		}
    }
    Up_dir[i-1] = 0;
	
    Right.index = Min_Index;
    Left.index = Min_Index;
    Right.next = Min_Index;
    Left.next = Min_Index;
	
    Left.x = 0;
    Right.x = 0;
    Left.dx = 0;
    Right.dx = 0;
	
	
    y.f = r0.f + Float2Int::magic;
	BuffPtr = Buffer + Width * y.i;
	
    for (;;) {
		yn.f = Poly[Right.next].y + Float2Int::magic;
		if (y.i >= yn.i) {
            do {
				Right.index = Right.next;
				Right.next = Up_dir[Right.next];
				if (Right.next == Left.index) return;
				yn.f = Poly[Right.next].y + Float2Int::magic;
            } while (y.i >= yn.i);
			
            r1.f = 1.0f / (Poly[Right.next].y - Poly[Right.index].y);
            yn.f = Poly[Right.index].y + Float2Int::magic;
			
            Right.dx = (Poly[Right.next].x - Poly[Right.index].x) * r1.f;
            Right.x = Poly[Right.index].x;
		}
		
		yn.f = Poly[Left.next].y + Float2Int::magic;
		if (y.i >= yn.i) {
            do {
				Left.index = Left.next;
				Left.next = Down_dir[Left.next];
				yn.f = Poly[Left.next].y + Float2Int::magic;
            } while (y.i >= yn.i);
			
            r1.f = 1.0f / (Poly[Left.next].y - Poly[Left.index].y);
            yn.f = Poly[Left.index].y + Float2Int::magic;
			
            Left.dx = (Poly[Left.next].x - Poly[Left.index].x) * r1.f;
            Left.x = Poly[Left.index].x;
		}
		
		Right.ix.f = Right.x + Float2Int::magic;
		Left.ix.f = Left.x + Float2Int::magic;
		Pixels_Run = Right.ix.i - Left.ix.i;
		
		if (Pixels_Run >= 0) {
			x0 = (Left.ix.i >> 3) & -4;
			x1 = (Right.ix.i >> 3) & -4;
			
			DwordPtr = (unsigned*)(BuffPtr + x0);
			
			//			 _ASSERTE (!((int)DwordPtr & 3));
			
			Mask0 = ~(0xffffffff >> (Left.ix.i & 31));
			Mask1 = ~(0xffffffff << (31 - (Right.ix.i & 31)));
			
			if (x1 == x0) {
				Mask0 |= Mask1;
				BytesPtr = (unsigned char*)DwordPtr;
				*(BytesPtr + 0) &= Mask0 >> 24;
				*(BytesPtr + 1) &= Mask0 >> 16;
				*(BytesPtr + 2) &= Mask0 >> 8;
				*(BytesPtr + 3) &= Mask0;
			} else {
				BytesPtr = (unsigned char*)DwordPtr;
				*(BytesPtr + 0) &= Mask0 >> 24;
				*(BytesPtr + 1) &= Mask0 >> 16;
				*(BytesPtr + 2) &= Mask0 >> 8;
				*(BytesPtr + 3) &= Mask0;
				DwordPtr ++;
				for (x0 = x0 + 4;	x0 < x1; x0 += 4)	{
					*DwordPtr = 0;
					DwordPtr ++;
				}
				BytesPtr = (unsigned char*)DwordPtr;
				*(BytesPtr + 0) &= Mask1 >> 24;
				*(BytesPtr + 1) &= Mask1 >> 16;
				*(BytesPtr + 2) &= Mask1 >> 8;
				*(BytesPtr + 3) &= Mask1;
			}
		}
		
		y.i ++;
		BuffPtr += Width;
		Left.x += Left.dx;
		Right.x += Right.dx;
	}
}




Bool OcclusionClass::Test_Poly (Vector *Poly, unsigned Vertices)
{
	unsigned i;
	int x0;
	int x1;
    int Min_Index;
	int Pixels_Run;
	unsigned Mask0;
	unsigned Mask1;
	unsigned Code;
	unsigned *DwordPtr;
	unsigned char *BytesPtr;
	unsigned char *BuffPtr;
    Float2Int::Sign r0;
    Float2Int::Sign r1;
    Float2Int y;
    Float2Int yn;
    SPAN_EDGE Right;
    SPAN_EDGE Left;	
    int Down_dir[32]; 
    int Up_dir[32];

	
//return TRUE;
    Min_Index = 0;
    Down_dir[0] = Vertices - 1;
    Down_dir[1] = 0;
	
    r0.f = Poly[0].y;
    for (i = 1; i < Vertices; i ++ )  {
		r1.f = Poly[i].y;
		Up_dir[i-1] = i;
		Down_dir[i+1] = i;
		if ( r1.i <= r0.i ) {
			Min_Index = i;
			r0.i = r1.i;
		}
    }
    Up_dir[i-1] = 0;
	
    Right.index = Min_Index;
    Left.index = Min_Index;
    Right.next = Min_Index;
    Left.next = Min_Index;
	
    Left.x = 0;
    Right.x = 0;
    Left.dx = 0;
    Right.dx = 0;

//static int xxx = 0;	
//xxx ++;
	
    y.f = r0.f + Float2Int::magic;
	BuffPtr = Buffer + Width * y.i;
	
    for(;;) {
		yn.f = Poly[Right.next].y + Float2Int::magic;
		if (y.i >= yn.i) {
            do {
				Right.index = Right.next;
				Right.next = Up_dir[Right.next];
				if (Right.next == Left.index) {
#ifdef DEBUG_OCCLUSION	
					ShowPoly (Poly, Vertices, RGBA_MAKE(255, 255, 255, 0xFF));
#endif
					return FALSE;
				}
				yn.f = Poly[Right.next].y + Float2Int::magic;
            } while (y.i >= yn.i);
			
            r1.f = 1.0f / (Poly[Right.next].y - Poly[Right.index].y);
            yn.f = Poly[Right.index].y + Float2Int::magic;
			
            Right.dx = (Poly[Right.next].x - Poly[Right.index].x) * r1.f;
            Right.x = Poly[Right.index].x;
		}
		
		yn.f = Poly[Left.next].y + Float2Int::magic;
		if (y.i >= yn.i) {
            do {
				Left.index = Left.next;
				Left.next = Down_dir[Left.next];
				yn.f = Poly[Left.next].y + Float2Int::magic;
            } while (y.i >= yn.i);
			
            r1.f = 1.0f / (Poly[Left.next].y - Poly[Left.index].y);
            yn.f = Poly[Left.index].y + Float2Int::magic;
			
            Left.dx = (Poly[Left.next].x - Poly[Left.index].x) * r1.f;
            Left.x = Poly[Left.index].x;
		}
		
		Right.ix.f = Right.x + Float2Int::magic;
		Left.ix.f = Left.x + Float2Int::magic;
		Pixels_Run = Right.ix.i - Left.ix.i;
		
		if (Pixels_Run >= 0) {
			x0 = (Left.ix.i >> 3) & -4;
			x1 = (Right.ix.i >> 3) & -4;
			
			DwordPtr = (unsigned*)(BuffPtr + x0);
			
			//			 _ASSERTE (!((int)DwordPtr & 3));
			
			Mask0 = 0xffffffff >> (Left.ix.i & 31);
			Mask1 = 0xffffffff << (31 - (Right.ix.i & 31));
			
			if (x1 == x0) {
				Mask0 &= Mask1;
				BytesPtr = (unsigned char*)DwordPtr;
				Code = (*(BytesPtr + 0) << 24)	| (*(BytesPtr + 1) << 16) | 
					(*(BytesPtr + 2) << 8)	| (*(BytesPtr + 3)); 
				if (Code & Mask0) {
					return TRUE;
				}
				
			} else {
				BytesPtr = (unsigned char*)DwordPtr;
				Code = (*(BytesPtr + 0) << 24) | (*(BytesPtr + 1) << 16) | 
					   (*(BytesPtr + 2) << 8) | (*(BytesPtr + 3)); 
				if (Code & Mask0) {
					return TRUE;
				}
				
				DwordPtr ++;
				for (x0 = x0 + 4; x0 < x1; x0 += 4)	{
					if (*DwordPtr) {
						return TRUE;
					}
					DwordPtr ++;
				}
				BytesPtr = (unsigned char*)DwordPtr;
				
				Code = (*(BytesPtr + 0) << 24)	| (*(BytesPtr + 1) << 16) | 
					(*(BytesPtr + 2) << 8)	| (*(BytesPtr + 3)); 
				if (Code & Mask0) {
					return TRUE;
				}
			}
		}
		
		y.i ++;
		BuffPtr += Width;
		Left.x += Left.dx;
		Right.x += Right.dx;
    }
}
