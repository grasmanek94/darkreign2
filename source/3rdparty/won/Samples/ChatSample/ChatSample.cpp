#pragma warning (disable:4786)				// STL bug in long names
#pragma warning (disable:4275)				// An exported derived class did not 
											// have an exported base class

#include "stdafx.h"

#include "ChatSample.h"
#include "Console.h"
#include "User.h"
#include "ChatSampleApp.h"


// Utility to Isolate a word from theInputBuf
int GetCommandToken(string&,char*,int,BOOL = FALSE);	

// A pointer to our application object
//		Note: The application object cannot be static because it 
//		contains objects from the TitanAPI
CChatSampleApp* theAppP; 

/********************************************************************************
 * Entry Point																	*
 ********************************************************************************/
int main(int argc, char *argv[])
{
	// Our application object, non-static
	CChatSampleApp theApp;
	theAppP = &theApp;

	// Initialize the application 
	if (theApp.Initialize()==FALSE)
		return -1;

	// Run the app
	return theApp.Run();

}//end main();

 
 
 
 
/********************************************************************************
 * Isolates a single token (word) from a char*, stores result in a string		*
 ********************************************************************************/
int GetCommandToken(string& theDestString, char* theSourceBuffer, 
					int theTokenNum, BOOL copyToEnd)
{
	int aTokenStartPos		= 0;	// the position of the first letter in the token
	int aTokenEndPos		=-2;	// the position of the last letter in the token
	int aTokenCounter		=-1;	// number of last token found (Base 0)

	// Create a string copy of the source buffer
	string theSourceString;
	theSourceString.assign(theSourceBuffer);

	do
	{
		// Extract a single token, skip over double delimiters (exa: "/login one  two")
		do
		{
			// A token starts at *2* spaces after the previous end (to skip the space)
			aTokenStartPos = aTokenEndPos+2;

			// A token ends at the character *before* the separating space
			aTokenEndPos = theSourceString.find_first_of(' ',aTokenStartPos)-1;
		}
		// The start and end positions are the same when it's a double space "1  2"
		while(aTokenEndPos+1 == aTokenStartPos);

		// one token extracted
		aTokenCounter++;
	}
	// Loop until we've either extract the token specified or we're out of input	
	while(aTokenCounter<theTokenNum && aTokenEndPos != -2);

	// Did we run out of input?
	if(aTokenCounter != theTokenNum)
		return -1; 

	// Should we copy the rest of the string?
	if(aTokenEndPos==-2 || copyToEnd==TRUE)
		theDestString = theSourceString.substr(aTokenStartPos);
	else 
		// or just the single token?		 (start position,-------length to copy--------)
		theDestString = theSourceString.substr(aTokenStartPos,aTokenEndPos-aTokenStartPos+1);

	// Send back the number of bytes extracted 
	return theDestString.size();
}