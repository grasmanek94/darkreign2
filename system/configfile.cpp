///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Configuration File Utilities
//
// 16-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "configfile.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ConfigFile
//


//
// ConfigFile
//
ConfigFile::ConfigFile(const char *name)
{
  filename = name;
}


//
// EnumSections
//
Bool ConfigFile::EnumSections(char *buffer, U32 &length)
{
  U32 err = GetPrivateProfileSectionNames(buffer, length, filename);

  if (err == length - 2)
  {
    return (FALSE);
  }
  else
  {
    length = err;
    return (TRUE);
  }
}


//
// ClearSection
//
Bool ConfigFile::ClearSection(const char *section)
{
  return (WritePrivateProfileSection(section, NULL, filename) ? TRUE : FALSE);
}


//
// EnumKeys
//
Bool ConfigFile::EnumKeys(const char *section, char *buffer, U32 &length)
{
  U32 err = GetPrivateProfileSection(section, buffer, length, filename);

  if (err == length - 2)
  {
    return (FALSE);
  }
  else
  {
    length = err;
    return (TRUE);
  }
}


//
// WriteInt : Writes an integer to the config file
//
Bool ConfigFile::WriteInt(const char *section, const char *name, int val)
{
  char buffer[32];
  Utils::Sprintf(buffer, 32, "i%d", val);
  return (WritePrivateProfileString(section, name, buffer, filename) ? TRUE : FALSE);
}


//
// WriteFloat : Writes a floating point number to the config file
//
Bool ConfigFile::WriteFloat(const char *section, const char *name, F32 val)
{
  char buffer[32];
  Utils::Sprintf(buffer, 32, "f%f", val);
  return (WritePrivateProfileString(section, name, buffer, filename) ? TRUE : FALSE);
}


//
// WriteString : Writes a string to the config file
//
Bool ConfigFile::WriteString(const char *section, const char *name, const char *val)
{
  char buffer[256];
  Utils::Sprintf(buffer, 256, "s%s", val);
  return (WritePrivateProfileString(section, name, buffer, filename) ? TRUE : FALSE);
}


//
// WriteBinary : Writes a binary to the config file
//
Bool ConfigFile::WriteBinary(const char *section, const char *name, const U8 *val, U32 length)
{
  char buffer[1024];
  buffer[0] = 'b';
  Utils::CharEncode(buffer + 1, val, length, 1023);
  buffer[length * 2 + 1] = '\0';
  return (WritePrivateProfileString(section, name, buffer, filename) ? TRUE : FALSE);
}


//
// ReadInt : Reads an integer from the config file
//
Bool ConfigFile::ReadInt(const char *section, const char *name, int &val, const char *def)
{
  char buffer[32];

  if (!GetPrivateProfileString(section, name, def, buffer, 32, filename))
  {
    return (FALSE);
  }
  
  if (buffer[0] != 'i')
  {
    return (FALSE);  
  }

  val = atoi(buffer + 1);
  return (TRUE);
}


//
// ReadInt : Reads an integer from the config file
//
int ConfigFile::ReadInt(const char *section, const char *name, const char *def)
{
  int i = 0;
  ConfigFile::ReadInt(section, name, i, def);
  return (i);
}


//
// ReadFloat : Reads a floating point number from the config file
//
Bool ConfigFile::ReadFloat(const char *section, const char *name, F32 &val, const char *def)
{
  char buffer[32];

  if (!GetPrivateProfileString(section, name, def, buffer, 32, filename))
  {
    return (FALSE);
  }
  
  if (buffer[0] != 'f')
  {
    return (FALSE);  
  }

  val = (F32) atof(buffer + 1);
  return (TRUE);
}


//
// ReadString : Reads a string from the config file
//
Bool ConfigFile::ReadString(const char *section, const char *name, char *val, U32 &length, const char *def)
{
  if (!GetPrivateProfileString(section, name, def, val, length, filename))
  {
    return (FALSE);
  }
  
  if (val[0] != 's')
  {
    return (FALSE);  
  }

  memmove(val, val + 1, strlen(val));
  return (TRUE);
}


//
// ReadBinary : Reads a binary from the config file
//
Bool ConfigFile::ReadBinary(const char *section, const char *name, U8 *val, U32 &length, const char *def)
{
  char buffer[1024];

  if (!GetPrivateProfileString(section, name, def, buffer, 1024, filename))
  {
    return (FALSE);
  }
  
  if (buffer[0] != 'b')
  {
    return (FALSE);  
  }

  Utils::CharDecode(val, buffer + 1, strlen(buffer + 1), length);
  return (TRUE);
}
