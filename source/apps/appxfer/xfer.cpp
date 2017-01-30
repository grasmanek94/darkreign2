#include "std.h"
#include "styxnet.h"
#include "styxnet_transfer.h"
#include "logging_destconsole.h"

#include <ios>
using namespace std;

void Usage()
{
  cout << "xfer Send|Recv" << endl;
  cout << "- Send filename" << endl;
  cout << "- Recv filename size address port" << endl;
}

U32 argCount;
const char ** args;

void CDECL Main()
{
  if (argCount <= 1)
  {
    Usage();
    return;
  }

  Bool quit = FALSE;

  StyxNet::Init();
  StyxNet::Transfer *transfer = new StyxNet::Transfer;

  LOG_ADDDEST(new Logging::DestConsole())

  switch (Crc::CalcStr(args[1]))
  {
    case 0x6E3CD283: // "Send"
    {
      if (argCount < 2)
      {
        Usage();
        quit = TRUE;
      }
      else
      {
        U16 port;
        U32 size;
        const char *filename = args[2];

        // Initiate send
        if (transfer->Send(filename, port, size))
        {
          // Print out the good stuff
          cout << "Ready to send file " << filename << endl;
          cout << "Port: " << U32(port) << endl;
          cout << "Size: " << size << endl;
        }
        else
        {
          cout << "Could not send file!" << endl;
          quit = TRUE;
        }
      }
      break;
    }

    case 0xEAB70B0D: // "Recv"
    {
      if (argCount < 5)
      {
        Usage();
        quit = TRUE;
      }
      else
      {
        const char *filename = args[2];
        U32 size = atoi(args[3]);
        const char *address = args[4];
        U16 port = static_cast<U16>(atoi(args[5]));
 
        // Initiate recv
        if (transfer->Recv(filename, Win32::Socket::Address(address, port), size))
        {
          cout << "Ready to recv file " << filename << endl;
        }
        else
        {
          cout << "Could not recv file!" << endl;
          quit = TRUE;
        }
      }
      break;
    }

    default:
      Usage();
      quit = TRUE;
      break;
  }

  if (!quit)
  {
    U32 i;
    cin >> i;
  }

  delete transfer;
  StyxNet::Done();
}

void CDECL main(int argc, const char **argv)
{
  argCount = argc;
  args = argv;

  Debug::Memory::Init();
  Debug::Init();
  Log::Init();
  Debug::Exception::Handler(Main);
  Log::Done();
  Debug::Memory::Check();
  Debug::Done();
}
