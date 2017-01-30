///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Benchmarking shell
//
// 29-APR-1998
//


#include "timing.h"
#include "hardware.h"
#include "random.h"
#include "file.h"
#include "debug_memory.h"
#include "filemap.h"
#include "mathtypes.h"


#include <strstrea.h>
#include <stdlib.h>
#include <stdio.h>
#include <mmsystem.h>
#include <math.h>

#ifdef BITMAPTEST
  #include "main.h"
  #include "bitmap.h"
  #include "vid.h"
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Externally declared routines
//


#ifdef BITMAPTEST
//
// Main::CreateMainWindow
//
// Window initialization
//
HWND Main::CreateMainWindow()
{
  return (NULL);
}

void Main::ExecInitialConfig()
{
}
#endif


///////////////////////////////////////////////////////////////////////////////
//
// The TESTS
//


///////////////////////////////////////////////////////////////////////////////
//
// CRC
//
void TestCRC()
{
  const int testLen[] = {1, 8, 64, 255};
  const int numTests = sizeof(testLen)/sizeof(testLen[0]);

  //
  // Crc timing runs
  //
  Clock::CycleWatch t1, t2, t3;
  char buf[256];
  int runs = 500;

  srand(0);

  for (int test = 0; test < numTests; test++)
  {
    // Fill the buffer
    int len = testLen[test];
    for (int j = 0; j < len; j++)
    {
      buf[j] = (U8)((rand() % 255) + 1);
    }
    buf[len] = 0;

    // Run some tests
    t1.Reset();
    t2.Reset();
    t3.Reset();

    for (int i = 0; i < runs; i++)
    {
      // crc
      t1.Start();
      Crc::Calc(buf, len);
      t1.Stop();

      // crclower
      t2.Start();
      Crc::CalcLower(buf, len);
      t2.Stop();

      // calcstr
      t3.Start();
      Crc::CalcStr(buf, len);
      t3.Stop();
    }

    TestWrite("Buffer of length %d", len);
    TestWrite("");
    TestReport("Crc::Calc", t1);
    TestReport("Crc::CalcLower", t2);
    TestReport("Crc::CalcStr", t3);
    TestWrite("");
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// StrFmt
//
void TestStrFmt()
{
  char s[17];
  Clock::CycleWatch t1, t2, t3, t4;

  S32 testno = -1242435;

  for (int i = 0; i < 1000; i++)
  {
    t1.Start();
    Utils::StrFmtDec(s, 16, testno);
    t1.Stop();

    t2.Start();
    ltoa(testno, s, 10);
    t2.Stop();

    t3.Start();
    Utils::StrFmtHex(s, 16, U32(testno));
    t3.Stop();

    t4.Start();
    Utils::StrFmtFloat(s, 10, 3, 342.33F);
    t4.Stop();

  }

  TestReport("StrFmtDec", t1);
   TestReport("ltoa     ", t2);
  TestReport("StrFmtHex", t3);
  TestReport("StrFmtFloat", t4);
}


///////////////////////////////////////////////////////////////////////////////
//
// Timers
//
void TestTimers()
{
  Clock::CycleWatch t1, t2, t3;

  for (int i = 0; i < 1000; i++)
  {
    t2.Start();
    GetTickCount();
    t2.Stop();

    t1.Start();
    timeGetTime();
    t1.Stop();

    LARGE_INTEGER t;

    t3.Start();
    QueryPerformanceCounter(&t);
    t3.Stop();
  }

  TestReport("timeGetTime", t1);
  TestReport("GetTickCount", t2);
  TestReport("QueryPerformanceCounter", t3);
}


///////////////////////////////////////////////////////////////////////////////
//
// Memory
//
void TestMemory()
{
  Clock::CycleWatch t1, t2;

  U8 *ptr;
  
  // Time to allocate a block and free a block
  for (int s = 1; s < 4096; s++)
  {
    for (int i = 0; i < 100; i++)
    {
      t1.Start();
      ptr = new U8[s];
      t1.Stop();

      t2.Start();
      delete ptr;
      t2.Stop();
    }

    printf("[%04d]\n", s);
    TestReport("Allocate", t1);
    TestReport("Free", t2);
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Trig
//
static void Fill(Matrix &m)
{
  Quaternion q;

  q.Set(Random::nonSync.Float() * PI * 2.0F, Matrix::I.up);
  q.Rotate(Random::nonSync.Float() * PI * 2.0F, Matrix::I.right);
  q.Rotate(Random::nonSync.Float() * PI * 2.0F, Matrix::I.front);
  m.Set(q);
}

void TestTrig()
{
  const F32 PI				      = 3.14159265358979323846f;

  Clock::CycleWatch tsin, tcos, ttan, tasin, tacos, tatan, tatan2, tmatmul, tvecxform;

  Utils::FP::MaskException(Utils::FP::EX_ALL);
  Utils::FP::UnmaskException(Utils::FP::EX_ZERODIVIDE|Utils::FP::EX_OVERFLOW);

  volatile F32 a;
  volatile F32 b;
  volatile F32 c;

  for (int i = 0; i < 1000; i++)
  {
    volatile F32 r;
    a = ((F32) Random::nonSync.Integer(10000000)) / 10000000.0f * PI;
    b = ((F32) Random::nonSync.Integer(10000000)) / 10000000.0f * 1;
    c = ((F32) Random::nonSync.Integer(10000000)) / 10000000.0f * 1;

    tsin.Start();
    r = (F32)sin(a);
    tsin.Stop();

    tcos.Start();
    r = (F32)cos(a);
    tcos.Stop();

    ttan.Start();
    r = (F32)tan(a);
    ttan.Stop();

    tasin.Start();
    r = (F32)asin(b);
    tasin.Stop();

    tacos.Start();
    r = (F32)acos(b);
    tacos.Stop();

    tatan.Start();
    r = (F32)atan(b);
    tatan.Stop();

    tatan2.Start();
    r = (F32)atan2(b, c);
    tatan2.Stop();

  }

  // Vector transform
  Vector v1[100];
  Vector v2[100];

  // Matrix multiply
  Matrix m1, m2, mt;

  Fill(m1);
  Fill(m2);
  Fill(mt);

  for (U32 x = 0; x < 100; x++)
  {
    v2[x].x = Random::nonSync.Float() * 1000.0F;
    v2[x].y = Random::nonSync.Float() * 1000.0F;
    v2[x].z = Random::nonSync.Float() * 1000.0F;
  }

  for (i = 0; i < 1000; i++)
  {
    tmatmul.Start();
    volatile Matrix mtemp = m1 * m2;
    tmatmul.Stop();
  }

  for (i = 0; i < 1000; i++)
  {
    memcpy(v1, v2, 100 * sizeof(Vector));

    tvecxform.Start();
    mt.Transform(v1, v1, 100);
    tvecxform.Stop();
  }

  TestReport("Sin", tsin);
  TestReport("Cos", tcos);
  TestReport("Tan", ttan);
  TestReport("ArcSin", tasin);
  TestReport("ArcCos", tacos);
  TestReport("ArcTan", tatan);
  TestReport("ArcTan2", tatan2);
  TestReport("Matrix*", tmatmul);
  TestReport("Vector*[100]", tvecxform);

  Utils::FP::MaskException(Utils::FP::EX_ALL);
}


void TestStream()
{
  char buf[256];
  char streambuf[256];
  ostrstream stream(streambuf, sizeof(streambuf));

  F32 f = 612341.1231F;
  U32 u = 243523434;
  char *s = "lk2345;lk2hj3;4kjgl52";

  Clock::CycleWatch t1, t2;

  for (int i = 0; i < 1000; i++)
  {
    stream.seekp(0, ios::beg);

    t1.Start();
    stream << f << ' ' << u << ' ' << s << '\0';
    t1.Stop();

    t2.Start();
    Utils::Sprintf(buf, sizeof(buf), "%f %u %s", f, u, s);
    t2.Stop();
  }

  TestWrite("strstream='%s'", stream.str());
  TestWrite("sprintf='%s'", buf);
  TestWrite("");

  TestReport("strstream", t1);
  TestReport("sprintf", t2);
}



///////////////////////////////////////////////////////////////////////////////
//
// Math
//


union FLOAT2INT
{
	int i;
	double f;
};

static double Float2Int = 65536.0 * 65536.0 * 65536.0 * 16.0 * 1.5;


NAKED int FASTCALL SetRounding(int)
{
   __asm
   {
      push  eax
      fstcw word ptr [esp]
      mov  edx, [esp]
      and  ecx, 000000c00h
      mov  eax, edx
      and  edx, 0fffff3ffh
      or   ecx, edx
      mov  [esp], ecx
      fldcw word ptr [esp]
      pop   ecx
      ret
   }
}

//
// Set rounding mode, return old control word
//
NAKED U16 FASTCALL SetRoundMode()
{
  static U32 tmpCW;

  __asm
  {
    fstcw   tmpCW
    mov     ecx, tmpCW
    mov     eax, ecx
    or      ecx, 0x00000C00
    mov     tmpCW, ecx
    fldcw   word ptr tmpCW
    ret
  }
}


//
// Restore previous rounding mode
//
NAKED void FASTCALL RestoreRoundMode(U16)
{
  static U32 tmpCW;

  __asm
  {
    mov     tmpCW, ecx
    fldcw   word ptr tmpCW
    ret
  }
}


inline NAKED S32 STDCALL FastFtoL(F32)
{
  __asm
  {
    fld     dword ptr [esp+4]
    fistp   dword ptr [esp+4]
    mov     eax, dword ptr [esp+4]
    ret     4
  }
}


NAKED S32 STDCALL FtoL(F32)
{
  static U16 tmpCW;

  __asm
  {
    // 1. Store control word into EAX
    fstcw   tmpCW

    // 7. Move float argument into FP register
    fld     dword ptr [esp+4]

    // 2. Store control word into EAX
    mov     eax, dword ptr tmpCW

    // 3. Save original control word into ECX
    mov     ecx, eax

    // 4. Set truncation mode, bit 10=1 11=1
    or      eax, 0x00000C00

    // 5. Move adjusted control word back to temp var
    mov     tmpCW, ax

    // 6. Set control word
    fldcw   tmpCW

    // 8. Store float as integer
    fistp   dword ptr [esp+4]

    // 9. Restore original control word
    mov     tmpCW, cx

    // 11. Store result into EAX
    mov     eax, dword ptr [esp+4]

    // 10. Restore original control word
    fldcw   tmpCW

    // 12. Return
    ret     4
  }


/*
  static U16 cw0, cw1;

  __asm
  {
    // Clear EAX
    xor     eax, eax

    // Store control word into EAX
    fstcw   cw0

    mov     ax, cw0
    mov     cw1, ax

    // Move float into FP register
    fld     dword ptr [esp+4]         

    // Set truncation mode, bit 10=1 11=1
    or      eax, 0x00000C00
    mov     cw0, ax

    // Set control word
    fldcw   cw0

    // Store float as integer
    fistp   dword ptr [esp+4]

    fldcw   cw1

    // Store resultant integer into EAX
    mov     eax, [esp+4]

    ret     4
  }
*/
}


void Display(F32 f)
{
  printf("(%f) ", f);
  printf("_ftol=%4d ", S32(f));
  printf("FastFToL=%4d ", Utils::FastFtoL(f));
  printf("NakedFtoL=%4d ", FtoL(f));
  printf("FtoL=%4d ", Utils::FtoL(f));
  
  U16 cw = Utils::FP::SetTruncMode();

  FLOAT2INT fti;
  fti.f = f + Float2Int;
  printf("FLOAT2INT=%4d ", fti.i);

  printf("SETCW FastFToL=%4d ", FastFtoL(f));
  printf("\n");

  Utils::FP::RestoreMode(cw);

}


void TestMath()
{
  Clock::CycleWatch t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
  U16 cw;

  Display(2.6F);
  Display(2.5F);
  Display(2.4F);

  for (int i = 0; i < 1000; i++)
  {
    volatile F32 f = F32(rand()) / F32(rand());
    volatile S32 s;

    t3.Start();
    s = Utils::FastFtoL(f);
    t3.Stop();

    t1.Start();
    s = S32(f);
    t1.Stop();

    t2.Start();
    s = Utils::FtoL(f);
    t2.Stop();

    t4.Start();
    s = FtoL(f);
    t4.Stop();

    t8.Start();
    Utils::FP::SetTruncMode();
    t8.Stop();

    t9.Start();
    Utils::FP::SetRoundUpMode();
    t9.Stop();

    t5.Start();
    SetRounding(0x11);
    t5.Stop();

    t6.Start();
    cw = SetRoundMode();
    t6.Stop();

    t7.Start();
    RestoreRoundMode(cw);
    t7.Stop();
  }

  TestReport("__ftol", t1);
  TestReport("FtoL", t2);
  TestReport("FastFtoL", t3);
  TestReport("naked FtoL", t4);
  TestReport("set cw", t5);
  TestReport("set rm", t6);
  TestReport("rest rm", t7);
  TestReport("Utils::FP::SetTruncMode", t8);
  TestReport("Utils::FP::SetRoundUpMode", t9);
}


void PrintRoundInfo(F32 f)
{
  FLOAT2INT ftl;
  ftl.f = f + Float2Int;

  TestWrite("    %f:  _ftol=%d FastFToL=%d Float2Int=%d", f, S32(f), Utils::FastFtoL(f), ftl.i);
}

void TestRounding()
{
  U16 cw;
  F32 f;

  ReadInput("Enter a F32");
  
  f = (F32)atof(LastInput());

  TestWrite("Round up");
  cw = Utils::FP::SetRoundUpMode();    
  PrintRoundInfo(f);
  Utils::FP::RestoreMode(cw);

  TestWrite("Round down");
  cw = Utils::FP::SetRoundDownMode();  
  PrintRoundInfo(f);
  Utils::FP::RestoreMode(cw);

  TestWrite("Truncate");
  cw = Utils::FP::SetTruncMode();   
  PrintRoundInfo(f);
  Utils::FP::RestoreMode(cw);

  TestWrite("Nearest");
  cw = Utils::FP::SetNearestMode();       
  PrintRoundInfo(f);
  Utils::FP::RestoreMode(cw);
}


void FloatTest()
{
  for (;;)
  {
    F32 a;

    cin >> a;
    cout << "FastFtoL=" << FastFtoL(a) << ' ';
    cout << "FtoL=" << FtoL(a) << ' ';
    //U16 cw = SetRoundMode();
    //cout << "FastFtoL=" << Utils::FastFtoL(a) << ' ';
    //RestoreRoundMode(cw);
    cout << "(S32)=" << S32(a) << endl;
         
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Bitmap
//
void TestBmp()
{
#ifdef BITMAPTEST
  const U32 BMPX = 640;
  const U32 BMPY = 480;

  DDPIXELFORMAT pf16, pf32;

  pf16.dwRGBBitCount      = 16;
  pf16.dwRBitMask         = 0xF800;
  pf16.dwGBitMask         = 0x7E00;
  pf16.dwBBitMask         = 0x001F;
  pf16.dwRGBAlphaBitMask  = 0x0000;

  Vid::backFormat.SetPixFmt(pf16);

  ClipRect minus1(1, 1, BMPX-1, BMPY-1);

  Bitmap *bmp = new Bitmap(bitmapNORMAL);

  bmp->Create(BMPX, BMPY, 16);
  bmp->FillRectangle(0, 0, BMPX, BMPY, 0);

  U32 red   = bmp->MakeRGBA(255, 0, 0);
  U32 green = bmp->MakeRGBA(0, 255, 0);
  U32 green2 = bmp->MakeRGBA(0, 128, 0);
  U32 blue  = bmp->MakeRGBA(0, 0, 255);
  U32 blue2  = bmp->MakeRGBA(0, 0, 128);
  U32 white = bmp->MakeRGBA(255,255,255);

  bmp->PutPixel(0,      0,      red);
  bmp->PutPixel(0,      BMPY-1, red);
  bmp->PutPixel(BMPX-1, 0,      red);
  bmp->PutPixel(BMPX-1, BMPY-1, red);
  bmp->PutPixel(BMPX, BMPY/2, blue);

  bmp->HLine(0, BMPX, 2, green);
  bmp->HLine(0, BMPX, 4, blue, &minus1);
  bmp->HLine(BMPX-1, BMPX, 6, green);
  bmp->HLine(BMPX-1, BMPX, 8, blue, &minus1);

  bmp->VLine(4, 0, BMPY, green2);
  bmp->VLine(6, 0, BMPY, blue2, &minus1);

  //bmp->Line(1, 1, 4, 1, white);
  //bmp->Line(0, 0, BMPX, BMPY, blue2, &minus1);
  //bmp->Rectangle(1, 1, 4, 4, white);

  //bmp->Rectangle(BMPX-5, BMPY-5, BMPX+5, BMPY+5, blue, &minus1);

  //bmp->FillRectangle(1, 1, 4, 4, white);
  //bmp->FillRectangle(BMPX-5, BMPY-5, BMPX, BMPY, blue, &minus1);

  bmp->Circle(4, 4, 5, white);
  bmp->Circle(BMPX-5, BMPY-5, 6, blue, &minus1);

  bmp->WriteBMP("test.bmp");
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
// Random
//
void TestRandom()
{
  Clock::CycleWatch t1, t2;

  Random::nonSync.SetSeed(0);
  volatile U32 r;

  for (int i = 0; i < 1000; i++)
  {
    t1.Start();
    r = rand();
    r %= 10;
    t1.Stop();

    t2.Start();
    r = Random::nonSync.Integer(10);
    t2.Stop();
  }

  TestReport("rand", t1);
  TestReport("Random::nonSync", t2);
}


///////////////////////////////////////////////////////////////////////////////
//
// FileMap
//
void TestFileMap()
{
  volatile U8 data;
  U32 i;
  U8 *ptr;
  U32 size;

  // Get a file
  ReadInput("Enter a filename");
  const char *name = LastInput();

  Clock::CycleWatch topen, tclose;

  topen.Start();

  // Create a file mapping of the file
  FileMap *filemap = new FileMap(name, FileMap::READ);
  U8 *filemapPtr = filemap->GetPtr();

  topen.Stop();

  // Perform linear access
  size = filemap->GetSize();
  ptr = filemapPtr;

  U32 t0 = Clock::Time::Ms();

  for (i = 0; i < size; i++)
  {
    data = *(ptr++);
  }

  F32 kps = F32(size >> 10) / F32(Clock::Time::Ms() - t0) * 1000.0F;

  // Close the file
  tclose.Start();
  delete filemap;
  tclose.Stop();

  TestReport("File Map Open", topen);
  TestReport("File Map Close", tclose);
  TestWrite ("Throughput: %.1fk/sec", kps);
}


///////////////////////////////////////////////////////////////////////////////
//
// File
//
void TestFile()
{
  volatile U8 data;
  U32 i;
  U8 *ptr;
  U32 size;

  // Get a file
  ReadInput("Enter a filename");
  const char *name = LastInput();

  Clock::CycleWatch topen, tlinear, trandom, tclose;

  topen.Start();

  // Create a file mapping of the file
  File file;
  file.Open(name, File::Mode::READ);
  size = file.GetSize();

  // Allocate memory
  U8 *fileptr;
  fileptr = new U8[size];

  // Load the entire file into memory
  file.Read(fileptr, size);

  topen.Stop();

  // Perform linear access
  ptr = fileptr;

  U32 t0 = Clock::Time::Ms();

  for (i = 0; i < size; i++)
  {
    data = *(ptr++);
  }

  F32 kps = F32(size >> 10) / F32(Clock::Time::Ms() - t0) * 1000.0F;

  // Close the file
  tclose.Start();
  delete fileptr;
  file.Close();
  tclose.Stop();

  TestReport("File Open", topen);
  TestReport("File Close", tclose);
  TestWrite ("Throughput: %1fk/sec", kps);
}


///////////////////////////////////////////////////////////////////////////////
//
// The TEST TABLE
//
TestFunc testTable[] = 
{
  TestCRC,    "CRC",      0,
  TestStrFmt, "StrFmt",   0, 
  TestTimers, "Timers",   0,
  TestMemory, "Memory",   0,
  TestTrig,   "TestTrig", 0,
  TestStream, "Stream",   0,
  TestMath,   "Math",     0,
  TestBmp,    "Bitmap",   TestFunc::LOW,
  TestRandom, "Random",   0,
  TestRounding,"Rounding", TestFunc::LOW,
  TestFile,    "File",    TestFunc::LOW,
  TestFileMap, "FileMap", TestFunc::LOW,
};

const U32 numTests = sizeof(testTable)/sizeof(testTable[0]);


///////////////////////////////////////////////////////////////////////////////
//
// System Info
//
void PrintCPUInfo()
{
  printf("\n");
  printf("CPU\n");
  printf("  %s\n", Hardware::CPU::GetDetailedDesc());
  printf("\n");
}

void PrintOSInfo()
{
  printf("Operating System:\n");
  printf("  %s\n", Hardware::OS::GetDesc());
  printf("\n");
}

void PrintDeviceInfo()
{
  printf("Memory\n");
  printf("  Physical   : %d bytes (%.2f Mb)\n", Hardware::Memory::GetPhysical(), ((F32) Hardware::Memory::GetPhysical()) / 1048576);
  printf("  Virtual    : %d bytes (%.2f Mb)\n", Hardware::Memory::GetVirtual(), ((F32) Hardware::Memory::GetVirtual()) / 1048576);
  printf("\n");

  // Devices
  printf("Devices\n");
  int i = 0;
  while (Hardware::Device::Enum(i))
  {
    printf("%s\n", Hardware::Device::Enum(i++));
  }
  printf("\n");
}


///////////////////////////////////////////////////////////////////////////////
//
// Testing main menu
//
Bool RunTest(U32 test)
{
  if (test > 0 && test <= numTests)
  {
    TestFunc *testFunc = &testTable[test-1];

    puts("\xDA");

    // Bump up thread priority
	  HANDLE hThread = GetCurrentThread();
	  S32 priority = THREAD_PRIORITY_NORMAL;

    if (!(testFunc->flags & TestFunc::LOW))
    {
      priority = GetThreadPriority(hThread);
      if (priority != THREAD_PRIORITY_ERROR_RETURN)
      {
        SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
      }
    }

    // Run the test
    testFunc->func();

    // Restore thread priority
    if (!(testFunc->flags & TestFunc::LOW))
    {
	    if (priority != THREAD_PRIORITY_ERROR_RETURN)
	    {
		    SetThreadPriority(hThread, priority);
	    }
    }

    puts("\xC0");
    fflush(stdin);
    return TRUE;
  }
  return FALSE;
}

void TestMenu()
{
  char prompt[256];
  S32 test = -1;
  S32 input;

  for(;;)
  {
    sprintf(prompt, "%s#)test  l)list  q)main", (test != -1) ? "ret)again  " : "");

    switch ((input = ReadInput(prompt, TOUPPER|ACCEPTCR|ACCEPTINT)))
    {
      case 0:
      {
        if (test != -1)
        {
          RunTest(test);
        }
        break;
      }
      case 'L':
      {
        for (S32 i = 0; i < numTests; i++)
        {
          printf("%2d: %s\n", i+1, testTable[i].desc);
        }
        break;
      }
      case 'Q':
      {
        return;
      }
      default:
      {
        S32 n = input>>8;
        if ((n > 0) && RunTest(n))
        {
          test = n;
        }
        break;
      }
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
//
// MAIN MENU
//
void CDECL MainMenu()
{
  // Display CPU info
  PrintCPUInfo();

  // Ensure that the CPU supports RDTSC
  if (!Hardware::CPU::HasFeature(Hardware::CPU::TSC))
  {
    printf("CPU does not have RDTSC feature\n");
    return;
  }

  for(;;)
  {
    switch (ReadInput("t)tests  q)quit  c)CPU  o)OS  d)devices", TOUPPER))
    {
      case 'T':
        TestMenu();
        break;
      case 'C':
        PrintCPUInfo();
        break;
      case 'D':
        PrintDeviceInfo();
        break;
      case 'O':
        PrintOSInfo();
        break;
      case 'F':
        FloatTest();
        break;
      case 'Q':
        return;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Entry point 
//
void CDECL main()
{
  Debug::Memory::Init();
  Debug::Exception::Handler(MainMenu);
  Debug::Memory::Check();
  Debug::Memory::Done();
}


///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
static char input[256] = "";
static char *prev = input;
static char outbuf[1024];

U32 ReadInput(char *prompt, U32 flags)
{
  for(;;)
  {
    printf("%s > ", prompt);

    prev = gets(input);

    if (prev)
    {
      while (*prev && isspace(*prev))
      {
        prev++;
      }
      if (*prev)
      {
        if (flags & ACCEPTINT && isdigit(*prev))
        {
          return (atoi(prev) << 8);
        }
        return (char)(flags & TOUPPER ? toupper(*prev) : (flags & TOLOWER ? tolower(*prev) : *prev));
      }
      else if (flags & ACCEPTCR)
      {
        return 0;
      }
    }
  }
}

char *LastInput()
{
  return prev;
}

void CDECL TestWrite(const char *format, ...)
{
  strcpy(outbuf, "\xB3 ");

  // Process the variable args
  va_list args;
  va_start(args, format);
  vsprintf(outbuf + strlen(outbuf), format, args);
  va_end(args);

  puts(outbuf);
}


static Clock::CycleWatch overhead;

void TestReport(const char *name, Clock::CycleWatch &t)
{
  // Record overhead
  if (overhead.GetSamples() == 0)
  {
    for (int i = 0; i < 1000; i++)
    {
      overhead.Start();
      overhead.Stop();
    }
  }

  char nameStr[20];

  Utils::Strmcpy(nameStr, name, sizeof(nameStr));

  printf("\xB3  %-20s %s   [%d]\n", nameStr, t.Report(), overhead.GetMin());
}


/*
void * CDECL operator new(
        unsigned int cb,
        int,
        const char *,
        int
        )
{
  return new U8[cb];
}



void CDECL main()
{
  Debug::Memory::Init();
  Mono::Init();

  {
    char streambuf[256];
    ostrstream stream (streambuf, 256);
  }
  //Debug::Exception::Handler(MainMenu);


  Mono::Done();
  Debug::Memory::Done();
}
*/
