///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// lex.h
//
// 13-JUL-1998
//

#ifndef __LEX_H
#define __LEX_H


#pragma warning(push, 3)
#include <strstream>
#pragma warning(pop)
using std::istrstream;


//----------------------------------------------------------------------------

template <class T> class LexFile
{
private:
	struct TokenItem
	{
		char            *name;
		T               token;
	}
	*tokList;
	
	BOOL IsAMatch(char *str, TokenItem **inItem)
	{
		TokenItem *item;
		
		for (item = tokList; item->name; item++)
		{
			if (!stricmp(item->name, str))
			{
				*inItem = item;
				return TRUE;
			}
		}
		return FALSE;
	}

public:
	LexFile(void *list)
	{
		tokList = (TokenItem *) list;
	}
	
	char *TokenString(T tok)
	{
		TokenItem *item;
		
		for (item = tokList; item->name; list++)
		{
			if (item->token == tok)
			{
				return item->name;
			}
		}
		return NULL;
	}
	
	T GetToken(istrstream *fs);
	void GetString(istrstream *fs, char *string);
};
//----------------------------------------------------------------------------

template <class T> T LexFile<T>::GetToken(istrstream *fs)
{
	char ch, str[128];
	TokenItem *item = NULL;
	
	for (;;)
	{
		do
		{
			// soak up whitespace
			fs->get(ch);
			if (fs->eof())
			{
				// end of file
				return _END_OF_FILE;
			}
		}
		while (isspace (ch));
		
		switch (ch)
		{
		case '{':
			// check for open brace
			str[0] = ch;
			str[1] = '\0';
			if (IsAMatch(str, &item) == TRUE)
			{
				return item->token;
			}
			break;
			
		case '}':
			// check for close brace
			str[0] = ch;
			str[1] = '\0';
			if (IsAMatch(str, &item) == TRUE)
			{
				return item->token;
			}
			break;
			
		default:
			// check for token strings
			fs->putback(ch);
			*fs >> str;
			if (IsAMatch(str, &item) == TRUE)
			{
				return item->token;
			}
			break;
		}
	}
}
//----------------------------------------------------------------------------

template <class T> void LexFile<T>::GetString(istrstream *fs, char *str)
{
	int len;
	char ch;
	
	*str = '\0';
	do
	{
		// soak up whitespace
		fs->get(ch);
		if (fs->eof())
		{
			// end of file
			return;
		}
	}
	while (isspace(ch));
	
	if (ch != '"')
	{
		// undo read quote
		fs->putback(ch);
	}
	
	if (ch == '"')
	{
		// read a quoted string
		len = 0;
		
		do
		{
			fs->get(ch);
			
			if (fs->eof())
			{
				// end of file
				return;
			}
			
			if (ch == '}')
			{
				// close brace
				fs->putback(ch);
				return;
			}
			
			if (ch != '"' && !iscntrl(ch))
			{
				// copy the string
				str[ len++ ] = ch;
				str[ len   ] = '\0';
			}
			
		}
		while (ch != '"');
		
		return;
	}
	
	if (isalnum(ch) || ch == '-')
	{
		// read the string
		*fs >> str;
		len = strlen(str) - 1;
		
		if (str[len] == '}')
		{
			// undo read
			fs->putback(str[len]);

			// chop off the close brace
			str[len] = '\0';
		}
	}
}
//----------------------------------------------------------------------------

#endif // __LEX_H