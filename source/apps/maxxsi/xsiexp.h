///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2000 Pandemic Studios, Dark Reign II
//
// xsiexp.h
//
//

#ifndef __ASCIIEXP__H
#define __ASCIIEXP__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"
//----------------------------------------------------------------------------

extern ClassDesc * GetXsiExpDesc();
extern TCHAR * GetString(int id);
extern HINSTANCE hInstance;

//#define VERSION			200			// Version number * 100
#define CFGFILENAME		_T("XSIEXP.CFG")	// Configuration file

class MtlKeeper {
public:
	BOOL	AddMtl(Mtl* mtl);
	int		GetMtlID(Mtl* mtl);
	int		Count();
	Mtl*	GetMtl(int id);

	Tab<Mtl*> mtlTab;
};
//----------------------------------------------------------------------------

// This is the main class for the exporter.

class XsiExp : public SceneExport {
public:
	XsiExp();
	~XsiExp();

	// SceneExport methods
	int    ExtCount();     // Number of extensions supported 
	const TCHAR * Ext(int n);     // Extension #n (i.e. "ASC")
	const TCHAR * LongDesc();     // Long ASCII description (i.e. "Ascii Export") 
	const TCHAR * ShortDesc();    // Short ASCII description (i.e. "Ascii")
	const TCHAR * AuthorName();    // ASCII Author name
	const TCHAR * CopyrightMessage();   // ASCII Copyright message 
	const TCHAR * OtherMessage1();   // Other message #1
	const TCHAR * OtherMessage2();   // Other message #2
	unsigned int Version();     // Version number * 100 (i.e. v3.01 = 301) 
	void	ShowAbout(HWND hWnd);  // Show DLL's "About..." box
	int		DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0); // Export	file
	BOOL	SupportsOptions(int ext, DWORD options);

	// Other methods

	// Node enumeration
	BOOL	nodeEnumMesh( INode * node, int indentLevel);
	BOOL	nodeEnumBone( INode * node, int indentLevel);
	BOOL	nodeEnumSkin( INode * node);
  BOOL  nodeEnumAnim( INode * node, int & animHit);

	void	PreProcess(INode* node, int& nodeCount);

	// High level export
	void	ExportGlobalInfo();
	void	ExportMaterialList();
	void	ExportMeshObject( INode * node, int indentLevel); 
  void  ExportBoneObject( INode * node, int indentLevel);

	// Mid level export
	void	ExportMesh( INode * node, TimeValue t, int indentLevel); 
	void	ExportMaterial( INode * node, int indentLevel); 
	void	ExportNodeTM( INode * node, int indentLevebl);
	void	ExportNodeHeader( INode * node, TCHAR * type, int indentLevel);
  BOOL  ExportSkinData( INode * node);
	void	ExportAnimKeys( INode * node, int & animHit);

	// Low level export
	void	DumpMatrix3(Matrix3* m, int indentLevel);
	void	DumpMaterial(Mtl* mtl, int mtlID, int subNo, int indentLevel);
	void	DumpTexture(Texmap* tex, Class_ID cid, int subNo, float amt, int	indentLevel);
  void	DumpRotKeys( INode * node, int indentLevel);
  void  DumpPosKeys( INode * node, int indentLevel);
  void  DumpScaleKeys( INode * node, int indentLevel);

	// Misc methods
	TCHAR*	GetMapID(Class_ID cid, int subNo);
	Point3	GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	BOOL	CheckForAndExportFaceMap(Mtl* mtl, Mesh* mesh, int indentLevel); 
	void	make_face_uv(Face *f, Point3 *tv);
	BOOL	TMNegParity(Matrix3 &m);
	TSTR	GetIndent(int indentLevel);
	TCHAR*	FixupName(TCHAR* name);
	void	CommaScan(TCHAR* buf);
	BOOL	CheckForAnimation(INode* node, BOOL& pos, BOOL& rot, BOOL& scale);
	TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	BOOL	IsKnownController(Control* cont);

	// Configuration methods
	TSTR	GetCfgFilename();
	BOOL	ReadConfig();
	void	WriteConfig();
	
	// Interface to member variables
	inline BOOL	GetIncludeAnim()			{ return bIncludeAnim; }
	inline BOOL	GetIncludeVertexColors()	{ return bIncludeVertexColors; }
	inline BOOL	GetAlwaysSample()			{ return bAlwaysSample; }
	inline int	GetMeshFrameStep()			{ return nMeshFrameStep; }
	inline int	GetKeyFrameStep()			{ return nKeyFrameStep; }
	inline int	GetPrecision()				{ return nPrecision; }
	inline TimeValue GetStaticFrame()		{ return nStaticFrame; }
	inline Interface*	GetInterface()		{ return ip; }

	inline void	SetIncludeAnim(BOOL val)			{ bIncludeAnim = val; }
	inline void	SetIncludeVertexColors(BOOL val)	{ bIncludeVertexColors = val; }
	inline void	SetAlwaysSample(BOOL val)			{ bAlwaysSample = val; }
	inline void SetMeshFrameStep(int val)			{ nMeshFrameStep = val; }
	inline void SetKeyFrameStep(int val)			{ nKeyFrameStep = val; }
	inline void SetPrecision(int val)				{ nPrecision = val; }
	inline void SetStaticFrame(TimeValue val)		{ nStaticFrame = val; }

private:
	BOOL	bIncludeAnim;
	BOOL	bIncludeVertexColors;
	BOOL	bAlwaysSample;
	int		nMeshFrameStep;
	int		nKeyFrameStep;
	int		nPrecision;
 	TimeValue	nStaticFrame;

	Interface*	ip;
	FILE*		pStream;
	int			nTotalNodeCount;
	int			nCurNode;
	TCHAR		szFmtStr[16];

	MtlKeeper	mtlList;
};
//----------------------------------------------------------------------------


#endif // __ASCIIEXP__H

