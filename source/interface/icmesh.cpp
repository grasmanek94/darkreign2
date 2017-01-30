/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 15-MAR-2000
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"
#include "icmesh.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "input.h"
#include "debug_memory.h"
#include "main.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class ICMesh - Standard mesh view control
//


//
// ICMesh::ICMesh
//
ICMesh::ICMesh(IControl *parent)
: IControl(parent),
  root(NULL),
  ent(NULL),
  rotateRate(PI/4),
  offsetAuto(1),
  pivotRoot(0),
  pivotSet(1),
  offset( 0, 0, 0),
  pivot(0, 0, 0),
  sunVector( -1, -1, -1),
  sunColor( 1.0f, 1.0f, 1.0f)
{
  sunVector.Normalize();

  // Default Control style
  controlStyle |= STYLE_TABSTOP;
}


//
// ICMesh::~ICMesh
//
ICMesh::~ICMesh()
{
  if (ent)
  {
    delete ent;
  }
}


void ICMesh::DrawBack( PaintInfo & pi, F32 zDepth, Color color, Bitmap * tex)
{
  pi;

  const Area<S32> & rect = Vid::CurCamera().ViewRect();

  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_NOSORT | RS_BLEND_MODULATE);

  Vid::SetBucketTexture( tex, color.a < 255, 0, RS_BLEND_MODULATE);

  // lock vertex memory
	VertexTL *vertmem;
	U16 *indexmem;
  if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 6, &indexmem, 6))
	{
    return;
  }

  vertmem[0].vv.x = (F32) rect.p0.x;
  vertmem[0].vv.y = (F32) rect.p0.y;
  vertmem[0].vv.z = Vid::ProjectZ(zDepth);
  vertmem[0].rhw  = Vid::RHW(zDepth);
  vertmem[0].diffuse  = color;
  vertmem[0].specular = 0xff000000;
  vertmem[0].u = 0.0f;
  vertmem[0].v = 0.0f;

  vertmem[1].vv.x = (F32) rect.p1.x;
  vertmem[1].vv.y = (F32) rect.p0.y;
  vertmem[1].vv.z = vertmem[0].vv.z;
  vertmem[1].rhw  = vertmem[0].rhw;
  vertmem[1].diffuse  = color;
  vertmem[1].specular = vertmem[0].specular;
  vertmem[1].u = 1.0f;
  vertmem[1].v = 0.0f;

  vertmem[2].vv.x = (F32) rect.p1.x;
  vertmem[2].vv.y = (F32) rect.p1.y;
  vertmem[2].vv.z = vertmem[0].vv.z;
  vertmem[2].rhw  = vertmem[0].rhw;
  vertmem[2].diffuse  = color;
  vertmem[2].specular = vertmem[0].specular;
  vertmem[2].u = 1.0f;
  vertmem[2].v = 1.0f;

  vertmem[3].vv.x = (F32) rect.p0.x;
  vertmem[3].vv.y = (F32) rect.p1.y;
  vertmem[3].vv.z = vertmem[0].vv.z;
  vertmem[3].rhw  = vertmem[0].rhw;
  vertmem[3].diffuse  = color;
  vertmem[3].specular = vertmem[0].specular;
  vertmem[3].u = 0.0f;
  vertmem[3].v = 1.0f;

  Utils::Memcpy( indexmem, Vid::rectIndices, 12);

  // submit the polygons
  Vid::UnlockIndexedPrimitiveMem( 4, 6);
}
//----------------------------------------------------------------------------


//
// ICMesh::DrawSelf
//
// Draw the button control
//
void ICMesh::DrawSelf( PaintInfo & pi)
{
  // flush buckets, release iface temp memory
  //
  IFace::DoneBuckets();

  // save camera
  //
  Camera & cam = Vid::CurCamera();
  cam.Save();
  
  // setup camera for control
  //
  Area<S32> rect(pi.client); // GetScreenPos(), GetSize());
  //rect.SetSize( rect.p1.x, rect.p1.y);

  cam.SetProjTransform(.1f, F32(fabs(offset.z)) + ent->RootBounds().Radius() * 2.2f, cam.FOV());
  cam.Setup( rect);

  // clear zbuffer and background
  //
  Vid::RenderClear( Vid::clearZBUFFER); 
  if (GetTexture() && GetTexture()->texture)
  {
    DrawBack( pi, Vid::Math::farPlane, 0xffffffff, GetTexture()->texture);
  }

  if (ent && ent->vertCount)
  {
    // render mesh
    //
    if (offsetAuto)
    {
      // calc offset
      //
      offset.x = 0;
      offset.y = 0;
//      offset.z = -(ent->RootBounds().Radius() * 2 + Vid::Math::nearPlane);
      offset.z = -(ent->ObjectBoundsRender().Radius() * 2 + Vid::Math::nearPlane);
    }
    // calc camera position
    //
    Matrix world( Quaternion( PI, Matrix::I.up));
    Vector v = ent->OriginRender() + offset;
    world.Transform( world.posit, v);
    cam.SetWorldAll( world);

    if (!ent->EntParent())
    {
      // move/animate, copy target states to current states
      //
      ent->UpdateSim( Main::elapSecs);
      ent->SimulateInt( Main::elapSecs);

      Quaternion q = ent->GetSimRotation() * Quaternion( Main::elapSecs * rotateRate, Matrix::I.up);
      Matrix m( q);
      Vector p;
      m.Rotate( p, pivot);
      ent->SetSimTarget( q, p);
    }
    // save and setup light for control
    //
    Vid::Light::SaveSun();
    Vid::Light::SetSun( sunVector, sunColor.r, sunColor.g, sunColor.b);
    U32 alphaNear = Vid::renderState.status.alphaNear;
    Vid::renderState.status.alphaNear = 0;

    ent->Render();

    Vid::renderState.status.alphaNear = alphaNear;
    Vid::Light::RestoreSun();
  }
  cam.Restore();

  // flush
  //

  U32  filter = Vid::SetFilterStateI( *Vid::Var::varFilter, *Vid::Var::varMipmap, *Vid::Var::varMipfilter);
  Bool fog    = Vid::SetFogStateI( FALSE);
  U32  persp  = Vid::SetPerspectiveStateI( TRUE);
  U32  alpha  = Vid::SetAlphaState( FALSE);

  Vid::FlushBuckets();
  Vid::SetAlphaState( alpha);
  Vid::FlushTranBuckets();

  Vid::SetPerspectiveStateI( persp);
  Vid::SetFogStateI( fog);
  Vid::SetFilterStateI( filter);

  // restore iface temp memory
  //
  IFace::InitBuckets();
}


//
// ICMesh::AdjustGeometry
//
// Adjust geometry of the button
//
void ICMesh::AdjustGeometry()
{
  // Call base class
  IControl::AdjustGeometry();
}


//
// ICMesh::Setup
//
// Configure this control with an FScope
//
void ICMesh::Setup( FScope * fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x63E93484: // "GodFile"
      if (const char * s = fScope->NextArgString())
      {
        root = Mesh::Manager::FindRead( s, Vid::Var::scaleFactor, Vid::Var::doMrmGen);
      }
      break;
    case 0xFE3B31A3: // "Offset"
      offset.x = fScope->NextArgFPoint();
      offset.y = fScope->NextArgFPoint();
      offset.z = fScope->NextArgFPoint();
      break;
    case 0xDA14B99B: // "SunVector"
      sunVector.x = fScope->NextArgFPoint();
      sunVector.y = fScope->NextArgFPoint();
      sunVector.z = fScope->NextArgFPoint();
      sunVector.Normalize();
      break;
    case 0xB0D4BB6F: // "SunColor"
      sunColor.r = fScope->NextArgFPoint();
      sunColor.g = fScope->NextArgFPoint();
      sunColor.b = fScope->NextArgFPoint();
      break;
    case 0x4EA17D75: // "OffsetAuto"
      offsetAuto = fScope->NextArgInteger();
      break;
    case 0x26C0F497: // "RotateRate"
      rotateRate = fScope->NextArgFPoint();
      break;
    case 0xBAB1E5A5: // "Animation"
      anim = fScope->NextArgString();
      break;
    case 0x5B5A02BC: // "Pivot"
      if (fScope->IsNextArgString())
      {
        const char * s = fScope->NextArgString();
        pivotRoot = Crc::CalcStr(s) == 0x212D6FAE; // "Root"
        pivotSet = TRUE;
      }
      else
      {
        pivot.x = fScope->NextArgFPoint();
        pivot.y = fScope->NextArgFPoint();
        pivot.z = fScope->NextArgFPoint();
        pivotSet = FALSE;
      }
      break;

    default:
    {
      // Pass it to the previous level in the hierarchy
      IControl::Setup(fScope);
      break;
    }
  }
};


void ICMesh::PostConfigure()
{
  if (!root)
  {
    root = Mesh::Manager::MakeGlobe(1, 11, Bitmap::Manager::FindCreate(Bitmap::reduceMED, "engine_earth.tga"));
    root->WriteXSI("sphere.xsi");
  }

  ASSERT(root)
  if (!ent)
  {
    ent = Mesh::Manager::Create(root);
    if (ent && *anim.str)
    {
      ent->SetAnimCycle(anim.str);
    }
    if (pivotSet)
    {
//      pivot = pivotRoot ? Vector(0,0,0) : ent->RootBounds().Offset() * -1;
      pivot = pivotRoot ? Vector(0,0,0) : ent->ObjectBoundsRender().Offset() * -1;
      pivot.y = 0;
    }
  }

  IControl::PostConfigure();
}


//
// ICMesh::HandleEvent
//
// Process input events
//
U32 ICMesh::HandleEvent(Event &e)
{
#if 0
  if (e.type == Input::EventID())
  {
    // Input events
    switch (e.subType)
    {
      case Input::MOUSEBUTTONDOWN:
      case Input::MOUSEBUTTONDBLCLK:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (controlState & STATE_DISABLED)
          {
            SendNotify(this, ICButtonNotify::DisabledDown);
          }
          else
          {
            if (buttonStyle & STYLE_TOGGLE)
            {
              // Generate message
              SendNotify(this, ICButtonMsg::Toggle, FALSE);

              // Play click sound
              IFace::Sound::Play(soundClick, this);
            }
            else
            {
              SetState(BS_DOWN);
              GetMouseCapture();
              GetKeyFocus();
              SendNotify(this, ICButtonNotify::Pressing);
              return TRUE;
            }
          }
        }
        break;
      }

      case Input::MOUSEBUTTONUP:
      case Input::MOUSEBUTTONDBLCLKUP:
      {
        if (e.input.code == Input::LeftButtonCode())
        {
          if (!(buttonStyle & STYLE_TOGGLE))
          {
            // If mouse is over the button, generate a message
            if (buttonState == BS_DOWN)
            {
              // Set button state
              SetState(BS_UP);

              // Generate message
              SendNotify(this, ICButtonMsg::Press, FALSE);
            }

            // Release keyboard focus
            if (HasKeyFocus())
            {
              ReleaseKeyFocus();
            }

            // Release mouse capture
            if (HasMouseCapture())
            {
              ReleaseMouseCapture();
            }

            // Play click sound
            IFace::Sound::Play(soundClick, this);
          }

          // Handled
          return (TRUE);
        }

        // Not handled
        break;
      }

      case Input::MOUSEMOVE:
      {
        if (HasMouseCapture())
        {
          // Toggle the up state depending on the mouse position
          if (InWindow(Point<S32>(e.input.mouseX, e.input.mouseY)))
          {
            SetState(BS_DOWN);
          }
          else
          {
            SetState(BS_UP);
          }

          // Handled
          return (TRUE);
        }

        // Not handled
        break;
      }

      case Input::KEYDOWN:
      {
        switch (e.input.code)
        {
          case DIK_RETURN:
          case DIK_SPACE:
          {
            // Enter and space simulate left button clicks
            if (!(controlState & STATE_DISABLED))
            {
              // Generate message
              if (buttonStyle & STYLE_TOGGLE)
              {
                SendNotify(this, ICButtonMsg::Toggle, FALSE);
              }
              else
              {
                SendNotify(this, ICButtonMsg::Press, FALSE);
              }

              // Handled
              return (TRUE);
            }

            // Not handled
            break;
          }
        }

        // Not handled
        break;
      }
    }
  }
  else 
    
  if (e.type == IFace::EventID())
  {
    // Interface events
    switch (e.subType)
    {
      // Notification events
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case ICButtonMsg::Press:
          {
            // Generate pressed notification
            SendNotify(this, ICButtonNotify::Pressed);

            // Handled
            return (TRUE);
          }

          case ICButtonMsg::Toggle:
          {
            if (!(controlState & STATE_DISABLED))
            {
              if (toggleVar)
              {
                VALIDATE(toggleVar);

                // Set the value of the var
                toggleVar->SetIntegerValue(!toggleVar->GetIntegerValue());

                // Generate toggled notification
                SendNotify(this, ICButtonNotify::Toggled);
              }
            }

            // Handled
            return (TRUE);
          }
        }
        break;
      }

      case IFace::LOSTCAPTURE:
      case IFace::LOSTFOCUS:
      {
        // Clear state
        if (!(buttonStyle & STYLE_TOGGLE))
        {
          SetState(BS_UP);
        }

        // Pass through to IControl
        break;
      }

      case IFace::MOUSEIN:
      {
        // Play mouse in sound
        IFace::Sound::Play(soundOver, this);
        break;
      }
    }
  }
#endif

  // Allow IControl class to process this event
  return (IControl::HandleEvent(e));
}


//
// Set mesh
//
void ICMesh::SetMesh(MeshRoot *r, const char *anim)
{
  root = r;
  if (ent)
  {
    delete ent;
  }

  ent = Mesh::Manager::Create(root);
  if (anim)
  {
    ent->SetAnimCycle(anim);

    if (pivotSet)
    {
//      pivot = pivotRoot ? Vector(0,0,0) : ent->RootBounds().Offset() * -1;
      pivot = pivotRoot ? Vector(0,0,0) : ent->ObjectBoundsRender().Offset() * -1;
      pivot.y = 0;
    }
  }
}