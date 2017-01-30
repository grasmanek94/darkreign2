////////////////////////////////////////////////////////////////////////////////
//
// DNS
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_dns.h"
#include "logging.h"


////////////////////////////////////////////////////////////////////////////////
//
// Libraries
//

//#pragma comment(lib, "ws2_32.lib")


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace DNS
  //
  namespace DNS
  {

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Lookup
    //

    struct CallbackWithContext
    {
      Callback callback;
      void *context;
      NList<CallbackWithContext>::Node node;

      CallbackWithContext(Callback callback, void *context)
      : callback(callback),
        context(context)
      {
      }
    };

    struct Lookup
    {
      // Tree node
      NBinTree<Lookup, HANDLE>::Node node;
      NBinTree<Lookup>::Node nodeName;
      NBinTree<Lookup>::Node nodeAddress;

      // Host entry data
      union
      {
        // Hostent
        HOSTENT hostent;

        // Buffer
        char buf[MAXGETHOSTSTRUCT];
      };

      // List of callbacks to call when this lookup completes
      NList<CallbackWithContext> callbacks;

      // Constructor
      Lookup()
      : callbacks(&CallbackWithContext::node)
      {
        // Clear lookup struct
        Utils::Memset(buf, 0x00, sizeof (MAXGETHOSTSTRUCT));
      }

      // Destructor
      ~Lookup()
      {
        callbacks.DisposeAll();
      }

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //

    // Pending lookups by lookup handle
    static NBinTree<Lookup, HANDLE> lookups(&Lookup::node);

    // Pending lookups by name
    static NBinTree<Lookup> lookupsByName(&Lookup::nodeName);

    // Pending lookups by address
    static NBinTree<Lookup> lookupsByAddress(&Lookup::nodeAddress);

    // Hosts by name
    static NBinTree<Host> hostsByName(&Host::nodeName);

    // Hosts by address
    static NBinTree<Host> hostsByAddress(&Host::nodeAddress);

    // Failed host names
    static BinTree<void> failedNames;

    // Failed host addresses
    static BinTree<void> failedAddresses;

    // Initialized flag
    static Bool initialized = FALSE;

    // Setup flag
    static Bool setup = FALSE;

    // Window handle
    static HWND hwnd;

    // Window message
    static U32 message;

  
    //
    // Construct from a lookup structure
    //
    Host::Host(const Lookup &lookup)
    {
      // Copy out the name
      name = Utils::Strdup(lookup.hostent.h_name);

      // Copy out the aliases
      char **a = lookup.hostent.h_aliases;
      while (*a)
      {
        aliases.Append(Utils::Strdup(*a));
        a++;
      }

      // Copy out the addresses
      a = lookup.hostent.h_addr_list;
      while (*a)
      {
        U32 addr = *reinterpret_cast<U32 *>(*a);
        addresses.Append(new Socket::Address(ntohl(addr), 0));
        a++;
      }
    }


    //
    // Destructor
    //
    Host::~Host()
    {
      delete name;
      aliases.DisposeAll();
      addresses.DisposeAll();
    }


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)

      initialized = TRUE;
    }


    //
    // Done
    //
    void Done()
    {
      ASSERT(initialized)

      hostsByAddress.UnlinkAll();
      hostsByName.DisposeAll();

      lookupsByAddress.UnlinkAll();
      lookupsByName.UnlinkAll();
      lookups.DisposeAll();

      failedNames.DisposeAll();
      failedAddresses.DisposeAll();

      initialized = FALSE;
    }

  
    //
    // Setup host system with a window handle
    //
    void Setup(HWND h, U32 m)
    {
      ASSERT(initialized)

      hwnd = h;
      message = m;

      setup = TRUE;
    }


    //
    // Window proc which is hooked into the setup window handle
    //
    void WindowProc(U32, UINT msg, WPARAM wParam, LPARAM lParam)
    {
      ASSERT(setup)

      if (message == msg)
      {
        Lookup *lookup = lookups.Find(HANDLE(wParam));

        if (lookup)
        {
          Host *host = NULL;

          switch (WSAGETASYNCERROR(lParam))
          {
            case 0:
            {
              // OSR2 fails by not touching the structure (ugh)
              if (lookup->hostent.h_name)
              {
                // Success create the new host record
                host = new Host(*lookup);
                hostsByName.Add(Crc::CalcStr(host->GetName()), host);
                hostsByAddress.Add(host->GetAddress()->GetIP(), host);
              }
              break;
            }

            case WSAENETDOWN:
              LDIAG("WSAAsyncGetHostByName: The network subsystem has failed.")
              break;

            case WSAENOBUFS:
              LERR("WSAAsyncGetHostByName: Insufficient buffer space is available.")
              break;

            case WSAHOST_NOT_FOUND:
            case WSANO_DATA:
              break;

            case WSATRY_AGAIN:
              LDIAG("WSAAsyncGetHostByName: Non-Authoritative Host not found, or SERVERFAIL.")
              break;

            case WSANO_RECOVERY:
              LDIAG("WSAAsyncGetHostByName: Nonrecoverable errors, FORMERR, REFUSED, NOTIMP.")
              break;

            case WSAEFAULT:
              LDIAG("WSAAsyncGetHostByName: Invalid name or address.")
              break;

            default:
              LERR("WSAASyncGetHostByName: Unknown Error " << dec << WSAGETASYNCERROR(lParam) << " [" << HEX(8, WSAGETASYNCERROR(lParam)) << "]")
          }

          if (!host)
          {
            // Host not found
            if (lookup->nodeName.InUse())
            {
              failedNames.Add(lookup->nodeName.GetKey(), NULL);
            }
            if (lookup->nodeAddress.InUse())
            {
              failedAddresses.Add(lookup->nodeAddress.GetKey(), NULL);
            }
          }

          // Call any callbacks in the lookup
          NList<CallbackWithContext>::Iterator c(&lookup->callbacks);
          while (CallbackWithContext *cwc = c++)
          {
            cwc->callback(host, cwc->context);
          }

          // Remove the lookup
          lookups.Unlink(lookup);
          if (lookup->nodeName.InUse())
          {
            lookupsByName.Unlink(lookup);
          }
          if (lookup->nodeAddress.InUse())
          {
            lookupsByAddress.Unlink(lookup);
          }

          delete lookup;
        }
        else
        {
          LWARN("Lookup was not found for the handle " << wParam)
        }
      }
    }

  
    //
    // Get host by name (return TRUE if host is valid or pending, and FALSE if not found)
    //
    Bool GetByName(const char *name, Host *&host, Callback callback, void *context)
    {
      ASSERT(setup)

      // Clear host
      host = NULL;

      // Get name crc
      U32 crc = Crc::CalcStr(name);

      // Is this is failed host ?
      if (failedNames.Exists(crc))
      {
        // If there's an callback, call it
        if (callback)
        {
          callback(NULL, context);
        }
        return (FALSE);
      }
      else
      {
        host = hostsByName.Find(crc);
        if (host)
        {
          // If there's an callback, call it
          if (callback)
          {
            callback(host, context);
          }
        }
        else
        {
          Lookup *lookup = lookupsByName.Find(crc);

          // Is there a pending lookup of this name ?
          if (!lookup)
          {
            lookup = new Lookup;
            HANDLE handle = WSAAsyncGetHostByName(hwnd, message, name, lookup->buf, MAXGETHOSTSTRUCT);

            if (!handle)
            {
              switch (WSAGetLastError())
              {
                case WSANOTINITIALISED:
                  LERR("WSAAsyncGetHostByName: A successful WSAStartup must occur before using this function.")
                  break;

                case WSAENETDOWN:
                  LERR("WSAAsyncGetHostByName: The network subsystem has failed.")
                  break;

                case WSAEINPROGRESS:
                  LERR("WSAAsyncGetHostByName: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")
                  break;

                case WSAEWOULDBLOCK:
                  LERR("WSAAsyncGetHostByName: The asynchronous operation cannot be scheduled at this time due to resource or other constraints within the Windows Sockets implementation.")
                  break;

                default:
                  LERR("WSAAsyncGetHostByName: Unknown error!");
              }
            }

            // Add to the lookups by handle tree
            lookups.Add(handle, lookup);

            // Add to the lookups by name tree
            lookupsByName.Add(crc, lookup);
          }

          // Is there an callback to be called on completion ?
          if (callback)
          {
            lookup->callbacks.Append(new CallbackWithContext(callback, context));
          }
        }
        return (TRUE);
      }
    }


    //
    // Get host by address (return TRUE if host is valid or pending, and FALSE if not found)
    //
    Bool GetByAddress(const Socket::Address &address, Host *&host, Callback callback, void *context)
    {
      ASSERT(setup)
    
      // Clear host
      host = NULL;

      // Is this is failed host ?
      if (failedAddresses.Exists(address.GetIP()))
      {
        // If there's an callback, call it
        if (callback)
        {
          callback(NULL, context);
        }
        return (FALSE);
      }
      else
      {
        host = hostsByAddress.Find(address.GetIP());
        if (host)
        {
          // If there's an callback, call it
          if (callback)
          {
            callback(host, context);
          }
        }
        else
        {
          Lookup *lookup = lookupsByAddress.Find(address.GetIP());

          // Is there a pending lookup of this address ?
          if (!lookup)
          {
            lookup = new Lookup;
            HANDLE handle = WSAAsyncGetHostByAddr(hwnd, message, (const char *) &address, sizeof (Socket::Address), PF_INET, lookup->buf, MAXGETHOSTSTRUCT);

            if (!handle)
            {
              switch (WSAGetLastError())
              {
                case WSANOTINITIALISED:
                  LERR("WSAAsyncGetHostByAddr: A successful WSAStartup must occur before using this function.")
                  break;

                case WSAENETDOWN:
                  LERR("WSAAsyncGetHostByAddr: The network subsystem has failed.")
                  break;

                case WSAEINPROGRESS:
                  LERR("WSAAsyncGetHostByAddr: A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.")
                  break;

                case WSAEWOULDBLOCK:
                  LERR("WSAAsyncGetHostByAddr: The asynchronous operation cannot be scheduled at this time due to resource or other constraints within the Windows Sockets implementation.")
                  break;
              }
            }

            // Add to the lookups by handle tree
            lookups.Add(handle, lookup);

            // Add to the lookups by address tree
            lookupsByAddress.Add(address.GetIP(), lookup);
          }

          // Is there an callback to be called on completion ?
          if (callback)
          {
            lookup->callbacks.Append(new CallbackWithContext(callback, context));
          }
        }
        return (TRUE);
      }
    }


    //
    // Abort an asynchronouse name lookup
    //
    void AbortByNameCallback(Callback callback)
    {
      for (NBinTree<Lookup>::Iterator l(&lookupsByName); *l; ++l)
      {
        Lookup *lookup = *l;
        NList<CallbackWithContext>::Iterator c(&(lookup)->callbacks);
        while (CallbackWithContext *cwc = c++)
        {
          if (cwc->callback == callback)
          {
            lookup->callbacks.Dispose(cwc);
          }
        }
      }
    }
  }
}

