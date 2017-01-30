///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Configuration File Utilities
//
// 16-JUL-1998
//

#ifndef __CONFIGFILE_H
#define __CONFIGFILE_H


///////////////////////////////////////////////////////////////////////////////
//
// CLASS ConfigFile
//

class ConfigFile
{
private:

  const char *filename;

public:

  // ConfigFile
  ConfigFile(const char *name);

  // EnumSections
  Bool EnumSections(char *buffer, U32 &length);

  // ClearSection
  Bool ClearSection(const char *section);

  // EnumKeys
  Bool EnumKeys(const char *section, char *buffer, U32 &length);

  // WriteInt : Writes an integer to the config file
  Bool WriteInt(const char *section, const char *name, int val);

  // WriteFloat : Writes a floating point number to the config file
  Bool WriteFloat(const char *section, const char *name, F32 val);

  // WriteString : Writes a string to the config file
  Bool WriteString(const char *section, const char *name, const char *val);

  // WriteBinary : Writes a binary to the config file
  Bool WriteBinary(const char *section, const char *name, const U8 *val, U32 length);

  // ReadInt : Reads an integer from the config file
  Bool ReadInt(const char *section, const char *name, int &val, const char *def = "i0");

  // ReadInt : Reads an integer from the config file
  int ReadInt(const char *section, const char *name, const char *def = "i0");

  // ReadFloat : Reads a floating point number from the config file
  Bool ReadFloat(const char *section, const char *name, F32 &val, const char *def = "f0");

  // ReadString : Reads a string from the config file
  Bool ReadString(const char *section, const char *name, char *val, U32 &length, const char *def = "s");

  // ReadBinary : Reads a binary from the config file
  Bool ReadBinary(const char *section, const char *name, U8 *val, U32 &length, const char *def = "b");

};

#endif