/*	
 *		MAX_node_protocol.h - def_generics for the operations on MAX node objects
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
 
/* node transforms */

	def_node_generic(move,			"move");
	def_node_generic(scale,			"scale");
	def_node_generic(rotate,		"rotate");

/* node ops */

	def_node_generic(copy,			"copy");
	def_node_generic(reference,		"reference");
	def_node_generic(instance,		"instance");
	def_visible_primitive(delete,	"delete");		// a prim so I can control collection traversal
	def_node_generic(isDeleted,		"isDeleted");
	def_node_generic(addModifier,	"addModifier");
	def_node_generic(deleteModifier, "deleteModifier");
	def_node_generic(collapseStack, "collapseStack");
	def_node_generic(bindSpaceWarp, "bindSpaceWarp");
	def_node_generic(intersects,	"intersects");
//	def_node_generic(dependsOn,		"dependsOn");

	def_node_generic       (instanceReplace,		"instanceReplace");
	def_node_generic       (referenceReplace,		"referenceReplace");
	def_node_generic       (snapShot,				"snapShot");

	def_visible_generic    (getModContextTM,		"getModContextTM");
	def_visible_generic    (getModContextBBoxMin,	"getModContextBBoxMin");
	def_visible_generic    (getModContextBBoxMax,	"getModContextBBoxMax");
	def_visible_primitive  (validModifier,			"validModifier");
	def_visible_generic    (canConvertTo,			"canConvertTo");
	def_node_generic       (convertTo,				"convertTo");
	def_node_generic       (flagForeground,			"flagForeground");

/* node state */
	
	def_node_generic(hide,			"hide");
	def_node_generic(unhide,		"unhide");
	def_node_generic(unfreeze,		"unfreeze");
	def_node_generic(freeze,		"freeze");
	def_node_generic(select,		"select");
	def_node_generic(deselect,		"deselect");
	def_visible_primitive(clearSelection, "clearSelection");
	def_node_generic(selectmore,	"selectmore");
	def_visible_primitive(group,	"group");
	def_visible_primitive(ungroup,	"ungroup");
	def_visible_primitive(explodeGroup,	"explodeGroup");

/* object xrefs */
	def_visible_generic	( updateXRef,	"updateXRef");

/* NURBS */

	def_node_generic	( getNURBSSet,		"getNURBSSet");
	def_node_generic	( addNURBSSet,		"addNURBSSet");
	use_generic			( transform,		"transform" );
	def_node_generic	( breakCurve,		"breakCurve" );
	def_node_generic	( breakSurface,		"breakSurface" );
	def_node_generic	( joinCurves,		"joinCurves" );
	def_node_generic	( joinSurfaces,		"joinSurfaces" );
	def_node_generic	( makeIndependent,	"makeIndependent" );
	def_node_generic    ( convertToNURBSCurve,		"convertToNURBSCurve" );
	def_node_generic    ( convertToNURBSSurface,	"convertToNURBSSurface" );
	def_node_generic    ( setViewApproximation,		"setViewApproximation" );
	def_node_generic    ( setRenderApproximation,	"setRenderApproximation" );
	def_node_generic    ( setSurfaceDisplay,		"setSurfaceDisplay" );

/* utilities */
	
	def_node_generic(intersectRay,		"intersectRay");
	def_node_generic(printstack,		"printstack");
	def_visible_primitive(uniqueName,	"uniqueName");

/* user prop access */
	
	def_node_generic( getUserProp,		"getUserProp");
	def_node_generic( setUserProp,		"setUserProp");
	def_node_generic( getUserPropBuffer, "getUserPropBuffer");
	def_node_generic( setUserPropBuffer, "setUserPropBuffer");
	
/* mesh ops -- if baseobject is a mesh */

	use_generic     ( plus,			"+" );		// mesh boolean ops
	use_generic     ( minus,		"-" );
	use_generic     ( times,		"*" );

	def_node_generic( convertToMesh, "convertToMesh");  // this works on those things convertable to meshes

	def_visible_generic( setvert,		 "setvert");
	def_visible_generic( getvert,		 "getvert");
	def_visible_generic( settvert,		 "settvert");
	def_visible_generic( gettvert,		 "gettvert");
	def_visible_generic( setvertcolor,	 "setvertcolor");
	def_visible_generic( getvertcolor,	 "getvertcolor");
	def_visible_generic( setnumverts,	 "setnumverts");
	def_visible_generic( getnumverts,	 "getnumverts");
	def_visible_generic( setnumtverts,	 "setnumtverts");
	def_visible_generic( getnumtverts,	 "getnumtverts");
	def_visible_generic( setnumcpvverts, "setnumcpvverts");
	def_visible_generic( getnumcpvverts, "getnumcpvverts");
	def_visible_generic( setnumfaces,	 "setnumfaces");
	def_visible_generic( getnumfaces,	 "getnumfaces");
	def_visible_generic( buildtvfaces,   "buildTVFaces");
	def_visible_generic( buildvcfaces,   "buildVCFaces");
	def_visible_generic( defaultvcfaces, "defaultVCFaces");
	def_visible_generic( getnormal,		 "getnormal");
	def_visible_generic( setnormal,		 "setnormal");
	def_visible_generic( setface,		 "setface");
	def_visible_generic( getface,		 "getface");
	def_visible_generic( settvface,		 "setTVFace");
	def_visible_generic( gettvface,		 "getTVFace");
	def_visible_generic( setvcface,		 "setVCFace");
	def_visible_generic( getvcface,		 "getVCFace");
	def_visible_generic( getfacenormal,  "getfacenormal");
	def_visible_generic( setfacenormal,  "setfacenormal");
	def_visible_generic( setfacematid,	 "setfaceMatID");
	def_visible_generic( getfacematid,	 "getfaceMatID");
	def_visible_generic( setfacesmoothgroup, "setfaceSmoothGroup");
	def_visible_generic( getfacesmoothgroup, "getfaceSmoothGroup");
	def_visible_generic( setedgevis,	 "setedgevis");
	def_visible_generic( getedgevis,	 "getedgevis");
	def_visible_generic( attach,	     "attach");
	def_visible_generic( detachVerts,	 "detachVerts");
	def_visible_generic( detachFaces,	 "detachFaces");
	def_visible_generic( extrudeface,	 "extrudeface");
	def_visible_generic( deletevert,	 "deletevert");
	def_visible_generic( deleteface,	 "deleteface");
	def_visible_generic( collapseface,	 "collapseface");
	def_visible_generic( setMesh,		 "setMesh");
	def_visible_generic( update,		 "update");
	def_visible_generic( getDisplacementMapping, "getDisplacementMapping");
	def_visible_generic( setDisplacementMapping, "setDisplacementMapping");
	def_visible_generic( getSubdivisionDisplacement, "getSubdivisionDisplacement");
	def_visible_generic( setSubdivisionDisplacement, "setSubdivisionDisplacement");
	def_visible_generic( getSplitMesh,		"getSplitMesh");
	def_visible_generic( setSplitMesh,		"setSplitMesh");
	def_visible_generic( displacementToPreset, "displacementToPreset" );

	def_node_generic( getVertSelection,	 "getVertSelection");  // getVertSelection <node> <nodemodifier>
	def_node_generic( setVertSelection,	 "setVertSelection"); 
	def_node_generic( getFaceSelection,	 "getFaceSelection");  
	def_node_generic( setFaceSelection,	 "setFaceSelection");  
	def_node_generic( getEdgeSelection,	 "getEdgeSelection");  
	def_node_generic( setEdgeSelection,	 "setEdgeSelection");  

	def_struct_primitive( mo_startCreate,		meshOps, "startCreate");
	def_struct_primitive( mo_startAttach,		meshOps, "startAttach");
	def_struct_primitive( mo_startExtrude,		meshOps, "startExtrude");
	def_struct_primitive( mo_startBevel,		meshOps, "startBevel");
	def_struct_primitive( mo_startChamfer,		meshOps, "startChamfer");
	def_struct_primitive( mo_startCut,			meshOps, "startCut");
	def_struct_primitive( mo_startSlicePlane,	meshOps, "startSlicePlane");
	def_struct_primitive( mo_startWeldTarget,	meshOps, "startWeldTarget");
	def_struct_primitive( mo_startFlipNormalMode, meshOps, "startFlipNormalMode");
	def_struct_primitive( mo_startDivide,		meshOps, "startDivide");
	def_struct_primitive( mo_startTurn,			meshOps, "startTurn");
	def_struct_primitive( mo_hideOp,			meshOps, "hide");
	def_struct_primitive( mo_unhideAllOp,		meshOps, "unhideAll");
	def_struct_primitive( mo_deleteOp,			meshOps, "delete");
	def_struct_primitive( mo_detachOp,			meshOps, "detach");
	def_struct_primitive( mo_weldOp,			meshOps, "weld");
	def_struct_primitive( mo_breakOp,			meshOps, "break");
	def_struct_primitive( mo_viewAlignOp,		meshOps, "viewAlign");
	def_struct_primitive( mo_gridAlignOp,		meshOps, "gridAlign");
	def_struct_primitive( mo_makePlanarOp,		meshOps, "makePlanar");
	def_struct_primitive( mo_collapseOp,		meshOps, "collapse");
	def_struct_primitive( mo_tesselateOp,		meshOps, "tessellate");
	def_struct_primitive( mo_explodeOp,			meshOps, "explode");
	def_struct_primitive( mo_sliceOp,			meshOps, "slice");
	def_struct_primitive( mo_removeIsolatedVertsOp,	meshOps, "removeIsolatedVerts");
	def_struct_primitive( mo_selectOpenEdgesOp,	meshOps, "selectOpenEdges");
	def_struct_primitive( mo_createShapeFromEdgesOp,meshOps, "createShapeFromEdges");
	def_struct_primitive( mo_flipNormalOp,		meshOps, "flipNormal");
	def_struct_primitive( mo_unifyNormalOp,		meshOps, "unifyNormal");
	def_struct_primitive( mo_visibleEdgeOp,		meshOps, "visibleEdge");
	def_struct_primitive( mo_invisibleEdgeOp,	meshOps, "invisibleEdge");
	def_struct_primitive( mo_autoEdgeOp,		meshOps, "autoEdge");

/* shape ops -- if baseobject is a shape */

	def_node_generic( pathinterp,			"pathInterp");			// MAX path interpolation (subcurve piecewise)
	def_node_generic( lengthinterp,			"lengthInterp");		// total arclength interpolation
	def_visible_primitive( resetlengthinterp, "resetLengthInterp");  // clear length interp caches
	def_node_generic( curvelength,			"curveLength");			// total arclength interpolation
	def_node_generic( nearestpathparam,		"nearestPathParam");    // path 'u' param at nearest point along curve to obj
	def_node_generic( pathtolengthparam,	"pathToLengthParam");  // give length 'u' param from path 'u' param
	def_node_generic( lengthtopathparam,	"lengthToPathParam");  // give path 'u' param from length 'u' param
	def_node_generic( pathtangent,			"pathTangent");			// MAX path interpolation tangent (subcurve piecewise)
	def_node_generic( lengthtangent,		"lengthTangent");		// total arclength interpolation tangent

/* bezier shape ops -- if baseobject is a bezier shape */

	def_node_generic( convertToSplineShape,	"convertToSplineShape");  // this works on those things convertable to splineshapes
	
	def_node_generic( addNewSpline,		"addNewSpline");		
	def_node_generic( deleteSpline,		"deleteSpline");		
	def_node_generic( numSplines,		"numSplines");
	def_node_generic( setFirstSpline,	"setFirstSpline");
	def_node_generic( resetShape,		"resetShape");
	def_node_generic( updateShape,		"updateShape");
	
	def_node_generic( numKnots,			"numKnots");		
	def_node_generic( numSegments,		"numSegments");		
	def_node_generic( isClosed,			"isClosed");		
	use_generic     ( close,			"close");		
	def_node_generic( open,				"open");		
	def_node_generic( addKnot,			"addKnot");		
	def_node_generic( deleteKnot,		"deleteKnot");		
	def_node_generic( setKnotType,		"setKnotType");		
	def_node_generic( getKnotType,		"getKnotType");		
	def_node_generic( setSegmentType,	"setSegmentType");		
	def_node_generic( getSegmentType,	"getSegmentType");		
	def_node_generic( refineSegment,	"refineSegment");		
	def_node_generic( reverse,			"reverse");		
	def_node_generic( setFirstKnot,		"setFirstKnot");		
	def_node_generic( setKnotPoint,		"setKnotPoint");		
	def_node_generic( getKnotPoint,		"getKnotPoint");		
	def_node_generic( getInVec,			"getInVec");		
	def_node_generic( setInVec,			"setInVec");		
	def_node_generic( getOutVec,		"getOutVec");		
	def_node_generic( setOutVec,		"setOutVec");
	def_node_generic( hideSelectedVerts,"hideSelectedVerts");
	def_node_generic( hideSelectedSplines,	"hideSelectedSplines");
	def_node_generic( hideSelectedSegments,  "hideSelectedSegments");
	def_node_generic( unhideSegments,	"unhideSegments");
	def_node_generic( updateBindList,	"updateBindList");
	def_node_generic( unbindKnot,		"unbindKnot");
	def_node_generic( bindKnot,			"bindKnot");
	def_node_generic( materialID,		"materialID");
	def_node_generic( addAndWeld,		"addAndWeld");


	def_visible_primitive( getKnotSelection,   "getKnotSelection");  // getKnotSelection <node> works only for editable splines
	def_visible_primitive( setKnotSelection,   "setKnotSelection"); 
	def_visible_primitive( getSegSelection,	   "getSegSelection");  
	def_visible_primitive( setSegSelection,	   "setSegSelection");  
	def_visible_primitive( getSplineSelection, "getSplineSelection");  
	def_visible_primitive( setSplineSelection, "setSplineSelection");  

	def_struct_primitive( so_startCreateLine,	splineOps, "startCreateLine");
	def_struct_primitive( so_startAttach,		splineOps, "startAttach");
	def_struct_primitive( so_startInsert,		splineOps, "startInsert");
	def_struct_primitive( so_startConnect,		splineOps, "startConnect");
	def_struct_primitive( so_startRefine,		splineOps, "startRefine");
	def_struct_primitive( so_startFillet,		splineOps, "startFillet");
	def_struct_primitive( so_startChamfer,		splineOps, "startChamfer");
	def_struct_primitive( so_startBind,			splineOps, "startBind");
	def_struct_primitive( so_startRefineConnect,splineOps, "startRefineConnect");
	def_struct_primitive( so_startOutline,		splineOps, "startOutline");
	def_struct_primitive( so_startTrim,			splineOps, "startTrim");
	def_struct_primitive( so_startExtend,		splineOps, "startExtend");
	def_struct_primitive( so_startCrossInsert,	splineOps, "startCrossInsert");
	def_struct_primitive( so_startBreak,		splineOps, "startBreak");
	def_struct_primitive( so_startUnion,		splineOps, "startUnion");
	def_struct_primitive( so_startSubtract,		splineOps, "startSubtract");

	def_struct_primitive( so_opHide,			splineOps, "hide");
	def_struct_primitive( so_opUnhideAll,		splineOps, "unhideAll");
	def_struct_primitive( so_opDelete,			splineOps, "delete");
	def_struct_primitive( so_opDetach,			splineOps, "detach");
	def_struct_primitive( so_opDivide,			splineOps, "divide");
	def_struct_primitive( so_opCycle,			splineOps, "cycle");
	def_struct_primitive( so_opUnbind,			splineOps, "unbind");
	def_struct_primitive( so_opWeld,			splineOps, "weld");
	def_struct_primitive( so_opMakeFirst,		splineOps, "makeFirst");
	def_struct_primitive( so_opAttachMultiple,	splineOps, "attachMultiple");
	def_struct_primitive( so_opExplode,			splineOps, "explode");
	def_struct_primitive( so_opReverse,			splineOps, "reverse");
	def_struct_primitive( so_opClose,			splineOps, "close");
	def_struct_primitive( so_opIntersect,		splineOps, "intersect");
	def_struct_primitive( so_opMirrorHoriz,		splineOps, "mirrorHoriz");
	def_struct_primitive( so_opMirrorVert,		splineOps, "mirrorVert");
	def_struct_primitive( so_opMirrorBoth,		splineOps, "mirrorBoth");
	def_struct_primitive( so_opSelectByID,		splineOps, "selectByID");

/* particle ops -- if baseobject is a particle system */

	def_node_generic( particlecount,	"particleCount");		
	def_node_generic( particlepos,		"particlePos");		
	def_node_generic( particlevelocity,	"particleVelocity");		
	def_node_generic( particleage,		"particleAge");	
	def_node_generic( particlesize,		"particleSize");

/* patch ops */

	def_struct_primitive( po_startAttach,		patchOps, "startAttach");
	def_struct_primitive( po_startExtrude,		patchOps, "startExtrude");
	def_struct_primitive( po_startBevel,		patchOps, "startBevel");
	def_struct_primitive( po_startBind,			patchOps, "startBind");

	def_struct_primitive( po_opUnbind,			patchOps, "unbind");
	def_struct_primitive( po_opHide,			patchOps, "hide");
	def_struct_primitive( po_opUnhideAll,		patchOps, "unhideAll");
	def_struct_primitive( po_opWeld,			patchOps, "weld");
	def_struct_primitive( po_opDelete,			patchOps, "delete");
	def_struct_primitive( po_opSubdivide,		patchOps, "subdivide");
	def_struct_primitive( po_opAddTri,			patchOps, "addTri");
	def_struct_primitive( po_opAddQuad,			patchOps, "addQuad");
	def_struct_primitive( po_opDetach,			patchOps, "detach");
