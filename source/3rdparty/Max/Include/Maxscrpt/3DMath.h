/*		3DMath.h - the 3D math family of classes - vectors, rays, quat, matrices for MAXScript
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_3DMATH
#define _H_3DMATH

#include "Max.h"

extern ScripterExport void _QuatToEuler(Quat &q, float *ang);
extern ScripterExport void _EulerToQuat(float *ang, Quat &q);

/* ------------------------ Point3Value ------------------------------ */

applyable_class (Point3Value)

class Point3Value : public Value
{
public:
	Point3		p;

 ScripterExport Point3Value(Point3 init_point);
 ScripterExport Point3Value(float x, float y, float z);
 ScripterExport Point3Value(Value* x, Value* y, Value* z);

				classof_methods(Point3Value, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
#	define		is_point3(p) ((p)->tag == class_tag(Point3Value))

	static Value* make(Value**arg_list, int count);
	
	/* operations */

#include "defimpfn.h"
#	include "vectpro.h"
	use_generic  ( coerce,	"coerce");
	use_generic  ( copy,	"copy");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );
	def_property ( z );

	Point3		to_point3() { return p; }
	AColor		to_acolor() { return AColor (p.x / 255.0f, p.y / 255.0f, p.z / 255.0f); }
	Point2		to_point2() { return Point2 (p.x, p.y); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

class ConstPoint3Value : public Point3Value
{
public:
 ScripterExport ConstPoint3Value(float x, float y, float z)
	 : Point3Value (x, y, z) { }

	void		collect() { delete this; }
	BOOL		is_const() { return TRUE; }

	Value* set_x(Value** arg_list, int count) { throw RuntimeError (_T("Constant vector, not settable")); }
	Value* set_y(Value** arg_list, int count) { throw RuntimeError (_T("Constant vector, not settable")); }
	Value* set_z(Value** arg_list, int count) { throw RuntimeError (_T("Constant vector, not settable")); }
};

/* ------------------------ RayValue ------------------------------ */

applyable_class (RayValue)

class RayValue : public Value
{
public:
	Ray			r;

 ScripterExport RayValue(Point3 init_origin, Point3 init_dir);
 ScripterExport RayValue(Ray init_ray);

				classof_methods (RayValue, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
	use_generic  ( copy,	"copy");

	/* built-in property accessors */

	def_property ( pos );
	def_property_alias ( position, pos );
	def_property ( dir );

	Ray			to_ray() { return r; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ QuatValue ------------------------------ */

applyable_class (QuatValue)

class QuatValue : public Value
{
public:
	Quat		q;

 ScripterExport QuatValue(const Quat& init_quat);
 ScripterExport QuatValue(float w, float x, float y, float z);
 ScripterExport QuatValue(Value* w, Value* x, Value* y, Value* z);
 ScripterExport QuatValue(Value* val);
 ScripterExport QuatValue(AngAxis& aa);
 ScripterExport QuatValue(float* angles);
 ScripterExport QuatValue(Matrix3& m);

				classof_methods (QuatValue, Value);
#	define		is_quat(o) ((o)->tag == class_tag(QuatValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */

#include "defimpfn.h"
#	include "quatpro.h"
	use_generic  ( copy,	"copy");

	/* built-in property accessors */

	def_property ( w );
	def_property ( x );
	def_property ( y );
	def_property ( z );
	def_property ( angle );
	def_property ( axis );

	Quat		to_quat() { return q; }
	AngAxis		to_angaxis() { return AngAxis(q); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ AngleAxis ------------------------------ */

applyable_class (AngAxisValue)

class AngAxisValue : public Value
{
public:
	AngAxis		aa;

 ScripterExport AngAxisValue(const AngAxis& iaa);
 ScripterExport AngAxisValue(const Quat& q);
 ScripterExport AngAxisValue(const Matrix3& m);
 ScripterExport AngAxisValue(float* angles);
 ScripterExport AngAxisValue(float angle, Point3 axis);
 ScripterExport AngAxisValue(Value*);
 ScripterExport AngAxisValue(Value* angle, Value* axis);

				classof_methods (AngAxisValue, Value);
#	define		is_angaxis(o) ((o)->tag == class_tag(AngAxisValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
#include "defimpfn.h"

	use_generic( coerce,	"coerce" );
	use_generic( eq,		"=");
	use_generic( ne,		"!=");
	use_generic( random,	"random");
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( angle );
	def_property ( axis );
	def_property ( numrevs );

	AngAxis		to_angaxis() { return aa; }
	Quat		to_quat() { return Quat (aa); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ EulerAngles ------------------------------ */

applyable_class (EulerAnglesValue)

class EulerAnglesValue : public Value
{
public:
	float		angles[3];

 ScripterExport EulerAnglesValue(float ax, float ay, float az);
 ScripterExport EulerAnglesValue(const Quat&);
 ScripterExport EulerAnglesValue(const Matrix3&);
 ScripterExport EulerAnglesValue(const AngAxis&);

				classof_methods (EulerAnglesValue, Value);
#	define		is_eulerangles(o) ((o)->tag == class_tag(EulerAnglesValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	/* operations */
	
#include "defimpfn.h"

	use_generic( coerce,	"coerce" );
	use_generic( eq,		"=");
	use_generic( ne,		"!=");
	use_generic( random,	"random");
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );
	def_property ( z );
	def_property ( x_rotation );
	def_property ( y_rotation );
	def_property ( z_rotation );

	AngAxis		to_angaxis() { return AngAxis (to_quat()); }
	Quat		to_quat() { Quat q; _EulerToQuat(angles, q); return Quat (q); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Matrix ------------------------------ */

applyable_class (Matrix3Value)

class Matrix3Value : public Value
{
public:
	Matrix3		m;

 ScripterExport Matrix3Value(int i);
 ScripterExport Matrix3Value(const Matrix3& im);
 ScripterExport Matrix3Value(const Quat& q);
 ScripterExport Matrix3Value(const AngAxis& aa);
 ScripterExport Matrix3Value(float* angles);
 ScripterExport Matrix3Value(const Point3& row0, const Point3& row1, const Point3& row2, const Point3& row3);

				classof_methods (Matrix3Value, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
#	define		is_matrix3(p) ((p)->tag == class_tag(Matrix3Value))

	/* operations */
	
#include "defimpfn.h"
#	include "matpro.h"
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( row1 );
	def_property ( row2 );
	def_property ( row3 );
	def_property ( row4 );
	def_property ( translation );
	def_property ( pos );
	def_property ( rotation );
	def_property ( scale );


	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	Matrix3&	to_matrix3() { return m; }
	Quat		to_quat() { return Quat (m); }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ------------------------ Point2Value ------------------------------ */

applyable_class (Point2Value)

class Point2Value : public Value
{
public:
	Point2		p;

	ScripterExport Point2Value(Point2 ipoint);
	ScripterExport Point2Value(POINT ipoint);
	ScripterExport Point2Value(float x, float y);
	ScripterExport Point2Value(Value* x, Value* y);

				classof_methods(Point2Value, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
#	define		is_point2(p) ((p)->tag == class_tag(Point2Value))

	static Value* make(Value**arg_list, int count);
	
	/* operations */

#include "defimpfn.h"
	use_generic( plus,		"+" );
	use_generic( minus,		"-" );
	use_generic( times,		"*" );
	use_generic( div,		"/" );
	use_generic( eq,		"=");
	use_generic( ne,		"!=");
	use_generic( random,	"random");
	use_generic( length,	"length");
	use_generic( distance,	"distance");
	use_generic( normalize,	"normalize");
	use_generic( copy,		"copy");

	/* built-in property accessors */

	def_property ( x );
	def_property ( y );

	Point2		to_point2() { return p; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

#endif
