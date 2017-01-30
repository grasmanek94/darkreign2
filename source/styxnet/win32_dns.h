////////////////////////////////////////////////////////////////////////////////
//
// DNS
//
// Copyright 1999-2000
// Matthew Versluys
//


#ifndef __WIN32_DNS_H
#define __WIN32_DNS_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_socket.h"
#include "win32_event.h"


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
    // Forward Declations
    //
    struct Lookup;
    class Host;


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Definitions
    //
    typedef void (*Callback)(const Host *host, void *context);

    
    ////////////////////////////////////////////////////////////////////////////////
    //
    // Class Host
    //
    class Host
    {
    public:

      // Tree nodes
      NBinTree<Host>::Node nodeName;
      NBinTree<Host>::Node nodeAddress;

    private:

      // The host name
      char *name;

      // Name aliases
      List<char> aliases;

      // Addresses
      List<Socket::Address> addresses;

    public:

      // Construct from a lookup structure
      Host(const Lookup &lookup);

      // Destructor
      ~Host();

    public:

      // Get the host name
      const char *GetName()
      {
        return (name);
      }

      // Get the list of aliases
      const List<char> & GetAliases()
      {
        return (aliases);
      }

      // Get the first address
      const Socket::Address * GetAddress() const
      {
        return (addresses.GetHead());
      }

      // Get the list of addresses
      const List<Socket::Address> & GetAddresses() const
      {
        return (addresses);
      }

    };


    // Initialize DNS system
    void Init();

    // Shutdown DNS system
    void Done();

    // Setup host system with a window handle
    void Setup(HWND hwnd, U32 msg);

    // Window proc which is hooked into the setup window handle
    void WindowProc(U32 id, UINT msg, WPARAM wParam, LPARAM lParam);

    // Get host by name (return TRUE if host is valid or pending, and FALSE if not found)
    Bool GetByName(const char *name, Host *&host, Callback callback = NULL, void *context = NULL);

    // Get host by address (return TRUE if host is valid or pending, and FALSE if not found)
    Bool GetByAddress(const Socket::Address &address, Host *&host, Callback callback = NULL, void *context = NULL);

    // Abort an asynchronouse name lookup
    void AbortByNameCallback(Callback callback);

  }

}

#endif