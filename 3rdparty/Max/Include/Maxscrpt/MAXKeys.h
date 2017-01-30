/*	
 *		MAXKeys.h - MAX controller keyframe access classes
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_MAXKEYS
#define _H_MAXKEYS

#include "Max.h"
#include "MaxObj.h"

visible_class (MAXKeyArray)

class MAXKeyArray : public MAXWrapper
{
public:
	Control*		 controller;		/* the controller							*/
	ParamDimension*  dim;				/* dimension from originating animatable	*/
	IKeyControl*	 ik;	

				MAXKeyArray(Control* icont, ParamDimension* idim);
	static ScripterExport Value* intern(Control* icont, ParamDimension* idim);

				classof_methods (MAXKeyArray, MAXWrapper);
	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name();

	/* operations */
	
#include "defimpfn.h"
#	include "arraypro.h"
	def_generic ( sortKeys,			"sortKeys");
	def_generic ( addNewKey,		"addNewKey");
	def_generic ( deleteKeys,		"deleteKeys");
	def_generic ( deleteKey,		"deleteKey");
	def_generic (show_props,		"showProperties");
	def_generic (get_props,			"getPropNames");

	ScripterExport Value* map(node_map& m);

	/* built-in property accessors */

	def_property ( count );
};

typedef union AnyKey AnyKey;
union AnyKey
{
	TCHAR lfk[sizeof ILinFloatKey];
	TCHAR lp3k[sizeof ILinPoint3Key];
	TCHAR lrk[sizeof ILinRotKey];
	TCHAR lsk[sizeof ILinScaleKey];
	TCHAR bfk[sizeof IBezFloatKey];
	TCHAR bp3k[sizeof IBezPoint3Key];
	TCHAR bqk[sizeof IBezQuatKey];
	TCHAR bsk[sizeof IBezScaleKey];
	TCHAR tfk[sizeof ITCBFloatKey];
	TCHAR tp3k[sizeof ITCBPoint3Key];
	TCHAR trk[sizeof ITCBRotKey];
	TCHAR tsk[sizeof ITCBScaleKey];
};

#define ToTCBUI(a) (((a)+1.0f)*25.0f)  // HEY!! pinched from TCBINTRP.CPP, why not in a header or documented?
#define FromTCBUI(a) (((a)/25.0f)-1.0f)
#define ToEaseUI(a) ((a)*50.0f)
#define FromEaseUI(a) ((a)/50.0f)

visible_class (MAXKey)

class MAXKey : public MAXWrapper
{
public:
	Control*	controller;			/* MAX-side controller						*/
	ParamDimension* dim;			/* dimension from originating animatable	*/
	int			key_index;

	ScripterExport MAXKey (Control* icont, int ikey, ParamDimension* dim);
	ScripterExport MAXKey (Control* icont, int ikey);
	static ScripterExport Value* intern(Control* icont, int ikey, ParamDimension* dim);
	static ScripterExport Value* intern(Control* icont, int ikey);

	static void setup();

				classof_methods (MAXKey, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name();

	def_generic ( delete,		"delete");
	def_generic ( copy,			"copy");
	def_generic	( show_props,	"showProperties");
	def_generic ( get_props,	"getPropNames");

	ScripterExport IKey* setup_key_access(AnyKey& ak, IKeyControl** kip);

	def_property	(time);
	def_property	(selected);
	def_property	(value);
	def_property	(inTangent);
	def_property	(outTangent);
	def_property	(inTangentType);
	def_property	(outTangentType);
	def_property	(x_locked);
	def_property	(y_locked);
	def_property	(z_locked);
	def_property	(constantVelocity);
	def_property	(tension);
	def_property	(continuity);
	def_property	(bias);
	def_property	(easeTo);
	def_property	(easeFrom);

#if 0 // HEY!! obsolete
	def_nested_prop	( angle );
	def_nested_prop	( x_rotation );
	def_nested_prop	( y_rotation );
	def_nested_prop	( z_rotation );
	def_nested_prop	( axis );
	def_nested_prop	( x );
	def_nested_prop	( y );
	def_nested_prop	( z );
#endif

// add implementations of the recursive time controller fns here to complain
// since they wuill default to operating on the key's controller which can be very confusing
// the user should use them on the controller or track, not the key
#ifdef def_time_fn
#	undef def_time_fn
#endif
#define def_time_fn(_fn)									\
	Value* MAXKey::_fn##_vf(Value** arg_list, int count) { ABSTRACT_FUNCTION(#_fn, this, Value*); }	 
#include "time_fns.h"

};

#endif
