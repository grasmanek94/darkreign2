///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Light Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_lighteditor.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "ifvar.h"
#include "icstatic.h"
#include "bitmap.h"
#include "input.h"
#include "environment_light.h"
#include "colorbutton.h"
#include "font.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class LightColor
  //
  class LightColor : public ICStatic
  {
    PROMOTE_LINK(LightColor, ICStatic, 0xE6E5C3AE) // "LightColor"

  public:

    // Light color
    Environment::Light::LightColor *lightColor;

  public:

    // Constructor and Destructor
    LightColor(Environment::Light::LightColor *lightColor, IControl *parent);
    ~LightColor();

  protected:

    // Redraw self
    void DrawSelf(PaintInfo &pi);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class LightEditor
  //

  //
  // Constructor
  //
  LightEditor::LightEditor(IControl *parent)
  : ICWindow(parent),
    mode(STOPPED),
    preview(0, 0),
    previewHeight(100),
    current(0, 0),
    currentHeight(50)
  {
    // Set the poll interval to 20x per second
    SetPollInterval(50);
  }


  //
  // Destructor
  //
  LightEditor::~LightEditor()
  {
  }


  //
  // Setup this control from one scope function
  //
  void LightEditor::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xD6AC7759: // "Preview"
        StdLoad::TypePoint<S32>(fScope, preview);
        break;

      case 0xC4C26FC5: // "PreviewHeight"
        previewHeight = StdLoad::TypeU32(fScope);
        break;

      case 0x587C9FAF: // "Current"
        StdLoad::TypePoint<S32>(fScope, current);
        break;

      case 0xB6F2B859: // "CurrentHeight"
        currentHeight = StdLoad::TypeU32(fScope);
        break;

      default:
        ICWindow::Setup(fScope);
        break;
    }
  }

  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 LightEditor::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            // Add a new light color and copy the values from the currently selected item
            case LightEditorMsg::Copy:
            {
              // Get the list box
              ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);

              // Get the currently selected color
              IControl *ctrl = listBox->GetSelectedItem();
              if (!ctrl)
              {
                break;
              }
              LightColor *currentColor = IFace::Promote<LightColor>(ctrl, TRUE);

              // Create a new light color
              Environment::Light::LightColor *color = new Environment::Light::LightColor(*(currentColor->lightColor));

              // Add it to the environment light system
              Environment::Light::AddColor(color);

              // Add it to the list
              char buf[10];
              Utils::Sprintf(buf, 10, "%.3f", color->fraction);
              LightColor *lightColor = new LightColor(color, listBox);
              listBox->AddItem(buf, lightColor);
              listBox->Sort();

              // Select it
              listBox->SetSelectedItem(lightColor);

              goto Edit;

              break;
            }

            // Add a new light color to the list
            case LightEditorMsg::Add:
            {
              // Create a new light color
              Environment::Light::LightColor *color = new Environment::Light::LightColor(Environment::Light::DefaultColor());

              // Add it to the environment light system
              Environment::Light::AddColor(color);

              // Add it to the list
              char buf[10];
              Utils::Sprintf(buf, 10, "%.3f", color->fraction);
              ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);
              LightColor *lightColor = new LightColor(color, listBox);
              listBox->AddItem(buf, lightColor);
              listBox->Sort();

              // Select it
              listBox->SetSelectedItem(lightColor);

              // Fall through into edit
            }

            case LightEditorMsg::Edit:
            Edit:
            {

              // Get the list box
              ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);

              // Get the currently selected color
              IControl *ctrl = listBox->GetSelectedItem();
              if (!ctrl)
              {
                break;
              }
              LightColor *color = IFace::Promote<LightColor>(ctrl, TRUE);

              // Find the light editor
              ICWindow *editor = IFace::Find<ICWindow>("LightEditor", NULL, TRUE);

              // Upload all of the color values
              VarSys::VarItem *var;
              var = VarSys::FindVarItem(editor->DynVarName("fraction"));
              if (var)
              {
                var->SetFloat(color->lightColor->fraction);
              }
              var = VarSys::FindVarItem(editor->DynVarName("ambient"));
              if (var)
              {
                var->SetInteger(color->lightColor->ambient);
              }
              var = VarSys::FindVarItem(editor->DynVarName("sun"));
              if (var)
              {
                var->SetInteger(color->lightColor->sun);
              }
              var = VarSys::FindVarItem(editor->DynVarName("fog"));
              if (var)
              {
                var->SetInteger(color->lightColor->fog);
              }

              // Activate it
              IFace::Activate(editor);
             
              break;
            }

            // Grab the color from the editor
            case LightEditorMsg::Grab:
            {
              // Get the list box
              ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);

              // Get the currently selected color
              IControl *ctrl = listBox->GetSelectedItem();
              if (!ctrl)
              {
                break;
              }
              LightColor *color = IFace::Promote<LightColor>(ctrl, TRUE);

              // Find the light editor
              ICWindow *editor = IFace::Find<ICWindow>("LightEditor", NULL, TRUE);

              // Download all of the color values
              VarSys::VarItem *var;
              var = VarSys::FindVarItem(editor->DynVarName("fraction"));
              if (var)
              {
                color->lightColor->fraction = var->Float();
              }
              var = VarSys::FindVarItem(editor->DynVarName("ambient"));
              if (var)
              {
                color->lightColor->ambient = var->Integer();
              }
              var = VarSys::FindVarItem(editor->DynVarName("sun"));
              if (var)
              {
                color->lightColor->sun = var->Integer();
              }
              var = VarSys::FindVarItem(editor->DynVarName("fog"));
              if (var)
              {
                color->lightColor->fog = var->Integer();
              }

              // Add it to the environment light system
              Environment::Light::RemoveColor(color->lightColor);
              Environment::Light::AddColor(color->lightColor);

              // Change the key of this item and resort the list
              char buf[10];
              Utils::Sprintf(buf, 10, "%.3f", color->lightColor->fraction);
              color->SetName(buf);
              listBox->Sort();

              break;
            }
           
            case LightEditorMsg::Delete:
            {
              // Get the list box
              ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);

              // Get the currently selected color
              IControl *ctrl = listBox->GetSelectedItem();
              if (!ctrl)
              {
                break;
              }
              LightColor *color = IFace::Promote<LightColor>(ctrl, TRUE);

              // Remove the color from the light system
              Environment::Light::RemoveColor(color->lightColor);

              // Delete the light color
              delete color->lightColor;

              // Remove this control
              color->MarkForDeletion();

              break;
            }

            case LightEditorMsg::Smooth:
            {
              // Get the list box
              ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);

              // Get the currently selected color
              IControl *ctrl = listBox->GetSelectedItem();
              if (!ctrl)
              {
                break;
              }
              LightColor *color = IFace::Promote<LightColor>(ctrl, TRUE);

              // Remove the color from the light system
              Environment::Light::RemoveColor(color->lightColor);

              // Fill in this color as if it wasn't here
              Environment::Light::FillInColor(*color->lightColor);

              // Readd the color
              Environment::Light::AddColor(color->lightColor);

              break;
            }

            case LightEditorMsg::Stop:
              mode = STOPPED;
              break;

            case LightEditorMsg::Play:
              mode = PLAYING;
              break;

            case LightEditorMsg::FastForward:
              mode = FASTFORWARDING;
              break;

            case LightEditorMsg::Reverse:
              mode = REVERSING;
              break;

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // DrawSelf
  //
  // Control draws itself
  //
  void LightEditor::DrawSelf(PaintInfo &pi)
  {
    ICWindow::DrawSelf(pi);

    // Draw the preview display

    // Draw the backgrounds

    ClipRect ambient(preview.x + 5, preview.y, preview.x + 35, preview.y + previewHeight);
    ClipRect sun(preview.x + 40, preview.y, preview.x + 70, preview.y + previewHeight);
    ClipRect fog(preview.x + 75, preview.y, preview.x + 105, preview.y + previewHeight);

    // Render the drop shadows
    IFace::RenderShadow(ambient + pi.client.p0, ambient + pi.client.p0 + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderShadow(sun + pi.client.p0, sun + pi.client.p0 + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderShadow(fog + pi.client.p0, fog + pi.client.p0 + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);

    // Get the colors from the environment light stuff
    Environment::Light::LightColor *colors = Environment::Light::GetColors();

    // Are there any colors ?
    if (colors)
    {
      // Move the colors pointer to the first point
      while (colors->fraction > colors->prev->fraction)
      {
        colors = colors->prev;
      }

      Environment::Light::LightColor *firstColor = colors;

      Environment::Light::LightColor color1;
      Environment::Light::LightColor color2;

      // Set the first color to be the color when fraction is zero
      color1.fraction = 0.0f;
      Environment::Light::FillInColor(color1);

      // Go through all of the colors
      do
      {
        // Copy the current color to color2
        color2 = *colors;

        // Render ambient
        IFace::RenderGradient(
          ClipRect(
            pi.client.p0 + ambient.p0 + Point<S32>(0, S32(previewHeight * color1.fraction)),
            pi.client.p0 + ambient.p0 + Point<S32>(ambient.Width(), S32(previewHeight * color2.fraction))),
          color1.ambient,
          color2.ambient);

        // Render sun
        IFace::RenderGradient(
          ClipRect(
            pi.client.p0 + sun.p0 + Point<S32>(0, S32(previewHeight * color1.fraction)),
            pi.client.p0 + sun.p0 + Point<S32>(sun.Width(), S32(previewHeight * color2.fraction))),
          color1.sun,
          color2.sun);

        // Render fog
        IFace::RenderGradient(
          ClipRect(
            pi.client.p0 + fog.p0 + Point<S32>(0, S32(previewHeight * color1.fraction)),
            pi.client.p0 + fog.p0 + Point<S32>(fog.Width(), S32(previewHeight * color2.fraction))),
          color1.fog,
          color2.fog);

        // Shuffle color2 to color1
        color1 = color2;

        // Go to the next color
        colors = colors->next;
      }
      while (colors != firstColor);

      // Set the last color to be the color when fraction is one
      color2.fraction = 1.0f;
      Environment::Light::FillInColor(color2);

      // Render ambient
      IFace::RenderGradient(
        ClipRect(
          pi.client.p0 + ambient.p0 + Point<S32>(0, S32(previewHeight * color1.fraction)),
          pi.client.p0 + ambient.p0 + Point<S32>(ambient.Width(), S32(previewHeight * color2.fraction))),
        color1.ambient,
        color2.ambient);

      // Render sun
      IFace::RenderGradient(
        ClipRect(
          pi.client.p0 + sun.p0 + Point<S32>(0, S32(previewHeight * color1.fraction)),
          pi.client.p0 + sun.p0 + Point<S32>(sun.Width(), S32(previewHeight * color2.fraction))),
        color1.sun,
        color2.sun);

      // Render fog
      IFace::RenderGradient(
        ClipRect(
          pi.client.p0 + fog.p0 + Point<S32>(0, S32(previewHeight * color1.fraction)),
          pi.client.p0 + fog.p0 + Point<S32>(fog.Width(), S32(previewHeight * color2.fraction))),
        color1.fog,
        color2.fog);
    }
    else
    {
      Environment::Light::LightColor color = Environment::Light::DefaultColor();

      IFace::RenderRectangle(ambient + pi.client.p0, color.ambient);
      IFace::RenderRectangle(sun + pi.client.p0, color.sun);
      IFace::RenderRectangle(fog + pi.client.p0, color.fog);
    }

    // Draw the current position
    IFace::RenderRectangle(
      ClipRect(
        pi.client.p0.x + preview.x,
        pi.client.p0.y + preview.y + S32(previewHeight * Environment::Light::GetStartTimeVar()),
        pi.client.p0.x + preview.x + 110,
        pi.client.p0.y + preview.y + S32(previewHeight * Environment::Light::GetStartTimeVar()) + 1),
        Color(1.0f, 1.0f, 1.0f));

    // Draw the current colors
    ambient = ClipRect(current.x + 5, current.y, current.x + 35, current.y + currentHeight);
    sun = ClipRect(current.x + 40, current.y, current.x + 70, current.y + currentHeight);
    fog = ClipRect(current.x + 75, current.y, current.x + 105, current.y + currentHeight);

    // Render the drop shadows
    IFace::RenderShadow(ambient + pi.client.p0, ambient + pi.client.p0 + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderShadow(sun + pi.client.p0, sun + pi.client.p0 + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderShadow(fog + pi.client.p0, fog + pi.client.p0 + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);

    Environment::Light::LightColor color;
    color.fraction = Environment::Light::GetStartTimeVar();
    Environment::Light::FillInColor(color);

    // Render the colors
    IFace::RenderRectangle(ambient + pi.client.p0, color.ambient);
    IFace::RenderRectangle(sun + pi.client.p0, color.sun);
    IFace::RenderRectangle(fog + pi.client.p0, color.fog);
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool LightEditor::Activate()
  {
    if (ICWindow::Activate())
    {
      // Upload to the control
      Upload();
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool LightEditor::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Poll
  //
  void LightEditor::Poll()
  {
    switch (mode)
    {
      case STOPPED:
        break;

      case PLAYING:
        if (Environment::Light::GetStartTimeVar() > 0.99f)
        {
          Environment::Light::GetStartTimeVar() = Environment::Light::GetStartTimeVar() + 0.01f - 1.0f;
        }
        else
        {
          Environment::Light::GetStartTimeVar() = Environment::Light::GetStartTimeVar() + 0.01f;
        }
        break;

      case FASTFORWARDING:
        if (Environment::Light::GetStartTimeVar() > 0.98f)
        {
          Environment::Light::GetStartTimeVar() = Environment::Light::GetStartTimeVar() + 0.02f - 1.0f;
        }
        else
        {
          Environment::Light::GetStartTimeVar() = Environment::Light::GetStartTimeVar() + 0.02f;
        }
        break;

      case REVERSING:
        if (Environment::Light::GetStartTimeVar() < 0.02f)
        {
          Environment::Light::GetStartTimeVar() = Environment::Light::GetStartTimeVar() - 0.02f + 1.0f;
        }
        else
        {
          Environment::Light::GetStartTimeVar() = Environment::Light::GetStartTimeVar() - 0.02f;
        }
        break;

    }
  }



  //
  // Upload
  //
  void LightEditor::Upload()
  {
    ICListBox *listBox = IFace::Find<ICListBox>("Colors", this, TRUE);

    // Delete all of the items in the list box
    listBox->DeleteAllItems();

    // Get the colors from the environment light stuff
    Environment::Light::LightColor *colors = Environment::Light::GetColors();

    if (colors)
    {
      Environment::Light::LightColor *ptr = colors;

      do
      {
        char buf[10];
        Utils::Sprintf(buf, 10, "%.3f", ptr->fraction);

        listBox->AddItem(buf, new LightColor(ptr, listBox));
        ptr = ptr->next;
      }
      while (ptr != colors);
    }

    // Sort the list
    listBox->Sort();
  }

  
  //
  // Download
  //
  void LightEditor::Download()
  {
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class LightColor
  //


  //
  // LightColor::LightColor
  //
  LightColor::LightColor(Environment::Light::LightColor *lightColor, IControl *parent)
  : ICStatic(parent),
    lightColor(lightColor)
  {
  }


  //
  // LightColor::~LightColor
  //
  LightColor::~LightColor()
  {
  }


  //
  // LightColor::DrawSelf
  //
  void LightColor::DrawSelf(PaintInfo &pi)
  {
    ASSERT(lightColor)

    DrawCtrlBackground(pi, GetTexture());
    DrawCtrlFrame(pi);

    // Draw the fraction
    if (pi.font)
    {
      U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;
      CH buf[30];
      Utils::Sprintf(buf, 30, L"%.3f", lightColor->fraction);

      pi.font->Draw(
        pi.client.p0.x + 2,
        pi.client.p0.y + yoffs, 
        buf, 
        Utils::Strlen(buf), 
        pi.colors->fg[ColorIndex()],
        &pi.client
      );
    }

    // Draw the ambient color
    ClipRect c(
      pi.client.p0.x + 40, pi.client.p0.y + 3,
      pi.client.p0.x + 70, pi.client.p1.y - 3);
    IFace::RenderShadow(c, c + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderRectangle(c, lightColor->ambient);

    // Draw the sun color
    c += Point<S32>(35, 0);
    IFace::RenderShadow(c, c + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderRectangle(c, lightColor->sun);

    // Draw the fog color
    c += Point<S32>(35, 0);
    IFace::RenderShadow(c, c + IFace::GetMetric(IFace::DROPSHADOW_UP), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
    IFace::RenderRectangle(c, lightColor->fog);
  }

}
