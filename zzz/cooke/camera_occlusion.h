//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// camera_occulsion.h
//
//
//

#ifndef _CAMERA_OCCLUSION_INCLUDED_
#define _CAMERA_OCCLUSION_INCLUDED_
//-----------------------------------------------------------------------------


class Camera;
#define OCCLUSION_PLANE_DIST		   100.0f

enum DrawOperation{
	TEST_POLY,
	DRAW_POLY
};


struct OCCLUSION_PLANE_DATA
{
	unsigned index_count;
	unsigned vertex_stride;
	unsigned vertex_count;
	void *vertex_ptr;
	unsigned short *index_list;
};

//-----------------------------------------------------------------------------

class OcclusionClass
{
	Bool Enable;
	unsigned Size;
	unsigned Width;
	unsigned Height;
	float nearPlane;
	unsigned char *Buffer;
	unsigned char *AllocatedBuffer;
	Bool LargeSphere;
	Bool GeometryIsNear;
	Bool GeometryClipPlane;
	float Occlusion_Plane;
	Camera *MyCamera;

	Bool Test_Box_Poly(unsigned short *IndexList, unsigned char *ClipBits);
	Bool Test_bSphere (const Sphere &sphere);
	void ShowPoly( Vector *Ptr, unsigned Count, unsigned color);
	Bool Test_Poly (Vector *Ptr, unsigned Count);
	void Draw_Poly (Vector *Ptr, unsigned Count);
	Bool ClipXY_And_Draw (Vector *vList, unsigned Count, unsigned Bits, DrawOperation Oper);
	void Draw_OcclusionPoly (unsigned i0, unsigned i1, unsigned i2);

	virtual void ClearOcclusionBuffer();

	public:
	OcclusionClass(Camera *Cam);
	~OcclusionClass();

	Bool SetOcclusion (Bool OnOff);
	Bool GetState () {return Enable;}

	virtual void Setup ();
	virtual void RenderToOcclusion(OCCLUSION_PLANE_DATA *Context);		

	void RenderToOcclusion(MeshEnt *Ent);		
	unsigned SphereTest(const Sphere &sphere, unsigned flags);
};

#endif
