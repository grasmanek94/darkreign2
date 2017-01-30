///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2000 Pandemic Studios, Dark Reign II
//
// xsiexp.cpp
//
//
#include "xsiexp.h"
//----------------------------------------------------------------------------

HINSTANCE hInstance;
int controlsInit = FALSE;

static BOOL showPrompts;
static BOOL exportSelected;

// Class ID. These must be unique and randomly generated!!
// If you use this as a sample project, this is the first thing
// you should change!
#define XSIEXP_CLASS_ID	Class_ID(0x52ee2e34, 0x40d4004f)
//----------------------------------------------------------------------------


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

	// Initialize the custom controls. This should be done only once.
	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
	
	return (TRUE);
}
//----------------------------------------------------------------------------

__declspec( dllexport ) const TCHAR* LibDescription() 
{
	return GetString(IDS_LIBDESCRIPTION);
}
//----------------------------------------------------------------------------

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS 
__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}
//----------------------------------------------------------------------------

__declspec( dllexport ) ClassDesc* LibClassDesc(int i) 
{
	switch(i) {
	case 0: return GetXsiExpDesc();
	default: return 0;
	}
}
//----------------------------------------------------------------------------

__declspec( dllexport ) ULONG LibVersion() 
{
	return VERSION_3DSMAX;
}
//----------------------------------------------------------------------------

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}
//----------------------------------------------------------------------------


class XsiExpClassDesc:public ClassDesc {
public:
	int				    IsPublic() { return 1; }
	void *		    Create(BOOL loading = FALSE) { return new XsiExp; } 
	const TCHAR *	ClassName() { return GetString(IDS_XSIEXP); }
	SClass_ID		  SuperClassID() { return SCENE_EXPORT_CLASS_ID; } 
	Class_ID		  ClassID() { return XSIEXP_CLASS_ID; }
	const TCHAR *	Category() { return GetString(IDS_CATEGORY); }
};
static XsiExpClassDesc XsiExpDesc;
//----------------------------------------------------------------------------

ClassDesc* GetXsiExpDesc()
{
	return &XsiExpDesc;
}
//----------------------------------------------------------------------------

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
  {
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
  }
	return NULL;
}
//----------------------------------------------------------------------------

XsiExp::XsiExp()
{
	// These are the default values that will be active when 
	// the exporter is ran the first time.
	// After the first session these options are sticky.
	bIncludeAnim = TRUE;
	bIncludeVertexColors = FALSE;
	bAlwaysSample = FALSE;
	nKeyFrameStep = 5;
	nMeshFrameStep = 5;
	nPrecision = 6;
	nStaticFrame = 0;
}
//----------------------------------------------------------------------------

XsiExp::~XsiExp()
{
}
//----------------------------------------------------------------------------

int XsiExp::ExtCount()
{
	return 1;
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::Ext(int n)
{
	switch(n)
  {
	case 0:
		// This cause a static string buffer overwrite
		// return GetString(IDS_EXTENSION1);
		return _T("XSI");
	}
	return _T("");
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::LongDesc()
{
	return GetString(IDS_LONGDESC);
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::ShortDesc()
{
	return GetString(IDS_SHORTDESC);
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::AuthorName() 
{
	return _T("John Cooke");
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::OtherMessage1() 
{
	return _T("");
}
//----------------------------------------------------------------------------

const TCHAR * XsiExp::OtherMessage2() 
{
	return _T("");
}
//----------------------------------------------------------------------------

unsigned int XsiExp::Version()
{
	return 100;
}
//----------------------------------------------------------------------------

static BOOL CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
  {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd)); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
    {
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       
//----------------------------------------------------------------------------

void XsiExp::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}
//----------------------------------------------------------------------------

// Dialog proc
static BOOL CALLBACK ExportDlgProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
//	Interval animRange;
//	ISpinnerControl  *spin;

	XsiExp *exp = (XsiExp*)GetWindowLong(hWnd,GWL_USERDATA); 
	switch (msg)
  {
	case WM_INITDIALOG:
		exp = (XsiExp*)lParam;
		SetWindowLong(hWnd,GWL_USERDATA,lParam); 
		CenterWindow(hWnd, GetParent(hWnd)); 

    CheckDlgButton(hWnd, IDC_ANIMKEYS, exp->GetIncludeAnim()); 
		CheckDlgButton(hWnd, IDC_VERTEXCOLORS,exp->GetIncludeVertexColors()); 

/*    
		CheckRadioButton(hWnd, IDC_RADIO_USEKEYS, IDC_RADIO_SAMPLE, 
  		exp->GetAlwaysSample() ? IDC_RADIO_SAMPLE : IDC_RADIO_USEKEYS);
	
		// Setup the spinner controls for the controller key sample rate 
		spin = GetISpinner(GetDlgItem(hWnd, IDC_CONT_STEP_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_CONT_STEP), EDITTYPE_INT ); 
		spin->SetLimits(1, 100, TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(exp->GetKeyFrameStep() ,FALSE);
		ReleaseISpinner(spin);
		
		// Setup the spinner controls for the mesh definition sample rate 
		spin = GetISpinner(GetDlgItem(hWnd, IDC_MESH_STEP_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_MESH_STEP), EDITTYPE_INT ); 
		spin->SetLimits(1, 100, TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(exp->GetMeshFrameStep() ,FALSE);
		ReleaseISpinner(spin);

		// Setup the spinner controls for the floating point precision 
		spin = GetISpinner(GetDlgItem(hWnd, IDC_PREC_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_PREC), EDITTYPE_INT ); 
		spin->SetLimits(1, 10, TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(exp->GetPrecision() ,FALSE);
		ReleaseISpinner(spin);

		// Setup the spinner control for the static frame#
		// We take the frame 0 as the default value
		animRange = exp->GetInterface()->GetAnimRange();
		spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
		spin->LinkToEdit(GetDlgItem(hWnd,IDC_STATIC_FRAME), EDITTYPE_INT ); 
		spin->SetLimits(animRange.Start() / GetTicksPerFrame(), animRange.End() / GetTicksPerFrame(), TRUE); 
		spin->SetScale(1.0f);
		spin->SetValue(0, FALSE);
		ReleaseISpinner(spin);
*/
		break;

/*
	case CC_SPINNER_CHANGE:
		spin = (ISpinnerControl*)lParam; 
		break;
*/
	case WM_COMMAND:
		switch (LOWORD(wParam))
    {
		case IDC_MESHDATA:
			// Enable / disable mesh options
			EnableWindow(GetDlgItem(hWnd, IDC_VERTEXCOLORS), IsDlgButtonChecked(hWnd, IDC_MESHDATA));
			break;
		case IDOK:
			exp->SetIncludeAnim(IsDlgButtonChecked(hWnd, IDC_ANIMKEYS)); 
			exp->SetIncludeVertexColors(IsDlgButtonChecked(hWnd, IDC_VERTEXCOLORS)); 

/*
			exp->SetAlwaysSample(IsDlgButtonChecked(hWnd, IDC_RADIO_SAMPLE));

			spin = GetISpinner(GetDlgItem(hWnd, IDC_CONT_STEP_SPIN)); 
			exp->SetKeyFrameStep(spin->GetIVal()); 
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd, IDC_MESH_STEP_SPIN)); 
			exp->SetMeshFrameStep(spin->GetIVal());
			ReleaseISpinner(spin);

			spin = GetISpinner(GetDlgItem(hWnd, IDC_PREC_SPIN)); 
			exp->SetPrecision(spin->GetIVal());
			ReleaseISpinner(spin);
		
			spin = GetISpinner(GetDlgItem(hWnd, IDC_STATIC_FRAME_SPIN)); 
			exp->SetStaticFrame(spin->GetIVal() * GetTicksPerFrame());
			ReleaseISpinner(spin);
*/			
			EndDialog(hWnd, 1);
			break;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       
//----------------------------------------------------------------------------

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}
//----------------------------------------------------------------------------

// Start the exporter!
// This is the real entrypoint to the exporter. After the user has selected
// the filename (and he's prompted for overwrite etc.) this method is called.
//
int XsiExp::DoExport( const TCHAR * name, ExpInterface * ei, Interface * i, BOOL suppressPrompts, DWORD options) 
{
	// Set a global prompt display switch
	showPrompts = suppressPrompts ? FALSE : TRUE;
	exportSelected = (options & SCENE_EXPORT_SELECTED) ? TRUE : FALSE;

	// Grab the interface pointer.
	ip = i;

	int numChildren = ip->GetRootNode()->NumberOfChildren();
  if (!numChildren)
  {
    return 1;
  }

	// Get the options the user selected the last time
	ReadConfig();

	if(showPrompts)
  {
		// Prompt the user with our dialogbox, and get all the options.
		if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_EXPORT_DLG),
			ip->GetMAXHWnd(), ExportDlgProc, (LPARAM)this))
    {
			return 1;
		}
	}
	else
  {	// Set default parameters here
  	bIncludeAnim = TRUE;
		bIncludeVertexColors = FALSE;

    bAlwaysSample = FALSE;
		nKeyFrameStep = 5;
		nMeshFrameStep = 5;
		nPrecision = 4;
		nStaticFrame = 0;
	}
	
	sprintf(szFmtStr, "%%4.%df", nPrecision);

	// Open the stream
	pStream = _tfopen(name,_T("wt"));
	if (!pStream)
  {
		return 0;
	}
	
	// Startup the progress bar.
	ip->ProgressStart(GetString(IDS_PROGRESS_MSG), TRUE, fn, NULL);

	// Get a total node count by traversing the scene
	// We don't really need to do this, but it doesn't take long, and
	// it is nice to have an accurate progress bar.
	nTotalNodeCount = 0;
	nCurNode = 0;
	PreProcess(ip->GetRootNode(), nTotalNodeCount);
	
	// First we write out a file header with global information. 
	ExportGlobalInfo();

  // export meshes
	for (int idx = 0; idx < numChildren; idx++)
  {
		if (ip->GetCancel())
    {
			break;
    }
		nodeEnumMesh( ip->GetRootNode()->GetChildNode(idx), 0);
	}
	for (idx = 0; idx < numChildren; idx++)
  {
		if (ip->GetCancel())
    {
			break;
    }
		nodeEnumBone( ip->GetRootNode()->GetChildNode(idx), 0);
	}
  // close brace for hierarchy
 	fprintf(pStream,"}\n\n");

  if (bIncludeAnim)
  {
    int animHit = FALSE;    // write anim list head only the first time
	  for (idx = 0; idx < numChildren; idx++)
    {
		  if (ip->GetCancel())
      {
			  break;
      }
		  nodeEnumAnim( ip->GetRootNode()->GetChildNode(idx), animHit);
	  }

    if (animHit)
    {
  	  fprintf(pStream,"}\n\n");
    }
  }

	for (idx = 0; idx < numChildren; idx++)
  {
		if (ip->GetCancel())
    {
			break;
    }
    nodeEnumSkin( ip->GetRootNode()->GetChildNode(idx));
  }

	// We're done. Finish the progress bar.
	ip->ProgressEnd();

	// Close the stream
	fclose(pStream);

	// Write the current options to be used next time around.
	WriteConfig();

	return 1;
}
//----------------------------------------------------------------------------

BOOL XsiExp::SupportsOptions(int ext, DWORD options)
{
	assert(ext == 0);	// We only support one extension
	return(options == SCENE_EXPORT_SELECTED) ? TRUE : FALSE;
}
//----------------------------------------------------------------------------

BOOL XsiExp::nodeEnumMesh(INode* node, int indentLevel) 
{
	if(exportSelected && node->Selected() == FALSE)
  {
		return TREE_CONTINUE;
  }

	nCurNode++;
	ip->ProgressUpdate( (int)((float)nCurNode/nTotalNodeCount*100.0f) ); 

	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
  {
		return FALSE;
  }
	TSTR indent = GetIndent(indentLevel);

/*  
	// If this node is a group head, all children are 
	// members of this group. The node will be a dummy node and the node name
	// is the actualy group name.
	if (node->IsGroupHead())
  {
		fprintf(pStream,"%s%s \"%s\" {\n", indent.data(), ID_GROUP, FixupName(node->GetName())); 
		indentLevel++;
	}
*/
  
	// Only export if exporting everything or it's selected
	if(!exportSelected || node->Selected())
  {
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(0); 

		// The obj member of ObjectState is the actual object we will export.
		if (os.obj)
    {
			// We look at the super class ID to determine the type of the object.
			switch(os.obj->SuperClassID())
      {
			case GEOMOBJECT_CLASS_ID: 
        ExportMeshObject(node, indentLevel); 
				break;
      default:
        return FALSE;
/*
			case HELPER_CLASS_ID:
				ExportBoneObject(node, indentLevel); 
				break;
			case CAMERA_CLASS_ID:
				if (GetIncludeObjCamera()) ExportCameraObject(node, indentLevel); 
				break;
			case LIGHT_CLASS_ID:
				if (GetIncludeObjLight()) ExportLightObject(node, indentLevel); 
				break;
			case SHAPE_CLASS_ID:
				if (GetIncludeObjShape()) ExportShapeObject(node, indentLevel); 
				break;
*/
			}
		}
	}	
	
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++)
  {
		if (!nodeEnumMesh(node->GetChildNode(c), indentLevel+1))
    {
			return FALSE;
    }
	}
	
/*
	// If thie is true here, it is the end of the group we started above.
	if (node->IsGroupHead())
  {
		fprintf(pStream,"%s}\n", indent.data());
		indentLevel--;
	}
*/

  if (indentLevel != 0)
  {
  	fprintf(pStream,"%s}\n\n", indent.data());
  }

	return TRUE;
}
//----------------------------------------------------------------------------

BOOL XsiExp::nodeEnumBone( INode * node, int indentLevel) 
{
	if(exportSelected && node->Selected() == FALSE)
  {
		return TREE_CONTINUE;
  }

	nCurNode++;
	ip->ProgressUpdate( (int)((float)nCurNode/nTotalNodeCount*100.0f) ); 

	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
  {
		return FALSE;
  }

	// Only export if exporting everything or it's selected
	if(!exportSelected || node->Selected())
  {
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(0); 

		// The obj member of ObjectState is the actual object we will export.
		if (os.obj)
    {
			// We look at the super class ID to determine the type of the object.
			switch(os.obj->SuperClassID())
      {
			case HELPER_CLASS_ID:
				ExportBoneObject(node, indentLevel); 
				break;
      default:
        return FALSE;
			}
		}
	}	
  
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++)
  {
		if (!nodeEnumBone(node->GetChildNode(c), indentLevel+1))
    {
			return FALSE;
    }
	}
	TSTR indent = GetIndent(indentLevel);

	fprintf(pStream,"%s}\n\n", indent.data());

	return TRUE;
}
//----------------------------------------------------------------------------

BOOL XsiExp::nodeEnumSkin( INode * node) 
{
  if (ExportSkinData( node))
  {
    return FALSE;
  }

  for (int c = 0; c < node->NumberOfChildren(); c++)
  {
		if (!nodeEnumSkin(node->GetChildNode(c)))
    {
			return FALSE;
    }
	}
  return TRUE;
}
//----------------------------------------------------------------------------

BOOL XsiExp::nodeEnumAnim(INode* node, int & animHit) 
{
	if(exportSelected && node->Selected() == FALSE)
  {
		return TREE_CONTINUE;
  }

	nCurNode++;
	ip->ProgressUpdate( (int)((float)nCurNode/nTotalNodeCount*100.0f) ); 

	// Stop recursing if the user pressed Cancel 
	if (ip->GetCancel())
  {
		return FALSE;
  }
	
	// Only export if exporting everything or it's selected
	if(!exportSelected || node->Selected())
  {
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(0); 

		// The obj member of ObjectState is the actual object we will export.
		if (os.obj)
    {
			// We look at the super class ID to determine the type of the object.
			switch(os.obj->SuperClassID())
      {
			case HELPER_CLASS_ID:
        {
      	  Object * obj = node->EvalWorldState(0).obj;
        	if (!obj && obj->ClassID() != Class_ID(BONE_CLASS_ID, 0))
          {
            return FALSE;
          }
        }
        // fall through
			case GEOMOBJECT_CLASS_ID: 
        ExportAnimKeys( node, animHit); 
				break;
			}
		}
	}	
	
	// For each child of this node, we recurse into ourselves 
	// until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++)
  {
		if (!nodeEnumAnim(node->GetChildNode(c), animHit))
    {
			return FALSE;
    }
	}

	return TRUE;
}
//----------------------------------------------------------------------------

void XsiExp::PreProcess(INode* node, int& nodeCount)
{
	nodeCount++;
	
	// Add the nodes material to out material list
	// Null entries are ignored when added...
	mtlList.AddMtl(node->GetMtl());

	// For each child of this node, we recurse into ourselves 
	// and increment the counter until no more children are found.
	for (int c = 0; c < node->NumberOfChildren(); c++)
  {
		PreProcess(node->GetChildNode(c), nodeCount);
	}
}
//----------------------------------------------------------------------------

TSTR XsiExp::GetCfgFilename()
{
	TSTR filename;
	
	filename += ip->GetDir(APP_PLUGCFG_DIR);
	filename += "\\";
	filename += CFGFILENAME;

	return filename;
}
//----------------------------------------------------------------------------

// NOTE: Update anytime the CFG file changes
#define CFG_VERSION 0x01

BOOL XsiExp::ReadConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "rb");
	if (!cfgStream)
  {
		return FALSE;
  }

	// First item is a file version
	int fileVersion = _getw(cfgStream);

	if (fileVersion > CFG_VERSION)
  {
		// Unknown version
		fclose(cfgStream);
		return FALSE;
	}

	SetIncludeAnim(fgetc(cfgStream));
  SetIncludeVertexColors(fgetc(cfgStream));

	SetAlwaysSample(fgetc(cfgStream));
	SetMeshFrameStep(_getw(cfgStream));
	SetKeyFrameStep(_getw(cfgStream));

	SetPrecision(_getw(cfgStream));

	fclose(cfgStream);

	return TRUE;
}
//----------------------------------------------------------------------------

void XsiExp::WriteConfig()
{
	TSTR filename = GetCfgFilename();
	FILE* cfgStream;

	cfgStream = fopen(filename, "wb");
	if (!cfgStream)
  {
		return;
  }

	// Write CFG version
	_putw(CFG_VERSION,				cfgStream);

	fputc(GetIncludeAnim(),			cfgStream);
	fputc(GetIncludeVertexColors(),	cfgStream);

	fputc(GetAlwaysSample(),		cfgStream);
	_putw(GetMeshFrameStep(),		cfgStream);
	_putw(GetKeyFrameStep(),		cfgStream);

  _putw(GetPrecision(),			cfgStream);

	fclose(cfgStream);
}
//----------------------------------------------------------------------------

BOOL MtlKeeper::AddMtl(Mtl* mtl)
{
	if (!mtl)
  {
		return FALSE;
	}

	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++)
  {
		if (mtlTab[i] == mtl)
    {
			return FALSE;
		}
	}
	mtlTab.Append(1, &mtl, 25);

	return TRUE;
}
//----------------------------------------------------------------------------

int MtlKeeper::GetMtlID(Mtl* mtl)
{
	int numMtls = mtlTab.Count();
	for (int i=0; i<numMtls; i++)
  {
		if (mtlTab[i] == mtl)
    {
			return i;
		}
	}
	return -1;
}
//----------------------------------------------------------------------------

int MtlKeeper::Count()
{
	return mtlTab.Count();
}
//----------------------------------------------------------------------------

Mtl* MtlKeeper::GetMtl(int id)
{
	return mtlTab[id];
}
//----------------------------------------------------------------------------
