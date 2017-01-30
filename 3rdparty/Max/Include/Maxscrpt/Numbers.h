/*		Numbers.h - the number family of classes - numbers for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_NUMBER
#define _H_NUMBER

#include "MSTime.h"

#define FLOAT_CACHE_SIZE	1024	// must be power of 2
#define INT_CACHE_SIZE		512		//   "       "       "
#define LOW_INT_RANGE		100

class Float;
class Integer;

extern Float* float_cache[];
extern Integer* int_cache[];

visible_class (Number)

class Number : public Value
{
public:
#	define  is_number(o) ((o)->tag == class_tag(Float) || (o)->tag == class_tag(Integer))
			classof_methods (Number, Value);

	static	Value* read(TCHAR* str);
	static	void setup();
	
#include "defimpfn.h"
	def_generic( coerce,	"coerce");
	def_generic( copy,		"copy");
};

visible_class (Float)
visible_class (Integer)		// forward decls for float class 
#define  is_integer(o) ((o)->tag == class_tag(Integer))

class Float : public Number
{
public:
	float value;

			Float() { }
			Float(float init_val);

	static ScripterExport Value* intern(float init_val);

			classof_methods (Float, Number);
#	define  is_float(o) ((o)->tag == class_tag(Float))
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "defimpfn.h"
#	include "mathpro.h"

	float	to_float() { return value; }
	int		to_int() { return (int)value; }
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames

	Value*	widen_to(Value* arg, Value** arg_list);
	BOOL	comparable(Value* arg) { return (is_integer(arg) || is_float(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class Integer : public Number
{
public:
	int value;

			Integer() { };
			Integer(int init_val);

	static  ScripterExport Value* intern(int init_val);

			classof_methods (Integer, Number);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	/* include all the protocol declarations */

#include "defimpfn.h"
#	include "mathpro.h"

	float	to_float() { return (float)value; }
	int		to_int() { return value; }
	TimeValue to_timevalue() { return (TimeValue)(value * GetTicksPerFrame()); }  // numbers used as times are in frames

	Value*	widen_to(Value* arg, Value** arg_list);
	BOOL	comparable(Value* arg) { return (is_integer(arg) || is_float(arg) || is_time(arg)); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

#endif
