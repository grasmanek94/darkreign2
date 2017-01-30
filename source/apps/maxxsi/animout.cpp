///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2000 Pandemic Studios, Dark Reign II
//
// animout.cpp
//
//
#include "xsiexp.h"
//----------------------------------------------------------------------------

extern Matrix3 topMatrix, correction;     // rotate max's axes to match soft's

#define ALMOST_ZERO 1.0e-3f
BOOL EqualPoint3(Point3 p1, Point3 p2);
//----------------------------------------------------------------------------

// Get hold of the transform controllers for the node... 
void XsiExp::ExportAnimKeys( INode * node, int & animHit) 
{
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
  //
	Object * obj = node->EvalWorldState(GetStaticFrame()).obj;
	if (!obj || obj->ClassID() == Class_ID( TARGET_CLASS_ID, 0))
  {
		return;
  }
	TSTR indent = GetIndent(1);
	BOOL bPosAnim, bRotAnim, bScaleAnim, bDoKeys = FALSE;
  TCHAR * name = FixupName( node->GetName());

  BOOL isBone = obj->ClassID() == Class_ID(BONE_CLASS_ID, 0) ? TRUE : FALSE;
  if (isBone)
  {
    // bone anims get passed to children
    if (!node->GetParentNode() || node->GetParentNode()->IsRootNode())
    {
      // can't anim top bone
      return;
    }
    node = node->GetParentNode();
  }

	// We can only export keys if all TM controllers are "known" to us.
	// The reason for that is that some controllers control more than what
	// they should. Consider a path position controller, if you turn on
	// follow and banking, this position controller will also control
	// rotation. If a node that had a path position controller also had a
	// TCB rotation controller, the TCB keys would not describe the whole
	// rotation of the node.
	// For that reason we will only export keys if all controllers
	// position, rotation and scale are linear, hybrid (bezier) or tcb.

	if (!GetAlwaysSample())
  {
		Control* pC = node->GetTMController()->GetPositionController();
		Control* rC = node->GetTMController()->GetRotationController();
		Control* sC = node->GetTMController()->GetScaleController();

		if (IsKnownController(pC) && IsKnownController(rC) && IsKnownController(sC))
    {
			bDoKeys = TRUE;
		}
	}
	if (bDoKeys)
  {
		// Only dump the track header if any of the controllers have keys
		if (node->GetTMController()->GetPositionController()->NumKeys()  > 1 ||
  			node->GetTMController()->GetRotationController()->NumKeys()  > 1 ||
	  		node->GetTMController()->GetScaleController()->NumKeys()     > 1)
    {
      if (!animHit)
      {
		    fprintf(pStream,"AnimationSet {\n"); 
        animHit = TRUE;
      }
		  fprintf(pStream,"%sAnimation anim-%s {\n", indent.data(), name ); 

	    indent = GetIndent(2);

		  fprintf(pStream,"%s{frm-%s}\n", indent.data(), name ); 

			DumpRotKeys(   node, 1);
			DumpPosKeys(   node, 1);
			DumpScaleKeys( node, 1);

	    indent = GetIndent(1);

			fprintf(pStream,"%s}\n\n", indent.data());
		}
	}
	else if (CheckForAnimation(node, bPosAnim, bRotAnim, bScaleAnim))
  {
    if (!animHit)
    {
		  fprintf(pStream,"AnimationSet {\n"); 
      animHit = TRUE;
    }

		fprintf(pStream,"%sAnimation anim-%s {\n", indent.data(), name ); 

	  indent = GetIndent(2);

		fprintf(pStream,"%s{frm-%s}\n", indent.data(), name ); 

		DumpRotKeys(   node, 1);
		DumpPosKeys(   node, 1);
		DumpScaleKeys( node, 1);

		fprintf(pStream,"%s}\n", indent.data());
	}
}
//----------------------------------------------------------------------------

// To really see if a node is animated we can step through the animation range
// and decompose the TM matrix for every frame and examine the components.
// This way we can identify position, rotation and scale animation separately.
// 
// Some controllers makes it problematic to examine the TMContollers instead of
// the actual TMMatrix. For example, a path controller is a position controller, 
// but if you turn on follow and banking, it will also affect the rotation component.
// If we want to, we can examine the position, rotation and scale controllers and
// if they all are Linear, Hybrid (bezier) or TCB, then we could export the actual keys.
// This is not at all difficult, but the importer has to know the exact interpolation
// algorithm in order to use it. The source code to the interpolation routines are available
// to ADN members.
// 
// For an example of how to export actual keys, look at DumpPoint3Keys() below.
// This method will check the actual controller to determine if the controller is known.
// If we know how to work this controller, its actual keys will be exported,
// otherwise the controller will be sampled using the user specified sampling frequency.

BOOL XsiExp::CheckForAnimation(INode* node, BOOL& bPos, BOOL& bRot, BOOL& bScale)
{
	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame();
	Matrix3 matrix;
  AffineParts ap;
	Point3 firstPos;
	float rotAngle, firstRotAngle;
	Point3 rotAxis, firstRotAxis;
	Point3 firstScaleFactor;

	bPos = bRot = bScale = FALSE;

	for (t=start; t<=end; t+=delta)
  {
		matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(matrix, &ap);

		AngAxisFromQ(ap.q, &rotAngle, rotAxis);

		if (t != start)
    {
			if (!bPos)
      {
				if (!EqualPoint3(ap.t, firstPos))
        {
					bPos = TRUE;
				}
			}
			// MAX 2.x:
			// We examine the rotation angle to see if the rotation component
			// has changed.
			// Although not entierly true, it should work.
			// It is rare that the rotation axis is animated without
			// the rotation angle being somewhat affected.
			// MAX 3.x:
			// The above did not work, I have a repro scene that doesn't export a rotation track
			// because of this. I fixed it to also compare the axis.
			if (!bRot)
      {
				if (fabs(rotAngle - firstRotAngle) > ALMOST_ZERO)
        {
					bRot = TRUE;
				}
				else if (!EqualPoint3(rotAxis, firstRotAxis))
        {
					bRot = TRUE;
				}
			}

			if (!bScale)
      {
				if (!EqualPoint3(ap.k, firstScaleFactor))
        {
					bScale = TRUE;
				}
			}
		}
		else
    {
			firstPos = ap.t;
			firstRotAngle = rotAngle;
			firstRotAxis = rotAxis;
			firstScaleFactor = ap.k;
			}

		// No need to continue looping if all components are animated
		if (bPos && bRot && bScale)
    {
			break;
    }
	}

	return bPos || bRot || bScale;
}
//----------------------------------------------------------------------------

void XsiExp::DumpPosKeys( INode * node, int indentLevel)
{
  Control * cont = node->GetTMController()->GetPositionController();
	IKeyControl * ikc = GetKeyControlInterface(cont);
	INode * parent = node->GetParentNode();
	if (!cont || !parent || (parent && parent->IsRootNode()) || !ikc)
  {
    // no controller or root node
		return;
  }
	int numKeys = ikc->GetNumKeys();
	if (numKeys <= 1)
  {  
    return;
  }
	Object * obj = node->EvalWorldState(0).obj;
  BOOL isBone = obj && obj->ClassID() == Class_ID(BONE_CLASS_ID, 0) ? TRUE : FALSE;

  // anim keys header
  TSTR indent = GetIndent(indentLevel);
	fprintf(pStream,"%s\tSI_AnimationKey {\n", indent.data()); 
	fprintf(pStream,"%s\t\t2;\n",  indent.data());     // 2 means position keys
	fprintf(pStream,"%s\t\t%d;\n", indent.data(), numKeys);
		
	int t, delta = GetTicksPerFrame();
 	Matrix3 matrix;
	for (int i = 0; i < numKeys; i++)
  {  
    // get the key's time
	  if (cont->ClassID() == Class_ID(TCBINTERP_POSITION_CLASS_ID, 0))
    {
  	  ITCBRotKey key;
			ikc->GetKey(i, &key);
      t = key.time;
    }
    else if (cont->ClassID() == Class_ID(HYBRIDINTERP_POSITION_CLASS_ID, 0))
	  {
		  IBezQuatKey key;
		  ikc->GetKey(i, &key);
      t = key.time;
    }
	  else if (cont->ClassID() == Class_ID(LININTERP_POSITION_CLASS_ID, 0))
    {
  	  ILinRotKey key;
		  ikc->GetKey(i, &key);
      t = key.time;
    }
    // sample the node's matrix
    matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
    if (!isBone)
    {
      matrix = matrix * topMatrix;
    }
    Point3 pos = matrix.GetRow(3);

		fprintf(pStream, "%s\t\t%d; 3; %.6f, %.6f, %.6f;;%s\n", 
			indent.data(),
			t / delta, 
			pos.x, pos.z, -pos.y,
      i == numKeys - 1 ? ";\n" : ",");
	}
  // anim keys close
	fprintf(pStream,"%s\t}\n\n", indent.data());
}
//----------------------------------------------------------------------------

void XsiExp::DumpRotKeys( INode * node, int indentLevel) 
{
  Control * cont = node->GetTMController()->GetRotationController();
	IKeyControl * ikc = GetKeyControlInterface(cont);
	INode * parent = node->GetParentNode();
	if (!cont || !parent || (parent && parent->IsRootNode()) || !ikc)
  {
    // no controller or root node
		return;
  }
	int numKeys = ikc->GetNumKeys();
	if (numKeys <= 1)
  {  
    return;
  }
	Object * obj = node->EvalWorldState(0).obj;
  BOOL isBone = obj && obj->ClassID() == Class_ID(BONE_CLASS_ID, 0) ? TRUE : FALSE;

  // anim keys header
	TSTR indent = GetIndent(indentLevel);
  fprintf(pStream,"%s\tSI_AnimationKey {\n", indent.data()); 
	fprintf(pStream,"%s\t\t0;\n", indent.data());     // 0 means rotation keys
	fprintf(pStream,"%s\t\t%d;\n", indent.data(), numKeys);

	int t, delta = GetTicksPerFrame();
 	Matrix3 matrix;
  AffineParts ap;
	for (int i = 0; i < numKeys; i++)
  {  
    // get the key's time
	  if (cont->ClassID() == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0))
    {
  	  ITCBRotKey key;
			ikc->GetKey(i, &key);
      t = key.time;
    }
    else if (cont->ClassID() == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0))
	  {
		  IBezQuatKey key;
		  ikc->GetKey(i, &key);
      t = key.time;
    }
	  else if (cont->ClassID() == Class_ID(LININTERP_ROTATION_CLASS_ID, 0))
    {
  	  ILinRotKey key;
		  ikc->GetKey(i, &key);
      t = key.time;
    }
    // sample the node's matrix
    matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
    if (!isBone)
    {
      matrix = matrix * topMatrix;
    }
    decomp_affine( matrix, &ap);
    AngAxis aa( ap.q);
    float temp = aa.axis.z;
    aa.axis.z = -aa.axis.y;
    aa.axis.y = temp;
    ap.q.Set( aa);

		fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;%s\n", 
			indent.data(),
			t / delta,
			ap.q.w, ap.q.x, ap.q.y, ap.q.z,
      i == numKeys - 1 ? ";\n" : ",");
	}
  // anim keys close
	fprintf(pStream,"%s\t}\n\n", indent.data());
}
//----------------------------------------------------------------------------

void XsiExp::DumpScaleKeys( INode * node, int indentLevel) 
{
  Control * cont = node->GetTMController()->GetScaleController();
	IKeyControl * ikc = GetKeyControlInterface(cont);
	INode * parent = node->GetParentNode();
	if (!cont || !parent || (parent && parent->IsRootNode()) || !ikc)
  {
    // no controller or root node
		return;
  }
	int numKeys = ikc->GetNumKeys();
	if (numKeys <= 1)
  {  
    return;
  }
	Object * obj = node->EvalWorldState(0).obj;
  BOOL isBone = obj && obj->ClassID() == Class_ID(BONE_CLASS_ID, 0) ? TRUE : FALSE;

  // anim keys header
  TSTR indent = GetIndent(indentLevel);
	fprintf(pStream,"%s\tSI_AnimationKey {\n", indent.data()); 
	fprintf(pStream,"%s\t\t1;\n", indent.data());     // 1 means scale keys
	fprintf(pStream,"%s\t\t%d;\n", indent.data(), numKeys);

	int t, delta = GetTicksPerFrame();
 	Matrix3 matrix;
  AffineParts ap;
	for (int i = 0; i < numKeys; i++)
  {  
    // get the key's time
	  if (cont->ClassID() == Class_ID(TCBINTERP_SCALE_CLASS_ID, 0))
    {
  	  ITCBRotKey key;
			ikc->GetKey(i, &key);
      t = key.time;
    }
    else if (cont->ClassID() == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID, 0))
	  {
		  IBezQuatKey key;
		  ikc->GetKey(i, &key);
      t = key.time;
    }
	  else if (cont->ClassID() == Class_ID(LININTERP_SCALE_CLASS_ID, 0))
    {
  	  ILinRotKey key;
		  ikc->GetKey(i, &key);
      t = key.time;
    }
    // sample the node's matrix
    matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
    if (!isBone)
    {
      matrix = matrix * topMatrix;
    }
    decomp_affine(matrix, &ap);

		fprintf(pStream, "%s\t\t%d; 3; %.6f, %.6f, %.6f;;%s\n", 
			indent.data(),
			t / delta,
			ap.k.x, ap.k.z, ap.k.y,
      i == numKeys - 1 ? ";\n" : ",");
	}
  // anim keys close
	fprintf(pStream,"%s\t}\n\n", indent.data());
}
//----------------------------------------------------------------------------

// Not truly the correct way to compare floats of arbitary magnitude...
BOOL EqualPoint3(Point3 p1, Point3 p2)
{
	if (fabs(p1.x - p2.x) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.y - p2.y) > ALMOST_ZERO)
		return FALSE;
	if (fabs(p1.z - p2.z) > ALMOST_ZERO)
		return FALSE;

	return TRUE;
}
//----------------------------------------------------------------------------

// Determine if a TM controller is known by the system.
XsiExp::IsKnownController(Control* cont)
{
	ulong partA, partB;

	if (!cont)
		return FALSE;

	partA = cont->ClassID().PartA();
	partB = cont->ClassID().PartB();

	if (partB != 0x00)
		return FALSE;

	switch (partA) {
		case TCBINTERP_POSITION_CLASS_ID:
		case TCBINTERP_ROTATION_CLASS_ID:
		case TCBINTERP_SCALE_CLASS_ID:
		case HYBRIDINTERP_POSITION_CLASS_ID:
		case HYBRIDINTERP_ROTATION_CLASS_ID:
		case HYBRIDINTERP_SCALE_CLASS_ID:
		case LININTERP_POSITION_CLASS_ID:
		case LININTERP_ROTATION_CLASS_ID:
		case LININTERP_SCALE_CLASS_ID:
			return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------

/*
void XsiExp::DumpPosSample(INode* node, int indentLevel) 
{	
	TSTR indent = GetIndent(indentLevel);
	
	fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_POS_TRACK);

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame() * GetKeyFrameStep();
	Matrix3 matrix;
	AffineParts ap;
	Point3	prevPos;

	for (t=start; t<=end; t+=delta) {
		matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
		decomp_affine(matrix, &ap);

		Point3 pos = ap.t;

		if (t!= start && EqualPoint3(pos, prevPos)) {
			// Skip identical keys 
			continue;
		}

		prevPos = pos;

		// Output the sample
		fprintf(pStream, "%s\t\t\t%s %d\t%s\n",
			indent.data(),
			ID_POS_SAMPLE,
			t,
			Format(pos));
	}

	fprintf(pStream,"%s\t\t}\n", indent.data());
}

void XsiExp::DumpRotSample(INode* node, int indentLevel) 
{	
	TSTR indent = GetIndent(indentLevel);
	
	fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_ROT_TRACK);

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame() * GetKeyFrameStep();
	Matrix3 matrix;
	AffineParts ap;
	Quat prevQ;

	prevQ.Identity();

	for (t=start; t<=end; t+=delta) {
		matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));

		decomp_affine(matrix, &ap);

		// Rotation keys should be relative, so we need to convert these
		// absolute samples to relative values.

		Quat q = ap.q / prevQ;
		prevQ = ap.q;

		if (q.IsIdentity()) {
			// No point in exporting null keys...
			continue;
		}

		// Output the sample
		fprintf(pStream, "%s\t\t\t%s %d\t%s\n",
			indent.data(),
			ID_ROT_SAMPLE,
			t,
			Format(q));
	}

	fprintf(pStream,"%s\t\t}\n", indent.data());
}

void XsiExp::DumpScaleSample(INode* node, int indentLevel) 
{	
	TSTR indent = GetIndent(indentLevel);
	
	fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_SCALE_TRACK);

	TimeValue start = ip->GetAnimRange().Start();
	TimeValue end = ip->GetAnimRange().End();
	TimeValue t;
	int delta = GetTicksPerFrame() * GetKeyFrameStep();
	Matrix3 matrix;
	AffineParts ap;
	Point3	prevFac;

	for (t=start; t<=end; t+=delta) {
		matrix = node->GetNodeTM(t) * Inverse(node->GetParentTM(t));
		decomp_affine(matrix, &ap);

		if (t!= start && EqualPoint3(ap.k, prevFac)) {
			// Skip identical keys 
			continue;
		}

		prevFac = ap.k;

		// Output the sample
		fprintf(pStream, "%s\t\t\t%s %d\t%s %s\n",
			indent.data(),
			ID_SCALE_SAMPLE,
			t,
			Format(ap.k),
			Format(ap.u));
	}

	fprintf(pStream,"%s\t\t}\n", indent.data());
}

// Output point3 keys if this is a known point3 controller that
// supports key operations. Otherwise we will sample the controller 
// once for each frame to get the value.
// Point3 controllers can control, for example, color.
void XsiExp::DumpPoint3Keys(Control* cont, int indentLevel) 
{
	if (!cont)	// Bug out if no controller.
		return;
	
	int i;
	TSTR indent = GetIndent(indentLevel);
	IKeyControl *ikc = NULL;

	// If the user wants us to always sample, we will ignore the KeyControlInterface
	if (!GetAlwaysSample())
		ikc = GetKeyControlInterface(cont);
	
	// TCB point3
	if (ikc && cont->ClassID() == Class_ID(TCBINTERP_POINT3_CLASS_ID, 0)) {
		fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_POINT3_TCB); 
		for (i=0; i<ikc->GetNumKeys(); i++) {
			ITCBPoint3Key key;
			ikc->GetKey(i, &key);
			fprintf(pStream, "%s\t\t\t%s %d\t%s",
				indent.data(),
				ID_TCB_POINT3_KEY,
				key.time,
				Format(key.val));
			// Add TCB specific data
			fprintf(pStream, "\t%s\t%s\t%s\t%s\t%s\n", Format(key.tens), Format(key.cont), Format(key.bias), Format(key.easeIn), Format(key.easeOut));
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
	// Bezier point3
	else if (ikc && cont->ClassID() == Class_ID(HYBRIDINTERP_POINT3_CLASS_ID, 0)) {
		fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_POINT3_BEZIER); 
		for (i=0; i<ikc->GetNumKeys(); i++) {
			IBezPoint3Key key;
			ikc->GetKey(i, &key);
			fprintf(pStream, "%s\t\t\t%s %d\t%s",
				indent.data(),
				ID_BEZIER_POINT3_KEY,
				key.time, 
				Format(key.val));
			fprintf(pStream, "\t%s\t%s\t%d\n", Format(key.intan), Format(key.outtan), key.flags);
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
	// Bezier color
	else if (ikc && cont->ClassID() == Class_ID(HYBRIDINTERP_COLOR_CLASS_ID, 0)) {
		fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_COLOR_BEZIER); 
		for (i=0; i<ikc->GetNumKeys(); i++) {
			IBezPoint3Key key;
			ikc->GetKey(i, &key);
			fprintf(pStream, "%s\t\t\t%s %d\t%s", 
				indent.data(),
				ID_BEZIER_POINT3_KEY,
				key.time,
				Format(key.val));
			fprintf(pStream, "\t%s\t%s\t%d\n", Format(key.intan), Format(key.outtan), key.flags);
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
	else {
		
		// Unknown controller, no key interface or sample on demand -
		// This might be a procedural controller or something else we
		// don't know about. The last resort is to get the value from the 
		// controller at every n frames.
		
		TSTR name;
		cont->GetClassName(name);
		fprintf(pStream,"%s\t\t%s \"%s\" {\n", indent.data(), ID_CONTROL_POINT3_SAMPLE,
			FixupName(name));
		
		// If it is animated at all...
		if (cont->IsAnimated()) {
			// Get the range of the controller animation 
			Interval range; 
			// Get range of full animation
			Interval animRange = ip->GetAnimRange(); 
			TimeValue t = cont->GetTimeRange(TIMERANGE_ALL).Start();
			Point3 value;
			
			// While we are inside the animation... 
			while (animRange.InInterval(t)) {
				// Sample the controller
				range = FOREVER;
				cont->GetValue(t, &value, range);
				
				// Set time to start of controller validity interval 
				t = range.Start();
				
				// Output the sample
				fprintf(pStream, "%s\t\t\t%s %d\t%s\n",
					indent.data(),
					ID_POINT3_KEY,
					t,
					Format(value));
				
				// If the end of the controller validity is beyond the 
				// range of the animation
				if (range.End() > cont->GetTimeRange(TIMERANGE_ALL).End()) {
					break;
				}
				else {
					t = (range.End()/GetTicksPerFrame()+GetKeyFrameStep()) * GetTicksPerFrame();
				}
			}
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
}

// Output float keys if this is a known float controller that
// supports key operations. Otherwise we will sample the controller 
// once for each frame to get the value.
void XsiExp::DumpFloatKeys(Control* cont, int indentLevel) 
{
	if (!cont)
		return;
	
	int i;
	TSTR indent = GetIndent(indentLevel);
	IKeyControl *ikc = NULL;

	// If the user wants us to always sample, we will ignore the KeyControlInterface
	if (!GetAlwaysSample())
		ikc = GetKeyControlInterface(cont);
	
	// TCB float
	if (ikc && cont->ClassID() == Class_ID(TCBINTERP_FLOAT_CLASS_ID, 0)) {
		fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_FLOAT_TCB); 
		for (i=0; i<ikc->GetNumKeys(); i++) {
			ITCBFloatKey key;
			ikc->GetKey(i, &key);
			fprintf(pStream, "%s\t\t\t%s %d\t%s",
				indent.data(),
				ID_TCB_FLOAT_KEY,
				key.time,
				Format(key.val));
			fprintf(pStream, "\t%s\t%s\t%s\t%s\t%s\n", Format(key.tens), Format(key.cont), Format(key.bias), Format(key.easeIn), Format(key.easeOut));
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
	// Bezier float
	else if (ikc && cont->ClassID() == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID, 0)) {
		fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_FLOAT_BEZIER); 
		for (i=0; i<ikc->GetNumKeys(); i++) {
			IBezFloatKey key;
			ikc->GetKey(i, &key);
			fprintf(pStream, "%s\t\t\t%s %d\t%s",
				indent.data(),
				ID_BEZIER_FLOAT_KEY,
				key.time, 
				Format(key.val));
			fprintf(pStream, "\t%s\t%s\t%d\n", Format(key.intan), Format(key.outtan), key.flags);
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
	else if (ikc && cont->ClassID() == Class_ID(LININTERP_FLOAT_CLASS_ID, 0)) {
		fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_FLOAT_LINEAR); 
		for (i=0; i<ikc->GetNumKeys(); i++) {
			ILinFloatKey key;
			ikc->GetKey(i, &key);
			fprintf(pStream, "%s\t\t\t%s %d\t%s\n",
				indent.data(),
				ID_FLOAT_KEY,
				key.time,
				Format(key.val));
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
	else {
		
		// Unknown controller, no key interface or sample on demand -
		// This might be a procedural controller or something else we
		// don't know about. The last resort is to get the value from the 
		// controller at every n frames.
		
		TSTR name;
		cont->GetClassName(name);
		fprintf(pStream,"%s\t\t%s \"%s\" {\n", indent.data(), ID_CONTROL_FLOAT_SAMPLE,
			FixupName(name));
		
		// If it is animated at all...
		if (cont->IsAnimated()) {
			// Get the range of the controller animation 
			Interval range; 
			// Get range of full animation
			Interval animRange = ip->GetAnimRange(); 
			TimeValue t = cont->GetTimeRange(TIMERANGE_ALL).Start();
			float value;
			
			// While we are inside the animation... 
			while (animRange.InInterval(t)) {
				// Sample the controller
				range = FOREVER;
				cont->GetValue(t, &value, range);
				
				// Set time to start of controller validity interval 
				t = range.Start();
				
				// Output the sample
				fprintf(pStream, "%s\t\t\t%s %d\t%s\n",
					indent.data(),
					ID_FLOAT_KEY,
					t,
					Format(value));
				
				// If the end of the controller validity is beyond the 
				// range of the animation
				if (range.End() > cont->GetTimeRange(TIMERANGE_ALL).End()) {
					break;
				}
				else {
					t = (range.End()/GetTicksPerFrame()+GetKeyFrameStep()) * GetTicksPerFrame();
				}
			}
		}
		fprintf(pStream,"%s\t\t}\n", indent.data());
	}
}


void XsiExp::DumpRotKeys(Control* cont, int indentLevel) 
{
	if (!cont)
		return;
	
	int i;
	TSTR indent = GetIndent(indentLevel);
	IKeyControl *ikc = GetKeyControlInterface(cont);

  Interval animRange = GetInterface()->GetAnimRange();
	int delta = GetTicksPerFrame();
  
	if (ikc && cont->ClassID() == Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0))
  {
		int numKeys;
		if (numKeys = ikc->GetNumKeys())
    {
			fprintf(pStream,"%s\tSI_AnimationKey {\n", indent.data()); 
			fprintf(pStream,"%s\t\t0;\n", indent.data());     // 0 means rotation keys
			fprintf(pStream,"%s\t\t%d;\n", indent.data(), numKeys);

  		ITCBRotKey key;
			for (i = 0; i < numKeys-1; i++)
      {
				ikc->GetKey(i, &key);
        Quat q(key.val);

				fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;,\n", 
					indent.data(),
					key.time / delta,
					q.w, q.x, q.z, q.y);
			}
			ikc->GetKey(i, &key);
      Quat q(key.val);

			fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;;\n", 
				indent.data(),
				key.time / delta,
				q.w, q.x, q.z, q.y);

			fprintf(pStream,"%s\t}\n\n", indent.data());
		}
	}
	else if (ikc && cont->ClassID() == Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0))
	{
		int numKeys;
		if (numKeys = ikc->GetNumKeys())
    {
			fprintf(pStream,"%s\tSI_AnimationKey {\n", indent.data()); 
			fprintf(pStream,"%s\t\t0;\n", indent.data());     // 0 means rotation keys
			fprintf(pStream,"%s\t\t%d;\n", indent.data(), numKeys);

			IBezQuatKey key;
			for (i = 0; i < numKeys-1; i++)
      {
				ikc->GetKey(i, &key);
        Quat q(key.val);

				fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;,\n", 
					indent.data(),
					key.time / delta,
					q.w, q.x, q.z, q.y);
			}
			ikc->GetKey(i, &key);
      Quat q(key.val);

			fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;;\n", 
				indent.data(),
				key.time / delta,
				q.w, q.x, q.z, q.y);

			fprintf(pStream,"%s\t}\n\n", indent.data());
		}
	}
	else if (ikc && cont->ClassID() == Class_ID(LININTERP_ROTATION_CLASS_ID, 0))
  {
		int numKeys;
		if (numKeys = ikc->GetNumKeys())
    {
			fprintf(pStream,"%s\tSI_AnimationKey {\n", indent.data()); 
			fprintf(pStream,"%s\t\t0;\n", indent.data());     // 0 means rotation keys
			fprintf(pStream,"%s\t\t%d;\n", indent.data(), numKeys);

  		ILinRotKey key;
			for (i = 0; i < numKeys-1; i++)
      {
				ikc->GetKey(i, &key);
        Quat q(key.val);

				fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;,\n", 
					indent.data(),
					key.time / delta,
					q.w, q.x, q.z, q.y);
			}
			ikc->GetKey(i, &key);
      Quat q(key.val);

			fprintf(pStream, "%s\t\t%d; 4; %.6f, %.6f, %.6f, %.6f;;;\n", 
				indent.data(),
				key.time / delta,
				q.w, q.x, q.z, q.y);

			fprintf(pStream,"%s\t}\n\n", indent.data());
		}
	}
}

void XsiExp::DumpScaleKeys(Control* cont, int indentLevel) 
{
	if (!cont)
		return;
	
	int i;
	TSTR indent = GetIndent(indentLevel);
	IKeyControl *ikc = GetKeyControlInterface(cont);
	
	if (ikc && cont->ClassID() == Class_ID(TCBINTERP_SCALE_CLASS_ID, 0))
	{
		int numKeys;
		if (numKeys = ikc->GetNumKeys()) {
			fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_SCALE_TCB); 
			for (i=0; i<numKeys; i++) {
				ITCBScaleKey key;
				ikc->GetKey(i, &key);
				fprintf(pStream, "%s\t\t\t%s %d\t%s",
					indent.data(),
					ID_TCB_SCALE_KEY,
					key.time,
					Format(key.val));
				fprintf(pStream, "\t%s\t%s\t%s\t%s\t%s\n", Format(key.tens), Format(key.cont), Format(key.bias), Format(key.easeIn), Format(key.easeOut));
			}
			fprintf(pStream,"%s\t\t}\n", indent.data());
		}
	}
	else if (ikc && cont->ClassID() == Class_ID(HYBRIDINTERP_SCALE_CLASS_ID, 0)) {
		int numKeys;
		if (numKeys = ikc->GetNumKeys()) {
			fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_SCALE_BEZIER); 
			for (i=0; i<numKeys; i++) {
				IBezScaleKey key;
				ikc->GetKey(i, &key);
				fprintf(pStream, "%s\t\t\t%s %d\t%s",
					indent.data(),
					ID_BEZIER_SCALE_KEY,
					key.time,
					Format(key.val));
				fprintf(pStream, "\t%s\t%s\t%d\n", Format(key.intan), Format(key.outtan), key.flags);
			}
			fprintf(pStream,"%s\t\t}\n", indent.data());
		}
	}
	else if (ikc && cont->ClassID() == Class_ID(LININTERP_SCALE_CLASS_ID, 0)) {
		int numKeys;
		if (numKeys = ikc->GetNumKeys()) {
			fprintf(pStream,"%s\t\t%s {\n", indent.data(), ID_CONTROL_SCALE_LINEAR); 
			for (i=0; i<numKeys; i++) {
				ILinScaleKey key;
				ikc->GetKey(i, &key);
				fprintf(pStream, "%s\t\t\t%s %d\t%s\n",
					indent.data(),
					ID_SCALE_KEY,
					key.time,
					Format(key.val));
			}
			fprintf(pStream,"%s\t\t}\n", indent.data());
		}
	}
}
*/

