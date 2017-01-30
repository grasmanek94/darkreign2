///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Monochrome video functions
//
// 06-DEC-1997
//


#ifndef __MONO_H
#define __MONO_H


///////////////////////////////////////////////////////////////////////////////
//
// Define MONO_DISABLED to turn off mono, use with macros
//
#ifndef DEVELOPMENT
  #define MONO_DISABLED
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Macros for mono functions
//

#ifdef MONO_DISABLED 

// Mono system
#define MonoInit()
#define MonoDone()
#define MonoScratchDone()
#define MonoActivatePanel(n)
#define MonoPanelChange(p)
#define MonoGetClientPanel(p)
#define MonoClearPtr(p)
#define MonoUpdate(f,e,x,y,t)
#define MonoProcessEvents(m,w,l)

// Mono panel
#define MonoSetEvtProc(b, e)

// Mono console
#define MonoConDec(c)
#define MonoConDef(c)
#define MonoConDefStatic(c)

// Mono buffer
#define MonoBufDec(b)
#define MonoBufDef(b)
#define MonoBufDefStatic(b)
#define MonoBufCreate(s,b)
#define MonoBufDestroy(b)
#define MonoBufChar(b,r,c,a,s)
#define MonoBufAttr(b,r,c,w,a)
#define MonoBufWrite(b,r,c,s,a)
#define MonoBufWriteV(b, f)
#define MonoBufFillRow(b,r,c,a)
#define MonoBufClear(b)

// Mono spinner
#define MonoSpinnerChar(c,a)

// Mono Scratch panel
#define MSWRITE(n,r,c,s)
#define MSWRITEV(n,s)   

#else

// Mono system
#define MonoInit()                Mono::Init()
#define MonoDone()                Mono::Done()
#define MonoScratchDone()         Mono::Scratch::Done()
#define MonoActivatePanel(n)      Mono::ActivatePanel(n)
#define MonoPanelChange(p)        Mono::PanelChange(p)
#define MonoGetClientPanel(p)     p = Mono::ClientPanel()
#define MonoClearPtr(p)           p = NULL
#define MonoUpdate(f,e,x,y,t)     Mono::Update(f,e,x,y,t)
#define MonoProcessEvents(m,w,l)  Mono::ProcessEvents(m,w,l)

// Mono panel
#define MonoSetEvtProc(p, e)      p->SetEventProc(e);

// Mono console
#define MonoConDec(c)             extern Mono::Console *(c)
#define MonoConDef(c)             Mono::Console *(c)
#define MonoConDefStatic(c)       static Mono::Console *(c)

// Mono buffer
#define MonoBufDec(b)             extern Mono::Buffer *(b)
#define MonoBufDef(b)             Mono::Buffer *(b)
#define MonoBufDefStatic(b)       static Mono::Buffer *(b)
#define MonoBufCreate(s,b)        Mono::CreateBuffer(s,b)
#define MonoBufDestroy(b)         Mono::DestroyBuffer(b)
#define MonoBufChar(b,r,c,a,s)    b->PutChar(r,c,a,s)
#define MonoBufAttr(b,r,c,w,a)    b->PutAttr(r,c,w,a)
#define MonoBufWrite(b,r,c,s,a)   b->Write(r,c,s,a)
#define MonoBufWriteV(b,f)        b->WriteV f
#define MonoBufFillRow(b,r,c,a)   b->FillRow(r,c,a)
#define MonoBufClear(b)           b->Clear()

// Mono Spinner
#define MonoSpinnerChar(c,a)      Mono::Spinner::PutChar(c,a)

// Mono Scratch panel
#define MSWRITE(n,r,c,s)          Mono::Scratch::Find(n)->Write(r, c, s)
#define MSWRITEV(n,s)             Mono::Scratch::Find(n)->WriteV s


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Mono
//
namespace Mono
{

  // Display attributes
  enum EAttrib
  {
    BLANK     = 0x00,
    UNDERLINE = 0x01,
    NORMAL    = 0x07,
    BLINK     = 0x87,
    BRIGHT    = 0x0F,
    REVERSE   = 0x70,
    BLINKREV  = 0xF0,
  };

  // Forward declarations
  class Scratch;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Panel
  //
  class Panel
  {
  public:
    friend class Scratch;

    // Max length of title string
    enum {MAXTITLE = 50};

    enum Type
    {
      PANEL,
      BUFFER,
      CONSOLE
    };

    // Event handler callback for this panel
    typedef Bool (EVENTPROC)(Panel *, U32, U32, S32);

  protected:
    U8  *buffer;
    Bool freeBuf;

    S32  width;
    S32  height;
    S32  virtRow;
    S32  virtCol;
    S32  rPos;
    S32  cPos;

    Bool dirty;
    S32  dirtyRectLeft;
    S32  dirtyRectRight;
    S32  dirtyRectTop;
    S32  dirtyRectBottom;

    char title[MAXTITLE];
    S32  len;

    Panel *parent;

    Type type;

    // Callback function for recieving messages when panel is active
    EVENTPROC *eventProc;

  public:

    // Node member for NBinTree
    NBinTree<Panel>::Node node;

  protected:
    Panel();

  public:

    // Dirty a region of the panel
    void Invalidate(S32 left, S32 top, S32 right, S32 bottom);

    // Blit the dirty region of the panel to the screen buffer
    void Repaint();

    Panel(S32 w, S32 h);
    ~Panel();

    // Create a panel to point at an existing block of memory
    void Create(S32 w, S32 h, U8 *screenMem);

    // Clear the panel with normal attribute and blank character
    void Clear();

    // Set the display title of the panel
    void SetTitle(const char *s);

    // Register an event handling function for the active panel
    void SetEventProc(EVENTPROC *proc);

    // Set the parent panel
    void SetParent(Panel *p);

    // Set the offset from the top left of the viewport
    void SetOffset(S32 r, S32 c);

    // Member data access functions
    S32  Width()  
    { 
      return width;  
    }

    S32  Height() 
    { 
      return height; 
    }

    S32  RowOffset()  
    { 
      return virtRow; 
    }

    S32  ColOffset()  
    { 
      return virtCol; 
    }

    // Get the panel type
    Type GetType()
    {
      return type;
    }

    // Return the title of the panel
    const char *GetTitle()
    {
      return title;
    }

    // Length
    U32 GetLength()
    {
      return len;
    }

    // EventProc
    EVENTPROC *GetEventProc()
    {
      return eventProc;
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Console
  //
  class Console : public Panel
  {
  protected:
    U8   *row1;
    U8   *rowN;

    S32  crsRow;
    S32  crsCol;
    S32  attrib;
    S32  scrollBy;
    Bool wrap;
    Bool follow;

    void Init();

  public:
    Console();
    Console(S32 w, S32 h);

    void Create(S32 w, S32 h, U8 *screenMem);

    // Write a string to the console
    void Write(const char *s, EAttrib attrib = NORMAL);

    // Write a formatted string to the console
    void CDECL WriteV(const char *format, ...);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Buffer
  //
  class Buffer : public Panel
  {
  public:
    enum
    {
      BOX_DOUBLELINE = 0x0001,
    };

  public:
    Buffer();
    Buffer(S32 w, S32 h);

    // Write text to the mono
    void Write(S32 row, S32 col, const char *s, EAttrib attrib = NORMAL);

    // Write vsprintf string to the buffer with NORMAL attribute
    void CDECL WriteV(S32 row, S32 col, const char *format, ...);

    // Write vsprintf string to the buffer with custom attribute
    void CDECL WriteV(S32 row, S32 col, EAttrib attrib, const char *format, ...);

    // Fill the rectangle with a character and attribute
    void FillRect(S32 left, S32 top, S32 right, S32 bottom, S32 ch, EAttrib attrib);

    // Fill the entire row with a character and attribute
    void FillRow(S32 row, S32 ch, EAttrib attrib);

    // Write a character to the screen with NORMAL attribute
    void PutChar(S32 row, S32 col, S32 ch);

    // Write a character to the screen with a custom attribute
    void PutChar(S32 row, S32 col, EAttrib attrib, S32 ch);

    // Set the attribute of a screen location
    void PutAttr(S32 row, S32 col, U32 width, EAttrib attrib);

    // Draw a box on the screen
    void Box(S32 x0, S32 y0, S32 x1, S32 y1, EAttrib attrib = NORMAL, U32 flags = 0);

    // Add a console to this panel
    Bool AttachConsole(S32 r, S32 c, S32 w, S32 h, Console **handle);

    // Detach a console
    Bool DetachConsole(Console **handle);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Spinner
  //
  class Spinner
  {
  private:
    static S32 row;
    static S32 col;
    static S32 frame;

  public:
    // Set the screen position of the spinner
    static void SetPos(S32 r, S32 c);

    // Draw a character where the spinner would appear, will be overwritten 
    // by the next call to Spinner::Advance
    static void PutChar(S32 ch, EAttrib attrib = REVERSE);

    // Spin it baby
    static void Advance();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Scratch
  //
  class Scratch
  {
  public:
    class Panel : public Buffer
    {
    public:
      char owner[32];

      friend class Scratch;

    public:
      // Create a scratch panel of the specified width and height
      Panel(S32 w, S32 h, const char *owner);
    };

  public:
    // Create a scratch panel, and set its owner and title
    static void Create(U32 id, const char *owner, const char *name = 0);

    // Destroy all scratch panels
    static void Done();

    // Find a scratch panel from its id
    static Buffer *Find(U32 id);

    // Activate the scratch panel with the specified id
    static void ActivatePanel(U32 id);
  };


  //
  // Exported Data
  //
  extern Panel         *current;
  extern NBinTree<Panel> panels;
  extern NBinTree<Panel> scratchPanels;
  extern NBinTree<Panel>::Iterator scratchItr;


  // Initialise the Mono display system
  void Init();

  // Shut down the Mono display system
  void Done();

  // Allocate a new console object and return a pointer to it
  void CreateConsole(const char *title, Console **handle);

  // Destroy the console, remove it from the list and invalidate the handle
  void DestroyConsole(Console **handle);

  // Find a console by name
  Console *FindConsole(const char *title);

  // Allocate a new mono buffer object and return a pointer to it
  void CreateBuffer(const char *title, Buffer **handle);

  // Destroy the buffer, remove it from the list and invalidate the handle
  void DestroyBuffer(Buffer **handle);

  // Find a buffer by name
  Buffer  *FindBuffer(const char *title);

  // Activate the panel with id of "n"
  void ActivatePanel(U32 n);

  // Change the panel to 'p' and repaint the screen
  void PanelChange(Panel *p);

  // Update the mono display with a new framerate and mouse position value.
  void Update(S32 fps, F32 elapSec, S32 mousex, S32 mousey, U32 triCount = 0);

  // Return a pointer to the start of video memory
  U8* ScreenPtr();

  // Return a pointer to the screen client area
  Buffer *ClientPanel();

  // Process keyboard events
  Bool ProcessEvents(U32 msg, U32 wParam, U32 lParam);
};

#endif

#endif
