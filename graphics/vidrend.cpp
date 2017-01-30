///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidrend.cpp      non-setup 3d rendering specific Vid stuff
//
// 22-APR-1998
//

#include "vid_private.h"
#include "main.h"
#include "console.h"
#include "mesh.h"
#include "perfstats.h"
#include "statistics.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  const F32 ZBIAS = 1.0f;

  extern VarFloat farPlane;
  extern VarFloat fogRange;

  Bool AreVerticesInRange( const VertexTL *vertex, S32 vertexCount, const U16 *index, S32 indexCount, U32 clipFlags, Bool guard) // = TRUE
  {
	  int	result = 1;

    Area<S32> range = viewRect;
    if (guard && Vid::renderState.status.clipGuard)
    {
      range.p0.x -= renderState.clipGuardSize;
      range.p0.y -= renderState.clipGuardSize;
      range.p1.x += renderState.clipGuardSize;
      range.p1.y += renderState.clipGuardSize;
    }

    const VertexTL * v, * ve = vertex + vertexCount;
	  for (v = vertex; v < ve; v++)
	  {
      if (clipFlags & DP_DONOTCLIP)
      {
        if (
             S32(v->vv.x) >= range.p0.x && S32(v->vv.x) <= range.p1.x
          && S32(v->vv.y) >= range.p0.y && S32(v->vv.y) <= range.p1.y
          && S32(v->vv.z) >= 0 && S32(v->vv.z) <= 1
        )
        {
	  		  continue;
        }
		    else
		    {
          result = 0;
          LOG_DIAG( ("vert out of range %f,%f,%f: cur %d; count %d", v->vv.x, v->vv.y, v->vv.z, v - vertex, vertexCount));
//          ERR_FATAL( ("vert out of range"));
        }
      }
      else if (S32(v->vv.z) >= 0 && S32(v->vv.z) <= 1.0f)
      {
	  		continue;
      }
		  else
		  {
        result = 0;

        LOG_DIAG( ("vert z out of range %f: tex: %s", v->vv.z));
//        ERR_FATAL( ("vert z out of range"));
  		}
	  }
    if (index)
    {
      const U16 * i, * ie = index + indexCount;
      for (i = index; i < ie; i++)
      {
        if (*i >= vertexCount)
        {
          result = 0;

          LOG_DIAG( ("index out of range: tex"));
//          ERR_FATAL( ("index out of range"));
        }
      }
	  }

	  return result;
  }
  //----------------------------------------------------------------------------

  Bool SetZBufferState( Bool doZBuffer)
  {
    Bool retValue = renderState.status.zbuffer;

    renderState.status.zbuffer = doZBuffer;
    U32 flag = doZBuffer ? D3DZB_TRUE : D3DZB_FALSE;
    if (renderState.status.wbuffer)
    {
      flag |= D3DZB_USEW;
    }
    dxError = device->SetRenderState( D3DRENDERSTATE_ZENABLE, flag );
	  dxError = device->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, doZBuffer);
    LOG_DXERR( ("SetZBufferState") );

    return retValue;
  }
  //----------------------------------------------------------------------------

  Bool SetZWriteState( Bool doZWrite)
  {
    U32 retValue;
    device->GetRenderState( D3DRENDERSTATE_ZWRITEENABLE, &retValue);

	  dxError = device->SetRenderState( D3DRENDERSTATE_ZWRITEENABLE, doZWrite);
    LOG_DXERR( ("SetZWriteState") );

    return (Bool) retValue;
  }
  //----------------------------------------------------------------------------

  void SetCullStateD3D( Bool doCull)
  {
    dxError = device->SetRenderState( D3DRENDERSTATE_CULLMODE, doCull ? D3DCULL_CCW : D3DCULL_NONE);
    LOG_DXERR( ("SetCullState") );
  }
  //----------------------------------------------------------------------------

  Bool SetAlphaState( Bool doAlpha)
  {
    Bool retValue = renderState.status.alpha;

    renderState.status.alpha = doAlpha;

    dxError = device->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, doAlpha);
    LOG_DXERR( ("device->SetRenderState") );

    return retValue;
  }
  //----------------------------------------------------------------------------

  U32 SetSrcBlendState( U32 flags)
  {
    U32 lastflags = renderState.renderFlags & RS_SRC_MASK;
    renderState.renderFlags &= ~RS_SRC_MASK;

    flags &= RS_SRC_MASK;
    renderState.renderFlags |= flags;
    flags >>= RS_SRC_SHIFT;

    dxError = device->SetRenderState( D3DRENDERSTATE_SRCBLEND, flags);
    LOG_DXERR( ("SetSrcBlendState") );

    return lastflags;
  }
  //-----------------------------------------------------------------------------

  U32 SetDstBlendState( U32 flags)
  {
    U32 lastflags = renderState.renderFlags & RS_DST_MASK;
    renderState.renderFlags &= ~RS_DST_MASK;

    flags &= RS_DST_MASK;
    renderState.renderFlags |= flags;
    flags >>= RS_DST_SHIFT;

    dxError = device->SetRenderState( D3DRENDERSTATE_DESTBLEND, flags);
    LOG_DXERR( ("SetDstBlendState") );

    return lastflags;
  }
  //-----------------------------------------------------------------------------
  //----------------------------------------------------------------------------

  U32 SetTexWrapState( U32 flags, U32 stage) // = 0
  {
    U32 lastFlags = renderState.renderFlags & RS_ADD_MASK;
    renderState.renderFlags &= ~RS_ADD_MASK;

    U32 flag = flags & RS_ADD_MASK;
    renderState.renderFlags |= flag;

    dxError = device->SetTextureStageState( stage, D3DTSS_ADDRESS, (flag >> RS_ADD_SHIFT) + 1);

    LOG_DXERR( ("SetTexWrapState") );

    return lastFlags;
  }
  //----------------------------------------------------------------------------

  void TexDecal( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: color") );

//      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
//      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexDecalSimple( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexDecalAlpha( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexModulate( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexModulateAlpha( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexModulateAlpha2x( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexModulateAlpha4x( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexAddAlpha( U32 stage)
  {
    if (stage)
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_ADD);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_CURRENT); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
    else
    {
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: color") );

      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
      dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

      LOG_DXERR( ("SetTextureStageState: alpha") );
    }
  }
  //----------------------------------------------------------------------------

  void TexAddSignedAlpha( U32 stage)
  {
    dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    dxError = device->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
    dxError = device->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 

    LOG_DXERR( ("SetTextureStageState: color") );

    dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
    dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    dxError = device->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

    LOG_DXERR( ("SetTextureStageState: alpha") );
  }
  //----------------------------------------------------------------------------

  typedef void (*TexBlendProcP)( U32 stage);    // texture blend setup function
  static TexBlendProcP blendToOp[] = 
  {
    0,
    TexDecal,            //  RS_TEX_DECAL          
    TexDecalAlpha,       //  RS_TEX_DECALALPHA
    TexModulateAlpha,    //  RS_TEX_MODULATE
    TexModulateAlpha2x,  //  RS_TEX_MODULATE2X
    TexModulateAlpha4x,  //  RS_TEX_MODULATE4X
    TexModulateAlpha,    //  RS_TEX_MODULATEALPHA
    TexAddAlpha          //  RS_TEX_ADD
  };

  static Bool blendValidation[8][2];
  static char * blendName[8] =
  { 
    "", 
    "decal", 
    "decalAlpha", 
    "modulate",
    "modulate2x",
    "modulate4x",
    "modulateAlpha",
    "add"
  };
  //----------------------------------------------------------------------------

  U32 SetTexBlendState( U32 flags, U32 stage) // = 0
  {
    U32 lastflags = renderState.renderFlags & RS_TEX_MASK;
    renderState.renderFlags &= ~RS_TEX_MASK;

    flags &= RS_TEX_MASK;
    renderState.renderFlags |= flags;
    flags >>= RS_TEX_SHIFT;

#ifdef DEVELOPMENT
    if ( flags == 0 || flags > 7)
    {
      ERR_FATAL(( "bad tex blend %d", flags));
    }
#endif

    blendToOp[ flags](stage);

    return lastflags;
  }
  //-----------------------------------------------------------------------------

  void ValidateBlends()
  {
    Bitmap * tex = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, "library\\engine\\engine_pandemic.pic");

    // reset
    Utils::Memset( blendValidation, 0, sizeof( blendValidation));
    blendToOp[1] = TexDecal;            //  RS_TEX_DECAL          
    blendToOp[2] = TexDecalAlpha;       //  RS_TEX_DECALALPHA
    blendToOp[3] = TexModulateAlpha;    //  RS_TEX_MODULATE
    blendToOp[4] = TexModulateAlpha2x;  //  RS_TEX_MODULATE2X
    blendToOp[5] = TexModulateAlpha4x;  //  RS_TEX_MODULATE4X
    blendToOp[6] = TexModulateAlpha;    //  RS_TEX_MODULATEALPHA
    blendToOp[7] = TexAddAlpha;         //  RS_TEX_ADD

    Bool hit = FALSE;

    // TEX_MODULATE
    //
    Vid::SetTextureDX( tex, 0, 3 << RS_TEX_SHIFT); 
    DWORD passes;
    blendValidation[3][0] = device->ValidateDevice( &passes) == D3D_OK;

    if (!blendValidation[3][0])
    {
      // can't do D3DTSS_ALPHAOP D3DTOP_MODULATE
      //
      blendToOp[3] = TexModulate;
      Vid::SetTextureDX( tex, 0, 3 << RS_TEX_SHIFT); 
      blendValidation[3][0] = device->ValidateDevice( &passes) == D3D_OK;

      caps.noAlphaMod = TRUE;

      if (!hit)
      {
        LOG_DIAG((""));
        hit = TRUE;
      }
      LOG_DIAG(("tex : ! stage 0 TexModulateAlpha invalid replacing with TexModulate"));
    }

    for (U32 i = 1; i < 8; i++)
    {
      Vid::SetTextureDX( tex, 0, i << RS_TEX_SHIFT); 

      blendValidation[i][0] = device->ValidateDevice( &passes) == D3D_OK;
      if (!blendValidation[i][0])
      {
        // find a good backup
        //
        S32 j = i - 1;
        if (i == 7 || i == 1) // RS_TEX_ADD || RS_TEX_DECAL
        {
          j = 5;    // modulate4x
        }
        while (j > 0 && !blendValidation[j][0]) j--;
        if (j == 0)
        {
          j = 3;
        }
        blendToOp[i] = blendToOp[j];

        if (!hit)
        {
          LOG_DIAG((""));
          hit = TRUE;
        }        
        LOG_DIAG(("tex : ! stage 0 blend %s invalid replacing with %s", blendName[i], blendName[j]));
      }
    }

    if (caps.texMulti)
    {
      // check for useful texMultiure modes
      //
      Bool hhit = FALSE;
      for (U32 i = 1; i < 8; i++)
      {
        Vid::SetTextureDX( tex, 0, RS_BLEND_MODULATE); 
        Vid::SetTextureDX( tex, 1, i << RS_TEX_SHIFT); 

        DWORD passes;
        blendValidation[i][1] = device->ValidateDevice( &passes) == D3D_OK;
        if (!blendValidation[i][1])
        {
          if (!hit)
          {
            LOG_DIAG((""));
            hit = TRUE;
          }
          LOG_DIAG(("tex : ! stage 1 blend %s invalid", blendName[i]));
        }
        else if (!hhit)
        {
          hhit = TRUE;
        }
      }

      if (!hhit)
      {
        Vid::SetTextureDX( tex, 0, RS_BLEND_MODULATE); 
        Vid::SetTextureDX( tex, 1, RS_BLEND_ADD); 
        TexDecalSimple( 0);
        TexAddAlpha( 1);
        DWORD passes;
        if (device->ValidateDevice( &passes) != D3D_OK)
        {
          if (!hit)
          {
            LOG_DIAG((""));
            hit = TRUE;
          }
          LOG_DIAG(("tex : ! stage 1 blend decalsimple x add invalid"));
        }
      }

      if (!blendValidation[RS_BLEND_DECAL >> RS_TEX_SHIFT][1]
       || !blendValidation[RS_BLEND_ADD   >> RS_TEX_SHIFT][1] )
      {
        // useless video trash
        //
        if (!hit)
        {
          LOG_DIAG((""));
          hit = TRUE;
        }
        Var::varMultiTex = caps.texMulti = CurDD().texMulti = CurD3D().texMulti = FALSE;
        LOG_DIAG(("tex : ! multitex doesn't handle required blends: canceling"));
      }

      SetTextureDX( NULL, 1, RS_BLEND_DEF);
    }
    SetTextureDX( NULL, 0, RS_BLEND_DEF);
    if (hit)
    {
      LOG_DIAG((""));
    }
  }
  //-----------------------------------------------------------------------------

  Bool ValidateBlend( U32 blend, U32 stage) // = 0)
  {
    blend = (blend & RS_TEX_MASK) >> RS_TEX_SHIFT;
    ASSERT( blend > 0);
    return blendValidation[blend][stage];
  }
  //-----------------------------------------------------------------------------

  void SetTextureFactor( Color color)
  {
    dxError = device->SetRenderState( D3DRENDERSTATE_TEXTUREFACTOR, 
      D3DRGBA( color.r, color.g, color.b, color.a) );
    LOG_DXERR( ("SetTextureFactor") );
  }
  //-----------------------------------------------------------------------------

  Bool SetCamera( Camera & cam, Bool force) // = FALSE)
  {
    if (!force && curCamera == &cam)
    {
      // already set
      return TRUE;
    }
    curCamera = &cam;

    Vid::clipRect.Set( (F32) cam.ViewRect().p0.x, (F32) cam.ViewRect().p0.y, (F32) cam.ViewRect().p1.x, (F32) cam.ViewRect().p1.y);

    viewDesc.dwX      = cam.ViewRect().p0.x;
    viewDesc.dwY      = cam.ViewRect().p0.y;
	  viewDesc.dwWidth	= cam.ViewRect().Width();
	  viewDesc.dwHeight	= cam.ViewRect().Height();

    // clip to view rect
    //
    if (cam.ViewRect().p0.x < 0)
    {
      viewDesc.dwWidth += cam.ViewRect().p0.x;
      viewDesc.dwX = 0;
      Vid::clipRect.p0.x = 0;
    }
    else if (cam.ViewRect().p0.x >= viewRect.Width())
    {
      viewDesc.dwWidth = 0;
      viewDesc.dwX = viewRect.Width() - 1;
      Vid::clipRect.p0.x = (F32) viewDesc.dwX;
    }
    if (cam.ViewRect().p0.y < 0)
    {
      viewDesc.dwHeight += cam.ViewRect().p0.y;
      viewDesc.dwY = 0;
      Vid::clipRect.p0.y = 0;
    }
    else if (cam.ViewRect().p0.y >= viewRect.Height())
    {
      viewDesc.dwHeight = 0;
      viewDesc.dwY = viewRect.Height() - 1;
      Vid::clipRect.p0.y = (F32) viewDesc.dwY;
    }
    if (S32(viewDesc.dwX + viewDesc.dwWidth) > viewRect.p1.x)
    {
      S32 dx = viewRect.p1.x - (viewDesc.dwX + viewDesc.dwWidth);
      viewDesc.dwWidth += dx;
      Vid::clipRect.p1.x += (F32) dx;
    }
    if (S32(viewDesc.dwY + viewDesc.dwHeight) > viewRect.p1.y)
    {
      S32 dy = viewRect.p1.y - (viewDesc.dwY + viewDesc.dwHeight);
      viewDesc.dwHeight += dy;
      Vid::clipRect.p1.y += (F32) dy;
    }
	  viewDesc.dvMinZ			= 0.0f;
	  viewDesc.dvMaxZ			= 1.0f;

    device->SetViewport( &viewDesc);

    Setup( *curCamera);

    //  reset fog
    //
    Vid::SetFogDepth( Vid::renderState.fogDepth);

    // reset bucket sorting
    //
    Vid::SetTranBucketMinZ( curCamera->NearPlane());
	  Vid::SetTranBucketMaxZ( curCamera->FarPlane() * curCamera->Zoom());

    return TRUE;
  }
  //-----------------------------------------------------------------------------

  void ClipScreen()
  {
    if (device)
    {
      ViewPortDescD3D desc = Vid::viewDesc;

      desc.dwWidth  = Vid::viewRect.Width();
      desc.dwX = 0;
      desc.dwHeight = Vid::viewRect.Height();
      desc.dwY = 0;
  	  desc.dvMinZ		= 0.0f;
      desc.dvMaxZ		= 1.0f;

      Vid::device->SetViewport( &desc);

      Vid::clipRect.Set( (F32) viewRect.p0.x, (F32) viewRect.p0.y, (F32) viewRect.p1.x, (F32) viewRect.p1.y);
    }
  }
  //-----------------------------------------------------------------------------

  void ClipRestore()
  {
    if (device)
    {
      Vid::device->SetViewport( &Vid::viewDesc);

      Vid::clipRect.SetSize( (F32) Vid::viewDesc.dwX, (F32) Vid::viewDesc.dwY, (F32) Vid::viewDesc.dwWidth, (F32) Vid::viewDesc.dwHeight); 
    }
  }
  //-----------------------------------------------------------------------------

  void RenderState::ClearData()
  {
	  status.filter         = filterFILTER | filterMIPMAP | filterMIPFILTER;
	  status.shade          = shadeGOURAUD;
	  status.texture        = TRUE;
	  status.zbuffer        = TRUE;
	  status.stencil        = FALSE;
//	  status.specular       = FALSE;
	  status.specular       = TRUE;

	  status.fog            = TRUE;
	  status.alpha          = TRUE;
	  status.texWrap        = TRUE;
	  status.antiAlias      = FALSE;
	  status.texture        = TRUE;
	  status.dither         = TRUE;
	  status.texMovie2D     = TRUE;
	  status.texMovie3D     = TRUE;
    status.clipGuard      = FALSE;
    status.alphaFar       = FALSE;
    status.alphaNear      = FALSE;
    status.hardTL         = FALSE;
    status.dxTL           = FALSE;
	  status.antiAlias	    = FALSE;
	  status.antiAliasEdge  = FALSE;
	  status.texColorKey    = FALSE;
	  status.tex32          = FALSE;
	  status.waitRetrace    = FALSE;
	  status.zbias          = FALSE;
	  status.xmm            = FALSE;

    status.weather        = TRUE;

    fogColorF32.Set( 0, 0, 0, 1);
    fogColor.Set( fogColorF32.r, fogColorF32.g, fogColorF32.b, 1.0f);
    fogMin = STARTNEARPLANE;
    fogMax = STARTFARPLANE;  
    fogDepth = 0.8f;
    fogDensity = 1.0;

    ambientColorF32.Set( 0, 0, 0, 1);

    clearFlags = 0;

    renderFlags = RS_BLEND_DEF;

    zBias = ZBIAS;

    textureReduction = 0;

    cullSign = 0;

    alphaNear = alphaFar = 0;

    texMinimapSize = 64;
  }
  //----------------------------------------------------------------------------

  void SetFogColor( F32 r, F32 g, F32 b)
  {
    renderState.fogColorF32.Set( r, g, b, 1);   
    renderState.fogColor.Set
    ( 
      renderState.fogColorF32.r,
      renderState.fogColorF32.g,
      renderState.fogColorF32.b,
      (U32) 255
    );
    SetFogColorD3D( renderState.fogColor);
  }
  //----------------------------------------------------------------------------

  void SetFogColorD3D( U32 fogColor)
  {
    if (Vid::isStatus.initialized)
    {
      dxError = device->SetRenderState( D3DRENDERSTATE_FOGCOLOR,  fogColor);
      LOG_DXERR( ("SetFogColorD3D") );  
    }
  }
  //----------------------------------------------------------------------------

  void SetFogRange( F32 min, F32 max, F32 density)
  {
    renderState.fogMin = min;
    renderState.fogMax = max;
    renderState.fogDensity = density;

    renderState.fogMinZ = Vid::ProjectZ( min);
    renderState.fogMaxZ = 1.0f;

  //  ASSERT( renderState.fogMaxZ > renderState.fogMinZ);
  //  renderState.fogFactor = 255.0f / (renderState.fogMaxZ - renderState.fogMinZ); 
    renderState.fogFactor = 1.0f / (renderState.fogMaxZ - renderState.fogMinZ); 

    renderState.fogMinZH = Vid::SetHomogeneousZ( min);
    renderState.fogMaxZH = Vid::SetHomogeneousZ( Vid::Math::farPlane);
    renderState.fogFactorH = 1.0f / (renderState.fogMaxZ - renderState.fogMinZ); 

    renderState.fogMinZZ = min;
    renderState.fogMaxZZ = max;
    renderState.fogFactorZ = 1.0f / (renderState.fogMaxZZ - renderState.fogMinZZ); 

#ifndef DODXLEANANDGRUMPY

    if (Vid::isStatus.initialized)
    {
    dxError = device->SetRenderState( D3DRENDERSTATE_FOGSTART, *((U32 *) &renderState.fogMin));
    LOG_DXERR( ("SetFogRange") );
    dxError = device->SetRenderState( D3DRENDERSTATE_FOGEND,   *((U32 *) &renderState.fogMax));
    LOG_DXERR( ("SetFogRange") );

/*
  //  if (caps.fogPixel)
      dxError = device->SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((U32 *) &renderState.fogMin));
      LOG_DXERR( ("SetFogRange") );
      dxError = device->SetRenderState( D3DRENDERSTATE_FOGTABLEEND,   *((U32 *) &renderState.fogMax));
      LOG_DXERR( ("SetFogRange") );
	    dxError = device->SetRenderState( D3DRENDERSTATE_FOGTABLEDENSITY, *((U32 *) &renderState.fogDensity));

  //  else if (caps.fogVertex)
*/
    }
#endif
  }
  //----------------------------------------------------------------------------

  void SetAmbientColor( F32 r, F32 g, F32 b)
  {
    renderState.ambientColorF32.Set( r, g, b, 1);
    renderState.ambientColor.Set( r, g, b, (U32)255);

    if (Vid::isStatus.initialized)
    {
  #ifndef DODXLEANANDGRUMPY
      dxError = device->SetRenderState( D3DRENDERSTATE_AMBIENT, (D3DCOLOR) renderState.ambientColor);
      LOG_DXERR( ("SetAmbientColor") );
  #endif
    }
  }
  //----------------------------------------------------------------------------

  void GetAmbientColor( F32 &r, F32 &g, F32 &b)
  {
    r = renderState.ambientColorF32.r;
    g = renderState.ambientColorF32.g;
    b = renderState.ambientColorF32.b;
  }
  //----------------------------------------------------------------------------

  Bool SetRenderState( Bool checkInit) // TRUE
  {
    if (checkInit && !isStatus.initialized)
    {
      return FALSE;
    }

    // basic features
    //
    dxError = device->SetRenderState( D3DRENDERSTATE_ZFUNC, renderState.status.wbuffer ? D3DCMP_GREATEREQUAL : D3DCMP_LESSEQUAL);
    LOG_DXERR( ("device->SetRenderState") );
 
    dxError = device->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
  //	dxError = device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
  //  dxError = device->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0);
    LOG_DXERR( ("device->SetRenderState: transparent") );

    dxError = device->SetRenderState( D3DRENDERSTATE_CLIPPING, 1);
    LOG_DXERR( ("device->SetRenderState( CLIPPING)") );

    dxError = device->SetRenderState( D3DRENDERSTATE_EXTENTS, 0 );
    LOG_DXERR( ("device->SetRenderState( EXTENTS)") );

    dxError = device->SetRenderState( D3DRENDERSTATE_LIGHTING, 0 );
    LOG_DXERR( ("device->SetRenderState( LIGHTING)") );

//    dxError = device->SetRenderState( D3DRENDERSTATE_COLORVERTEX, renderState.status.dxTL );
//    LOG_DXERR( ("device->SetRenderState( COLORVERTEX)") );

    // vid system features
    //
    SetAmbientColor( renderState.ambientColorF32.r, renderState.ambientColorF32.g, renderState.ambientColorF32.b);

    SetShadeState( renderState.status.shade);
    SetFogState( renderState.status.fog);
    SetFogColor( renderState.fogColorF32.r, renderState.fogColorF32.g, renderState.fogColorF32.b);
    SetFogRange( renderState.fogMin, renderState.fogMax, renderState.fogDensity);

    SetZBufferState(  renderState.status.zbuffer);
    SetSpecularState( renderState.status.specular);
    SetDitherState( renderState.status.dither);
    SetAlphaState(  renderState.status.alpha);

    dxError = device->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE);  // dx fog
    LOG_DXERR( ("device->SetRenderState") );

    SetAntiAliasStateI( renderState.status.antiAlias);
    SetEdgeAntiAliasStateI( renderState.status.antiAliasEdge);
    SetColorKeyStateI( caps.noAlphaMod);
    SetPerspectiveStateI( TRUE);

    SetTextureFactor( Color( U32(255), U32(255), U32(255), U32(122)) );

    SetSrcBlendState( renderState.renderFlags);
    SetDstBlendState( renderState.renderFlags);

    // texture stage related
    //
    SetTexBlendState( renderState.renderFlags);
    SetTexWrapState( renderState.renderFlags);
    SetFilterState( renderState.status.filter);

//    SetBorderColor( 0);

    renderState.clearFlags = clearBACK;
    if (renderState.status.zbuffer)
	  {
		  renderState.clearFlags |= clearZBUFFER;
	  }
    if (renderState.status.stencil)
	  {
		  renderState.clearFlags |= clearSTENCIL;
	  }

    // reset view parameters
    // SetCamera checks current; trick it by setting NULL
    Camera * cam = curCamera;
    curCamera = NULL;
    SetCamera( *cam);

	  return TRUE;
  }
  //----------------------------------------------------------------------------

  Bool RenderBegin()
  {
    bucket.Flush(FALSE);
    tranbucket.Flush(FALSE);

	  dxError = device->BeginScene();
	  if (dxError)
	  {
		  LOG_DXERR( ("BeginScene: device->BeginScene") );
	  }
    texMemPerFrame = 0;

    return dxError == DD_OK;
  }
  //----------------------------------------------------------------------------

  Bool RenderEnd()
  {
    bucket.Flush(TRUE);
    tranbucket.Flush(TRUE);

    if (showTexSwap && texMemPerFrame > totalTexMemory && turtleTex)
    {
      // texture swapping, render turtle
      //
      const Area<S32> rect
      ( 
        viewRect.p1.x - turtleTex->Width() - 4, viewRect.p0.y + 4,
        viewRect.p1.x, viewRect.p0.y + turtleTex->Height()
      );

      Vid::SetTexture( turtleTex, 0, RS_BLEND_ADD);

      // lock vertex memory
	    VertexTL vertmem[4];

      vertmem[0].vv.x = (F32) rect.p0.x;
      vertmem[0].vv.y = (F32) rect.p0.y;
      vertmem[0].vv.z = 0;
      vertmem[0].rhw  = 1;
      vertmem[0].diffuse  = 0xffff0000;
      vertmem[0].specular = 0xff000000;
      vertmem[0].u = 0.0f;
      vertmem[0].v = 0.0f;

      vertmem[1].vv.x = (F32) rect.p1.x;
      vertmem[1].vv.y = (F32) rect.p0.y;
      vertmem[1].vv.z = vertmem[0].vv.z;
      vertmem[1].rhw  = vertmem[0].rhw;
      vertmem[1].diffuse  = 0xffff0000;
      vertmem[1].specular = vertmem[0].specular;
      vertmem[1].u = 1.0f;
      vertmem[1].v = 0.0f;

      vertmem[2].vv.x = (F32) rect.p1.x;
      vertmem[2].vv.y = (F32) rect.p1.y;
      vertmem[2].vv.z = vertmem[0].vv.z;
      vertmem[2].rhw  = vertmem[0].rhw;
      vertmem[2].diffuse  = 0xffff0000;
      vertmem[2].specular = vertmem[0].specular;
      vertmem[2].u = 1.0f;
      vertmem[2].v = 1.0f;

      vertmem[3].vv.x = (F32) rect.p0.x;
      vertmem[3].vv.y = (F32) rect.p1.y;
      vertmem[3].vv.z = vertmem[0].vv.z;
      vertmem[3].rhw  = vertmem[0].rhw;
      vertmem[3].diffuse  = 0xffff0000;
      vertmem[3].specular = vertmem[0].specular;
      vertmem[3].u = 0.0f;
      vertmem[3].v = 1.0f;

		  Vid::DrawIndexedPrimitive(
            PT_TRIANGLELIST,
            FVF_TLVERTEX,
            vertmem, 4, Vid::rectIndices, 6, 
            DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_ADD);
    }


	  dxError = device->EndScene();
 	  LOG_DXERR( ("EndScene: device->EndScene") );

    return dxError == DD_OK;
  }
  //----------------------------------------------------------------------------

  void SetMaterialDX( const Material * mat)
  {
    if (Material::Manager::GetMaterial() != mat)
	  {
	    if (!mat)
	    {
        mat = defMaterial;
	    }
      Material::Manager::SetMaterial( mat);

		  const MaterialDescD3D & matD3D = mat->GetDesc();

		  dxError = device->SetMaterial( (MaterialDescD3D *) &matD3D);
		  LOG_DXERR( ("device->SetMaterial") );
  
    }
  }
  //----------------------------------------------------------------------------

  Bool SetTexture( Bitmap * tex, U32 stage, U32 blend) // = 0, = RS_BLEND_DEF
  {
    if (Vid::renderState.status.texture)
    {
      if(Bitmap::Manager::GetTexture(stage) != tex)
	    {
        Bitmap::Manager::SetTexture(tex, stage);

        TextureHandle texH = tex ? tex->GetTexture() : NULL;

        dxError = device->SetTexture(stage, texH);
        LOG_DXERR( ("SetRenderState( mat): device->RenderState( TEXTUREHANDLE)") );
      }

      SetTexBlendState( blend, stage);

      if (tex)
      {
        SetTexWrapState( blend, stage);
      }
    }
    ASSERT( device);

    if (stage > 0)
    {
      // turn off texturing above this stage
      dxError = device->SetTextureStageState( stage + 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
      dxError = device->SetTextureStageState( stage + 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

      LOG_DXERR( ("SetTextureStageState()") );

      renderState.status.texStaged = TRUE;
    }
    else if (renderState.status.texStaged) 
    {
      // turn off texturing above this stage
      dxError = device->SetTextureStageState( stage + 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
      dxError = device->SetTextureStageState( stage + 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

      LOG_DXERR( ("SetTextureStageState()") );

      renderState.status.texStaged = FALSE;
    }

	  return dxError == DD_OK;
  }
  //----------------------------------------------------------------------------

  Bool SetTextureDX( const Bitmap * tex, U32 stage, U32 blend) // = 0, = RS_BLEND_DEF
  {
    TextureHandle texH = tex ? tex->GetTexture() : NULL;

    dxError = device->SetTexture( stage, texH);
    LOG_DXERR( ("SetRenderState( mat): device->RenderState( TEXTUREHANDLE)") );

    if (tex)
    {
      SetTexBlendState( blend, stage);
      SetTexWrapState( blend, stage);
    }

    // turn off texturing above this stage
    dxError = device->SetTextureStageState( stage + 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    dxError = device->SetTextureStageState( stage + 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    LOG_DXERR( ("SetTextureStageState()") );

	  return dxError == DD_OK;
  }
  //----------------------------------------------------------------------------

  void SetBucketMaterialProc( const Material * material)
  {
    BucketMan::SetMaterial( material);

    if (!renderState.status.dxTL)
    {
      return;
    }

    if (BucketMan::forceTranslucent || (material && material->GetStatus().translucent))
	  {
      LOG_DIAG(( "%s: force %d; mat %d", material->GetName(), BucketMan::forceTranslucent, material->GetStatus().translucent));

      currentBucketMan = &tranbucket;
	  }
	  else
	  {
		  currentBucketMan = &bucket;
	  }
  }
  //----------------------------------------------------------------------------

  // stage 0 MUST be set first!!!
  //
  void SetBucketTexture( const Bitmap *texture, Bool translucent, U32 stage, U32 blend) // = FALSE, = 0, = RS_BLEND_DEF
  {
    if (stage == 0)
    {
      if (BucketMan::forceTranslucent)
      {
        translucent = TRUE;
      }
      if (translucent || (texture && texture->GetStatus().translucent))
	    {
        currentBucketMan = &tranbucket;
	    }
	    else
	    {
		    currentBucketMan = &bucket;
	    }
    }
    BucketMan::SetTexture(texture, stage, blend);
  }
  //----------------------------------------------------------------------------

  Bool DrawPrimitive( 
	  PRIMITIVE_TYPE prim_type,
	  VERTEX_TYPE vert_type,
	  LPVOID verts,
	  DWORD vert_count,
	  DWORD flags)
  {
    // sanity check vertex count
    ASSERT( vert_count >= 1 );
    ASSERT(
      ((prim_type == PT_POINTLIST)) ||
      ((prim_type == PT_LINELIST) && (vert_count % 2 == 0)) ||
      ((prim_type == PT_LINESTRIP) && (vert_count >= 2)) ||
      ((prim_type == PT_TRIANGLELIST) && (vert_count % 3 == 0)) ||
      ((prim_type == PT_TRIANGLESTRIP) && (vert_count >= 3)) ||
      ((prim_type == PT_TRIANGLEFAN) && (vert_count >= 3))
    );

    // Check for errors
	  if (renderState.status.checkVerts && !AreVerticesInRange( (VertexTL *) verts, vert_count, NULL, 0, flags))
    {
      flags &= ~DP_DONOTCLIP;

      LOG_DIAG( ("texture = %s", Bitmap::Manager::GetTexture(0) ? Bitmap::Manager::GetTexture(0)->GetName() : "null") );
    }

#ifndef DODXLEANANDGRUMPY
    if (renderState.status.dxTL)
    {
      SetCullStateD3D( (flags & RS_2SIDED) ? FALSE : TRUE);
    
      dxError = device->SetRenderState( D3DRENDERSTATE_LIGHTING, DWORD( vert_type == FVF_VERTEX ? TRUE : FALSE));
      LOG_DXERR( ("device->SetRenderState( LIGHTING)") );
    }
#endif

    SetSrcBlendState( flags);
    SetDstBlendState( flags);

    dxError = device->SetRenderState( D3DRENDERSTATE_CLIPPING, (DWORD)( flags & DP_DONOTCLIP ? 0 : 1));
    LOG_DXERR( ("device->SetRenderState( CLIPPING)") );

    flags &= DP_MASK;
//    flags &= ~(DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | DP_DONOTLIGHT);

	  // do the d3d call
	  dxError = device->DrawPrimitive( 
		  (D3DPRIMITIVETYPE) prim_type, 
		  vert_type, 
		  verts, 
		  vert_count,
		  flags);
	  LOG_DXERR( ("device->DrawPrimitive: trilist") );

    indexCount += vert_count;

	  return (dxError == D3D_OK);
  }
  //----------------------------------------------------------------------------

  Bool DrawFanStripPrimitive( 
	  PRIMITIVE_TYPE prim_type,
	  VERTEX_TYPE vert_type,
	  LPVOID verts,
	  DWORD vert_count,
	  DWORD flags)
  {
    // sanity check vertex count
    ASSERT( vert_count >= 1 );
    ASSERT(
      ((prim_type == PT_POINTLIST)) ||
      ((prim_type == PT_LINELIST) && (vert_count % 2 == 0)) ||
      ((prim_type == PT_LINESTRIP) && (vert_count >= 2)) ||
      ((prim_type == PT_TRIANGLELIST) && (vert_count % 3 == 0)) ||
      ((prim_type == PT_TRIANGLESTRIP) && (vert_count >= 3)) ||
      ((prim_type == PT_TRIANGLEFAN) && (vert_count >= 3))
    );

    // Check for errors
    if (renderState.status.checkVerts && !AreVerticesInRange( (VertexTL *) verts, vert_count, NULL, 0, flags))
    {
      flags &= ~DP_DONOTCLIP;

      LOG_DIAG( ("texture = %s", Bitmap::Manager::GetTexture(0) ? Bitmap::Manager::GetTexture(0)->GetName() : "null") );
    }

#ifndef DODXLEANANDGRUMPY
    if (renderState.status.dxTL)
    {
      SetCullStateD3D( (flags & RS_2SIDED) ? FALSE : TRUE);

      dxError = device->SetRenderState( D3DRENDERSTATE_LIGHTING, DWORD( vert_type == FVF_VERTEX ? TRUE : FALSE));
      LOG_DXERR( ("device->SetRenderState( LIGHTING)") );
    }
#endif

    SetSrcBlendState( flags);
    SetDstBlendState( flags);

    dxError = device->SetRenderState( D3DRENDERSTATE_CLIPPING, (DWORD)( flags & DP_DONOTCLIP ? 0 : 1));
    LOG_DXERR( ("device->SetRenderState( CLIPPING)") );

    flags &= DP_MASK;
//    flags &= ~(DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | DP_DONOTLIGHT);

	  // do the d3d call
	  dxError = device->DrawPrimitive( 
		  (D3DPRIMITIVETYPE) prim_type, 
		  vert_type, 
		  verts, 
		  vert_count,
		  flags);
	  LOG_DXERR( ("device->DrawPrimitive: trilist") );

    indexCount += vert_count - 2;

	  return (dxError == D3D_OK);
  }
  //----------------------------------------------------------------------------

  Bool DrawIndexedPrimitive( 
	  PRIMITIVE_TYPE prim_type,
	  VERTEX_TYPE vert_type,
	  LPVOID verts,
	  DWORD vert_count,
	  LPWORD indices,
	  DWORD index_count,
	  DWORD flags)
  {
    // sanity check vertex count
    ASSERT( index_count >= 1 );
    ASSERT(
	    ((prim_type == PT_POINTLIST)) ||
	    ((prim_type == PT_LINELIST) && (index_count % 2 == 0)) ||
	    ((prim_type == PT_LINESTRIP) && (index_count >= 2)) ||
	    ((prim_type == PT_TRIANGLELIST) && (index_count % 3 == 0)) ||
	    ((prim_type == PT_TRIANGLESTRIP) && (index_count >= 3)) ||
	    ((prim_type == PT_TRIANGLEFAN) && (index_count >= 3))
	  );

    // Check for errors
	  if (renderState.status.checkVerts && !AreVerticesInRange( (VertexTL *) verts, vert_count, indices, index_count, flags))
    {
      flags &= ~DP_DONOTCLIP;

      LOG_DIAG( ("texture = %s", Bitmap::Manager::GetTexture(0) ? Bitmap::Manager::GetTexture(0)->GetName() : "null") );
    }

#ifndef DODXLEANANDGRUMPY
    if (renderState.status.dxTL && vert_type != FVF_TLVERTEX)
    {
      SetCullStateD3D( (flags & RS_2SIDED) ? FALSE : TRUE);

      dxError = device->SetRenderState( D3DRENDERSTATE_LIGHTING, DWORD( vert_type == FVF_VERTEX ? TRUE : FALSE));
      LOG_DXERR( ("device->SetRenderState( LIGHTING)") );
    }
    else
    {
      SetCullStateD3D( FALSE);

      dxError = device->SetRenderState( D3DRENDERSTATE_LIGHTING, TRUE);
      LOG_DXERR( ("device->SetRenderState( LIGHTING)") );
    }
#endif

    SetSrcBlendState( flags);
    SetDstBlendState( flags);

    dxError = device->SetRenderState( D3DRENDERSTATE_CLIPPING, (DWORD)( flags & DP_DONOTCLIP ? 0 : 1));
    LOG_DXERR( ("device->SetRenderState( CLIPPING)") );

    flags &= DP_MASK;
//    flags &= ~(DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | DP_DONOTLIGHT);

/*
    if (bucketdump)
    {
//      LOG_DIAG( ("pt = %d, vt = %d, f = %d", 
//        prim_type,`
//        /vert_type,
//        flags) );

      U32 i;
      VertexTL *v = (VertexTL *) verts;
      for (i = 0; i < vert_count; i++, v++)
      {
//        LOG_DIAG( ("vv = %f, %f, %f rhw = %f, d = %u s = %u, uv %f, %f", 
//          v->vv.x, v->vv.y, v->vv.z, v->rhw, v->diffuse, v->specular, v->u, v->v) ); 
        v->vv.x = 0.0f;
        v->vv.y = 0.0f;
      }
//      for (i = 0; i < index_count; i++)
//      {
//        U32 in = indices[i];
//        LOG_DIAG( ("i = %d", in) );
//      }
    }
*/

    if (renderState.status.texStaged) 
    {
    }

    // do the d3d call
	  dxError = device->DrawIndexedPrimitive( 
		  (D3DPRIMITIVETYPE) prim_type, 
		  vert_type, 
		  verts, 
		  vert_count,
		  (LPWORD) indices, 
		  index_count,
		  flags);
  #ifdef DEVELOPMENT
    if (dxError)
    {
      LOG_DXERR( ("device->DrawIndexedPrimitive: trilist") );
    }
  #else
    LOG_DXERR( ("device->DrawIndexedPrimitive: trilist") );
  #endif

    indexCount += index_count;

    return (dxError == D3D_OK);
  }
  //----------------------------------------------------------------------------

}
//----------------------------------------------------------------------------

// stage 0 MUST be set first!!!
//
void Bitmap::Manager::SetTexture( Bitmap *texture, U32 stage) // = 0)
{
	ASSERT( stage < MAX_TEXTURE_STAGES );

  if (stage == 0)
  {
		ClearTextures();
	}
  curTextureList[stage] = texture;

  stage += 1;
  if (stage > textureCount)
  {
  	textureCount = stage;
  }

  // textures per frame usage monitoring
  //
  if (Vid::showTexSwap && texture && texture->frameNumber != Main::frameNumber)
  {
    Vid::texMemPerFrame += texture->MemSize();
   
    texture->frameNumber = Main::frameNumber;
  }
}
//----------------------------------------------------------------------------

void VertexTL::SetFog()
{
  // set vertex fog
  //
  S32 fogval = 255;

  if (vv.z > Vid::renderState.fogMinZ)
  {
//    specular.a = Utils::FtoL( 255.f - ((vv.z - renderState.fogMinZ) * renderState.fogFactor));
    F32 fog = (vv.z - Vid::renderState.fogMinZ) * Vid::renderState.fogFactor;
    fog *= fog;   // non-linear
    fogval = Utils::FtoL( 255.f - (fog * 255.0f));
  }
  fogval = Max<S32>( 0, fogval - Vid::extraFog);
  specular.a = (U8) fogval;

  // set vertex transluceny
  //
  if (Vid::renderState.status.alphaFar && vv.z > Vid::renderState.alphaFar)
  {
    Float2Int fa( (vv.z - Vid::renderState.alphaFar) / (1-Vid::renderState.alphaFar) * diffuse.a + Float2Int::magic);
    diffuse.a = U8(diffuse.a - fa.i);
  }
/*
  if (Vid::renderState.status.alphaNear && vv.z < Vid::renderState.alphaNear)
  {
    Float2Int fa( (1 - ((Vid::renderState.alphaNear - vv.z) / Vid::renderState.alphaNear)) * diffuse.a + Float2Int::magic);
    diffuse.a = U8(diffuse.a + fa.i);
  }
*/
}
//----------------------------------------------------------------------------

void VertexTL::SetFogX()
{
  // set vertex fog
  //
  S32 fogval = 255;

  if (vv.z > Vid::renderState.fogMinZ)
  {
    F32 fog = (vv.z - Vid::renderState.fogMinZ) * Vid::renderState.fogFactor;
    fog *= fog;   // non-linear
    fogval = Utils::FtoL( 255.f - (fog * 255.0f));
  }
  fogval = Max<S32>( 0, fogval - specular.a);
  specular.a = (U8) fogval;

  // set vertex transluceny
  //
  if (Vid::renderState.status.alphaFar && vv.z > Vid::renderState.alphaFar)
  {
    Float2Int fa( (vv.z - Vid::renderState.alphaFar) / (1-Vid::renderState.alphaFar) * diffuse.a + Float2Int::magic);
    diffuse.a = U8(diffuse.a - fa.i);
  }
/*
  if (Vid::renderState.status.alphaNear && vv.z < Vid::renderState.alphaNear)
  {
    Float2Int fa( (vv.z - Vid::renderState.alphaFar) / Vid::renderState.alphaNear * diffuse.a + Float2Int::magic);
    diffuse.a = U8(diffuse.a + fa.i);
  }
*/
}
//----------------------------------------------------------------------------

void VertexTL::SetFogH()
{
  // set vertex fog
  if (vv.z > Vid::renderState.fogMinZH)
  {
    F32 fog = (vv.z - Vid::renderState.fogMinZH) * Vid::renderState.fogFactorH;
    fog *= fog;   // non-linear
    specular.a = (U8)Utils::FtoL( 255.f - (fog * 255.0f));
  }
}
//----------------------------------------------------------------------------
