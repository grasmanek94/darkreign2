/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Listbox Control
//
// 20-JUL-98
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icmonoview.h"
#include "mono.h"
#include "font.h"
#include "iface_util.h"
#include "stdload.h"

//
// ICMonoView::ICMonoView
//
ICMonoView::ICMonoView(IControl *parent)
: IControl(parent)
{
  cell.x = 5;
  cell.y = 5;

  colorBg.Set(0L, 0L, 0L);
  colorFg.Set(200, 100L, 0L);
  colorBright.Set(255L, 128L, 0L);
}


//
// ICMonoView::Setup
//
void ICMonoView::Setup(FScope *fScope)
{
  switch (fScope->NameCrc())
  {
    case 0x1FD28C97: // "Cell"
    {
      StdLoad::TypePoint<S32>(fScope, cell);
      break;
    }

    case 0x0D9DD4C1: // "FgColor"
    {
      StdLoad::TypeColor(fScope, colorFg);
      break;
    }

    case 0x34016932: // "BgColor"
    {
      StdLoad::TypeColor(fScope, colorBg);
      break;
    }

    case 0xBC16B564: // "BrightColor"
    {
      StdLoad::TypeColor(fScope, colorBright);
      break;
    }

    default:
      IControl::Setup(fScope);
  }
}


//
// ICMonoView::DrawSelf
//
void ICMonoView::DrawSelf(PaintInfo &pi)
{
  // Standard drawing
  DrawCtrlBackground(pi, GetTexture());
  DrawCtrlFrame(pi);

#ifndef MONO_DISABLED
  if (pi.font)
  {
    U8 *monoPtr = Mono::ScreenPtr();

    if (monoPtr)
    {
      for (U32 r = 0; r < 25; r++)
      {
        U8 *rowPtr = monoPtr + (r * 160);

        for (U32 c = 0; c < 80; c++)
        {
          U8 chr = rowPtr[c * 2];
          U8 atr = rowPtr[c * 2 + 1];

          if (atr != Mono::BLANK)
          {
            S32 x = pi.client.p0.x + (c * cell.x);
            S32 y = pi.client.p0.y + (r * cell.y);
            Color textClr = colorFg;

            if (atr == Mono::REVERSE)
            {
              IFace::RenderRectangle(ClipRect(x, y, x + cell.x + 1, y + cell.y + 1), colorFg);
              textClr = colorBg;
            }
            else if (atr == Mono::BRIGHT)
            {
              textClr = colorBright;
            }

            if (isprint(chr))
            {
              CH wideCh = CH(chr);

              pi.font->Draw(
                x, y,
                &wideCh, 1,
                textClr,
                &pi.client,
                pi.alphaScale);
            }
          }
        }
      }
    }
  }
#else
  pi;
#endif
}
