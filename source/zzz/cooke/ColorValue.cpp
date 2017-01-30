///////////////////////////////////////////////////////////////////////////////
//
// ColorValue.cpp
//
// 19-Jan-99  Harry Travis
//

#include "ColorValue.h"

void ColorValue::SetZero()
{
  r = g = b = a = 0.0f;
}

void ColorValue::Scale()
{
  r *= 255.0f;
  g *= 255.0f;
  b *= 255.0f;
  a *= 255.0f;
}

Color ColorValue::GetRGBA()
{ 
  return ((Utils::FtoLDown(a) << 24) | 
          (Utils::FtoLDown(r) << 16) | 
          (Utils::FtoLDown(g) << 8 ) | 
          (Utils::FtoLDown(b) << 0 ));
}

void ColorValue::Set(F32 _r, F32 _g, F32 _b, F32 _a)
{
  r = _r;
  g = _g;
  b = _b;
  a = _a;
}

void ColorValue::Set(D3DCOLORVALUE c)
{
  r = c.r;
  g = c.g;
  b = c.b;
  a = c.a;
}

void ColorValue::Set(Color c)
{
  r = (F32) c.r;
  g = (F32) c.g;
  b = (F32) c.b;
  a = (F32) c.a;
}

void ColorValue::Save( FScope *parent, const char *name) const
{
  FScope *fScope = parent->AddFunction(name);

  fScope->AddArgFPoint( r);
  fScope->AddArgFPoint( g);
  fScope->AddArgFPoint( b);
  fScope->AddArgFPoint( a);
}

void ColorValue::Load( FScope *parent, const char *name)
{
  FScope *fScope = parent->GetFunction(name, FALSE);
  if (fScope)
  {
    r = fScope->NextArgFPoint();
    g = fScope->NextArgFPoint();
    b = fScope->NextArgFPoint();
    a = fScope->NextArgFPoint();
  }
}