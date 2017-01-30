/*		CodeTree.h - the CodeTree class - parser output
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 *
 */

#ifndef _H_CODETREE
#define _H_CODETREE

class CodeTree : public Value
{
public:
	Value*	fn;				/* the function to apply					*/
	short	count;			/* number of arguments						*/
	Value**	arg_list;		/* the argument list						*/
	long	pos;			/* source stream pos						*/

			CodeTree(CharStream* source, Value* codeFn, ...);
			~CodeTree();

	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

    ScripterExport Value* eval();

	Value*	add(Value* arg1, ...);
	Value*	append(Value* arg);
	Value*	put(int index, Value* arg);
};

/*  Maker class, a special CodeTree node that encodes runtime object 
 *               instantiation.  contains a reference to a maker
 *               static method on the class to be instantiated.  It is
 *				 supplied an arg_list like any other codetree fn apply
 */
 
class Maker : public Value
{
	value_cf maker;			/* the maker class static fn				*/
	short	count;			/* number of arguments						*/
	Value**	arg_list;		/* the argument list						*/

public:
			Maker(value_cf maker_fn, ...);
			Maker(Value** arg_list, int count, value_cf maker_fn);
			~Maker();

	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	ScripterExport Value* eval();

	Value*	add(Value* arg1, ...);
	Value*	append(Value* arg);
};

/* ------------- debugging support classes -------------- */

// SourceFileWrapper wraps a piece of code in a source file
// context.  Evaling this pushes the 'source-file' thread-local,
// evals the wrapped code & pops source-file.

class SourceFileWrapper : public Value
{
public:
	Value*	file_name;
	int		pos;
	Value*	code;

			SourceFileWrapper(Value* file_name, Value* code, int pos = -1);

	void	gc_trace();
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s) { code->sprin1(s); }
	BOOL	_is_function() { return code->_is_function(); }

				   Value* classOf_vf(Value** arg_list, int count) { return code->classOf_vf(arg_list, count); }
				   Value* superClassOf_vf(Value** arg_list, int count) { return code->superClassOf_vf(arg_list, count); }
				   Value* isKindOf_vf(Value** arg_list, int count) { return code->isKindOf_vf(arg_list, count); }
				   BOOL	  is_kind_of(ValueMetaClass* c) { return code->is_kind_of(c); }
    ScripterExport Value* eval();
 	ScripterExport Value* eval_no_wrapper();
    ScripterExport Value* apply(Value** arglist, int count);
};

// SourceLineMarker instances are stuck in code
// as lines are change

class SourceLineMarker : public Value
{
public:
	int		pos;

			SourceLineMarker(int pos);

	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s) { s->printf(_T("<line %d>"), pos); }

    ScripterExport Value* eval() { thread_local(source_pos) = pos; return &ok; }
};

// top-level code block, provides a storage for top-level locals

class CodeBlock : public Value
{
public:
	Value*		code;			// the code
	Value**		locals;			// local var array	
	int			local_count;	//   "    "  count	

				CodeBlock() { tag = INTERNAL_CODEBLOCK_TAG; code = NULL; locals = NULL; local_count = 0; }
	void		init(Value* code) { this->code = code; }
			   ~CodeBlock();

	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s);

	void		add_local();
	Value*		eval();
};

#endif
