// User.h: interface for the CUser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USER_H__0E54A3E8_1F5B_11D4_803B_0050046D274E__INCLUDED_)
#define AFX_USER_H__0E54A3E8_1F5B_11D4_803B_0050046D274E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUser;

// Our command map (strings to function pointers)
typedef std::map<string, int(CUser::*)(void)>	TCommandMap;

class CUser  
{
// Attributes
public:
	Identity m_Identity;

	BOOL m_IsConnectedToChat;
	BOOL m_IsLoggedIn;

	TCommandMap m_CmdMap;
	WORD theUserStatus;						// What is our user doing?

	string m_UserName;
	string m_Password;

// Operations
public:
	int DoBlankCmd();						// Displays "Command not implemented..."
	int DoChatCmd();						// Send a message (no command trigger)
	int DoExitCmd();						// Exits and logs command parameters
	int DoHelpCmd();						// Lists available commands
	int DoLoginCmd();						// Logs in. <username> <password>
	int DoRoomsCmd();						// List the chat rooms
	int DoJoinCmd();						// Join a chat room
	int DoCreateCmd();						// Create a chat room
	int DoUsersCmd();						// Display room occupants
	int DoWhisperCmd();						// Send chat to a single user
	int DoEmoteCmd();						// Perform an action message

	void InitCommands();
	BOOL Init();
	BOOL ProcessCommand(string&);

public:
	CUser();
	virtual ~CUser();

};

#endif // !defined(AFX_USER_H__0E54A3E8_1F5B_11D4_803B_0050046D274E__INCLUDED_)
