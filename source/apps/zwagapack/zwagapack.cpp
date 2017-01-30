///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Pack file management code
//
// 08-JAN-1998
//

#include <stdlib.h>
#include <string.h>
#include "packtool.h"

// executable name
static const char *exeName;


//
// Display
//
// passed to the packtool for progress display
//
static void Display(const char *str)
{
  printf("%s\n", str);
}


//
// CreatePack
//
// create a new pack file
//
static void CreatePack(const char *pack, const char *dir, const char *mask, int subs)
{
  PackTool pTool(Display);
 
  // create the pack file
  if (!pTool.Create(pack))
  {
    printf("Unable to create requested pack file : %s\n", pack);
    return;
  }

  // add directory
  pTool.AddDir(dir, mask, subs);
  pTool.Close();
}


//
// ExtractPack
//
// extract contents
//
static void ExtractPack(const char *pack, const char *dir)
{
  PackTool pTool(Display);
 
  if (!pTool.Open(pack))
  {
    printf("Unable to open requested pack file : %s\n", pack);
    return; 
  }

  pTool.ExtractAll(dir);
  pTool.Close();
}


//
// ListPack
//
// list contents
//
static void ListPack(const char *pack)
{
  PackTool pTool(Display);
 
  if (!pTool.Open(pack))
  {
    printf("Unable to open requested pack file : %s\n", pack);
    return; 
  }

  pTool.ListContents();
  pTool.Close();
}


//
// Usage
//
// displays usage information
//
void Usage()
{
  printf("Usage: %s <command> [-options] <pakfile> [directory]\n", exeName);
  printf("\n<command>\n");
  printf("  c : create new pack   (eg. %s c data.zwp c:\\srcdir)\n", exeName);
  printf("  x : extract pack      (eg. %s x data.zwp c:\\destdir)\n", exeName);
  printf("  l : list contents     (eg. %s l data.zwp)\n", exeName);
  printf("\n[options]\n");
  printf("  r : recurse sub-dirs  (eg. %s c -r data.zwp)\n", exeName);
  printf("  m : set file mask     (eg. %s c -m=*.wav data.zwp)\n\n", exeName);

  Debug::Done();
  exit(1);
}


//
// main
//
// da big cheese
//
int __cdecl main(int argc, char *argv[])
{
  char pack[_MAX_FNAME] = "";
  char dir[_MAX_PATH] = ".";
  char *mask = "*.*";
  int subs = 0;
  enum { CREATE, EXTRACT, LIST } command = CREATE;

  printf("Dark Reign 2 - Pack Utility, %s %s\n\n", __DATE__, __TIME__);

  // get exe name
  exeName = PackTool::NameFromPath(*argv);

  // required arguments
  if (argc < 3)
  {
    Usage();
  }

  // be picky about single char command
  if (argv[1][1] != '\0')
  {
    printf("Unknown command '%s'\n\n", argv[1]);   
    Usage();
  }

  // set command
  switch (*argv[1])
  {
    case 'c' :
      command = CREATE;     
      break;

    case 'x' :
      command = EXTRACT;
      break;

    case 'l' :
      command = LIST;
      break;

    default:
      printf("Unknown command '%s'\n\n", argv[1]);
      Usage();
  }

  // parse remaining arguments
  for (int i = 2; i < argc; i++)
  {
    // is this an option
    if (*argv[i] == '-')
    {
      switch (argv[i][1])
      {
        case 'r' : 
          subs = 1;
          break;

        case 'm' :
          if ((mask = strchr(argv[i], '=')) == 0)
          {
            Usage();
          }
          mask++;         
          break;

        default:
          printf("Unknown option '-%c'\n\n", argv[i][1]);
          Usage();
      }
    }
    else
    {
      // is this the pack name
      if (*pack == '\0')
      {
        strcpy(pack, argv[i]);
      }
      else
      {
        // must be the dir
        strcpy(dir, argv[i]);
      }
    }
  }

  // check that we got a pack name
  if (*pack == '\0')
  {
    printf("No pack file name supplied\n\n");
    Usage();  
  }

  // packtool throws char* exceptions
  try 
  {
    // do the operation
    switch (command)
    {
      case CREATE:
        CreatePack(pack, dir, mask, subs);
        break;

      case EXTRACT:
        ExtractPack(pack, dir);
        break;

      case LIST:
        ListPack(pack);
        break;

      default:
        Usage();
    }
	} 
  catch (char *msg)
	{
    printf("\nError : %s", msg);
	}

  return (0);  
}