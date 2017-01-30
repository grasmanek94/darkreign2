///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// area.h
//
// 16-MAY-2000
//

#ifndef __AREA_H
#define __AREA_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes 
//
#pragma warning(push, 3)
#include <iomanip>
#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////
//
// Template Point
//
template <class TYPE> struct Point
{
  TYPE x;
  union
  {
    TYPE y;
    TYPE z;
  };

  // Constructor
  Point() { };
  Point(TYPE x, TYPE y) : x(x), y(y) { }

  // Addition operator
  Point &operator+=(const Point &rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return (*this);
  }
  Point operator+(const Point &rhs) const
  {
    Point tmp(*this);
    tmp += rhs;
    return (tmp);
  }

  // Subtraction operator
  const Point &operator-=(const Point &rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return (*this);
  }
  Point operator-(const Point &rhs) const
  {
    Point tmp(*this);
    tmp -= rhs;
    return (tmp);
  }

  // Scalar Addition operator
  const Point &operator+=(const TYPE &rhs)
  {
    x += rhs;
    y += rhs;
    return (*this);
  }
  Point operator+(const TYPE &rhs) const
  {
    Point tmp(*this);
    tmp += rhs;
    return (tmp);
  }

  // Scalar Subtraction operator
  const Point &operator-=(const TYPE &rhs)
  {
    x -= rhs;
    y -= rhs;
    return (*this);
  }
  Point operator-(const TYPE &rhs) const
  {
    Point tmp(*this);
    tmp -= rhs;
    return (tmp);
  }

  // Scalar Multiplication operator
  const Point &operator*=(const TYPE &rhs)
  {
    x *= rhs;
    y *= rhs;
    return (*this);
  }
  Point operator*(const TYPE &rhs) const
  {
    Point tmp(*this);
    tmp *= rhs;
    return (tmp);
  }

  // Scalar Division operator
  const Point &operator/=(const TYPE &rhs)
  {
    x /= rhs;
    y /= rhs;
    return (*this);
  }
  Point operator/(const TYPE &rhs) const
  {
    Point tmp(*this);
    tmp /= rhs;
    return (tmp);
  }

  // Relational Operators
  friend Bool operator==(const Point<TYPE> &p1, const Point<TYPE> &p2)
  {
    return (p1.x == p2.x && p1.y == p2.y);
  }
  friend Bool operator!=(const Point<TYPE> &p1, const Point<TYPE> &p2)
  {
    return (p1.x != p2.x || p1.y != p2.y);
  }
  friend Bool operator>(const Point<TYPE> &p1, const Point<TYPE> &p2)
  {
    return (p1.x > p2.x && p1.y > p2.y);
  }
  friend Bool operator>=(const Point<TYPE> &p1, const Point<TYPE> &p2)
  {
    return (p1.x >= p2.x && p1.y >= p2.y);
  }
  friend Bool operator<(const Point<TYPE> &p1, const Point<TYPE> &p2)
  {
    return (p1.x < p2.x && p1.y < p2.y);
  }
  friend Bool operator<=(const Point<TYPE> &p1, const Point<TYPE> &p2)
  {
    return (p1.x <= p2.x && p1.y <= p2.y);
  }

  // Set value
  void Set(const TYPE &xIn, const TYPE &yIn)
  {
    x = xIn;
    y = yIn;
  }

  // Normalize
  void Normalize()
  {
    TYPE magnitude = GetMagnitude();
    x /= magnitude;
    z /= magnitude;
  }

  // Get the magnitude
  TYPE GetMagnitude() const
  {
    return ((TYPE) sqrt(GetMagnitude2()));
  }

  // Get the magnitude squared
  TYPE GetMagnitude2() const
  {
    return (x * x + z * z);
  }

  Bool IsInPolyConvex( Point<TYPE> * p, U32 count)
  {
    S32 i, j = 0;
    for (i = count - 1; i >= 0; j = i, i--)
    {
  		Point<F32> vect, enorm;
      enorm.x = F32( (p[j].y - p[i].y) );
      enorm.y = F32(-(p[j].x - p[i].x) );

  		vect.x = F32( x - p[i].x );
	  	vect.y = F32( z - p[i].z );

		  F32 dotp = vect.z * enorm.z + vect.x * enorm.x;
			if (dotp > 0.0f)
      {
        return FALSE;
      }
		}
	  return TRUE;
  }

  Bool IsInPolyConcave( Point<TYPE> * p, U32 count)
  {
    S32 i, j = 0;
    for (i = 2; i >= 0; j = i, i--)
    {
  		Point<F32> vect, enorm;
      enorm.x = F32( (p[j].y - p[i].y) );
      enorm.y = F32(-(p[j].x - p[i].x) );

  		vect.x = F32( x - p[i].x );
	  	vect.y = F32( z - p[i].z );

		  F32 dotp = vect.z * enorm.z + vect.x * enorm.x;
			if (dotp > 0.0f)
      {
        return FALSE;
      }
		}
	  return TRUE;
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Template Area
//
template <class TYPE> struct Area
{
  Point<TYPE> p0;
  Point<TYPE> p1;

  // Constructor
  Area() { };
  Area(Point<TYPE> p0, Point<TYPE> p1) : p0(p0), p1(p1) { }
  Area(TYPE x0, TYPE y0, TYPE x1, TYPE y1) : p0(x0, y0), p1(x1, y1) {}
  Area(Point<TYPE> p0, TYPE x1, TYPE y1) : p0(p0), p1(x1, y1) {}
  Area(TYPE x0, TYPE y0, Point<TYPE> p1) : p0(x0, y0), p1(p1) {}
  Area(TYPE x1, TYPE y1) : p0(0, 0), p1(x1, y1) {}

  // Dimenstions
  TYPE Width() const
  {
    return (p1.x - p0.x);
  }
  TYPE Height() const
  {
    return (p1.y - p0.y);
  }
	TYPE HalfHeight() const 
	{
		return Height() / 2;
	}
	TYPE HalfWidth() const 
	{
		return Width() / 2;
	}
	TYPE MidX() const 
	{
		return p0.x + HalfWidth();
	}
	TYPE MidY() const 
	{
		return p0.y + HalfHeight();
	}
	F32 AspectYX() const
	{
		return (F32) Height() / (F32) Width();
	}

  // Addition operator
  const Area &operator+=(const Area &rhs)
  {
    p0 += rhs.p0;
    p1 += rhs.p1;
    return (*this);
  }
  Area operator+(const Area &rhs) const
  {
    Area tmp(*this);
    tmp += rhs;
    return (tmp);
  }

  // Subtraction operator
  const Area &operator-=(const Area &rhs)
  {
    p0 -= rhs.p0;
    p1 -= rhs.p1;
    return (*this);
  }
  Area operator-(const Area &rhs) const
  {
    Area tmp(*this);
    tmp -= rhs;
    return (tmp);
  }

  // Point Addition operator
  const Area &operator+=(const Point<TYPE> &rhs)
  {
    p0 += rhs;
    p1 += rhs;
    return (*this);
  }
  Area operator+(const Point<TYPE> &rhs) const
  {
    Area tmp(*this);
    tmp += rhs;
    return (tmp);
  }

  // Point Subraction operator
  const Area &operator-=(const Point<TYPE> &rhs)
  {
    p0 -= rhs;
    p1 -= rhs;
    return (*this);
  }
  Area operator-(const Point<TYPE> &rhs) const
  {
    Area tmp(*this);
    tmp -= rhs;
    return (tmp);
  }

  // Scalar Addition operator
  const Area &operator+=(const TYPE &rhs)
  {
    p0 += rhs;
    p1 += rhs;
    return (*this);
  }
  Area operator+(const TYPE &rhs) const
  {
    Area tmp(*this);
    tmp += rhs;
    return (tmp);
  }

  // Scalar Subtraction operator
  const Area &operator-=(const TYPE &rhs)
  {
    p0 -= rhs;
    p1 -= rhs;
    return (*this);
  }
  Area operator-(const TYPE &rhs) const
  {
    Area tmp(*this);
    tmp -= rhs;
    return (tmp);
  }

  // Scalar Multiplication operator
  const Area &operator*=(const TYPE &rhs)
  {
    p0 *= rhs;
    p1 *= rhs;
    return (*this);
  }
  Area operator*(const TYPE &rhs) const
  {
    Area tmp(*this);
    tmp *= rhs;
    return (tmp);
  }

  // Scalar Division operator
  const Area &operator/=(const TYPE &rhs)
  {
    p0 /= rhs;
    p1 /= rhs;
    return (*this);
  }
  Area operator/(const TYPE &rhs) const
  {
    Area tmp(*this);
    tmp /= rhs;
    return (tmp);
  }

  Bool Clip( const Area<TYPE> & other)
  {
    Bool retValue = TRUE;

    if (p0.x < other.p0.x)
    {
      p0.x = other.p0.x;

      if (p1.x < other.p0.x)
      {
        p1.x = other.p0.x;
        retValue = FALSE;
      }
    }
    else if (p0.x > other.p1.x)
    {
      p0.x = p1.x = other.p1.x;
      retValue = FALSE;
    }

    if (p0.y < other.p0.y)
    {
      p0.y = other.p0.y;

      if (p1.y < other.p0.y)
      {
        p1.y = other.p0.y;
        retValue = FALSE;
      }
    }
    else if (p0.y > other.p1.y)
    {
      p0.y = p1.y = other.p1.y;
      retValue = FALSE;
    }

    if (p1.x > other.p1.x)
    {
      p1.x = other.p1.x;
    }
    if (p1.y > other.p1.y)
    {
      p1.y = other.p1.y;
    }
    return retValue;
  }

  void Merge( const Area<TYPE> & other)
  {
    p0.x = Min<TYPE>( p0.x, other.p0.x);
    p0.z = Min<TYPE>( p0.z, other.p0.z);
    p1.x = Max<TYPE>( p1.x, other.p1.x);
    p1.z = Max<TYPE>( p1.z, other.p1.z);
  }

  // Tests
  Bool In(const Point<TYPE> &p) const
  {
    return (p >= p0 && p <= p1);
  }

  // Ensure first point contains smallest values
  void Sort()
  {
    if (p0.x > p1.x) 
    { 
      Swap(p0.x, p1.x);
    }

    if (p0.y > p1.y) 
    { 
      Swap(p0.y, p1.y);
    }  
  }

  // Set value
  void Set(const Point<TYPE> _p0, const Point<TYPE> _p1)
  {
    p0 = _p0;
    p1 = _p1;
  }
  void Set(TYPE x0, TYPE y0, TYPE x1, TYPE y1)
  {
    p0.x = x0;
    p0.y = y0;
    p1.x = x1;
    p1.y = y1;
  }
  void Set(TYPE w, TYPE h)
  {
    p0.x = 0;
    p0.y = 0;
    p1.x = w;
    p1.y = h;
  }

  void SetSize(TYPE x0, TYPE y0, TYPE w, TYPE h)
  {
    p0.x = x0;
    p0.y = y0;
    p1.x = p0.x + w;
    p1.y = p0.y + h;
  }
  void SetSize(TYPE w, TYPE h)
  {
    p1.x = p0.x + w;
    p1.y = p0.y + h;
  }

  // windows conversion
  // 
  operator RECT&()
  {
    return *((RECT *) this);
  }
  operator RECT*()
  {
    return (RECT *) this;
  }

};

#endif
