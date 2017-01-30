///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Hardware detection routines
//
// 06-DEC-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "hardware.h"
#include "registry.h"


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "advapi32.lib")


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Hardware
//
namespace Hardware
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CPU
  //
  namespace CPU
  {

    // CPU Name lookup table
    static const char *CPUNAMES[MAXTYPE] =
    {
      // Unknown or un-identifiable
      "Unknown",
      "486",

      // Intel family
      "Pentium",
      "Pentium MMX",
      "Pentium II",
      "Pentium III",
      "Pentium Pro",
      "Celeron",
      "Celeron-2",

      // AMD family
      "AMD 5x86",
      "AMD K5",
      "AMD K6",
      "AMD K6-2",
      "AMD K6-III",
      "AMD Athlon",

      // Cyrix family
      "Cyrix 6x86",
      "Cyrix 6x86MX",
      "Cyrix MediaGX",
      "Cyrix GXm",

      // IDT winchip family
      "WinChip C6",
      "WinChip C6+",
    };


    // CPU Feature lookup table
    static const char *FEATURENAMES[MAXFEATURES] =
    {
      "FPU",
      "TSC",
      "MMX",
      "3DNow",
      "SSE",
      "FDIV BUG",
      "PS#"
    };


    // Static data members
    static Bool queried = FALSE;
    static Bool valid = FALSE;

    // Maximum function
    static U32 functions;
    static U32 exFunctions;

    // Signature
    static U32 signature;
    static U32 exSignature;
    static U32 brand;

    // Features
    static U32 features;
    static U32 exFeatures;

    // Vendor string
    static char vendor[13];
    static char exVendor[49];

    // Decoded signature
    static U32 sigType;
    static U32 sigFamily;
    static U32 sigModel;
    static U32 sigStepping;

    // Serial number
    static U32 serialLo;
    static U32 serialHi;

    // Detected type
    static Type type;

    // Detected features
    static Bool hasFeature[MAXFEATURES];

    // Was cache detected?
    static Bool detectCacheL1 = FALSE;
    static Bool detectCacheL2 = FALSE;
    static U32  cacheL1  = 0;
    static U32  cacheL2  = 0;

    // Clock speed
    static U32 mhz = 0;

    // CPU name (from table)
    const char *cpuName;

    // Description strings
    char descNormal[128];
    char descDetail[256];
    char serialDesc[32];

    // Register data
    static struct
    {
      U32 Eax;
      U32 Ebx;
      U32 Ecx;
      U32 Edx;
    } regData;


    // Forward declarations
    static void Detect();
    static void TranslateCacheInfo(U8 desc);
    static void Identify();


    U32 GetSpeed()
    {
      return mhz;
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // CPU Detection
    //

    //
    // Check for CPUID support
    //
    static Bool STDCALL HasCpuId()
    {
      Bool rc;

      #if defined(_MSC_VER)

      __asm
      {
        pushfd
        pop     eax
        mov     ebx, eax
        xor     eax, 200000h
        push    eax
        popfd
        pushfd
        pop     eax
        xor     eax, ebx
        shr     eax, 21
        mov     [rc], eax
      }

      #elif defined(__GNUC__)

      asm __volatile__
      (
        ".byte  0x9C;"
        "popl   %%eax;"
        "movl   %%eax, %%ebx;"
        "xorl   $0x200000, %%eax;"
        "pushl  %%eax;"
        ".byte  0x9D;"
        ".byte  0x9C;"
        "popl   %%eax;"
        "xorl   %%ebx, %%eax;"
        "shrl   $21, %%eax;"
        "movl   %%eax, %0;"

        : "=m" (rc)
        :
        : "eax", "ebx", "cc"
      );

      #endif

      return (rc);
    }

    //
    // Execute CPUID, store result in regData
    //
    static void STDCALL CpuId(U32 function)
    {
      #if defined(_MSC_VER)

      __asm
      {
        // Compiler can't detect that ebx changes
        push    ebx

        mov     eax, function
        _emit   0x0F
        _emit   0xA2

        mov     regData.Eax, eax
        mov     regData.Ebx, ebx
        mov     regData.Ecx, ecx
        mov     regData.Edx, edx

        pop     ebx
      }

      #elif defined(__GNUC__)

      asm __volatile__
      (
        "movl   %4, %%eax;"
        "cpuid;"
        "movl   %%eax, %0;"
        "movl   %%ebx, %1;"
        "movl   %%ecx, %2;"
        "movl   %%edx, %3;"

        : "=m" (regData.Eax),
          "=m" (regData.Ebx),
          "=m" (regData.Ecx),
          "=m" (regData.Edx)
        : "r"  (function)
        : "eax", "ebx", "ecx", "edx"
      );

      #endif
    }


    #pragma warning (disable: 4725) // Turn off FDIV warning

    //
    // Test for the FDIV bug
    //
    static Bool STDCALL TestFDIV()
    {
      F64 x = 4195835.0;
      F64 y = 3145727.0;
      Bool hasBug;

      __asm
      {
        fninit
        fld       x
        fdiv      y
        fmul      y
        fld       x
        fsubp     st(1),st(0)
        fistp     [hasBug]
        fwait
        fninit
      }

      return (hasBug);
    }


    //
    // Check for OS support of katmai, assumes CPUID and bit 25 already checked
    //
    /*
    static Bool TestKatmai()
    {
      Bool cpuSupport = TRUE;
      Bool osSupport = TRUE;
      Bool floatSupport = TRUE;
      Bool intSupport = TRUE;
	    Bool unmaskedException = FALSE;

	    __try
	    {
		    _asm
		    {
			    movaps		xmm0, xmm0
		    }
	    }
	    __except(EXCEPTION_EXECUTE_HANDLER)
	    {
        // either no CPU or emulation support available
			  // or no OS support available
			  // or floating-point emulation is enabled
        floatSupport = FALSE;
      }

	    __try
	    {
		    _asm
		    {
			    pmaxub		mm0, mm0
		    }
	    }
	    __except(EXCEPTION_EXECUTE_HANDLER)
	    {
        // either no CPU or emulation support available
        // or floating-point emulation is enabled
        intSupport = FALSE;
      }

	    __try
	    {
        DWORD controlStatus;
        float One = 1.0;

        // determine if OS supports unmasked exception handling for Katmai new instructions
		    // if supported, a software exception will be invoked
		    _asm
		    {
			    stmxcsr		controlStatus				      // store control/status register to memory
			    and			  controlStatus, 0xFFFFFDFF	// clear zero divide mask bit
			    ldmxcsr		controlStatus				      // load altered control/status register

			    xorps		xmm0, xmm0						      // cause a divide by zero exception
			    movss		xmm1, One
			    divss		xmm1, xmm0
		    }
	    }
	    __except(EXCEPTION_EXECUTE_HANDLER)
	    {
        unmaskedException = TRUE;
      }

	    if (cpuSupport && (!floatSupport || !unmaskedException))
      {
		    osSupport = FALSE;
      }

	    if (cpuSupport && !intSupport)
      {
        // floating point emulation is on so integer Katmai new instructions
        // and original MMX instructions are unavailable
		    osSupport = FALSE;
      }

      return (osSupport);
    }
    */


    //
    // Translate a cache descriptor byte into useful information
    //
    static void TranslateCacheInfo(U8 desc)
    {
      switch (desc)
      {
        case 0x06:
          // Instruction cache, 8K, 4-way set associative, 32 byte line size
          cacheL1 += 8;
          break;

        case 0x08:
          // 16KB instruction cache, 4-way set associative, 32 byte line size
          cacheL1 += 16;
          break;

        case 0x0A:
          // Data cache, 8K, 2-way set associative, 32 byte line size
          cacheL1 += 8;
          break;

        case 0x0C:
          // 16KB data cache, 4-way set associative, 32 byte line size
          cacheL1 += 16;
          break;

        case 0x40:
          // No L2 cache
          cacheL2 = 0;
          break;

        case 0x41:
          // Unified cache, 32 byte cache line,4-way set associative, 128K
          cacheL2 = 128;
          break;

        case 0x42:
          // Unified cache, 32 byte cache line, 4-way set associative, 256K
          cacheL2 = 256;
          break;

        case 0x43:
          // Unified cache, 32 byte cache line, 4-way set associative, 512K
          cacheL2 = 512;
          break;

        case 0x44:
          // Unified cache, 32 byte cache line, 4-way set associative, 1M
          cacheL2 = 1024;
          break;

        case 0x45:
          // Unified cache, 32 byte cache line, 4-way set associative, 2M
          cacheL2 = 2048;
          break;

        case 0x82:
          // Unified cache, 32 byte cache line, 8-way set associative, 256K
          cacheL2 = 256;
          break;
      }
    }


    //
    // DetectSpeedTSC
    //
    static U32 DetectSpeedTSC()
    {
      S64 l1, l2, freq;
      U32 ticks = 1000; // test for 1000
      U32 cycles;

      // Calculate speed for 1/100th of a second
      if (!QueryPerformanceFrequency((LARGE_INTEGER *)&freq))
      {
        return (0);
      }

      ticks = (U32)(freq / (S64)100);

      // Increase priority to realtime
	    HANDLE hThread = GetCurrentThread();
	    S32 priority = GetThreadPriority(hThread);
      U32 pclass = GetPriorityClass(hThread);

      SetPriorityClass(hThread, REALTIME_PRIORITY_CLASS);
      SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);

      // Wait for the next tick
      QueryPerformanceCounter((LARGE_INTEGER *)&l1);
      do
      {
        QueryPerformanceCounter((LARGE_INTEGER *)&l2);
      }
      while (l1 == l2);
      QueryPerformanceCounter((LARGE_INTEGER *)&l1);

      // Start timing
      Clock::CycleTimer::Start();
      do
      {
        QueryPerformanceCounter((LARGE_INTEGER *)&l2);
      }
      while (l1+ticks > l2);
      Clock::CycleTimer::Stop();

	    // Reset thread priority
      SetPriorityClass(hThread, pclass);
      SetThreadPriority(hThread, priority);

      // Calculate the speed
      ticks = (U32)(l2 - l1);
      cycles = Clock::CycleTimer::GetCount();

      return (((U32)(F32(freq) / F32(ticks) * F32(cycles)) + 500000) / 1000000);
    }


    //
    // Detects the family, and if CPUID is supported, the vendor, name and model
    //
    static void Detect()
    {
      type = UNKNOWN;

      if (HasCpuId())
      {
        valid = TRUE;

        // Function 0
        CpuId(0);

        functions = regData.Eax;

        *(U32 *)(vendor + 0) = regData.Ebx;
        *(U32 *)(vendor + 4) = regData.Edx;
        *(U32 *)(vendor + 8) = regData.Ecx;
        vendor[12] = '\0';

        // Function 1
        if (functions >= 1)
        {
          // Function 1
          CpuId(1);

          signature = regData.Eax;
          features  = regData.Edx;
          brand     = regData.Ebx & 0xFF;

          // Translate signature
          sigType     = (signature & 0x3000) >> 12;
          sigFamily   = (signature & 0x0F00) >> 8;
          sigModel    = (signature & 0x00F0) >> 4;
          sigStepping = (signature & 0x000F);

          // Translate features
          hasFeature[FPU] = (features & 0x00000001) ? TRUE : FALSE;
          hasFeature[TSC] = (features & 0x00000010) ? TRUE : FALSE;
          hasFeature[MMX] = (features & 0x00800000) ? TRUE : FALSE;
          hasFeature[PSN] = (features & 0x00040000) ? TRUE : FALSE;

          // Katmai instructions
          if (features & 0x02000000)
          {
            hasFeature[KNI] = TRUE;//TestKatmai() ? TRUE : FALSE;
          }
        }

        // Function 2
        if (functions >= 2)
        {
          detectCacheL1 = TRUE;
          detectCacheL2 = TRUE;

          // Determine how many iterations of CPUID to run for cache info
          CpuId(2);
          U8 count = U8(regData.Eax & 0xFF);

          while (count--)
          {
            CpuId(2);

            // If EAL==0 no more info
            if (regData.Eax & 0xFF == 0)
            {
              break;
            }

            // Translate cache descriptors information
            S32 loop;

            // EAX
            for (loop = 0; loop < 3; loop++, regData.Eax <<= 8)
            {
              TranslateCacheInfo(U8((regData.Eax & 0xFF000000) >> 24));
            }

            // EBX,ECX,EDX
            for (loop = 0; loop < 4; loop++, regData.Ebx <<= 8, regData.Ecx <<= 8, regData.Edx <<= 8)
            {
              TranslateCacheInfo(U8((regData.Ebx & 0xFF000000) >> 24));
              TranslateCacheInfo(U8((regData.Ecx & 0xFF000000) >> 24));
              TranslateCacheInfo(U8((regData.Edx & 0xFF000000) >> 24));
            }
          }
        }

        // Function 3
        if (functions >= 3)
        {
          CpuId(3);
          serialLo = regData.Ecx;
          serialHi = regData.Edx;
        }

        // Detect extended information
        CpuId(0x80000000);
        exFunctions = regData.Eax;

        // Function Ex 1
        if (exFunctions >= 0x80000001)
        {
          CpuId(0x80000001);
          exSignature = regData.Eax;
          exFeatures  = regData.Edx;

          // AMD3D
          hasFeature[AMD3D] = (exFeatures & 0x80000000) ? TRUE : FALSE;
        }

        // Function E4
        if (exFunctions >= 0x80000004)
        {
          char *s = exVendor;

          for (U32 function = 0x80000002, i = 0; i < 3; i++, s += 16, function++)
          {
            CpuId(function);
            *(U32 *)(s +  0) = regData.Eax;
            *(U32 *)(s +  4) = regData.Ebx;
            *(U32 *)(s +  8) = regData.Ecx;
            *(U32 *)(s + 12) = regData.Edx;
          }
          exVendor[48] = '\0';
        }

        // Function E5
        if (exFunctions >= 0x80000005)
        {
          CpuId(0x80000005);
          cacheL1 = (regData.Ecx >> 24) + (regData.Edx >> 24);
          detectCacheL1 = TRUE;
        }

        // Function E6
        if (exFunctions >= 0x80000006)
        {
          CpuId(0x80000006);
          cacheL2 = (regData.Ecx >> 16) & 0xFFFF;
          detectCacheL2 = TRUE;
        }

        // Work out CPU model from cpuid results
        switch (Crc::CalcStr(vendor))
        {
          case 0xAD7B0D75: // "GenuineIntel"
          {
            switch (sigFamily)
            {
              case 5:
              {
                // Family "5" includes Pentium and Pentium/MMX
                switch (sigModel)
                {
                  case 1:
                  case 2:
                    type = Pentium;
                    break;

                  case 4:
                    type = PentiumMMX;
                    break;
                }
                break;
              }

              case 6:
              {
                // Family "6" includes Pentium-Pro and Pentium II
                switch (sigModel)
                {
                  case 1:
                    type = PentiumPro;
                    break;

                  case 3:
                  case 4:
                  case 5:
                  {
                    // Use cache to differentiate between P2 and Celeron
                    if (cacheL2 == 128)
                    {
                      type = Celeron;
                    }
                    else
                    {
                      type = PentiumII;
                    }
                    break;
                  }

                  case 6:
                    type = Celeron;
                    break;

                  case 7:
                    type = PentiumIII;
                    break;

                  default:
                    // Models later than 7 are at least PentiumIII
                    if (sigModel > 7)
                    {
                      type = PentiumIII;

                      // Model 8 introduced brand id
                      switch (brand)
                      {
                        case 1:
                          // 1=Celeron
                          type = Celeron_2;
                          break;

                        case 2:
                        case 3:
                          // 2=P3, 3=Xeon
                          type = PentiumIII;
                          break;
                      }
                    }
                    break;
                }
                break;
              }
            }
            break;
          }

          case 0x7E4E38F1: // "AuthenticAMD"
          {
            switch (sigFamily)
            {
              case 4:
              {
                // Family "4" includes Am486 and Am5x86 processors
                type = AMD5x86;
                break;
              }

              case 5:
              {
                // Family "5" includes all K5 and K6 processors
                switch (sigModel)
                {
                  case 0:
                  case 1:
                  case 2:
                  case 3:
                    type = AMDK5;
                    break;

                  case 6:
                  case 7:
                    type = AMDK6;
                    break;

                  case 8:
                    type = AMDK6_2;
                    break;

                  default:
                    /// Models greater than 8 are at least K6-3
                    if (sigModel > 8)
                    {
                      type = AMDK6_3;
                    }
                    break;
                }
                break;
              }

              case 6:
              {
                type = AMDK7;
                break;
              }
            }
            break;
          }

          case 0x3F6F565C: // "CyrixInstead"
          {
            switch (sigFamily)
            {
              case 4:
                // Family "4" includes MediaGX
                switch (sigModel)
                {
                  case 4:
                    type = CyrixMediaGX;
                    break;
                }
                break;

              case 5:
                // Family "5" includes 6x86 and GXm
                switch (sigModel)
                {
                  case 2:
                    type = Cyrix686;
                    break;

                  case 4:
                    type = CyrixGXm;
                    break;
                }
                break;

              case 6:
                // Family "6" includes 6x86MX
                switch (sigModel)
                {
                  case 0:
                    type = Cyrix686MX;
                    break;
                }
                break;
            }
            break;
          }

          case 0xA6B5635B: // "CentaurHauls"
          {
            switch (sigFamily)
            {
              case 5:
              {
                // Family "5" include C6
                switch (sigModel)
                {
                  case 4:
                    type = IDTC6;
                    break;
                }
                break;
              }
            }
            break;
          }
        }

        // Test for the FDIV bug
        hasFeature[FDIVBUG] = TestFDIV();

        // ... which of course fucks with the FPU control word
        Utils::FP::Reset();

        // Setup name
        cpuName = CPUNAMES[type];

        // If no serial number, make one up
        if (!hasFeature[PSN])
        {
          //serialHi = Crc::CalcStr(vendor) ^ signature;
        }
      }
      else
      {
        // Unknown CPU
        valid = FALSE;
      }
    }


    //
    // Attempt CPU identification
    //
    static void Identify()
    {
      if (queried)
      {
        return;
      }

      // Initialise data
      for (int i = 0; i < MAXFEATURES; i++)
      {
        hasFeature[i] = FALSE;
      }

      // Perform the detection
      __try
      {
        // Determine the family
        Detect();

        // If the Time Stamp Counter feature exists use it to get the speed
        if (hasFeature[TSC])
        {
          mhz = DetectSpeedTSC();
        }
        else
        {
          // Any other way to detect speed?
        }
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
      }

      // Build up description
      if (valid)
      {
        // Build a normal description
        Utils::Sprintf(descNormal, sizeof(descNormal), "%s/%uMHz", (type == UNKNOWN) ? vendor : cpuName, mhz);

        // Build a detailed description with family/model/stepping
        S32 n = 0;

        n += Utils::Sprintf(descDetail, sizeof(descDetail), "%s/%uMHz", (type == UNKNOWN) ? (*exVendor ? exVendor : vendor) : cpuName, mhz);

        // Cache information
        if (detectCacheL1)
        {
          n += Utils::Sprintf(descDetail + n, sizeof(descDetail) - n, " L1:%uK", cacheL1);
        }
        if (detectCacheL2)
        {
          n += Utils::Sprintf(descDetail + n, sizeof(descDetail) - n, " L2:%uK", cacheL2);
        }

        // Stepping/model etc
        n += Utils::Sprintf(descDetail + n, sizeof(descDetail) - n, " T:%d F:%d M:%d S:%X", sigType, sigFamily, sigModel, sigStepping);

        // Feature flags
        int fcount = 0;

        for (int i = MAXFEATURES-1; i >= 0; i--)
        {
          if (hasFeature[i])
          {
            if (n + strlen(FEATURENAMES[i]) >= sizeof(descDetail) - 3)
            {
              break;
            }

            n += Utils::Sprintf(descDetail + n, sizeof(descDetail) - n, fcount ? "," : " (");
            n += Utils::Sprintf(descDetail + n, sizeof(descDetail) - n, FEATURENAMES[i]);
            fcount++;
          }
        }

        if (fcount)
        {
          n += Utils::Sprintf(descDetail + n, sizeof(descDetail) - n, ")");
        }

        // Serial number
        for (U32 lo = serialLo, hi = serialHi, d = 0; d < 8; d++, lo <<= 4, hi <<= 4)
        {
          const char *H2A = "0123456789ABCDEF";

          serialDesc[d    ] = H2A[(hi >> 28) & 0xF];
          serialDesc[d + 8] = H2A[(lo >> 28) & 0xF];
        }
        serialDesc[16] = '\0';
      }
      else
      {
        // Undetectable CPU
        strcpy(descNormal, "Unknown");
        strcpy(descDetail, descNormal);
      }

      queried = TRUE;
    }

    // Get the CPU Type
    Type GetType()
    {
      Identify();
      return type;
    }

    // Get the description
    const char *GetDesc()
    {
      Identify();
      return (descNormal);
    }

    // Get detailed description
    const char *GetDetailedDesc()
    {
      Identify();
      return (descDetail);
    }

    // Get serial number string
    const char *GetSerial()
    {
      Identify();
      return (serialDesc);
    }

    // Test for a CPU feature
    Bool HasFeature(Features f)
    {
      Identify();
      return hasFeature[f];
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Memory
  //
  namespace Memory
  {

    //
    // Static data
    //
    static Bool queried = FALSE;
    static U32  physicalMem = 0;
    static U32  virtualMem = 0;
    static char desc[256] = "";


    //
    // Detect Memory configuration for this system
    //
    void Identify()
    {
      if (!queried)
      {
        MEMORYSTATUS memStat;

        memStat.dwLength = sizeof (MEMORYSTATUS);
        GlobalMemoryStatus(&memStat);

        physicalMem = memStat.dwTotalPhys;
        virtualMem = memStat.dwTotalPageFile;
        Utils::Sprintf(desc, 256, "%.2f/%.2f", ((F32) physicalMem) / 1048576, ((F32) virtualMem / 1048576));

        queried = TRUE;
      }
    }

    U32  GetPhysical()
    {
      Identify();
      return (physicalMem);
    }

    U32  GetVirtual()
    {
      Identify();
      return (virtualMem);
    }

    const char *GetDesc()
    {
      Identify();
      return (desc);
    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Device
  //
  namespace Device
  {

    //
    // Static data
    //
    static Bool queried = FALSE;
    static U32  numDevices = 0;
    static char devices[128][128];


    //
    // Identify
    //
    // Dectect Hardware Devices in the registry
    // Look for ALL devices in HKEY_DYN_DATA\Config Manager\Enum
    // For each device look in HKEY_LOCAL_MACHINE\Enum for the description
    //
    static void Identify()
    {
      if (!queried)
      {
        numDevices = 0;

        #ifdef DEVELOPMENT

        if (Hardware::OS::GetType() == OS::Win9x)
        {
          U32 length;
          BinTree<char> deviceTree;

          // Enum the devices
          Registry::Key keyDevices(HKEY_DYN_DATA, "Config Manager\\Enum", Registry::ENUM, FALSE);
          Registry::Key keyDeviceDesc(HKEY_LOCAL_MACHINE, "Enum", Registry::ENUM, FALSE);

          int i = 0;
          String subkeyname;
          while (keyDevices.Enum(i++, subkeyname))
          {
            // Open the subkey
            Registry::Key subkey(keyDevices, subkeyname, Registry::READ, FALSE);

            char buffer[1024];
            length = 1024;

            // The DRIVER string tells us if this is a device of interest
            if (subkey.GetString("HardWareKey", buffer, &length))
            {
              length = 1024;

              // This is the key which we use in HKLM\Enum
              Registry::Key keyDevice(keyDeviceDesc, buffer, Registry::READ, FALSE);

              // Get the device type and its description
              if (!keyDevice.NotFound() && keyDevice.GetString("Driver", buffer, &length))
              {
                *strchr(buffer, '\\') = '\0';

                // Add this device to the device tree
                U32 crc = Crc::CalcStr(buffer);
                switch (crc)
                {
                  // Filter off some not so usefull part of the tree
                  case 0xF81D1051: // "system"
                  case 0x6A6203EB: // "ports"
                  case 0x8E47CC6A: // "hdc"
                  case 0x84514340: // "fdc"
                  case 0x84970DEB: // "unknown"
                    break;

                  // Add the rest
                  default:
                  {
                    char *deviceString = new char[256];
                    Utils::Sprintf(deviceString, 256, "[%s] ", buffer);
                    length = 128;
                    keyDevice.GetString("DeviceDesc", deviceString + strlen(deviceString), &length);
                    deviceTree.Add(crc, deviceString);
                    break;
                  }
                }
              }
            }
          }

          // No put the sorted tree into the array
          BinTree<char>::Iterator it(&deviceTree);
          for (!it; *it; it++)
          {
            Bool exists = FALSE;

            // Check to see if this device is already listed
            for (U32 id = 0; id < numDevices; id++)
            {
              if (!strcmp(*it, devices[id]))
              {
                exists = TRUE;
              }
            }

            if (!exists)
            {
              strcpy(devices[numDevices++], *it);
            }
          }

          // Destroy the temporary device tree
          deviceTree.DisposeAll();
        }

        #endif

        queried = TRUE;
      }
    }


    //
    // Return number of detected devices
    //
    U32 GetNum()
    {
      Identify();
      return (numDevices);
    }


    //
    // Enumerate detected devices
    //
    const char *Enum(U32 number)
    {
      Identify();

      if (number < numDevices)
      {
        return (devices[number]);
      }
      else
      {
        return (NULL);
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace OS
  //
  namespace OS
  {

    //
    // Static data
    //
    static Bool queried   = FALSE;
    static char desc[256] = "";
    static Type type      = Unknown;
    static U32  versionMajor = 0;
    static U32  versionMinor = 0;
    static U32  build     = 0;
    static char computer[256] = "";
    static char user[256] = "";


    //
    // Attempt to indentify the operating system
    //
    static void Identify()
    {
      if (!queried)
      {
        queried = TRUE;

        OSVERSIONINFO ver;
        ver.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        GetVersionEx(&ver);

        switch (ver.dwPlatformId)
        {
          case VER_PLATFORM_WIN32s:
            type = Win32s;
            versionMajor = 3;
            versionMinor = 1;
            build = 0;
            Utils::Sprintf(desc, 256, "Windows 3.1 (with Win32s)");
            break;

          case VER_PLATFORM_WIN32_WINDOWS:
            type = Win9x;
            versionMajor = HIWORD(ver.dwBuildNumber) >> 8;
            versionMinor = HIWORD(ver.dwBuildNumber) & 0xFF;
            build = LOWORD(ver.dwBuildNumber);
            Utils::Sprintf(desc, 256, "Windows 9x v%d.%d %s Build %d", versionMajor, versionMinor, ver.szCSDVersion, build);
            break;

          case VER_PLATFORM_WIN32_NT:
            type = WinNT;
            versionMajor = ver.dwMajorVersion;
            versionMinor = ver.dwMinorVersion;
            build = ver.dwBuildNumber;
            Utils::Sprintf(desc, 256, "Windows NT v%d.%d %s Build %d", versionMajor, versionMinor, ver.szCSDVersion, build);
            break;

          default:
            type = Unknown;
            versionMajor = 0;
            versionMinor = 0;
            build = 0;
            Utils::Sprintf(desc, 256, "Unknown");
            break;
        }

        U32 len = 256;
        GetComputerName(computer, &len);
        len = 256;
        GetUserName(user, &len);
      }
    }


    //
    // GetDesc
    //
    const char *GetDesc()
    { 
      Identify(); 
      return (desc); 
    }


    //
    // GetType
    //
    Type GetType()
    { 
      Identify(); 
      return (type); 
    }


    //
    // GetVersionMajor
    //
    U32 GetVersionMajor()
    { 
      Identify(); 
      return (versionMajor); 
    }


    //
    // GetVersionMinor
    //
    U32 GetVersionMinor()
    { 
      Identify(); 
      return (versionMinor); 
    }


    //
    // GetBuild
    //
    U32 GetBuild()
    { 
      Identify(); 
      return (build); 
    }


    //
    // GetComputer
    //
    const char *GetComputer()
    { 
      Identify(); 
      return (computer); 
    }


    //
    // GetUser
    //
    const char *GetUser()
    { 
      Identify(); 
      return (user); 
    }

  }

}