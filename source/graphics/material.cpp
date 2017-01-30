///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// material.cpp
//
// 17-APR-1998
//

#include "vid_public.h"
#include "godfile.h"
#include "stdio.h"
#include "console.h"
//----------------------------------------------------------------------------

// static material manager data
NBinTree<Material>          Material::Manager::tree;
NBinTree<Material::Wrap>    Material::Manager::wrapTree;

const Material *            Material::Manager::curMaterial;
F32                         Material::Manager::diffuseVal = 1.0f;
//----------------------------------------------------------------------------

Material::Material( const char * name)
{
  ClearData();

  Manager::Setup( *this, name);
}
//----------------------------------------------------------------------------

Material::~Material()
{
  if (treeNode.InUse())
  {
    Manager::tree.Unlink( this);
  }
}
//----------------------------------------------------------------------------

void Material::ClearData()
{
  Utils::Memset( &desc, 0, sizeof( desc));

  RootObj::ClearData();
  status.ClearData();

  SetDiffuse(  Manager::diffuseVal, Manager::diffuseVal, Manager::diffuseVal);
  SetSpecular( 0, 0, 0, 1, 0);
  SetEmissive( 0, 0, 0);
}
//----------------------------------------------------------------------------

void Material::SetDiffuse( F32 r, F32 g, F32 b, F32 a) // = 1.0f)
{
  desc.ambient.r = desc.diffuse.r = r;
  desc.ambient.g = desc.diffuse.g = g;
  desc.ambient.b = desc.diffuse.b = b;
  desc.ambient.a = desc.diffuse.a = a;
  status.translucent = a < 1.0f ? TRUE : FALSE;
}
//----------------------------------------------------------------------------

void Material::SetEmissive( F32 r, F32 g, F32 b, F32 a) // = 1.0f)
{
  desc.emissive.r = r;
  desc.emissive.g = g;
  desc.emissive.b = b;
  desc.emissive.a = a;
  status.translucent = a < 1.0f ? TRUE : FALSE;
}
//----------------------------------------------------------------------------

void Material::SetAmbient( F32 r, F32 g, F32 b, F32 a) // = 1.0f)
{
  desc.ambient.r = r;
  desc.ambient.g = g;
  desc.ambient.b = b;
  desc.ambient.a = a;
  status.translucent = a < 1.0f ? TRUE : FALSE;
}
//----------------------------------------------------------------------------

void Material::SetSpecular( F32 r, F32 g, F32 b, F32 a, F32 power) // = 1.0f = 1.0f)
{
  desc.specular.r = r;
  desc.specular.g = g;
  desc.specular.b = b;
  desc.specular.a = a;
  status.specular = (r > 0.0f || g > 0.0f || b > 0.0f) ? TRUE : FALSE;

  desc.power = power;

  // pre-compute specular power integer counter
  powerCount = Utils::FtoL( power + 0.5f);
}
//----------------------------------------------------------------------------

void Material::Manager::DisposeAll()
{
  tree.DisposeAll();
  wrapTree.DisposeAll();
  ResetData();

  Vid::defMaterial = NULL;
}
//----------------------------------------------------------------------------

void Material::Manager::DisposeWraps()
{
  wrapTree.DisposeAll();
}
//----------------------------------------------------------------------------

void Material::Manager::Delete( Material & material)
{
  if (material.treeNode.InUse())
  {
    tree.Dispose( &material);
  }
}
//----------------------------------------------------------------------------

void Material::Manager::Setup( Material & material, const char * name)
{
  U32 key = Crc::CalcStr( name);
  tree.Add( key, &material);

  material.SetName( name);
}
//----------------------------------------------------------------------------

Material * Material::Manager::Create( const char * name)
{
  Material * mat = new Material;
  if (mat)
  {
    Setup( *mat, name);
  }
  return mat;
}
//----------------------------------------------------------------------------

Material * Material::Manager::FindCreate( const char * name)
{
  Material * mat = tree.Find( Crc::CalcStr( name));
  if (!mat)
  {
    mat = Create( name);
  }
  return mat;
}
//----------------------------------------------------------------------------

Material * Material::Manager::FindCreate( 
      ColorF32 & diffuse,  ColorF32 & specular, F32 power, 
      ColorF32 & emissive, ColorF32 & ambient,
      U32 blend, Bool teamColor, Bool envMap, Bool overlay)
{
  NameString name;
  GenerateName( name.str, 
    diffuse, specular, power, emissive, ambient,
    blend, teamColor, envMap, overlay);

  return FindCreate( name.str);
}
//----------------------------------------------------------------------------

Material * Material::Manager::FindCreate()
{
  ColorF32 diffuse( diffuseVal, diffuseVal, diffuseVal);
  ColorF32 zero( 0, 0, 0);

  return FindCreate( diffuse, zero, 0, zero, zero);
}
//----------------------------------------------------------------------------

void Material::Manager::Save( GodFile * god, const Material & material)
{
  god->SaveStr(material.name.str);
  God::Save(*god, material.Diffuse());
  God::Save(*god, material.Specular());
  God::Save(*god, F32(material.SpecularPower()));
  God::Save(*god, material.Emissive());
  God::Save(*god, material.Ambient());
}
//----------------------------------------------------------------------------

Material * Material::Manager::Load( GodFile * god)
{
  NameString name;
  ColorF32 diffuse, specular, emissive, ambient;
  F32 specularPower = 0;

  god->LoadStr(name.str, MAX_GAMEIDENT);
  God::Load(*god, diffuse);
  God::Load(*god, specular);
  God::Load(*god, specularPower);
  God::Load(*god, emissive);
  God::Load(*god, ambient);

  U32 teamColor = 0, envMap = 0, overlay = 0;
  name.str[0] = '\0';

  if (god->version > 1 && god->version < 10)
  {
    God::Load(*god, teamColor);
    God::Load(*god, envMap);
    God::Load(*god, overlay);
  }
  GenerateName( name.str, diffuse, specular, specularPower, ambient, emissive);

  Material * material = Find( name.str);
  if (!material)
  {
    material = Create( name.str);
    ASSERT( material);

    material->SetDiffuse(  diffuse.r,  diffuse.g,  diffuse.b,  diffuse.a);
//    material->SetSpecular( specular.r, specular.g, specular.b, 1.0f, specularPower);
//    material->SetEmissive( emissive.r, emissive.g, emissive.b, emissive.a);
//    material->SetAmbient(  ambient.r,  ambient.g,  ambient.b,  ambient.a);
  }
  return material;
}
//----------------------------------------------------------------------------

void Material::Manager::SetDiffuse( F32 diff)
{
  diffuseVal = diff;

  NBinTree<Material>::Iterator li(&tree); 
  while (Material * mat = li++)
  {
    mat->SetDiffuse( diff, diff, diff, mat->desc.diffuse.a);
  }
}
//----------------------------------------------------------------------------

Material::Wrap::~Wrap()
{
  if (treeNode.InUse())
  {
    Manager::wrapTree.Unlink( this);
  }
}
//----------------------------------------------------------------------------

void Material::Wrap::ClearData()
{
  material  = NULL;

  RootObj::ClearData();
  status.ClearData();
  blendFlags = RS_BLEND_DEF;
  crcMat = 0;
}
//----------------------------------------------------------------------------

Material::Wrap * Material::Manager::FindCreateWrap( 
  ColorF32 & diffuse,  ColorF32 & specular, F32 power, 
  ColorF32 & emissive, ColorF32 & ambient,
  U32 blend, Bool teamColor, Bool envMap, Bool overlay)
{
  BuffString name;

  specular.r = specular.g = specular.b = 0;
  specular.a = 1;
  power = 0;
  emissive.r = emissive.g = emissive.b = 0;
  emissive.a = 1;
  ambient.r = diffuse.r;
  ambient.g = diffuse.g;
  ambient.b = diffuse.b;
  ambient.a = diffuse.a;

  GenerateName( name.str,
    diffuse, specular, power, emissive, ambient, 
    blend, teamColor, envMap, overlay);

  Material::Wrap * wrap = wrapTree.Find( Crc::CalcStr( name.str));
  if (!wrap)
  {
    wrap = new Material::Wrap;
    U32 key = Crc::CalcStr( name.str);
    wrapTree.Add( key, wrap);
    wrap->SetName( name.str);
  }

  GenerateName( name.str, diffuse, specular, power, emissive, ambient);

  Material * mat = Find( name.str);
  if (!mat)
  {
    mat = Create( name.str);
    ASSERT( mat);

    mat->SetDiffuse(  diffuse.r, diffuse.g, diffuse.b, diffuse.a);
//  mat->SetSpecular( specular.r, specular.g, specular.b, specular.a, power);
  }
  wrap->material = mat;

  return wrap;
}
//----------------------------------------------------------------------------

void Material::Manager::GenerateName( char * matName, 
  ColorF32 & diffuse,  ColorF32 & specular, F32 power, 
  ColorF32 & emissive, ColorF32 & ambient,
  U32 blend, Bool teamColor, Bool envMap, Bool overlay)
{
  ambient; 
  emissive;

  specular.r = specular.g = specular.b = 0;
  specular.a = 1;
  power = 0;
  emissive.r = emissive.g = emissive.b = 0;
  emissive.a = 1;

  ambient.r = diffuse.r;
  ambient.g = diffuse.g;
  ambient.b = diffuse.b;
  ambient.a = diffuse.a;

	// generate a unique material name
	U32 crc = 0;
	crc = Crc::Calc( &diffuse,   sizeof(diffuse),  crc);
	crc = Crc::Calc( &specular,  sizeof(specular), crc);
	crc = Crc::Calc( &power,     sizeof(power),    crc);
//	crc = Crc::Calc(&ambient,   sizeof(ambient), crc);
//	crc = Crc::Calc(&emissive,  sizeof(emissive), crc);

	crc = Crc::Calc( &blend,     sizeof(U32),  crc);
	crc = Crc::Calc( &teamColor, sizeof(Bool), crc);
	crc = Crc::Calc( &envMap,    sizeof(Bool), crc);
	crc = Crc::Calc( &overlay,   sizeof(Bool), crc);

	sprintf( matName, "mat%08x", crc);
}
//----------------------------------------------------------------------------

U32 Material::Manager::Report( Material & mat)
{
  U32 mem = mat.GetMem();

  CON_DIAG(( "%-36s:                   %3d %3d %3d %3d", 
    mat.name.str, 
    S32( mat.Diffuse().r * 255),
    S32( mat.Diffuse().g * 255),
    S32( mat.Diffuse().b * 255),
    S32( mat.Diffuse().a * 255)
  ));
  LOG_DIAG(( "%-36s:                   %3d %3d %3d %3d", 
    mat.name.str, 
    S32( mat.Diffuse().r * 255),
    S32( mat.Diffuse().g * 255),
    S32( mat.Diffuse().b * 255),
    S32( mat.Diffuse().a * 255)
  ));

/*
  CON_DIAG(( "%-36s                    %3d %3d %3d %3d %.0f", 
    "", 
    S32( mat.Specular().r * 255),
    S32( mat.Specular().g * 255),
    S32( mat.Specular().b * 255),
    S32( mat.Specular().a * 255),
    S32( mat.SpecularPower())
  ));
  LOG_DIAG(( "%-36s                    %3d %3d %3d %3d %.0f", 
    "", 
    S32( mat.Specular().r * 255),
    S32( mat.Specular().g * 255),
    S32( mat.Specular().b * 255),
    S32( mat.Specular().a * 255),
    S32( mat.SpecularPower())
  ));
*/
  return mem;
}
//----------------------------------------------------------------------------

U32 Material::Manager::ReportList( const char * name) // = NULL
{
  U32 mem = 0, count = 0;

  U32 len = name ? strlen( name) : 0;

  NBinTree<Material>::Iterator i(&tree);
  while (Material * type = i++)
  {
    if (!name || !Utils::Strnicmp( name, type->name.str, len))
    {
      mem += Report( *type);
      count++;
    }
  }
  CON_DIAG(( "%4ld %-31s: %9ld", count, "materials", mem ));
  LOG_DIAG(( "%4ld %-31s: %9ld", count, "materials", mem ));

  return mem;
}
//----------------------------------------------------------------------------

U32 Material::Manager::Report()
{
  U32 mem = 0, count = 0;

  NBinTree<Material>::Iterator li(&tree); 
  while (Material * mat = li++)
  {
    mem += mat->GetMem();
    count++;
  }
  CON_DIAG(( "%4ld %-31s: %9ld", count, "materials", mem ));
  LOG_DIAG(( "%4ld %-31s: %9ld", count, "materials", mem ));

  return mem;
}
//----------------------------------------------------------------------------

