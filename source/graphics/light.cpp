///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidlight.cpp
//
// 17-APR-1998
//

#include "vid_public.h"
#include "light_priv.h"
#include "mesh.h"
#include "perfstats.h"
#include "console.h"
#include "stdload.h"
//----------------------------------------------------------------------------

namespace Vid
{
  namespace Light
  {
    #define RANGEFACTOR_SPOT      0.5f
    #define RANGEFACTOR_POINT     0.3f

    // static light manager stuff
    NBinTree<Obj>             tree;

    NList<Obj>                activeList;    // active light list
    U32                       lastActiveCount;
    Obj *                     closest;
    Obj *                     sun;
    Color                     sunColor;
    Color                     shadowColor;
    Matrix                    shadowMatrix;
    Matrix                    sunMatrix;
    Quaternion                sunAngle; 
    Vector                    lastSunFront;
    Bool                      isSunUp;

    Bitmap *                  coneTex;
    Bitmap *                  flareTex0;
    Bitmap *                  flareTex1;

    Bool                      switchedOn;

    F32                       sunTime;
    F32                       saveTime;
    ColorF32                  saveColor;
    Bool                      saveSunUp;
    F32                       sunMinAngle;
    F32                       sunRange;
    //----------------------------------------------------------------------------

    Desc::Desc( FScope * fScope)
    {
      FScope * sScope;

      // Allocate a new light
      memset( this, 0, sizeof(Desc));

      // Get the light type
      const char * typeName = fScope->NextArgString();

      // And convert to the enumeration value
      switch (Crc::CalcStr(typeName))
      {
        case 0x5FE84C61: // "point" : 
          type = lightPOINT; break;
        case 0x2831DB4A: // "parallel"
          type = lightPARALLEL; break;
        case 0x04BC5B80: // "direction"
          type = lightDIRECTION; break;
        case 0xEB041EC4: // "spot"
          type = lightSPOT; break;
        case 0x65CFBDCD: // "dword"
          type = lightDWORD; break;
     
        default:
          fScope->ScopeError( "Unknown light type '%s'", typeName);
      }

      // Name of the point to attach to
      pointIdent = StdLoad::TypeString( fScope, "Attach");

      // Color of the light
      StdLoad::TypeColor( fScope, "Color", color);

      // Range of the light
      range = StdLoad::TypeF32(fScope, "Range");

      // Attenuation
      sScope = fScope->GetFunction("Attenuation");
      att[0] = sScope->NextArgFPoint();
      att[1] = sScope->NextArgFPoint();
      att[2] = sScope->NextArgFPoint();

      // Required values for spotlights
      if (type == lightSPOT)
      {
        FScope *sScope = fScope->GetFunction("Cone");
        cone[0] = sScope->NextArgFPoint();
        cone[1] = sScope->NextArgFPoint();
      }

      // not required; visible beam brightness factor
      beam = 1.0f;
      sScope = fScope->GetFunction("Beam", FALSE);
      if (sScope)
      {
        beam = sScope->NextArgFPoint();
      }

      // not required; is the light day/night switchable
      switchable = TRUE;
      sScope = fScope->GetFunction("Switchable", FALSE);
      if (sScope)
      {
        switchable = sScope->NextArgInteger();
      }

      // not required; how important is it?  0 - very important, 1 - not important
      priority = 0.0f;
      sScope = fScope->GetFunction("Priority", FALSE);
      if (sScope)
      {
        priority = sScope->NextArgFPoint();
      }
    }
    //----------------------------------------------------------------------------

    Obj::Obj()
    {
      ClearData(); 
    }
    //----------------------------------------------------------------------------

    Obj::Obj( const char * name, Type type, Bool on) // = lightPOINT, TRUE
    {
      ClearData();

      Light::Setup( *this, name, type);

      // turn it on/off
      Switch( IsSwitchable() ? on : TRUE);
    }
    //----------------------------------------------------------------------------

    void Obj::ClearData()
    {
      FamilyNode::ClearData();

      nodeType = nodeLight;

      // setup FamilyNode::state and FamilyState::node pointers
      SetState( state);

      Utils::Memset( &d3d, 0, sizeof( d3d));
	    d3d.dltType = D3DLIGHT_POINT;
      d3d.dcvAmbient.r = d3d.dcvSpecular.r = 0;
      d3d.dcvAmbient.g = d3d.dcvSpecular.g = 0;
      d3d.dcvAmbient.b = d3d.dcvSpecular.b = 0;

      *((Vector *) &d3d.dvPosition)  = Matrix::I.posit;
      *((Vector *) &d3d.dvDirection) = Matrix::I.front;

      beam = 1.0f;
      switchable = FALSE;
      active = TRUE;

      SetColor( 1, 1, 1);
      SetAtten( 1, 0, 0);
      SetRange( 10);
      SetCone( .25f, .5f, 1);
    }
    //----------------------------------------------------------------------------

    Obj::~Obj()
    {
      if (activeNode.InUse())
      {
        activeList.Unlink( this);
      }

      if (treeNode.InUse())
      {
        tree.Unlink( this);
      }
    }
    //----------------------------------------------------------------------------

    void Obj::SetBeamFactor( F32 _beam)
    {
      beam = _beam;

      colorBeam.Set( 
/*
        Min<F32>( d3d.dcvDiffuse.r * beam, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g * beam, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g * beam, 1.0f),
*/
        Min<F32>( d3d.dcvDiffuse.r, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g, 1.0f),        
        Min<F32>( beam, 1.0f)
      );
    }
    //----------------------------------------------------------------------------

    void Obj::SetColor( F32 r, F32 g, F32 b)
    {
      d3d.dcvDiffuse.r = r;
	    d3d.dcvDiffuse.g = g;
	    d3d.dcvDiffuse.b = b;

      colorBeam.Set( 
/*
        Min<F32>( d3d.dcvDiffuse.r * beam, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g * beam, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g * beam, 1.0f),
*/
        Min<F32>( d3d.dcvDiffuse.r, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g, 1.0f),
        Min<F32>( d3d.dcvDiffuse.g, 1.0f),        
        Min<F32>( beam, 1.0f)
      );
      colorFull.Set( d3d.dcvDiffuse.r, d3d.dcvDiffuse.g, d3d.dcvDiffuse.b, 1.0f);
    }
    //----------------------------------------------------------------------------

    void Obj::SetType( Type type)
    {
      d3d.dltType = (D3DLIGHTTYPE) type;
      bounds.SetRadius( d3d.dvRange * (d3d.dltType == D3DLIGHT_SPOT ? RANGEFACTOR_SPOT : RANGEFACTOR_POINT));

      if (type == lightDIRECTION)
      {
        SetRange( D3DLIGHT_RANGE_MAX);
      }
    }
    //----------------------------------------------------------------------------

    void Obj::SetRange( F32 range)
    {
	    d3d.dvRange = range;
      bounds.SetRadius( d3d.dvRange * (d3d.dltType == D3DLIGHT_SPOT ? RANGEFACTOR_SPOT : RANGEFACTOR_POINT));

      invRange = (F32)fabs(d3d.dvRange) > F32_EPSILON ? 1.0f / d3d.dvRange : 1e6f;
      bounds.SetRadius( d3d.dvRange * (d3d.dltType == D3DLIGHT_SPOT ? RANGEFACTOR_SPOT : RANGEFACTOR_POINT));
    }
    //----------------------------------------------------------------------------

    void Obj::SetCone( F32 theta, F32 phi, F32 falloff) //  = -1.0 = 1.0f)
    {
      d3d.dvTheta = theta;
      d3d.dvPhi   = phi == -1.0f ? theta : phi;
      d3d.dvFalloff = falloff;

	    cosTheta = (F32) cos((double) (d3d.dvTheta * 0.5f));
	    sinTheta = (F32) sin((double) (d3d.dvTheta * 0.5f));
	    cosPhi   = (F32) cos((double) (d3d.dvPhi * 0.5f));
	    sinPhi   = (F32) sin((double) (d3d.dvPhi * 0.5f));
      invAngle = (F32)fabs(cosTheta - cosPhi) > F32_EPSILON ? 1.0f / (cosTheta - cosPhi) : 1e6f;
    }
    //----------------------------------------------------------------------------

    void Obj::SetAtten( F32 a0, F32 a1, F32 a2)
    {
	    d3d.dvAttenuation0 = a0;
      d3d.dvAttenuation1 = a1;
	    d3d.dvAttenuation2 = a2;
    }
    //----------------------------------------------------------------------------

    void Obj::SetWorldRecurse( const Matrix & world)
    {
      FamilyNode::SetWorldRecurse( world);

      Setup( WorldMatrix());
    }
    //----------------------------------------------------------------------------

    void Obj::SetWorldRecurseRender( const Matrix & world, FamilyState * stateArray)
    {
      CalcWorldMatrix( world);

      NList<FamilyNode>::Iterator kids(&children);
      FamilyNode *node;
      while ((node = kids++) != NULL)
      {
        node->SetWorldRecurseRender( statePtr->WorldMatrix(), stateArray);
      }

      Setup( WorldMatrix());
    }
    //----------------------------------------------------------------------------

    void Obj::Light( Color * lightvals, const Vector * norms, U32 count)
    {
      // setup
      // align front with normals
      direction = WorldMatrix().front * -1.0f;

      // get current material values
      ColorF32 diffInit, diff = GetMaterialDiffuse( diffInit);
      Color color( diffInit.r, diffInit.g, diffInit.b, diffInit.a);

      // factor in light color
      diff.r *= d3d.dcvDiffuse.r; 
      diff.g *= d3d.dcvDiffuse.g; 
      diff.b *= d3d.dcvDiffuse.b; 

      // loop
      const Vector * n = norms;
	    for (Color * le = lightvals + count, * ls = lightvals; ls < le; ls++, n++)
      {
	      F32 diffuse_reflect = n->Dot(direction); // -1.0f <= diffuse_reflect <= 1.0f

	      if (diffuse_reflect > 0.0f)
	      {
		      // calculate the ambient + diffuse component for the vertex
          ls->SetInline( 
            diffInit.r + diffuse_reflect * diff.r,
            diffInit.r + diffuse_reflect * diff.g,
            diffInit.r + diffuse_reflect * diff.b,
            U32(color.a)
          );
	      }
        else
        {
		      // set the ambient
          *ls = color;
        }
	    }
    }  
    //----------------------------------------------------------------------------

    void Obj::Switch( Bool switchOn)
    {
      if (switchable && IsActive() != switchOn)
      {
        SetActive( switchOn);
      }
    }
    //----------------------------------------------------------------------------

    void ResetData()
    {
      closest = NULL;
      lastActiveCount = 0;
    }
    //----------------------------------------------------------------------------

    Bool Init()
    {
      tree.SetNodeMember( &Obj::treeNode);
      activeList.SetNodeMember( &Obj::activeNode);

      sun = NULL;

      ResetData();

      shadowMatrix.ClearData();

      sunAngle.Set( PI * 0.5f, Matrix::I.Up());

      coneTex = flareTex0 = flareTex1 = NULL;

      switchedOn = TRUE;

      return TRUE;
    }
    //----------------------------------------------------------------------------

    void Done()
    {
      DisposeAll();

      sun = NULL;
    }
    //----------------------------------------------------------------------------

    void InitResources()
    {
      coneTex   = Bitmap::Manager::FindCreate( Bitmap::reduceMED, "engine_light_spotcone.tga");
      flareTex0 = Bitmap::Manager::FindCreate( Bitmap::reduceMED, "engine_light_halo.tga");
      flareTex1 = Bitmap::Manager::FindCreate( Bitmap::reduceMED, "engine_light_flare.tga");
    }
    //----------------------------------------------------------------------------

    // seasonal shift from pure east/west
    //
    void SetSunAngle( F32 degrees)
    {
      sunAngle.Set( PI * 0.5f + degrees * DEG2RAD, Matrix::I.Up());
    }
    //----------------------------------------------------------------------------

    void SaveSun()
    {
      ASSERT(sun);
      saveTime = sunTime;
      saveColor.Set( sun->R(), sun->G(), sun->B(), 1);
      saveSunUp = isSunUp;
    }
    //----------------------------------------------------------------------------

    void RestoreSun()
    {
      SetSun( saveTime, saveColor.r, saveColor.g, saveColor.b, saveSunUp);
    }
    //----------------------------------------------------------------------------

    void SetSunMinAngle( F32 angle)
    {
      sunMinAngle = angle * DEG2RAD;
      sunRange = 2 * sunMinAngle - PI;
    }
    //----------------------------------------------------------------------------

    void SetSun( F32 time, F32 r, F32 g, F32 b, Bool _isSunUp) // = TRUE
    {
      static Quaternion q1( PI * 0.5f, Matrix::I.Up());

      ASSERT( sun);

      sunTime = time;
      isSunUp = _isSunUp;

      Quaternion q( time * sunRange - sunMinAngle, Matrix::I.Right());

      // shadow matrix doesn't use the seasonal angle
      //
      Quaternion sq = q;
      sq *= q1;
      shadowMatrix.Set( sq);

      if (shadowMatrix.front.y > -Vid::renderState.shadowY)
      {
        // clamp angle
        //
        shadowMatrix.front.y = -Vid::renderState.shadowY;
      }
      shadowMatrix.front.z = 0;
      shadowMatrix.front.Normalize();

      // sun matrix
      //
      q *= sunAngle;    // factor in seasonal angle
      sun->SetWorld( q);

      // sun/moon image
      //
      q.Set( PI * 0.08f + time * -PI * 1.16f, Matrix::I.Right());
      q *= q1;
      sunMatrix.Set( q);

      sun->SetColor( r, g, b);

      sunColor.Set( r, g, b, (U32)255);

      SetupShadow();
    }
    //----------------------------------------------------------------------------

    void SetSun( const Vector & vect, F32 r, F32 g, F32 b, Bool _isSunUp) // = TRUE
    {
      static Quaternion q1( PI * 0.5f, Matrix::I.Up());

      ASSERT( sun);

      isSunUp = _isSunUp;

      shadowMatrix.ClearData();
      shadowMatrix.SetFromFront( vect);

      if (shadowMatrix.front.y > -Vid::renderState.shadowY)
      {
        // clamp angle
        //
        shadowMatrix.front.y = -Vid::renderState.shadowY;
      }
      shadowMatrix.front.z = 0;
      shadowMatrix.front.Normalize();

      sun->SetWorld( shadowMatrix);

      sun->SetColor( r, g, b);

      sunColor.Set( r, g, b, (U32)255);

      SetupShadow();
    }
    //----------------------------------------------------------------------------

    void SetupShadow()
    {
      F32 alpha = F32(sunColor.r + sunColor.g + sunColor.b) * F32( Vid::renderState.shadowAlpha) * U8toNormF32;

      if (sunTime < .01f)
      {
        alpha *= sunTime / .01f;
      }
      else if (sunTime > .99f)
      {
        alpha *= (1 - sunTime) / .01f;
      }

      shadowColor.Set( (U32)0, (U32)0, (U32)0, 
        Min<U32>((U32) Utils::FtoL( alpha), 255));
    }
    //----------------------------------------------------------------------------

    Obj * Find( const char * name)
    {
      return tree.Find( Crc::CalcStr( name));
    }
    //----------------------------------------------------------------------------

    Obj * FindCreateSun( const char * name)
    {
      return sun = FindCreate( name, lightDIRECTION);
    }
    //----------------------------------------------------------------------------

    Obj * FindCreate( const char * name, Type type, F32 priority, Bool on) // = lightPOINT, = 0.0f, TRUE
    {
      Obj * light = Find( name);
      if (!light)
      {
        return Create( name, type, priority, on);
      }
      light->SetType( type);

      return light;
    }
    //----------------------------------------------------------------------------

    Obj * Create( const char * name, Type type, F32 priority, Bool on) // = lightPOINT, 0.0f, TRUE
    {
      Obj * light = NULL;

      if (!Vid::renderState.status.lightSingle || (tree.GetCount() == 0 && type == lightDIRECTION))
      {
        if (priority * (1.0f - Vid::renderState.perfs[3]) < 0.5f)
        {
          light = new Obj;

          Setup( *light, name, type);

          // turn it on/off
          light->Switch( light->IsSwitchable() ? on : TRUE);
        }
      }
      return light;
    }
    //----------------------------------------------------------------------------

    Obj * Create( const Desc & desc, Bool on) // = TRUE
    {
      Obj * light = Create( desc.pointIdent.str, desc.type, desc.priority, on);

      if (light)
      {
        light->SetBeamFactor( desc.beam);
        light->SetSwitchable( desc.switchable);
        light->SetColor( desc.color);
        light->SetRange( desc.range);
        light->SetAtten( desc.att[0], desc.att[1], desc.att[2]);
        light->SetCone( desc.cone[0], desc.cone[1]);

        // turn it on/off
        light->Switch( light->IsSwitchable() ? on : TRUE);
      }
      return light;
    }
    //----------------------------------------------------------------------------

    void Setup( Obj & light, const char * name, Type type) // = lightPOINT)
    {
      U32 key = Crc::CalcStr( name);
      tree.Add( key, &light);

      light.SetName( name);
      light.SetType( type);

      activeList.Append( &light);
    }
    //----------------------------------------------------------------------------

    void SetActiveListSun()
    {
      ASSERT( sun);
      SetActiveList( *sun);
    }
    //----------------------------------------------------------------------------

    void SetActiveList( const Vector & origin, const Bounds & bounds)
    {
    #ifdef DOCLOSESTLIGHT
      closest = NULL;
      F32 mindist = 1000000.0f;
    #endif

	    NBinTree<Obj>::Iterator li(&tree); 
	    while (Obj * light = li++)
	    {
        Bool active = light->IsActive();

        if (active)
        {
          switch (light->GetType())
          {
          case lightDIRECTION:
            break;

          case lightSPOT:
          case lightPOINT:
            {
              if (Vid::renderState.perfs[0] <= .5f)
              {
                continue;
              }

              F32 range2 = light->GetRange() + bounds.Radius();
              range2 *= range2;

              Vector d = light->WorldMatrix().Position();
              d -= origin;
              F32 dist2 = d.Magnitude2();

              active = dist2 > range2 ? FALSE : TRUE;

    #ifdef DOCLOSESTLIGHT
              if (active && dist2 < mindist)
              {
                closest = light;
                mindist = dist2;
              }
    #endif
            }
            break;
          }
        }

        if (light->activeNode.InUse())
        {
          if (!active)
          {
            activeList.Unlink( light);
          }
        }
        else if (active)
        {
          if (activeList.GetCount() < caps.maxLights)
          {
            activeList.Append( light);
          }
        }
      }  

#ifndef DODXLEANANDGRUMPY
      if (renderState.status.dxTL)
      {
        // set lights
        NList<Obj>::Iterator i(&activeList);
        U32 activeIndex = 0;
        while (Obj * light = i++)     // FIXME
        {
          if (activeIndex >= caps.maxLights)
          {
            break;
          }

          dxError = device->SetLight( activeIndex, light->D3D());
          LOG_DXERR(( "Light::SetActiveList: device->SetLight" ));

          dxError = device->LightEnable( activeIndex, TRUE);
          LOG_DXERR(( "Light::SetActiveList: device->EnableLight(TRUE)" ));

          activeIndex++;
        }
        // clear last lights
        U32 last = lastActiveCount;
        lastActiveCount = activeIndex;
        while (activeIndex < last)
        {
          dxError = device->LightEnable( activeIndex++, FALSE);
          LOG_DXERR(( "Light::SetActiveList: device->EnableLight( FALSE)" ));
        }

      }
#endif
    }
    //----------------------------------------------------------------------------

    void SetActiveList( Obj & light)
    {
      activeList.UnlinkAll();
      activeList.Append( &light);
    }
    //----------------------------------------------------------------------------

    void SwitchLights( Bool switchOn)
    {
      switchedOn = switchOn;

      NBinTree<Obj>::Iterator li( &tree); 
	    while (Obj * light = li++)
	    {
        light->Switch( switchOn);
      }
    }
    //----------------------------------------------------------------------------

    ColorF32 GetMaterialDiffuse( ColorF32 & diffInit)
    {
      ASSERT( BucketMan::GetMaterial());
      ColorF32 diff = BucketMan::GetMaterial()->Diffuse();

      diffInit.r = diff.r * Vid::renderState.ambientColorF32.r;
      diffInit.g = diff.g * Vid::renderState.ambientColorF32.g;
      diffInit.b = diff.b * Vid::renderState.ambientColorF32.b;
      diffInit.a = diff.a;

      return diff;
    }
    //----------------------------------------------------------------------------

    void DisposeAll()
    {
      activeList.UnlinkAll();
      tree.DisposeAll();

      ResetData();
    }
    //----------------------------------------------------------------------------

    U32 Report( Obj & light)
    {
      U32 mem = light.GetMem();

      CON_DIAG(( "%-36s:             %-8s %3d %3d %3d", 
        light.GetName(),
        light.GetType() == lightDIRECTION ? "sun" : light.GetType() == lightPOINT ? "point" : "spot",
        S32( light.R() * 255), S32( light.G() * 255), S32( light.B())
      ));
      LOG_DIAG(( "%-36s:             %-8s %3d %3d %3d", 
        light.GetName(),
        light.GetType() == lightDIRECTION ? "sun" : light.GetType() == lightPOINT ? "point" : "spot",
        S32( light.R() * 255), S32( light.G() * 255), S32( light.B())
      ));

      return mem;
    }
    //----------------------------------------------------------------------------

    U32 ReportList( const char * name) // = NULL
    {
      U32 mem = 0, count = 0;

      U32 len = name ? strlen( name) : 0;

      NBinTree<Obj>::Iterator i(&tree);
      while (Obj * type = i++)
      {
        if (!name || !Utils::Strnicmp( name, type->GetName(), len))
        {
          mem += Report( *type);
          count++;
        }
      }
      CON_DIAG(( "%4ld %-31s: %9ld", count, "lights", mem ));
      LOG_DIAG(( "%4ld %-31s: %9ld", count, "lights", mem ));

      return mem;
    }
    //----------------------------------------------------------------------------

    U32 Report()
    {
      U32 mem = 0, count = 0;

      NBinTree<Obj>::Iterator li(&tree); 
      while (Obj * light = li++)
      {
        mem += light->GetMem();
        count++;
      }
      CON_DIAG(( "%4ld %-31s: %9ld", count, "lights", mem ));
      LOG_DIAG(( "%4ld %-31s: %9ld", count, "lights", mem ));

      return mem;
    }
    //----------------------------------------------------------------------------

    #if 0 //ndef DODXLEANANDGRUMPY
    void Obj::Light( VertexL * dst, Vertex * src, U32 count)
    {
	    Matrix inverse_world_matrix;
	    inverse_world_matrix.SetInverse( Vid::world_matrix);
      Vector dir = WorldMatrix().Front() * -1.0f;
	    inverse_world_matrix.Rotate( dir);
    //	dir.Normalize();

	    U32 i;
	    for (i = 0; i < count; i++)
      {
		    Vid::world_matrix.Transform( dst[i].vv, src[i].vv);

        F32 d = dir.Dot( src[i].nv);
        if (d < 0.0f)
        {
          d = 0.0f;
        }

        dst[i].diffuse = Color( R() * d, G() * d, B() * d, (U32) 255);
		    dst[i].specular = 0xff00000;
	    }
    }  
    //----------------------------------------------------------------------------
    #endif
  };
};