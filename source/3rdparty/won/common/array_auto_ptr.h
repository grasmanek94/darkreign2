#ifndef ARRAYAUTOPTR_H
#define ARRAYAUTOPTR_H

namespace WONCommon {

template<class _Ty> class array_auto_ptr {
public:
	typedef _Ty element_type;
	explicit array_auto_ptr(_Ty *_P = 0) throw()
		: _Owns(_P != 0), _Ptr(_P) {}
	array_auto_ptr(const array_auto_ptr<_Ty>& _Y) throw()
		: _Owns(_Y._Owns), _Ptr(_Y.release()) {}
	array_auto_ptr<_Ty>& operator=(const array_auto_ptr<_Ty>& _Y) throw()
		{if (this != &_Y)
			{if (_Ptr != _Y.get())
				{if (_Owns)
					delete _Ptr;
				_Owns = _Y._Owns; }
			else if (_Y._Owns)
				_Owns = true;
			_Ptr = _Y.release(); }
		return (*this); }
	~array_auto_ptr()
		{if (_Owns)
			delete [] _Ptr; }
	_Ty& operator*() const throw()
		{return (*get()); }
	_Ty *operator->() const throw()
		{return (get()); }
	_Ty *get() const throw()
		{return (_Ptr); }
	_Ty *release() const throw()
		{((array_auto_ptr<_Ty> *)this)->_Owns = false;
		return (_Ptr); }
private:
	bool _Owns;
	_Ty *_Ptr;
};

}; // namespace WONCommon

#endif // ARRAYAUTOPTR_H