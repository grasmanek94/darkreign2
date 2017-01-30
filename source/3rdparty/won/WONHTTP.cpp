

#include "WONHTTP.h"

#if defined(macintosh) && (macintosh == 1)
#include <utime.h>
#include <sys/stat.h>
//#include <ctime>
#include <Files.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _LINUX
#include <utime.h>
#else
#include <sys/utime.h>
#endif
#endif

#include "Socket/TCPSocket.h"
#include "common/Event.h"


using namespace std;
using namespace WONAPI;
using namespace WONCommon;


// 8 K download chunks
#define HTTP_DOWNLOAD_BLOCK_SIZE		(1024 * 8)


static const char* monthTable[] = {	"Jan", "Feb", "Mar",
									"Apr", "May", "Jun",
									"Jul", "Aug", "Sep",
									"Oct", "Nov", "Dec" };


static const unsigned long Jan_Value = ('J' * 0x00010000) + ('a' * 0x00000100) + 'n';
static const unsigned long JAN_Value = ('J' * 0x00010000) + ('A' * 0x00000100) + 'N';
static const unsigned long Feb_Value = ('F' * 0x00010000) + ('e' * 0x00000100) + 'b';
static const unsigned long FEB_Value = ('F' * 0x00010000) + ('E' * 0x00000100) + 'B';
static const unsigned long Mar_Value = ('M' * 0x00010000) + ('a' * 0x00000100) + 'r';
static const unsigned long MAR_Value = ('M' * 0x00010000) + ('A' * 0x00000100) + 'R';
static const unsigned long Apr_Value = ('A' * 0x00010000) + ('p' * 0x00000100) + 'r';
static const unsigned long APR_Value = ('A' * 0x00010000) + ('P' * 0x00000100) + 'R';
static const unsigned long May_Value = ('M' * 0x00010000) + ('a' * 0x00000100) + 'y';
static const unsigned long MAY_Value = ('M' * 0x00010000) + ('A' * 0x00000100) + 'Y';
static const unsigned long Jun_Value = ('J' * 0x00010000) + ('u' * 0x00000100) + 'n';
static const unsigned long JUN_Value = ('J' * 0x00010000) + ('U' * 0x00000100) + 'N';
static const unsigned long Jul_Value = ('J' * 0x00010000) + ('u' * 0x00000100) + 'l';
static const unsigned long JUL_Value = ('J' * 0x00010000) + ('U' * 0x00000100) + 'L';
static const unsigned long Aug_Value = ('A' * 0x00010000) + ('u' * 0x00000100) + 'g';
static const unsigned long AUG_Value = ('A' * 0x00010000) + ('U' * 0x00000100) + 'G';
static const unsigned long Sep_Value = ('S' * 0x00010000) + ('e' * 0x00000100) + 'p';
static const unsigned long SEP_Value = ('S' * 0x00010000) + ('E' * 0x00000100) + 'P';
static const unsigned long Oct_Value = ('O' * 0x00010000) + ('c' * 0x00000100) + 't';
static const unsigned long OCT_Value = ('O' * 0x00010000) + ('C' * 0x00000100) + 'T';
static const unsigned long Nov_Value = ('N' * 0x00010000) + ('o' * 0x00000100) + 'v';
static const unsigned long NOV_Value = ('N' * 0x00010000) + ('O' * 0x00000100) + 'V';
static const unsigned long Dec_Value = ('D' * 0x00010000) + ('e' * 0x00000100) + 'c';
static const unsigned long DEC_Value = ('D' * 0x00010000) + ('E' * 0x00000100) + 'C';


static string MakeInternetDateTime(time_t theTime)
{
	struct tm aTime = *localtime(&theTime);
	
	string result;

	switch (aTime.tm_wday)
	{
	case 0:
		result = "Sun";
		break;
	case 1:
		result = "Mon";
		break;
	case 2:
		result = "Tue";
		break;
	case 3:
		result = "Wed";
		break;
	case 4:
		result = "Thu";
		break;
	case 5:
		result = "Fri";
		break;
//	case 6:
	default:
		result = "Sat";
		break;
	};

	result += ", ";
	result += char((aTime.tm_mday / 10) % 10) + 48;
	result += char(aTime.tm_mday % 10) + 48;
	result += " ";
	result += monthTable[aTime.tm_mon % 12];
	result += " ";
	unsigned short year = 1900 + aTime.tm_year;
	result += char((year / 1000) % 10) + 48;
	result += char((year / 100) % 10) + 48;
	result += char((year / 10) % 10) + 48;
	result += char(year % 10) + 48;
	result += " ";
	result += char((aTime.tm_hour / 10) % 10) + 48;
	result += char(aTime.tm_hour % 10) + 48;
	result += ":";
	result += char((aTime.tm_min / 10) % 10) + 48;
	result += char(aTime.tm_min % 10) + 48;
	result += ":";
	result += char((aTime.tm_sec / 10) % 10) + 48;
	result += char(aTime.tm_sec % 10) + 48;
	result += " GMT";	// claims GMT for now

	return result;
}


// Potential internet date/time formats:
//
//      Sun, 06 Nov 1994 08:49:37 GMT    ; RFC 822, updated by RFC 1123
//      Sunday, 06-Nov-94 08:49:37 GMT   ; RFC 850, obsoleted by RFC 1036
//      Sun Nov  6 08:49:37 1994         ; ANSI C's asctime() format

// returns 0 on failure
// ignores time zone for now
static time_t ParseInternetDateTime(const char* str, bool parseTime = true)
{
	const char* tmpStr = str;

	// skip leading spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	// skip day of week
	for (;*tmpStr != ' ' ; tmpStr++)
	{
		if (!*tmpStr)
			return 0;
	}

	// skip spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	bool gotDay = false;
	int day = 0;
	int month;

	// if the next char is numeric, then we're recving a day, else we're recving a month
	if (*tmpStr >= '0' && *tmpStr <= '9')
	{
		// read day of month
		for (;*tmpStr != ' ' && *tmpStr != '-'; tmpStr++)
		{
			if (!*tmpStr)
				return 0;

			day *= 10;
			day += *tmpStr - 48;
		}

		// skip spaces or dashes
		for (;*tmpStr == ' ' || *tmpStr == '-'; tmpStr++)
			;
		gotDay = true;
	}

	// read month

	//  Cheap trick here.  Since the month must be 3 chars, and 3 bytes fits in a long, we can make a switch statement
	unsigned long dateValue  = *(tmpStr++) * 0x00010000;
	dateValue += *(tmpStr++) * 0x00000100;
	dateValue += *(tmpStr++);
	switch (dateValue)
	{
	case Jan_Value:
	case JAN_Value:
		month = 0;
		break;
	case Feb_Value:
	case FEB_Value:
		month = 1;
		break;
	case Mar_Value:
	case MAR_Value:
		month = 2;
		break;
	case Apr_Value:
	case APR_Value:
		month = 3;
		break;
	case May_Value:
	case MAY_Value:
		month = 4;
		break;
	case Jun_Value:
	case JUN_Value:
		month = 5;
		break;
	case Jul_Value:
	case JUL_Value:
		month = 6;
		break;
	case Aug_Value:
	case AUG_Value:
		month = 7;
		break;
	case Sep_Value:
	case SEP_Value:
		month = 8;
		break;
	case Oct_Value:
	case OCT_Value:
		month = 9;
		break;
	case NOV_Value:
	case Nov_Value:
		month = 10;
		break;
	case Dec_Value:
	case DEC_Value:
		month = 11;
		break;
	default:
		return 0;
	}

	// skip spaces
	for (;*tmpStr == ' ' || *tmpStr == '-'; tmpStr++)
		;

	if (!gotDay)
	{
		// read day of month

		for (;*tmpStr != ' ' && *tmpStr != '-'; tmpStr++)
		{
			if (!*tmpStr)
				return 0;

			day *= 10;
			day += *tmpStr - 48;
		}

		// skip spaces or dashes
		for (;*tmpStr == ' ' || *tmpStr == '-'; tmpStr++)
			;
	}

	int year = 0;

	bool gotYear = false;
	if (*(tmpStr+2) != ':')	// Check to see if year preceeds time
	{
		// read year

		for (;*tmpStr != ' '; tmpStr++)
		{
			if (!*tmpStr)
			{
				if (year != 0)	// year might still be good
					break;
				return 0;
			}

			year *= 10;
			year += *tmpStr - 48;
		}
		gotYear = true;
	}

	int hour = 0;
	int mins = 0;
	int sec = 0;

	// skip spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	// parse 2 digit hour
	
	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;
	
	hour = (*tmpStr - 48) * 10;
	
	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;
	
	hour += *tmpStr - 48;

	// skip ':'
	tmpStr++;

	if (*tmpStr != ':')
		return 0;

	// parse 2 digit min
	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	mins = (*tmpStr - 48) * 10;

	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	mins += *tmpStr - 48;

	// skip ':'
	tmpStr++;

	if (*tmpStr != ':')
		return 0;

	// parse 2 digit seconds
	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	sec = (*tmpStr - 48) * 10;

	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	sec += *tmpStr - 48;

	tmpStr++;

	// skip spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	if (!gotYear)
	{
		// read year

		for (;*tmpStr != ' '; tmpStr++)
		{
			if (!*tmpStr)
			{
				if (year != 0)	// year might still be good
					break;
				return 0;
			}

			year *= 10;
			year += *tmpStr - 48;
		}
	}

	if (year < 100)
		year += 1900;

	struct tm timeStruct;

	timeStruct.tm_sec = sec;
	timeStruct.tm_min = mins;
	timeStruct.tm_hour = hour - 1;
	timeStruct.tm_mday = day;
	timeStruct.tm_mon = month;
	timeStruct.tm_year = year - 1900;

	// dunno
	timeStruct.tm_wday = 0;
	timeStruct.tm_yday = 0;
	timeStruct.tm_isdst = 0;

	return mktime(&timeStruct);
}



class HTTPGetData
{
public:
	Event doneEvent;
	bool doResume;
	std::string getPath;
	std::string saveAsFile;
	void* recvBuf;
	unsigned long* recvBufSize;
	unsigned long maxRecvBufSize;
	bool useFile;
	long timeout;
	bool autoDel;
	TCPSocket tcpSocket;
	Error error;
	std::string hostName;
	unsigned short httpPort;
	IPSocket::Address proxyAddr;
	unsigned long existingSize;
	unsigned long recvSize;
	unsigned long recvProgress;
	bool ignoreByte;
	time_t* modTime;
	time_t localModTime;
	time_t serverModTime;
	unsigned char redirectCount;
	ProgressCallback callback;
	bool* isNew;
	void* callbackPrivData;
	string recvedLine;
	bool recvedAny;
	FILE* file;
	CompletionContainer<Error> completion;
		
	HTTPGetData()
	{
		file = 0;
		ignoreByte = false;
		redirectCount = 0;
	}

	void Done(Error err)
	{
		error = err;

		if (!serverModTime)		// if the server didn't give us a modified time
			serverModTime = localModTime;

		if (file)
		{
			fclose(file);
			file = 0;			// close file early, so it's closed before Complete() is called

			time_t fileTime = serverModTime;
			
#if defined(macintosh) && (macintosh == 1)
			fileTime -= 126230400;	// # of seconds Mac time_t differs in stat and utime
#endif

			struct utimbuf times = { fileTime, fileTime };
			utime(saveAsFile.c_str(), &times );
			
#if defined(macintosh) && (macintosh == 1)
			// Mac hack to fix created time 
			OSErr err;
			ParamBlockRec pb;
			Str255 s;
			
			unsigned long nameLen = saveAsFile.size();
			if (nameLen < 255)
			{
				s[0] = nameLen;
				strncpy((char*)s+1, saveAsFile.c_str(), s[0]);
				pb.fileParam.ioNamePtr = s;
				pb.fileParam.ioVRefNum = 0;
				pb.fileParam.ioFVersNum = 0;
				pb.fileParam.ioFDirIndex = 0;
				err = PBGetFInfoSync(&pb);
				if (err == noErr)
				{
					unsigned long cr = pb.fileParam.ioFlCrDat;
					unsigned long md = pb.fileParam.ioFlMdDat;
					if (cr > md)
					{
						pb.fileParam.ioFlCrDat = md;
						err = PBSetFInfoSync(&pb);
					}
				}
			}
#endif

			
		}

		if (modTime)
			*modTime = serverModTime;

		completion.Complete(err);

		if (autoDel)
			delete this;
		else
			doneEvent.Set();
	}

	~HTTPGetData()
	{
		if (file)
			fclose(file);
	}
};


static void RecvFile(const Socket::TransmitResult& result, HTTPGetData* getData)
{
	bool userAbort = false;
	unsigned long bytesUsed = result.bytesUsed;
	void* buffer = result.buffer;

	if (bytesUsed)
	{
		if (getData->ignoreByte)
		{
			bytesUsed--;
			buffer = (void*)((char*)buffer + 1);
			getData->ignoreByte = false;
			getData->recvProgress++;
		}
	}

	if (bytesUsed)
	{
		if (getData->isNew)
			*(getData->isNew) = true;
		if (getData->useFile)
		{
			size_t writeSize = fwrite(buffer, 1, bytesUsed, getData->file);
			if (writeSize != bytesUsed)
			{
				getData->Done(Error_UnableToWriteToFile);
				return;
			}
		}
		else
		{
			unsigned long copySize = bytesUsed;
			unsigned long spaceLeft = 0;
			if (getData->maxRecvBufSize > *(getData->recvBufSize))
				spaceLeft = getData->maxRecvBufSize - *(getData->recvBufSize);
			if (copySize > spaceLeft)
				copySize = spaceLeft;
			if (copySize)
				memcpy((char*)(getData->recvBuf) + *(getData->recvBufSize), buffer, copySize);
			*(getData->recvBufSize) += bytesUsed;
		}
		getData->recvProgress += bytesUsed;

		if (getData->callback && !getData->callback(getData->existingSize + getData->recvProgress, getData->existingSize + getData->recvSize, getData->callbackPrivData))
			userAbort = true;
	}

	if (getData->recvProgress == getData->recvSize)	// successfully finished
		getData->Done(Error_Success);
	else if (userAbort)
		getData->Done(Error_Aborted);
	else if (result.closed)			// connection lost
		getData->Done(Error_ConnectionClosed);
	else if (result.bytesUsed != result.bytesRequested)	// Completed before we got everything
		getData->Done(Error_Timeout);
	else
	{
		unsigned long blockSize = HTTP_DOWNLOAD_BLOCK_SIZE;
		unsigned long sizeLeft = getData->recvSize - getData->recvProgress;
		if (blockSize > sizeLeft)
			blockSize = sizeLeft;

		getData->tcpSocket.RecvEx(blockSize, 0, getData->timeout, true, RecvFile, getData);
	}
}

void DoneHTTPOpen(const WSSocket::OpenResult& result, HTTPGetData* getData);
static void DoneHTTPRecv(const Socket::TransmitResult& result, HTTPGetData* getData)
{
	if (result.closed)
	{
		getData->Done(Error_ConnectionClosed);
		return;
	}
	if (result.bytesUsed != result.bytesRequested)
	{
		getData->Done(Error_Timeout);
		return;
	}

	char thisChar = *(const char*)(result.buffer);
	if (thisChar == '\n')
	{
		if (getData->recvedLine.empty())	// must be second CR in a row
		{
			// no more header
			if (!getData->recvedAny)
			{
				getData->Done(Error_InvalidMessage);
				return;
			}

			if	(		!getData->callback
					||	(getData->ignoreByte && getData->recvSize == 1)
					||	 getData->callback(getData->existingSize, getData->existingSize + getData->recvSize, getData->callbackPrivData)
				)
			{
				if (getData->useFile)
				{
					if (getData->doResume)
					{
						getData->file = fopen(getData->saveAsFile.c_str(),"a+b");
						
						// might want to check if the file has changed out from under us
						// after we sent the request.  We might be stomping some new file.  ?
					}
					else
						getData->file = fopen(getData->saveAsFile.c_str(), "wb");
					
					if (!getData->file)
					{
						getData->Done(Error_UnableToOpenFile);
						return;
					}
				}
				// issue recv chain
				unsigned long blockSize = HTTP_DOWNLOAD_BLOCK_SIZE;
				if (blockSize > getData->recvSize)
					blockSize = getData->recvSize;
				getData->recvProgress = 0;

				getData->tcpSocket.RecvEx(blockSize, 0, getData->timeout, true, RecvFile, getData);
			}
			else
				getData->Done(Error_Aborted);
			return;
		}
		else
		{
			const char* tmpStr = getData->recvedLine.c_str();

			// parse recvedLine
			if (!getData->recvedAny)
			{
				getData->recvedAny = true;

				struct {
					unsigned long whole;
					unsigned long sub;
				} httpVersion = { 0, 0 };
				unsigned long httpResult = 0;
				const char* resultString;

				// "HTTP/" 1*DIGIT "." 1*DIGIT SP 3DIGIT SP

				if (strnicmp(tmpStr, "HTTP/", 5) != 0)
				{
					getData->Done(Error_InvalidMessage);
					return;
				}

				// skip HTTP/, and scan numbers until '.' in version #
				for (tmpStr += 5; *tmpStr != '.'; tmpStr++)	
				{
					if (*tmpStr >= '0' && *tmpStr <= '9')
					{
						httpVersion.whole *= 10;
						httpVersion.whole += *tmpStr - 48;	// convert ascii into a value
					}
					else
					{
						getData->Done(Error_InvalidMessage);
						return;
					}
				}

				// skip past '.', read numbers until the next space
				for (tmpStr++; *tmpStr != ' '; tmpStr++)
				{
					if (*tmpStr >= '0' && *tmpStr <= '9')
					{
						httpVersion.sub *= 10;
						httpVersion.sub += *tmpStr - 48;	// convert ascii into a value
					}
					else
					{
						getData->Done(Error_InvalidMessage);
						return;
					}
				}

				// skip past spaces
				for (;*tmpStr == ' '; tmpStr++)
					;

				// read HTTP result into a value, terminate on space or EOL
				for (; *tmpStr != ' ' && *tmpStr != 0; tmpStr++)
				{
					if (*tmpStr >= '0' && *tmpStr <= '9')
					{
						httpResult *= 10;
						httpResult += *tmpStr - 48;	// convert ascii into a value
					}
					else
					{
						getData->Done(Error_InvalidMessage);
						return;
					}
				}

				// skip past spaces
				for (;*tmpStr == ' '; tmpStr++)
					;

				resultString = tmpStr;	// just in case we need to report this to something
				
				switch (httpResult)
				{
				case 304: // Not Modified
					getData->Done(Error_Success);
					return;
				case 200: // complete GET
					getData->ignoreByte = false;
					getData->doResume = false;
					getData->existingSize = 0;
					break;
				case 206: // partial GET
					if (getData->doResume)
						break;
				case 302: // redirect
					break;
				default:
					getData->Done(Error_InvalidMessage);
					return;
				}
			}
			else
			{
				// header fields

				// Check for Content-Length
				if (strnicmp(tmpStr, "Content-Length:", 15) == 0)
				{	// get file size out of var
					getData->recvSize = 0;

					tmpStr += 15;

					// skip past spaces
					for (;*tmpStr == ' '; tmpStr++)
						;

					// read file size, terminate on space or EOL
					for (; *tmpStr != ' ' && *tmpStr != 0; tmpStr++)
					{
						if (*tmpStr >= '0' && *tmpStr <= '9')
						{
							getData->recvSize *= 10;
							getData->recvSize += *tmpStr - 48;	// convert ascii into a value
						}
						else
						{
							getData->Done(Error_InvalidMessage);
							return;
						}
					}
				}
				else if (strnicmp(tmpStr, "Last-Modified:", 14) == 0)
				{	// get date outta var
					tmpStr += 14;

					getData->serverModTime = ParseInternetDateTime(tmpStr);
					if (!getData->serverModTime)
					{
						getData->Done(Error_InvalidMessage);
						return;
					}
				}
				else if (strnicmp(tmpStr, "Location:", 9) == 0)
				{
					// abort if we seem to be in a loop
					if (++getData->redirectCount > 10)
					{
						getData->Done(Error_Failure);
						return;
					}

					tmpStr += 9;
					while (isspace(*tmpStr))
						++tmpStr;
					if (strnicmp(tmpStr, "http://", 7) == 0)
						tmpStr += 7;
					string aRedirectLocation = tmpStr;
					int aFirstSlash = aRedirectLocation.find_first_of("/");
					getData->hostName = aRedirectLocation.substr(0, aFirstSlash);
					getData->getPath = (aFirstSlash != aRedirectLocation.npos) ? aRedirectLocation.substr(aFirstSlash) : "/";
					getData->recvedLine.erase();
					getData->tcpSocket.Close();
					getData->tcpSocket.OpenEx(getData->proxyAddr, getData->timeout, true, DoneHTTPOpen, getData);
					return;
				}
			}
			
			getData->recvedLine.erase();
		}
	}
	else if (thisChar != '\r')
		getData->recvedLine += thisChar;
	
	getData->tcpSocket.RecvEx(1, 0, getData->timeout, true, DoneHTTPRecv, getData);
}


// Well, crap...  An If-Range GET only works in 2 of 3 situations.  If we ask for anything
// beyond the size of the file, and the file was a fragment, then it will resume properly.  If
// the file has been replaced on the server, we'll download the whole new file properly.  But,
// if the file we have is up to date, and complete, If-Range will try to send us the whole damn
// file again.  Because we don't know if the download is a fragment or not (I don't want to have
// to keep track of that), we're going to have to request at least one byte of the file.
// If all we get is one byte back, then we won't bother the user about the progress, and tell them
// the file didn't need to be downloaded.  A hack.  I blame HTTP.  Damn text protocol.  Why do
// some people think computers need to use English to talk to eachother?
//
// The most significant drawback to this is; the server has to read the file, in order to retrieve
// that last byte.  This puts a small load on the server that shouldn't be necessary.
//
static void DoneHTTPOpen(const WSSocket::OpenResult& result, HTTPGetData* getData)
{
	if (result.error != Error_Success)
		getData->Done(result.error);
	else
	{
		char tmpPortStr[50];
		sprintf(tmpPortStr, "%ld", (unsigned long)(getData->httpPort));

			
		string request("GET ");
//		request += "http://";
//		request += getData->hostName;
//		if (getData->httpPort != 80)
//			request += tmpPortStr;
//		if (getData->getPath[0] != '/')
//			request += "/";
		request += getData->getPath;
		request += " HTTP/1.1\r\nHost: ";
		request += getData->hostName;
		if (getData->httpPort != 80)
		{
			request += ":";
			request += tmpPortStr;
		}
		request += "\r\n";
		
		if (getData->doResume)
		{
			request += "If-Range: ";
			request += MakeInternetDateTime(getData->localModTime);
			request += "\r\n";

			char tmpSizeStr[50];
			
			if (getData->existingSize)
			{
				getData->existingSize--;
				getData->ignoreByte = true;
			}
			sprintf(tmpSizeStr, "%ld", getData->existingSize);

			request += "Range: bytes=";
			request += tmpSizeStr;
			request += "-\r\n";
		}
		else if (getData->localModTime)
		{
			request += "If-Modified-Since: ";
			request += MakeInternetDateTime(getData->localModTime);
			request += "\r\n";
		}
		request += "\r\n";
		getData->recvedAny = false;

		getData->tcpSocket.Send(request.length(), request.data(), -1, true, true);
		getData->tcpSocket.RecvEx(1, 0, getData->timeout, true, DoneHTTPRecv, getData);
	}
}


Error HTTPGetLL(const IPSocket::Address& proxyAddr,
				const std::string& hostName, unsigned short httpPort,
				const std::string& getPath,
				const std::string& saveAsFile, bool allowResume, void* recvBuf, unsigned long* recvBufSize, bool useFile,
				bool* isNew, time_t* modTime, ProgressCallback callback, void* callbackPrivData,
				long timeout, bool async, const CompletionContainer<Error>& completion)
{
	if (getPath.empty() || 
		(useFile	?	(saveAsFile.empty())
					:	(!recvBuf || !recvBufSize || !*recvBufSize)))
	{
		completion.Complete(Error_InvalidParams);
		return Error_InvalidParams;
	}
	
	struct stat fileInfo;
	unsigned long existingSize = 0;
	bool doResume = false;
	time_t localModTime = 0;
	if (modTime)
		localModTime = *modTime;

	if (useFile && allowResume)
	{
		if (!stat(saveAsFile.c_str(), &fileInfo))
		{
		
#if defined(macintosh) && (macintosh == 1)
			fileInfo.st_mtime += 126230400;	// # of seconds Mac time_t differs in stat and utime
#endif

			if (!localModTime)
				localModTime = fileInfo.st_mtime;
			existingSize = fileInfo.st_size;
			if (existingSize)
				doResume = true;
		}
	}

	HTTPGetData* getData = new HTTPGetData;
	if (!getData)
	{
		completion.Complete(Error_OutOfMemory);
		return Error_OutOfMemory;
	}
	getData->existingSize = existingSize;
	getData->callback = callback;
	getData->callbackPrivData = callbackPrivData;
	getData->doResume = doResume;
	getData->modTime = modTime;
	getData->localModTime = localModTime;
	getData->serverModTime = 0;
	getData->getPath = getPath;
	getData->saveAsFile = saveAsFile;
	getData->recvBuf = recvBuf;
	getData->recvBufSize = recvBufSize;
	if (recvBufSize)
	{
		getData->maxRecvBufSize = *recvBufSize;
		*recvBufSize = 0;
	}
	else
		getData->maxRecvBufSize = 0;
	getData->useFile = useFile;
	getData->timeout = timeout;
	getData->autoDel = async;
	getData->proxyAddr = proxyAddr;
	getData->hostName = hostName;
	getData->httpPort = httpPort;
	getData->recvSize = 0;
	getData->isNew = isNew;
	if (isNew)
		*isNew = false;
	getData->completion = completion;
	
	getData->tcpSocket.OpenEx(proxyAddr, timeout, true, DoneHTTPOpen, getData);

	if (async)
		return Error_Pending;

	WSSocket::PumpUntil(getData->doneEvent, timeout);
	Error result = getData->error;
	delete getData;
	return result;
}


Error WONAPI::HTTPGet(const IPSocket::Address& proxyAddr,
					  const std::string& hostName, unsigned short httpPort,
					  const std::string& getPath, const std::string& saveAsFile,
					  bool* isNew, time_t* modTime, bool allowResume, ProgressCallback callback,
					  void* callbackPrivData, long timeout, bool async,
					  const CompletionContainer<Error>& completion)
{
	return HTTPGetLL(proxyAddr, hostName, httpPort, getPath, saveAsFile, allowResume, 0, 0, true,
		isNew, modTime, callback, callbackPrivData, timeout, async, completion);
}


Error WONAPI::HTTPGet(const IPSocket::Address& proxyAddr,
					  const std::string& hostName, unsigned short httpPort,
					  const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
					  bool* isNew, time_t* modTime, ProgressCallback callback, 
					  void* callbackPrivData, long timeout, bool async,
					  const CompletionContainer<Error>& completion)
{
	return HTTPGetLL(proxyAddr, hostName, httpPort, getPath, "", false, recvBuf, recvBufSize, false,
					 isNew, modTime, callback, callbackPrivData, timeout, async, completion);
}


#include "wondll.h"


class C_CallbackData
{
public:
	WONProgressCallback c_callback;
	void* c_privs;
};

bool progressCallbackTranslator(unsigned long progress, unsigned long size, void* callbackPrivData)
{
	C_CallbackData* callbackData = (C_CallbackData*)callbackPrivData;
	return callbackData->c_callback(progress, size, callbackData->c_privs) ? TRUE : FALSE;
}


WONError WONHTTPGetFile(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName,
						unsigned short httpPort, WON_CONST char* getPath, WON_CONST char* saveAsFile, 
						BOOL* isNew, time_t* modTime, BOOL allowResume, WONProgressCallback callback,
						void* callbackPrivData, long timeout)
{
	if (!proxyServer)
		return Error_InvalidParams;

	IPSocket::Address addr(*proxyServer);

	C_CallbackData* callbackData = 0;
	ProgressCallback tmp_callback = 0;

	if (callback)
	{
		tmp_callback = progressCallbackTranslator;
		callbackData = new C_CallbackData;
		callbackData->c_callback = callback;
		callbackData->c_privs = callbackPrivData;
	}

	bool _isNew;
	
	Error result = HTTPGet(addr, hostName, httpPort, getPath, saveAsFile, &_isNew, modTime, allowResume ? TRUE : FALSE,
		tmp_callback, callbackData, timeout);

	if (isNew)
		*isNew = _isNew ? TRUE : FALSE;

	delete callbackData;

	return result;
}


WONError WONHTTPGetData(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName,
						unsigned short httpPort, WON_CONST char* getPath, void* recvBuf,
						unsigned long* recvBufSize, BOOL* isNew, time_t* modTime,
						WONProgressCallback callback,
						void* callbackPrivData, long timeout)
{
	if (!proxyServer)
		return Error_InvalidParams;

	IPSocket::Address addr(*proxyServer);

	C_CallbackData* callbackData = 0;
	ProgressCallback tmp_callback = 0;

	if (callback)
	{
		tmp_callback = progressCallbackTranslator;
		callbackData = new C_CallbackData;
		callbackData->c_callback = callback;
		callbackData->c_privs = callbackPrivData;
	}

	bool _isNew;
	
	Error result = HTTPGet(addr, hostName, httpPort, getPath, recvBuf, recvBufSize, &_isNew, modTime,
		tmp_callback, callbackData, timeout);

	if (isNew)
		*isNew = _isNew ? TRUE : FALSE;
		
	delete callbackData;

	return result;
}


class C_HTTPGetData
{
public:
	BOOL* isNew;
	HWONCOMPLETION hCompletion;
	C_CallbackData* callbackData;
	bool _isNew;

	void Done(Error err)
	{
		if (isNew)
			*isNew = _isNew ? TRUE : FALSE;
		WONComplete(hCompletion, (void*)err);
		
		delete this;
	}

	~C_HTTPGetData()
	{
		delete callbackData;
	}
};


void DoneHTTPGet(Error err, C_HTTPGetData* httpGetData)
{
	httpGetData->Done(err);
	
}


void WONHTTPGetFileAsync(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName,
						 unsigned short httpPort, WON_CONST char* getPath, WON_CONST char* saveAsFile,
						 BOOL* isNew, time_t* modTime, BOOL allowResume,
						 WONProgressCallback callback,
						 void* callbackPrivData, long timeout, HWONCOMPLETION hCompletion)
{
	if (!proxyServer)
	{
		WONComplete(hCompletion, (void*)Error_InvalidParams);
		return;
	}
	IPSocket::Address addr(*proxyServer);
	
	C_HTTPGetData* httpGetData = new C_HTTPGetData;
	if (!httpGetData)
	{
		WONComplete(hCompletion, (void*)Error_OutOfMemory);
		return;
	}
	httpGetData->isNew = isNew;
	httpGetData->hCompletion = hCompletion;
	httpGetData->callbackData = 0;

	ProgressCallback tmp_callback = 0;

	if (callback)
	{
		tmp_callback = progressCallbackTranslator;
		httpGetData->callbackData = new C_CallbackData;
		httpGetData->callbackData->c_callback = callback;
		httpGetData->callbackData->c_privs = callbackPrivData;
	}

	HTTPGet(addr, hostName, httpPort, getPath, saveAsFile, &(httpGetData->_isNew), modTime, allowResume ? TRUE : FALSE,
		tmp_callback, httpGetData->callbackData, timeout, true, DoneHTTPGet, httpGetData);
}


void WONHTTPGetDataAsync(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName,
						 unsigned short httpPort, WON_CONST char* getPath, void* recvBuf,
						 unsigned long* recvBufSize, BOOL* isNew, time_t* modTime,
						 WONProgressCallback callback,
						 void* callbackPrivData, long timeout, HWONCOMPLETION hCompletion)
{
	if (!proxyServer)
	{
		WONComplete(hCompletion, (void*)Error_InvalidParams);
		return;
	}
	IPSocket::Address addr(*proxyServer);

	C_HTTPGetData* httpGetData = new C_HTTPGetData;
	if (!httpGetData)
	{
		WONComplete(hCompletion, (void*)Error_OutOfMemory);
		return;
	}
	httpGetData->isNew = isNew;
	httpGetData->hCompletion = hCompletion;
	httpGetData->callbackData = 0;

	ProgressCallback tmp_callback = 0;

	if (callback)
	{
		tmp_callback = progressCallbackTranslator;
		httpGetData->callbackData = new C_CallbackData;
		httpGetData->callbackData->c_callback = callback;
		httpGetData->callbackData->c_privs = callbackPrivData;
	}

	HTTPGet(addr, hostName, httpPort, getPath, recvBuf, recvBufSize, &(httpGetData->_isNew), modTime,
		tmp_callback, httpGetData->callbackData, timeout, true, DoneHTTPGet, httpGetData);
}

