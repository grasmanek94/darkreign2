///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// VersionMaker
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "std.h"
#include "file.h"
#include "hardware.h"


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")


//
// Usage
//
void Usage()
{
  printf("VersionMaker filename options\n");
  printf(" -company=Company Name\n");
  printf(" -description=Description\n");
  printf(" -version=Version\n");
  printf(" -comments=Comments\n");
  printf(" -include=Include\n");
  printf(" -roll=0 or 1");
}


//
// WinMain
//
int CDECL main(int argc, const char **argv)
{
  BinTree<char> data;
  const char *fileName = NULL;

  // Parse the args
  for (int i = 1; i < argc; i++)
  {
    // Is this an option
    if (*argv[i] == '-')
    {
      // Get the key pair
      char *key = Utils::Strdup(argv[i] + 1);
      char *value = strtok(key, "=");
      if (value)
      {
        value = strtok(NULL, "=");
        data.Add(Crc::CalcStr(key), Utils::Strdup(value));
      }
      delete key;
    }
    else
    {
      fileName = argv[i];
    }
  }

  if (!fileName)
  {
    Usage();
    return (TRUE);
  }

  const char *company = data.Find(0xB2401AA0);      // "company";
  const char *description = data.Find(0x47CB37F2);  // "description"
  const char *version = data.Find(0x1ACCF75F);      // "version";
  const char *include = data.Find(0x8CBCE90A);      // "include"
  const char *comments = data.Find(0x62E46F43);     // "comments"
  const char *roll = data.Find(0x39DF06B5);         // "roll"
  
  U32 build;

  company = company ? company : "Vandelay Industries";
  description = description ? description : "Insert Description Name Here";
  version = version ? version : "0,0";
  roll = roll ? roll : "1";

  File file;

  if (file.Open("c:\\build.ver", File::READ | File::WRITE))
  {
    file.Read(&build, sizeof (U32));

    if (*roll == '1')
    {
      build++;

      file.Seek(0, File::SET);
      file.Write(&build, sizeof (U32));
      file.Close();
    }
  }
  else
  {
    build = 0;
  }

  if (!file.Open(fileName, File::CREATE | File::WRITE))
  {
    printf("Could not open file: %s", fileName);
    return (TRUE);
  }

  printf("Creating version '%s'\n", fileName);

  if (include)
  {
    file.WriteString("#include \"%s\"\n", include);
  }

  file.WriteString("\n");
  file.WriteString("#include \"afxres.h\"\n");
  file.WriteString("\n");
  file.WriteString("LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US\n");
  file.WriteString("#pragma code_page(1252)\n");
  file.WriteString("\n");
  file.WriteString("VS_VERSION_INFO VERSIONINFO\n");
  file.WriteString(" FILEVERSION %s,%d,0\n", version, build, 0);
  file.WriteString(" PRODUCTVERSION %s,%d,0\n", version, build, 0);
  file.WriteString(" FILEFLAGSMASK 0x3fL\n");
  file.WriteString("#ifdef _DEBUG\n");
  file.WriteString(" FILEFLAGS 0x1L\n");
  file.WriteString("#else\n");
  file.WriteString(" FILEFLAGS 0x0L\n");
  file.WriteString("#endif\n");
  file.WriteString(" FILEOS 0x40004L\n");
  file.WriteString(" FILETYPE 0x1L\n");
  file.WriteString(" FILESUBTYPE 0x0L\n");
  file.WriteString("BEGIN\n");
  file.WriteString("  BLOCK \"StringFileInfo\"\n");
  file.WriteString("  BEGIN\n");
  file.WriteString("    BLOCK \"040904b0\"\n");
  file.WriteString("    BEGIN\n");
  file.WriteString("      VALUE \"FileVersion\", \"%s\\0\"\n", version);
  file.WriteString("      VALUE \"FileDescription\", \"%s\\0\"\n", description);
  file.WriteString("      VALUE \"LegalCopyright\", \"Copyright © 1997-2000\\0\"\n");
  file.WriteString("      VALUE \"CompanyName\", \"%s\\0\"\n", company);

  if (comments)
  {
    file.WriteString("      VALUE \"Comments\", \"%s\\0\"\n", comments);
  }

  char strt[32];
  strcpy(strt, Clock::Date::GetVerbose());
  strt[24] = '\0';

  char str[256];
  sprintf(str, "%s b%d [%s] %s", description, build, comments, strt);
  file.WriteString("      VALUE \"Build String\", \"%s\\0\"\n", str);

  Clock::Time::GetStr(str);
  file.WriteString("      VALUE \"Build Time\", \"%s\\0\"\n", str);
  Clock::Date::GetStr(str);
  file.WriteString("      VALUE \"Build Date\", \"%s\\0\"\n", str);

  file.WriteString("      VALUE \"Build Number\", \"%d\\0\"\n", build);

  U32 size = 256;
  GetUserName(str, &size);
  file.WriteString("      VALUE \"Build User\", \"%s\\0\"\n", str);

  size = 256;
  GetComputerName(str, &size);
  file.WriteString("      VALUE \"Build Machine\", \"%s\\0\"\n", str);

  file.WriteString("      VALUE \"Build OS\", \"%s\\0\"\n", Hardware::OS::GetDesc());

  file.WriteString("    END\n");
  file.WriteString("  END\n");
  file.WriteString("  BLOCK \"VarFileInfo\"\n");
  file.WriteString("  BEGIN\n");
  file.WriteString("    VALUE \"Translation\", 0x409, 1200\n");
  file.WriteString("  END\n");
  file.WriteString("END\n");

  file.Close();

  data.DisposeAll();

  return (FALSE);
}

