///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bitmap graphics library
//
// 06-NOV-1997
//


#include "bitmapprimitive.h"
#include "bitmap.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace BitmapPrim
//
namespace BitmapPrim
{
  static Bool init = FALSE;

  // Data for line drawing routines
  static U8 *data;
  static U32 lineColor;
  static S32 stepX;
  static S32 pitch;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Horizontal line
  //

  // Jump table for start labels
  static U32 hLine8JT0[4];

  // Jump table for end labels
  static U32 hLine8JT1[5];    


  //
  // 8-bit Horizontal line
  //
  void STDCALL HLine8(void *data, U32 len, U32 color)
  {
  #ifdef ASM_ROUTINES

    _asm
    {
      cmp     hl8jtLoad, 1                // Load jump table?
      je      begin                       // No, start drawing

      mov     [hl8jt0+ 0], offset start0  // Load offsets of start labels
      mov     [hl8jt0+ 4], offset start1
      mov     [hl8jt0+ 8], offset start2
      mov     [hl8jt0+12], offset start3

      mov     [hl8jt1+ 0], offset end0    // Load offsets of end labels
      mov     [hl8jt1+ 4], offset end1
      mov     [hl8jt1+ 8], offset end2
      mov     [hl8jt1+12], offset end3
      mov     [hl8jt1+16], offset end4

      mov     hl8jtLoad, 1                // Flag jump table as loaded

    begin:
      mov     ecx, [len]
      mov     edi, [data]

      mov     eax, [color]
      cmp     ecx, 4

      jle     short end
      mov     edx, [color]

      mov     ah, al
      mov     dh, dl

      shl     eax, 16
      and     edi, 3

      add     eax, edx
      mov     ebx, 4

      mov     edx, [data]
      sub     ebx, edi

      jmp     [dword ptr hl8jt0 + edi*4]

    start0:
      mov     [edx+3], al
    start1:
      mov     [edx+2], al
    start2:
      mov     [edx+1],al
    start3:
      mov     [edx], al

      lea     edi, [edx+ebx]
      sub     ecx, ebx

      cmp     ecx, 4
      mov     edx, ecx

      jle     short end
      and     edx, 3

      shr     ecx, 2
      jz      short doneMiddle

    middleLoop:
      mov     [edi], eax
      add     edi, 4
      dec     ecx
      jnz     middleLoop

    doneMiddle:
      mov     ecx, edx

    end:
      jmp     [dword ptr hl8jt1 + ecx*4]

    end4:
      mov     [edi+3], al
    end3:
      mov     [edi+2], al
    end2:
      mov     [edi+1], al
    end1:
      mov     [edi], al
    end0:
    }
  #else

    U8 *pixel = (U8 *)data;

    for (U32 i = 0; i < len; i++)
    {
      *pixel++ = (U8)color;
    }

  #endif
  }


  //
  // 16-bit Horiontal line
  //
  void STDCALL HLine16(void *data, U32 len, U32 color)
  {
    U16 *pixel = (U16 *)data;

    for (U32 i = 0; i < len; i++)
    {
      *pixel++ = (U16)color;
    }
  }


  //
  // 24-bit Horiontal line
  //
  void STDCALL HLine24(void *data, U32 len, U32 color)
  {
    Bit24Color *pixel = (Bit24Color *)data;

    for (U32 i = 0; i < len; i++)
    {
      *pixel++ = *((Bit24Color*)&color);
    }
  }

  //
  // 32-bit Horizontal line
  //
  void STDCALL HLine32(void *data, U32 len, U32 color)
  {
    U32 *pixel = (U32 *)data;

    for (U32 i = 0; i < len; i++)
    {
      *pixel++ = (U32)color;
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Vertical line
  //


  //
  // 8-bit Vertical line
  //
  void STDCALL VLine8(void *data, U32 pitch, U32 len, U32 color)
  {
  #ifdef ASM_ROUTINES

    _asm
    {
      mov     edi, [data]
      mov     ecx, [len]
      mov     edx, [pitch]
      mov     eax, [color]

      test    ecx,ecx
      jz      short done

    vloop:
      mov     byte ptr [edi], al
      add     edi, edx
      dec     ecx
      jnz     vloop

    done:
    }
  #else

    U8 *pixel = (U8 *)data;

    for (U32 i = 0; i < len; i++)
    {
      *pixel = (U8)color;
      pixel += pitch;
    }

  #endif
  }


  //
  // 16-bit vertical line
  //
  void STDCALL VLine16(void *data, U32 pitch, U32 len, U32 color)
  {
  #ifdef ASM_ROUTINES

    _asm
    {
      mov     edi, [data]
      mov     ecx, [len]
      mov     edx, [pitch]
      mov     eax, [color]

      test    ecx,ecx
      jz      short done

    vloop:
      mov     word ptr [edi], ax
      add     edi, edx
      dec     ecx
      jnz     vloop

    done:
    }

  #else

    U16 *pixel = (U16 *)data;
    pitch >>= 1;

    for (U32 i = 0; i < len; i++)
    {
      *pixel = (U16)color;
      pixel += pitch;
    }

  #endif
  }

  //
  // 24-bit vertical line
  //
  void STDCALL VLine24(void *data, U32 pitch, U32 len, U32 color)
  {
    Bit24Color *pixel = (Bit24Color *)data;
    pitch /= sizeof(Bit24Color);

    for (U32 i = 0; i < len; i++)
    {
      *pixel = *((Bit24Color*)&color);
      pixel += pitch;
    }
  }


  //
  // 32-bit vertical line
  //
  void STDCALL VLine32(void *data, U32 pitch, U32 len, U32 color)
  {
  #ifdef ASM_ROUTINES

    _asm
    {
      mov     edi, [data]
      mov     ecx, [len]
      mov     edx, [pitch]
      mov     eax, [color]

      test    ecx,ecx
      jz      short done

    vloop:
      mov     dword ptr [edi], eax
      add     edi, edx
      dec     ecx
      jnz     vloop

    done:
    }

  #else

    U32 *pixel = (U32 *)data;
  
    pitch >>= 2;

    for (U32 i = 0; i < len; i++)
    {
      *pixel = (U32)color;
      pixel += pitch;
    }

  #endif
  }


  /////////////////////////////////////////////////////////////////////
  //
  // Line
  //

  // Prototype of a pointer to slice drawing routine
  typedef void (FASTCALL *PFNLINEDRAWRUN)(U32);

  // Jump for horizontal/vertical slice drawing function
  // the first 4 items are for 8/16/24/32bpp horizontal functions
  // the second 4 items are for 8/16/24/32bpp vertical functions
  static PFNLINEDRAWRUN lineSliceProc[8];

  //
  // 8-bit horizontal slice
  //
  void FASTCALL HRun8(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *(U8 *)data = (U8)lineColor;
      data += stepX;
    }
    data += pitch;
  }


  //
  // 16-bit horizontal slice
  //
  void FASTCALL HRun16(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *(U16 *)data = (U16)lineColor;
      data += stepX;
    }
    data += pitch;
  }

  //
  // 24-bit horizontal slice
  //
  void FASTCALL HRun24(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *((Bit24Color *)data) = *((Bit24Color*)&lineColor);
      data += stepX;
    }
    data += pitch;
  }

  //
  // 32-bit horizontal slice
  //
  void FASTCALL HRun32(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *(U32 *)data = (U32)lineColor;
      data += stepX;
    }
    data += pitch;
  }


  //
  // 8-bit vertical slice
  //
  void FASTCALL VRun8(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *(U8 *)data = (U8)lineColor;
      data += pitch;
    }
    data += stepX;
  }


  //
  // 16-bit vertical slice
  //
  void FASTCALL VRun16(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *(U16 *)data = (U16)lineColor;
      data += pitch;
    }
    data += stepX;
  }

  //
  // 24-bit vertical slice
  //
  void FASTCALL VRun24(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *((Bit24Color *)data) = *((Bit24Color*)&lineColor);
      data += pitch;
    }
    data += stepX;
  }

  //
  // 32-bit vertical slice
  //
  void FASTCALL VRun32(U32 n)
  {
    for (U32 i = 0; i < n; i++)
    {
      *(U32 *)data = (U32)lineColor;
      data += pitch;
    }
    data += stepX;
  }


  //
  // Setup jump tables
  //
  void Init()
  {
    if (!init)
    {
      // Horizontal slice drawing functions
      lineSliceProc[0] = HRun8;
      lineSliceProc[1] = HRun16;
      lineSliceProc[2] = HRun24;
      lineSliceProc[3] = HRun32;

      // Vertical slice drawing functions
      lineSliceProc[4] = VRun8;
      lineSliceProc[5] = VRun16;
      lineSliceProc[6] = VRun24;
      lineSliceProc[7] = VRun32;

      HLine8 (NULL, 0, 0);
      HLine16(NULL, 0, 0);
      HLine24(NULL, 0, 0);
      HLine32(NULL, 0, 0);

      init = TRUE;
    }
  }


  //
  // Line clipping
  //
  static Bool ClipT(S32 denom, S32 num, F32 &tE, F32 &tL)
  {
    F32 t;
    Bool accept = TRUE;

    if (denom > 0)
    {
      t = F32(num) / F32(denom);
      if (t > tL)
      {
        accept = FALSE;
      }
      else if (t > tE)
      {
        tE = t;
      }
    }
    else if (denom < 0)
    {
      t = F32(num) / F32(denom);
      if (t < tE)
      {
        accept = FALSE;
      }
      else if (t < tL)
      {
        tL = t;
      }
    }
    else if (num > 0)
    {
      accept = FALSE;
    }
    return accept;
  }


  //
  // Line clipping
  //
  static Bool ClipLine(S32 &x0, S32 &y0, S32 &x1, S32 &y1, const ClipRect *clip)
  {
    F32 tE, tL;
    S32 dx = x1 - x0;
    S32 dy = y1 - y0;
    Bool vis = FALSE;

    // Test for degenerate case first
    if (dx == 0 && dy == 0 && (x0 >= clip->p0.x && x1 < clip->p1.x && y0 >= clip->p0.y && y1 < clip->p1.y))
    {
      vis = TRUE;
    }
    else
    {
      tE = 0.0F;
      tL = 1.0F;

      if (ClipT(dx, clip->p0.x - x0, tE, tL))
      {
        if (ClipT(-dx, x0 - clip->p1.x, tE, tL))
        {
          if (ClipT(dy, clip->p0.y - y0, tE, tL))
          {
            if (ClipT(-dy, y0 - clip->p1.y, tE, tL))
            {
              vis = TRUE;
              if (tL < 1.0F)
              {
                x1 = x0 + S32(tL * F32(dx));
                y1 = y0 + S32(tL * F32(dy));
              }
              if (tE > 0.0F)
              {
                x0 = x0 + S32(tE * F32(dx));
                y0 = y0 + S32(tE * F32(dy));
              }
            }
          }
        }
      }
    }
    return (vis);
  }
}


/////////////////////////////////////////////////////////////////////
//
// PutPixel
//


//
// 8-bit clipped PutPixel
//
#ifdef ASM_ROUTINES
NAKED void STDCALL Bitmap::PutPixel8(S32, S32, U32, const ClipRect *)
{
  #define STACK 16

  // stack usage: (modifies ebx esi edi ebp)
  //
  // +- edi           <-- esp
  // |  esi
  // |  ebx         
  // +- ebp         
  //    ****          <-- esp+STACK
  //    this
  //    x            
  //    y
  //    color
  //    clip
  //
  // register usage:
  //    esi = this
  //    edi = clip,bmpData
  //    eax = y
  //    ebx = x
  //    ecx = clip->p0.x,clip->p0.y
  //    edx = clip->p1.x,clip->p1.y,color
  //    ebp = bmpPitch
  //
  _asm
  {
    push    ebp                     ; 1 
    push    ebx                     ; 0 
    push    esi                     ; 1 
    push    edi                     ; 0 

    mov     esi, [esp+STACK+ 4]     ; 1 // esi = this
    mov     edi, [esp+STACK+20]     ; 0 // edi = clip

    mov     eax, [esp+STACK+12]     ; 1 // eax = y
    mov     ebx, [esp+STACK+ 8]     ; 0 // ebx = x

    mov     ecx, [edi+ 4]           ; 1 // ecx = clip->p0.y
    mov     edx, [edi+12]           ; 0 // edx = clip->p1.y

    cmp     eax, ecx                ; 1 // if (y < clip->p0.y) exit
    jl      _bail                   ; 0

    cmp     eax, edx                ; 1 // if (y >= clip->p0.y) exit
    jge     _bail                   ; 0 
                                        // ** eax (y) is now free

    mov     ecx, [edi+ 0]           ; 1 // ecx = clip->p0.x
    mov     edx, [edi+ 8]           ; 0 // edx = clip->p1.x
                                        // ** edi (clip) is now free

    mov     ebp, [esi]this.bmpPitch ; 1 // ebp = bmpPitch
    mov     edi, [esi]this.bmpData  ; 0 // edi = bmpData
                                        // ** esi (this) is now free

    cmp     ebx, ecx                ; 1 // if (x < clip->p0.x) exit
    jl      _bail                   ; 0

    cmp     ebx, edx                ; 1 // if (x >= clip->p1.x) exit
    jge     _bail                   ; 0

    imul    ebp                     ;10 // eax = y * bmpPitch

    add     eax, ebx                ; 1 // eax = y * bmpPitch + x
    mov     edx, [esp+STACK+16]     ; 0 // edx = color

    mov     byte ptr [edi+eax], dl  ; 2 // [AGI] set color byte on bitmap surface
    nop                             ; 0 // so that pop instructions are pairable

  _bail:
    pop     edi                     ; 1
    pop     esi                     ; 0
    pop     ebx                     ; 1
    pop     ebp                     ; 0

    ret     20
  }
}
#else
void Bitmap::PutPixel8(S32 x, S32 y, U32 color, const ClipRect *clip)
{
  ASSERT(bmpData);

  if ((x >= clip->p0.x) && (x < clip->p1.x) && (y >= clip->p0.y) && (y < clip->p1.y))
  {
    *((U8 *)(bmpData) + (y * bmpPitch + x)) = (U8)color;
  }
}
#endif


//
// 8-bit clipped PutPixel on surfaces with pitch of 640 bytes
//
void Bitmap::PutPixel8Pitch640(S32 x, S32 y, U32 color, const ClipRect *clip)
{
  ASSERT(bmpData);

  if ((x >= clip->p0.x) && (x < clip->p1.x) && (y >= clip->p0.y) && (y < clip->p1.y))
  {
    *((U8 *)(bmpData) + ((y << 9) + (y << 7) + x)) = (U8)color;
  }
}


//
// 16-bit clipped PutPixel
//
void Bitmap::PutPixel16(S32 x, S32 y, U32 color, const ClipRect *clip)
{
  ASSERT(bmpData);

  if ((x >= clip->p0.x) && (x < clip->p1.x) && (y >= clip->p0.y) && (y < clip->p1.y))
  {
    U32 offset = (y * bmpPitch) + (x << 1);

    *(U16 *)((U8 *)(bmpData) + offset) = (U16)color;
  }
}

//
// 24-bit clipped PutPixel
//
void Bitmap::PutPixel24(S32 x, S32 y, U32 color, const ClipRect *clip)
{
  ASSERT(bmpData);

  if ((x >= clip->p0.x) && (x < clip->p1.x) && (y >= clip->p0.y) && (y < clip->p1.y))
  {
    U32 offset = (y * bmpPitch) + (x * 3);

    *((Bit24Color *)((U8 *)(bmpData) + offset)) = *((Bit24Color*)&color);
  }
}


//
// 32-bit clipped PutPixel
//
void Bitmap::PutPixel32(S32 x, S32 y, U32 color, const ClipRect *clip)
{
  ASSERT(bmpData);

  if ((x >= clip->p0.x) && (x < clip->p1.x) && (y >= clip->p0.y) && (y < clip->p1.y))
  {
    U32 offset = (y * bmpPitch) + (x << 2);

    *(U32 *)((U8 *)(bmpData) + offset) = (U32)color;
  }
}


/////////////////////////////////////////////////////////////////////
//
// PutPixelP
//


//
// 8-bit PutPixelP
//
void *Bitmap::PutPixelP8(void *data, U32 color)
{
  *(U8 *)data = U8(color);
  return ((U8 *)data) + 1;
}


//
// 16-bit PutPixelP
//
void *Bitmap::PutPixelP16(void *data, U32 color)
{
  *(U16 *)data = U16(color);
  return ((U8 *)data) + 2;
}

//
// 24-bit PutPixelP
//
void *Bitmap::PutPixelP24(void *data, U32 color)
{
  (*(Bit24Color *)data) = *((Bit24Color*)&color);
  return ((U8 *)data) + 3;
}


//
// 32-bit PutPixelP
//
void *Bitmap::PutPixelP32(void *data, U32 color)
{
  *(U32 *)data = color;
  return ((U8 *)data) + 4;
}


/////////////////////////////////////////////////////////////////////
//
// GetPixel
//

//
// 8-bit clipped GetPixel
//
U32 Bitmap::GetPixel8(S32 x, S32 y) const
{
  ASSERT(bmpData);

  return (U32)*(((U8 *)bmpData) + (y * bmpPitch + x));
}


//
// 16-bit clipped GetPixel
//
U32 Bitmap::GetPixel16(S32 x, S32 y) const
{
  ASSERT(bmpData);

  return (U32)(*(U16 *)(((U8 *)bmpData) + (y * bmpPitch) + (x << 1)));
}


//
// 24-bit clipped GetPixel
//
U32 Bitmap::GetPixel24(S32 x, S32 y) const
{
  ASSERT(bmpData);

  Bit24Color color = (*(Bit24Color *)(((U8 *)bmpData) + (y * bmpPitch) + (x * 3)));

  U32 c = color.b << 24 | color.g << 16 | color.r;

  return c;
}

//
// 32-bit clipped GetPixel
//
U32 Bitmap::GetPixel32(S32 x, S32 y) const
{
  ASSERT(bmpData);

  return *((U32 *)(((U8 *)bmpData) + (y * bmpPitch) + (x << 2)));
}


/////////////////////////////////////////////////////////////////////
//
// HLine
//

//
// 8-bit horizontal line
//
void Bitmap::HLine8(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip)
{
  S32 swap;
  U8 *data;

  ASSERT(bmpData);

  if ((y < clip->p0.y) || (y >= clip->p1.y) || (x1 < clip->p0.x) || (x0 >= clip->p1.x))
    return;

  if (x0 > x1)
  {
    swap = x0;
    x0 = x1;
    x1 = swap;
  }

  if (x0 < clip->p0.x)
  {
    x0 = clip->p0.x;
  }
  if (x1 > clip->p1.x)
  {
    x1 = clip->p1.x;
  }

  ASSERT(x1 >= x0);

  data = (U8 *)(bmpData) + (y * bmpPitch + x0);

  BitmapPrim::HLine8(data, x1-x0, color);
}


//
// 16-bit horizontal line
//
void Bitmap::HLine16(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip)
{
  S32 swap;
  U16 *data;

  ASSERT(bmpData);

  if ((y < clip->p0.y) || (y >= clip->p1.y) || (x1 < clip->p0.x) || (x0 >= clip->p1.x))
    return;

  if (x0 > x1)
  {
    swap = x0;
    x0 = x1;
    x1 = swap;
  }

  if (x0 < clip->p0.y)
  {
    x0 = clip->p0.x;
  }
  if (x1 > clip->p1.x)
  {
    x1 = clip->p1.x;
  }

  ASSERT(x1 >= x0);

  data = (U16 *)((U8 *)(bmpData) + (y * bmpPitch + (x0<<1)));

  BitmapPrim::HLine16(data, x1-x0, color);
}


//
// 24-bit horizontal line
//
void Bitmap::HLine24(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip)
{
  S32 swap;

  ASSERT(bmpData);

  if ((y < clip->p0.y) || (y >= clip->p1.y) || (x1 < clip->p0.x) || (x0 >= clip->p1.x))
    return;

  if (x0 > x1)
  {
    swap = x0;
    x0 = x1;
    x1 = swap;
  }

  if (x0 < clip->p0.y)
  {
    x0 = clip->p0.x;
  }
  if (x1 > clip->p1.x)
  {
    x1 = clip->p1.x;
  }

  ASSERT(x1 >= x0);

  Bit24Color *data = (Bit24Color *)((U8 *)(bmpData) + (y * bmpPitch + (x0 * 3)));

  BitmapPrim::HLine24(data, x1-x0, color);
}


//
// 32-bit horizontal line
//
void Bitmap::HLine32(S32 x0, S32 x1, S32 y, U32 color, const ClipRect *clip)
{
  S32 swap;
  U32 *data;

  ASSERT(bmpData);

  if ((y < clip->p0.y) || (y > clip->p1.y) || (x1 < clip->p0.x) || (x0 > clip->p1.x))
    return;

  if (x0 > x1)
  {
    swap = x0;
    x0 = x1;
    x1 = swap;
  }

  if (x0 < clip->p0.x)
  {
    x0 = clip->p0.x;
  }
  if (x1 > clip->p1.x)
  {
    x1 = clip->p1.x;
  }

  ASSERT(x1 >= x0);

  data = (U32 *)((U8 *)(bmpData) + (y * bmpPitch + (x0<<2)));

  BitmapPrim::HLine32(data, x1-x0, color);
}


/////////////////////////////////////////////////////////////////////
//
// VLine
//

//
// 8-bit vertical line
//
void Bitmap::VLine8(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip)
{
  S32 swap;
  U8 *data;

  ASSERT(bmpData);

  if ((x < clip->p0.x) || (x > clip->p1.x) || (y1 < clip->p0.y) || (y0 > clip->p1.y))
    return;

  if (y0 > y1)
  {
    swap = y0;
    y0 = y1;
    y1 = swap;
  }

  if (y0 < clip->p0.y)
  {
    y0 = clip->p0.y;
  }
  if (y1 > clip->p1.y)
  {
    y1 = clip->p1.y;
  }

  ASSERT(y1 >= y0);

  data = (U8 *)(bmpData) + (y0 * bmpPitch + x);

  BitmapPrim::VLine8(data, bmpPitch, y1-y0, color);
}


//
// 16-bit vertical line
//
void Bitmap::VLine16(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip)
{
  S32 swap;
  U16 *data;

  ASSERT(bmpData);

  if ((x < clip->p0.x) || (x > clip->p1.x) || (y1 < clip->p0.y) || (y0 > clip->p1.y))
    return;

  if (y0 > y1)
  {
    swap = y0;
    y0 = y1;
    y1 = swap;
  }

  if (y0 < clip->p0.y)
  {
    y0 = clip->p0.y;
  }
  if (y1 > clip->p1.y)
  {
    y1 = clip->p1.y;
  }

  ASSERT(y1 >= y0);

  data = (U16 *)((U8 *)(bmpData) + (y0 * bmpPitch + (x<<1)));

  BitmapPrim::VLine16(data, bmpPitch, y1-y0, color);
}


//
// 24-bit vertical line
//
void Bitmap::VLine24(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip)
{
  S32 swap;

  ASSERT(bmpData);

  if ((x < clip->p0.x) || (x > clip->p1.x) || (y1 < clip->p0.y) || (y0 > clip->p1.y))
    return;

  if (y0 > y1)
  {
    swap = y0;
    y0 = y1;
    y1 = swap;
  }

  if (y0 < clip->p0.y)
  {
    y0 = clip->p0.y;
  }
  if (y1 > clip->p1.y)
  {
    y1 = clip->p1.y;
  }

  ASSERT(y1 >= y0);

  Bit24Color *data = (Bit24Color *)((U8 *)(bmpData) + (y0 * bmpPitch + (x * 3)));

  BitmapPrim::VLine16(data, bmpPitch, y1-y0, color);
}



//
// 32-bit vertical line
//
void Bitmap::VLine32(S32 x, S32 y0, S32 y1, U32 color, const ClipRect *clip)
{
  S32 swap;
  U32 *data;

  ASSERT(bmpData);

  if ((x < clip->p0.x) || (x > clip->p1.x) || (y1 < clip->p0.y) || (y0 > clip->p1.y))
    return;

  if (y0 > y1)
  {
    swap = y0;
    y0 = y1;
    y1 = swap;
  }

  if (y0 < clip->p0.y)
  {
    y0 = clip->p0.y;
  }
  if (y1 > clip->p1.y)
  {
    y1 = clip->p1.y;
  }

  ASSERT(y1 >= y0);

  data = (U32 *)((U8 *)(bmpData) + (y0 * bmpPitch + (x<<2)));

  BitmapPrim::VLine32(data, bmpPitch, y1-y0, color);
}


///////////////////////////////////////////////////////////////////////////////
//
// Line
//
void Bitmap::LineX(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip)
{
  ASSERT(bmpData);

  // Is drawing 1 pixel more than it should
  FIXME(919378333, "aiarossi"); // Thu Feb 18 14:52:13 1999

  BitmapPrim::PFNLINEDRAWRUN horzRun, vertRun;
  S32 errTerm, adjUp, adjDown, step;
  S32 dx, dy, runLength, firstRun, lastRun;

  // Clip line to bounding box
  if (!BitmapPrim::ClipLine(x0, y0, x1, y1, clip))
  {
    return;
  }

  // Always draw from top to bottom
  if (y0 > y1) 
  {
    Swap<S32>(y0, y1);
    Swap<S32>(x0, x1);
  }

  // Set pointers for run drawing functions
  horzRun = BitmapPrim::lineSliceProc[bmpBytePP - 1];
  vertRun = BitmapPrim::lineSliceProc[bmpBytePP - 1 + 4];

  ASSERT(horzRun);
  ASSERT(vertRun);

  // Setup data for the low level routine
  BitmapPrim::lineColor = color;
  BitmapPrim::data = (U8 *)bmpData + (y0 * bmpPitch) + (x0 * bmpBytePP);
  BitmapPrim::pitch = bmpPitch;

  // Calculate x distance and step
  if ((dx = x1 - x0) < 0)
  {
    BitmapPrim::stepX = -bmpBytePP;
    dx = -dx;
  }
  else
  {
    BitmapPrim::stepX = bmpBytePP;
  }

  // Calculate y distance
  dy = y1 - y0;

  // Special cases
  if (dx == 0)
  {
    VLine(x0, y0, y1, color);
    return;
  }
  if (dy == 0)
  {
    HLine(x0, x1, y0, color);
    return;
  }
  if (dx == dy)
  {
    LOG_DIAG(("Ooops, line not implemented"));
    return;
  }

  if (dx >= dy)
  {
    step = dx / dy;
    adjUp = (dx % dy) * 2;
    adjDown = dy * 2;
    errTerm = (dx % dy) - (dy * 2);

    firstRun = (step / 2) + 1;
    lastRun = firstRun;

    if ((adjUp == 0) && ((step & 0x01) == 0))
    {
      --firstRun;
    }

    if ((step & 0x01) != 0)
    {
      errTerm += dy;
    }

    horzRun(firstRun);

    for (S32 i = 0; i < (dy-1); i++)
    {
      runLength = step;

      if ((errTerm += adjUp) > 0)
      {
        ++runLength;
        errTerm -= adjDown;
      }

      horzRun(runLength);
    }

    horzRun(lastRun);
  }
  else
  {
    step = dy / dx;
    adjUp = (dy % dx) * 2;
    adjDown = dx * 2;
    errTerm = (dy % dx) - (dx * 2);

    firstRun = (step / 2) + 1;
    lastRun = firstRun;

    if ((adjUp == 0) && ((step & 0x01) == 0))
    {
      --firstRun;
    }

    if ((step & 0x01) != 0)
    {
      errTerm += dx;
    }

    vertRun(firstRun);

    for (S32 i = 0; i < (dx-1); i++)
    {
      runLength = step;

      if ((errTerm += adjUp) > 0)
      {
        ++runLength;
        errTerm -= adjDown;
      }

      vertRun(runLength);
    }

    vertRun(lastRun);
  }
}


/////////////////////////////////////////////////////////////////////
//
// Rectangle
//
void STDCALL Bitmap::RectangleX(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip)
{
  ASSERT(clip);

  if (x0 > x1)
  {
    Swap<S32>(x0, x1);
    Swap<S32>(y0, y1);
  }

  HLine(x0  , x1  , y0  , color, clip);
  HLine(x0+1, x1-1, y1-1, color, clip);
  VLine(x0  , y0+1, y1  , color, clip);
  VLine(x1-1, y0+1, y1  , color, clip);
}


/////////////////////////////////////////////////////////////////////
//
// Filled Rectangle
//
void STDCALL Bitmap::FillRectX(S32 x0, S32 y0, S32 x1, S32 y1, U32 color, const ClipRect *clip)
{
  ASSERT(clip);
  ASSERT(bmpData);

  if (x0 > x1)
  {
    Swap<S32>(x0, x1);
    Swap<S32>(y0, y1);
  }

  if (x0 < clip->p0.x)
  {
    x0 = clip->p0.x;
  }
  if (x1 > clip->p1.x)
  {
    x1 = clip->p1.x;
  }
  if (y0 < clip->p0.y)
  {
    y0 = clip->p0.y;
  }
  if (y1 > clip->p1.y)
  {
    y1 = clip->p1.y;
  }

  if (x0 > clip->p1.x || x1 < clip->p0.x || y0 > clip->p1.y || y1 < clip->p0.y)
  {
    return;
  }

  U8 *data = (U8 *)bmpData + (y0 * bmpPitch) + (x0 * bmpBytePP);
  S32 width = x1 - x0;

  for (S32 row = y0; row < y1; ++row)
  {
    fnLLHLine(data, width, color);
    data += bmpPitch;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Circle
//
void STDCALL Bitmap::CircleX(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip)
{
  S32 d, xa, ya;

  ASSERT(bmpData);

  d = 3 - 2 * radius;
  ya = radius;

  for (xa = 0; xa <= ya; xa++)
  {
    // plot 8 points around axes of symmetry
    PutPixel(x + xa, y + ya, color, clip);
    PutPixel(x + xa, y - ya, color, clip);
    PutPixel(x - xa, y + ya, color, clip);
    PutPixel(x - xa, y - ya, color, clip);

    PutPixel(x + ya, y + xa, color, clip);
    PutPixel(x + ya, y - xa, color, clip);
    PutPixel(x - ya, y + xa, color, clip);
    PutPixel(x - ya, y - xa, color, clip);

    // update decision variable
    if (d < 0)
    {
      d = d + 4 * xa + 6;
    }
    else
    {
      d = d + 4 * (xa - ya) + 10;
      ya--;
    }
  }
}


/////////////////////////////////////////////////////////////////////
//
// Filled Circle
//
void STDCALL Bitmap::FillCircleX(S32 x, S32 y, S32 radius, U32 color, const ClipRect *clip)
{
  S32 d, xa, ya;

  ASSERT(bmpData);

  d = 3 - 2 * radius;
  ya = radius;

  for (xa = 0; xa <= ya; xa++)
  {
    HLine(x - xa, x + xa, y - ya, color, clip);
    HLine(x - ya, x + ya, y - xa, color, clip);
    HLine(x - ya, x + ya, y + xa, color, clip);
    HLine(x - xa, x + xa, y + ya, color, clip);

    // update decision variable
    if (d < 0)
    {
      d = d + 4 * xa + 6;
    }
    else
    {
      d = d + 4 * (xa - ya) + 10;
      ya--;
    }
  }
}


/////////////////////////////////////////////////////////////////////
//
// MakeRGB
//

//
// 8 bit MakeRGB uses color lookup table
//
U32 Bitmap::MakeRGBA8(U32 r, U32 g, U32 b, U32) const // = 255
{
  ASSERT(sysCLUT);
  return sysCLUT[(((r&0xFF) >> (8 - Palette::LUTREDBITS)) << (Palette::LUTGREENBITS + Palette::LUTBLUEBITS)) +
                 (((g&0xFF) >> (8 - Palette::LUTGREENBITS)) << (Palette::LUTBLUEBITS)) +
                 ((b&0xFF) >> (8 - Palette::LUTBLUEBITS))];
}


//
// Standard MakeRGB using shifts and masks
//
U32 Bitmap::MakeRGBASafe(U32 r, U32 g, U32 b, U32 a) const // = 255
{
  return ((r >> pixForm->rScaleInv) << pixForm->rShift) 
       + ((g >> pixForm->gScaleInv) << pixForm->gShift) 
       + ((b >> pixForm->bScaleInv) << pixForm->bShift)
       + ((a >> pixForm->aScaleInv) << pixForm->aShift);
}


//
// MakeRGB optimized for RGB 565
//
#ifdef ASM_ROUTINES
NAKED U32 STDCALL Bitmap::MakeRGBA565(U32, U32, U32, U32) const
{
  // stack usage:
  //
  //    ****          <-- esp
  //    ****
  //    r            
  //    g
  //    b
  //
  // register usage:
  //   ecx = red
  //   edx = green
  //   eax = blue
  _asm
  {
    mov     eax, [esp+16]               ; 1 // eax = b
    mov     edx, [esp+12]               ; 0 // edx = g
    shr     eax, 3                      ; 1 // convert b to 5 bits
    mov     ecx, [esp+ 8]               ; 0 // ecx = r
    shl     ecx, 8                      ; 1 // shift red component into place
    lea     edx, [edx*8]                ; 0 // shift green component into place
    and     ecx, 0x0000F800             ; 1 // mask out unnecessary red bits
    and     edx, 0x000007E0             ; 0 // mask out unnecessary green bits
    add     eax, ecx                    ; 1 // add red component to color
    add     eax, edx                    ; 1 // add green component to color

    ret     16
  }
}
#else
U32 Bitmap::MakeRGBA565(U32 r, U32 g, U32 b, U32 a) const // = 255
{
  a;
  return ((r << 8) & 0x0000F800) +
         ((g << 3) & 0x000007E0) +
         ((b >> 3));
}
#endif


//
// MakeRGB optimized for RGB 555
//
#ifdef ASM_ROUTINES
NAKED U32 STDCALL Bitmap::MakeRGBA555(U32, U32, U32, U32) const
{
  // stack usage:
  //
  //    ****          <-- esp
  //    ****
  //    r            
  //    g
  //    b
  //
  // register usage:
  //   ecx = red
  //   edx = green
  //   eax = blue
  _asm
  {
    mov     eax, [esp+16]               ; 1 // eax = b
    mov     edx, [esp+12]               ; 0 // edx = g
    shr     eax, 3                      ; 1 // convert b to 5 bits
    mov     ecx, [esp+ 8]               ; 0 // ecx = r
    shl     ecx, 7                      ; 1 // shift red component into place
    lea     edx, [edx*4]                ; 0 // shift green component into place
    and     ecx, 0x00007C00             ; 1 // mask out unnecessary red bits
    and     edx, 0x000003E0             ; 0 // mask out unnecessary green bits
    add     eax, ecx                    ; 1 // add red component to color
    add     eax, edx                    ; 1 // add green component to color

    ret     16
  }
}
#else
U32 Bitmap::MakeRGBA555(U32 r, U32 g, U32 b, U32 a) const // = 255
{
  a;
  return ((r << 7) & 0x00007C00) +
         ((g << 2) & 0x000003E0) +
         ((b >> 3));
}
#endif


//
// MakeRGB optimised for RGB 888
//
#ifdef ASM_ROUTINES
NAKED U32 STDCALL Bitmap::MakeRGBA888(U32, U32, U32, U32) const
{
  // stack usage:
  //
  //    ****          <-- esp
  //    ****
  //    r            
  //    g
  //    b
  //
  // register usage:
  //   ecx = red
  //   edx = green
  //   eax = blue
  _asm
  {
    mov     ecx, [esp+ 8]               ; 1 // ecx = r
    mov     edx, [esp+12]               ; 0 // edx = g
    shl     ecx, 16                     ; 1 // move red component into place
    mov     eax, [esp+16]               ; 0 // add blue component to color
    shl     edx, 8                      ; 1 // move green component into place
    add     eax, ecx                    ; 0 // add red component to color
    add     eax, edx                    ; 1 // add green component to color

    ret     16
  }
}
#else
U32 Bitmap::MakeRGBA888(U32 r, U32 g, U32 b, U32 a) const // = 255
{
  a;
  return (r << 16) + (g << 8) + b;
}
#endif


//
// MakeRGB from pixform
//
U32 Bitmap::MakeRGBAPixForm(U32 r, U32 g, U32 b, U32 a) const // = 255
{
  return ((r >> pixForm->rScaleInv) << pixForm->rShift) 
       + ((g >> pixForm->gScaleInv) << pixForm->gShift) 
       + ((b >> pixForm->bScaleInv) << pixForm->bShift)
       + ((a >> pixForm->aScaleInv) << pixForm->aShift);
}

// calculates a texture format's shift, scale, and name
//
void Pix::Setup()
{
  rMask = pixFmt.dwRBitMask;
  gMask = pixFmt.dwGBitMask;
  bMask = pixFmt.dwBBitMask;
  aMask = pixFmt.dwFlags & DDPF_ALPHAPIXELS ? pixFmt.dwRGBAlphaBitMask : 0;

  rScale = gScale = bScale = aScale = 0;
  rScaleInv = gScaleInv = bScaleInv = aScaleInv = 8;

	U32 m;
	if (rMask) 
	{
	    for (rShift = 0, m = rMask; !(m & 1); rShift++, m >>= 1);
	    m = rMask >> rShift;
	    if (m) 
    	{
	      for (rScale = 0; m; rScale++, m >>= 1);
        rScaleInv = 8 - rScale;
	    }
	}
	if (gMask) 
	{
	    for (gShift = 0, m = gMask; !(m & 1); gShift++, m >>= 1);
	    m = gMask >> gShift;
	    if (m) 
    	{
	      for (gScale = 0; m; gScale++, m >>= 1);
	    }
      gScaleInv = 8 - gScale;
	}
	if (bMask) 
	{
	    for (bShift = 0, m = bMask; !(m & 1); bShift++, m >>= 1);
	    m = bMask >> bShift;
	    if (m) 
    	{
	      for (bScale = 0; m; bScale++, m >>= 1);
	    }
      bScaleInv = 8 - bScale;
  }
	if (aMask)
	{
	    for (aShift = 0, m = aMask; !(m & 1); aShift++, m >>= 1);
	    m = aMask >> aShift;
	    if (m) 
    	{
	      for (aScale = 0; m; aScale++, m >>= 1);
	    }
      aScaleInv = 8 - aScale;
	}
	SetName();
}
//----------------------------------------------------------------------------

// generate a string describing the surface's pixelformat
//
void Pix::SetName()
{
	if (pixFmt.dwFlags & DDPF_PALETTEINDEXED1)
	{
		wsprintf( name.str, "pal1");
	}
	else if (pixFmt.dwFlags & DDPF_PALETTEINDEXED2)
	{
		wsprintf( name.str, "pal2");
	}
	else if (pixFmt.dwFlags & DDPF_PALETTEINDEXED4)
	{
		wsprintf( name.str, "pal4");
	}
	else if (pixFmt.dwFlags & DDPF_PALETTEINDEXED8)
	{
		wsprintf( name.str, "pal8");
	}
	else if (pixFmt.dwFlags & DDPF_PALETTEINDEXEDTO8)
	{
		wsprintf( name.str, "palto8");
	}
	else if (pixFmt.dwFlags & DDPF_RGB)
	{
		if (pixFmt.dwFlags & DDPF_ALPHAPIXELS)
		{
			if (aShift > bShift)
			{
				wsprintf( name.str, "%d%d%d%d", rScale, gScale, bScale, aScale);
			}
			else
			{
				wsprintf( name.str, "%d%d%d%d", aScale, rScale, gScale, bScale);
			}
		}
		else
		{
			wsprintf( name.str, "%d%d%d", rScale, gScale, bScale);
		}
	}
	else if (pixFmt.dwFlags & DDPF_ALPHA)
	{
		wsprintf( name.str, "alpha");
	}
}	
//----------------------------------------------------------------------------
