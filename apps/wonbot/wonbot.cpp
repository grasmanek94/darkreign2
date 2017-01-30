////////////////////////////////////////////////////////////////////////////////
//
// WonBot
//
// Copyright 2000
// Pandemic Studios
//


////////////////////////////////////////////////////////////////////////////////
//
// Include
//
#include "std.h"
#include "woniface.h"

#include "fscope.h"
#include "ptree.h"
#include "stdload.h"


////////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")


////////////////////////////////////////////////////////////////////////////////
//
// Namespace WonBot
//
namespace WonBot
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //
  static const char * configFile = ".\\wonbot.cfg";


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //
  #define CAST(type, var, value) type var = reinterpret_cast<type>(value);

  static StrBuf<64> wonUserName;
  static StrBuf<64> wonPassword;

  static const U32 maxPlayers = 1000000;

  static const CH * dow[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
  static const CH * moy[] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
  static const CH * hod[] = { L"12am", L"1am", L"2am", L"3am", L"4am", L"5am", L"6am", L"7am", L"8am", L"9am", L"10am", L"11am",
                              L"12pm", L"1pm", L"2pm", L"3pm", L"4pm", L"5pm", L"6pm", L"7pm", L"8pm", L"9pm", L"10pm", L"11pm" };

  const CH * niceTime(U32 secs)
  {
    static StrBuf<64, CH> buf;
    Utils::Sprintf(buf.str, buf.GetSize(), L"%d days %02d:%02d.%02d", secs / 86400, secs / 3600 % 24, secs / 60 % 60, secs % 60);
    return (buf.str);
  }

  void Tell(const char *user, const CH *message)
  {
    if (user)
    {
      WonIface::PrivateMessage(user, message);
    }
    else
    {
      WonIface::EmoteMessage(message);
    }
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Info
  //
  struct Info
  {

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct Stat
    //
    struct Stat
    {
      U32 samples;
      U32 total;
      U32 sample;
      U32 lastSample;
      U32 maximum;

      void Reset()
      {
        samples = 0;
        total = 0;
        sample = 0;
        lastSample = 0;
        maximum = 0;
      }

      // Add a sample to the statistics
      void Sample(U32 amount)
      {
        // Add this amount in
        total += amount;
        sample += amount;
      }

      // Check for maximum's being exceeded
      void CheckMax(const CH *title)
      {
        if (title)
        {
          if (sample > maximum)
          {
            StrBuf<64, CH> buf;
            Utils::Sprintf(buf.str, buf.GetSize(), L"New Maximum %s! Old Max %d New Max %d", title, maximum, sample);
            Tell(NULL, buf.str);
          }
          else if (sample == maximum)
          {
            StrBuf<64, CH> buf;
            Utils::Sprintf(buf.str, buf.GetSize(), L"Equal Maximum %s - %d", title, maximum);
            Tell(NULL, buf.str);
          }
        }
      }

      // Roll over the statistic
      void Roll()
      {
        // Increment the number of samples taken
        samples++;

        // Adjust the maximum if neccesary
        maximum = Max(maximum, sample);

        lastSample = sample;
        sample = 0;
      }

      // Get the average sample
      F32 GetAvg() const
      {
        if (samples)
        {
          return (total / ((F32) samples));
        }
        else
        {
          return (0);
        }
      }

      // Report
      void Report(const char *user, const CH *title)
      {
        StrBuf<1024, CH> buffer;

        Tell(user, title);

        Utils::Sprintf(buffer.str, buffer.GetSize(), L"Current: %d, Last: %d, Total: %d", sample, lastSample, total);
        Tell(user, buffer.str);

        Utils::Sprintf(buffer.str, buffer.GetSize(), L"Maximum: %d, Average %.1f, Samples: %d", maximum, GetAvg(), samples);
        Tell(user, buffer.str);
      }

    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct StatGroup
    //
    struct StatGroup
    {
      // Number of stats ever
      U32 ever;

      // The time when we started collecting
      long timeEverMark;


      // Last hour a sample was taken
      long timeHour;

      // The time when the hour was last adjusted
      long timeHourMark;

      // Stats for each hour of the day
      Stat hours[24];

      // Stats for the hour
      Stat hour;


      // Last day a sample was taken
      long timeDay;

      // The time when the day was last adjusted
      long timeDayMark;

      // Stats for each day of the week
      Stat days[7];

      // Stats for the day
      Stat day;


      // Last week a sample was taken
      long timeWeek;

      // The time when the week was last adjusted
      long timeWeekMark;

      // Stats for the week
      Stat week;


      // Last month a sample was taken
      long timeMonth;

      // The time when the month was last adjusted
      long timeMonthMark;

      // Stats for each month of the year
      Stat months[12];  

      // Stats for the month
      Stat month;


      // Last year a sample was taken
      long timeYear;

      // The time when the year was last adjusted
      long timeYearMark;

      // Stats for the year
      Stat year;


      // Reset
      void Reset()
      {
        // Get the current time
        long t = time(NULL);
        tm *lt = localtime(&t);

        int i;

        timeEverMark = t;

        timeHour = lt->tm_hour;
        timeHourMark = t;
        hour.Reset();
        for (i = 0; i < 24; i++)
        {
          hours[i].Reset();
        }

        timeDay = lt->tm_wday;
        timeDayMark = t;
        day.Reset();
        for (i = 0; i < 7; i++)
        {
          days[i].Reset();
        }

        timeWeek = t / 604800;
        timeWeekMark = t;
        week.Reset();

        timeMonth = lt->tm_mon;
        timeMonthMark = t;
        month.Reset();
        for (i = 0; i < 12; i++)
        {
          months[i].Reset();
        }

        timeYear = lt->tm_year;
        timeYearMark = t;
        year.Reset();
      }

      // CheckArray
      void CheckArray(Stat *array, long &currentIndex, long newIndex, long time, long &mark, Stat &current, const CH *title, const CH *stat, const CH **expand)
      {
        // Has the index changed ?
        if (currentIndex != newIndex)
        {
          // Roll the current sample
          if (title)
          {
            StrBuf<64, CH> buf;
            Utils::Sprintf(buf.str, buf.GetSize(), L"%s for %s", title, expand[currentIndex]);
            array[currentIndex].CheckMax(buf.str);
          }
          array[currentIndex].Roll();

          if (stat && title)
          {
            StrBuf<64, CH> buf;
            Utils::Sprintf(buf.str, buf.GetSize(), L"%s for any %s", title, stat);
            current.CheckMax(buf.str);
          }
          current.Roll();

          // Update the index
          currentIndex = newIndex;

          // Update the mark
          mark = time;
        }
      }

      // Check
      void Check(const CH *title, long t, tm *lt)
      {
        // Has a year gone by ?
        if (lt->tm_year != timeYear)
        {
          year.CheckMax(title);
          year.Roll();

          timeYear = lt->tm_year;
          timeYearMark = t;
        }

        // Has a week gone by ?
        long w = t / 604800;
        if (w != timeWeek)
        {
          week.CheckMax(title);
          week.Roll();

          timeWeek = w;
          timeWeekMark = t;
        }

        // Check the months, days and ours
        CheckArray(months, timeMonth, lt->tm_mon, t, timeMonthMark, month, title, L"Month", moy);
        CheckArray(days, timeDay, lt->tm_wday, t, timeDayMark, day, title, L"Day", dow);
        CheckArray(hours, timeHour, lt->tm_hour, t, timeHourMark, hour, title, L"Hour", hod);
      }

      // Take a sample
      void Sample(U32 sample = 1, long lastTime = 0)
      {
        long t = time(NULL);
        tm *lt = localtime(&t);

        // Check to see if we should roll over 
        // any of the stats due to time changes
        Check(NULL, t, lt);

        if (timeEverMark > lastTime)
        {
          // Add into ever
          ever += sample;
        }

        if (timeYearMark > lastTime)
        {
          // Sample the statistics for this year
          year.Sample(sample);
        }

        // Update the month
        if (timeMonthMark > lastTime)
        {
          // Sample the statistics for this month
          month.Sample(sample);
          months[lt->tm_mon].Sample(sample);
        }

        // Update the week
        if (timeWeekMark > lastTime)
        {
          // Sample the statistics for this week
          week.Sample(sample);
        }

        // Update the day
        if (timeDayMark > lastTime)
        {
          // Sample the statistics for today
          day.Sample(sample);
          days[lt->tm_wday].Sample(sample);
        }

        // Update the hour
        if (timeHourMark > lastTime)
        {
          // Sample this statistics for this hour
          hour.Sample(sample);
          hours[lt->tm_hour].Sample(sample);
        }
      }

      void Report(const char *user, const CH *title, int argc, const char **argv)
      {
        long t = time(NULL);
        tm *lt = localtime(&t);

        // Make sure the time is up to date
        Check(title, t, lt);

        StrBuf<1024, CH> buffer;

        // If there are no arguments give a simple report
        if (argc == 1)
        {
          Utils::Sprintf
          (
            buffer.str, 
            buffer.GetSize(), 
            L"%s this hour %d, last hour %d, today %d, yesterday %d, ever %d",
            title,
            hour.sample,
            hour.lastSample,
            day.sample,
            day.lastSample,
            ever
          );

          Tell(user, buffer.str);
        }
        else
        {
          switch (Crc::CalcStr(argv[1]))
          {
            case 0xB3A4F922: // "hour"
              if (argc == 2)
              {
                Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s by hour; last(max)[ever]", title);
                Tell(user, buffer.str);

                Utils::Sprintf(buffer.str, buffer.GetSize(), L"Any Hour %d(%d)[%d]",
                  hour.sample, hour.maximum, hour.total);
                Tell(user, buffer.str);

                int i = 0;
                for (int l = 0; l < 4; l++)
                {
                  buffer = L"";
                  for (int r = 0; r < 6; r++)
                  {
                    StrBuf<32, CH> buf;
                    Utils::Sprintf(buf.str, buf.GetSize(), L"%s %d(%d)[%d]",
                      hod[i], hours[i].lastSample, hours[i].maximum, hours[i].total);

                    Utils::Strcat(buffer.str, buf.str);
                    if (r < 5)
                    {
                      Utils::Strcat(buffer.str, L"; ");
                    }
                    i++;
                  }
                  Tell(user, buffer.str);
                }
              }
              else
              {
                if (Crc::CalcStr(argv[2]) == 0xC87BD42E) // "Any"
                {
                  Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s for any hour", title);
                  hour.Report(user, buffer.str);
                }
                else
                {
                  // The next arg is the hour we want to display
                  U32 hour = Utils::AtoI(argv[2]);

                  if (hour > 23)
                  {
                    Tell(user, L"Hour must be in the range 0 to 23.");
                  }
                  else
                  {
                    Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s for %s", title, hod[hour]);

                    // Generate a report for that hour
                    hours[hour].Report(user, buffer.str);
                  }
                }
              }
              break;

            case 0x7346F79D: // "day"
            {
              if (argc == 2)
              {
                Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s by day; last(max)[ever]", title);
                Tell(user, buffer.str);

                Utils::Sprintf(buffer.str, buffer.GetSize(), L"Any Day %d(%d)[%d]",
                  day.sample, day.maximum, day.total);
                Tell(user, buffer.str);

                buffer = L"";
                for (int i = 0; i < 7; i++)
                {
                  StrBuf<32, CH> buf;
                  Utils::Sprintf(buf.str, buf.GetSize(), L"%s %d(%d)[%d]",
                    dow[i],
                    days[i].lastSample, days[i].maximum, days[i].total);
                  Utils::Strcat(buffer.str, buf.str);

                  if (i < 6)
                  {
                    Utils::Strcat(buffer.str, L"; ");
                  }
                }
                Tell(user, buffer.str);
              }
              else
              {
                if (Crc::CalcStr(argv[2]) == 0xC87BD42E) // "Any"
                {
                  Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s for any day", title);
                  day.Report(user, buffer.str);
                }
                else
                {
                  // The next arg is the hour we want to display
                  U32 day = Utils::AtoI(argv[2]);

                  if (day > 7 || day < 1)
                  {
                    Tell(user, L"Day must be in the range 1 to 7 or \"any\".");
                  }
                  else
                  {
                    Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s for %s", title, dow[day - 1]);

                    // Generate a report for that day
                    days[day - 1].Report(user, buffer.str);
                  }
                }
              }
              break;
            }

            case 0xD2253FBA: // "week"
              week.Report(user, title);
              break;

            case 0xB2D7E0AA: // "month"
            {
              if (argc == 2)
              {
                Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s by month; last(max)[ever]", title);
                Tell(user, buffer.str);

                Utils::Sprintf(buffer.str, buffer.GetSize(), L"Any Month %d(%d)[%d]",
                  month.sample, month.maximum, month.total);
                Tell(user, buffer.str);

                buffer = L"";

                for (int i = 0; i < 12; i++)
                {
                  StrBuf<32, CH> buf;
                  Utils::Sprintf(buf.str, buf.GetSize(), L"%s %d(%d)[%d]",
                    moy[i],
                    months[i].lastSample, months[i].maximum, months[i].total);
                  Utils::Strcat(buffer.str, buf.str);

                  if (i < 11)
                  {
                    Utils::Strcat(buffer.str, L"; ");
                  }
                }
                Tell(user, buffer.str);
              }
              else
              {
                if (Crc::CalcStr(argv[2]) == 0xC87BD42E) // "Any"
                {
                  Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s for any month", title);
                  month.Report(user, buffer.str);
                }
                else
                {
                  // The next arg is the month we want to display
                  U32 month = Utils::AtoI(argv[2]);

                  if (month > 12 || month < 1)
                  {
                    Tell(user, L"Month must be in the range 1 to 12.");
                  }
                  else
                  {
                    Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s for %s", title, moy[month - 1]);

                    // Generate a report for that day
                    months[month - 1].Report(user, buffer.str);
                  }
                }
              }
              break;
            }

            case 0xC0DCC9BA: // "year"
              Utils::Sprintf(buffer.str, buffer.GetSize(), L"%s this year", title);
              year.Report(user, buffer.str);
              break;

            default:
              Tell(user, L"try hour, day, week, month, or year");
              break;
          }
        }
      }
    };


    ////////////////////////////////////////////////////////////////////////////////
    //
    // Struct User
    //
    struct User
    {
      U32 crc;          // crc of the user name
      U32 login;        // last time they logged in
      U32 logout;       // last time they logged out
      U32 hostings;     // number of games hosted
      Stat online;      // time online (in seconds)

      // Reset this user
      void Reset(U32 crc)
      {
        this->crc = crc;
        login = 0;
        logout = 0;
        hostings = 0;
        online.Reset();
      }
    };


    // Stats about games
    StatGroup games;

    // Stats about logins
    StatGroup logins;

    // Stats about people
    StatGroup people;

    // Number of users in the database
    U32 numUsers;

    // Number of available user slots
    U32 freeUsers;

    // Array of all know users in CRC order
    User users[];
    
  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Internal data
  //
  static Info *info;
  static U32 startTime;
  static System::Thread *timerThread;
  static BinTree<void> knownUsers;

  static HANDLE fileHandle;
  static HANDLE mapHandle;


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  void Init();
  void Done();
  void CDECL Start();
  void OpenDataFile();
  void LoadConfig();
  Bool Process();
  U32 STDCALL TimerProcess(void *);

  // Find a user
  U32 FindUser(U32 crc);

  // Get a user
  const Info::User * GetUser(U32 crc);

  // User login and logout
  void UserLogin(const char *user);
  void UserLogout(const char *user);

}


//
// main
//
void CDECL main()
{
  printf("WonBot v0.1\n");

  WonBot::Init();
  Debug::Exception::Handler(WonBot::Start);
  WonBot::Done();
}


////////////////////////////////////////////////////////////////////////////////
//
// Namespace WonBot
//
namespace WonBot
{

  //
  // Init
  //
  void Init()
  {
    Debug::SetupInst(GetModuleHandle(NULL));
    Debug::Init();
    
    startTime = time(NULL);
    OpenDataFile();

    timerThread = new System::Thread(TimerProcess, NULL);
  }


  //
  // Done
  //
  void Done()
  {
    delete timerThread;
    knownUsers.UnlinkAll();
  }


  //
  // Start
  //
  void CDECL Start()
  {
    // Turning on Logging
    WonIface::Logging(TRUE);

    // Initialize WON
    printf("Initializing WON\n");
    WonIface::Init();

    printf("Loading Configuration\n");
    LoadConfig();

    printf("Connecting to WON\n");
    WonIface::Connect();

    while (Process())
    {
      WonIface::GetEventQueue().Wait();
    }

    printf("Shutting Down WON\n");
    WonIface::Done();
  }


  //
  // Open the data file
  //
  void OpenDataFile()
  {
    Bool created = FALSE;

    fileHandle = CreateFile("wonbot.dat", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);

    if (fileHandle == NULL)
    {
      ERR_FATAL(("Could not open data file"))
    }

    // Get the size of the file (assume that its is less than 4GB)
    U32 size = GetFileSize(fileHandle, NULL);

    if (!size)
    {
      // We need to determine the size of a fresh data pack
      size = sizeof (Info) + sizeof (Info::User) * maxPlayers;
      created = TRUE;
    }
    if (size < sizeof (Info) + sizeof (Info::User) * maxPlayers)
    {
      size = sizeof (Info) + sizeof (Info::User) * maxPlayers;
    }

    // Create the file mapping
    mapHandle = CreateFileMapping(fileHandle, NULL, PAGE_READWRITE, 0, size, NULL);

    // Could the file be mapped
    if (mapHandle == NULL)
    {
      ERR_FATAL(("Could not create file mapping for data file"))
    }

    // Get a pointer to the mapping
    U8 *ptr = (U8 *) MapViewOfFile(mapHandle, FILE_MAP_WRITE, 0, 0, size);

    // Could we get a view of the mapping ?
    if (ptr == NULL)
    {
      ERR_FATAL(("Could not create a view of the mapping for the data file"))
    }

    // Set the pointer of info
    info = (Info *) ptr;

    // If we were just created 
    if (created)
    {
      info->games.Reset();
      info->logins.Reset();
      info->people.Reset();
      info->numUsers = 0;
      info->freeUsers = maxPlayers;
    }
    else
    {
      info->freeUsers = maxPlayers - info->numUsers;
    }
  }


  //
  // LoadConfig
  //
  void LoadConfig()
  {
    // Load the config file
    PTree pTree;

    // Parse the file
    if (pTree.AddFile(configFile))
    {
      // Get the global scope
      FScope *gScope = pTree.GetGlobalScope();
      FScope *fScope;

      // Process each function
      while ((fScope = gScope->NextFunction()) != NULL)
      {
        switch (fScope->NameCrc())
        {
          case 0x29C666A8: // "Bot"
          {
            FScope *sScope;
            while ((sScope = fScope->NextFunction()) != NULL)
            {
              switch (sScope->NameCrc())
              {
                case 0x19BE1F7E: // "User"
                {
                  wonUserName = StdLoad::TypeString(sScope);
                  printf("User: %s\n", wonUserName.str);
                  break;
                }

                case 0x4FA7EDBB: // "Password"
                {
                  wonPassword = StdLoad::TypeString(sScope);
                  printf("Password: %s\n", wonPassword.str);
                  break;
                }

                case 0x546F03C2: // "CDKey"
                {
                  const char *cdkey = StdLoad::TypeString(sScope);
                  printf("CDKey: %s ... ", cdkey);
                  if (WonIface::CheckKey(cdkey))
                  {
                    printf("validated\n");
                  }
                  else
                  {
                    printf("invalid\n");
                  }
                  break;
                }
              }
            }
            break;
          }

          case 0x7F6E34C2: // "DirectoryServers"
          {
            List<char> servers;

            FScope *sScope;
            while ((sScope = fScope->NextFunction()) != NULL)
            {
              switch (sScope->NameCrc())
              {
                case 0x9F1D54D0: // "Add"
                {
                  const char *server = StdLoad::TypeString(sScope);
                  servers.Append(const_cast<char *>(server));
                  printf("Directory Server %s\n", server);
                  break;
                }
              }
            }
            WonIface::SetDirectoryServers(servers);
            servers.UnlinkAll();
            break;
          }

          default:
            break;
        }
      }
    }
    else
    {
      //ERR_FATAL(("Unable to read configuration file"))
    }
  }


  //
  // Process
  //
  Bool Process()
  {
    // Get any events which have occured and handle them
    U32 message;
    void *data;

    while (WonIface::Process(message, data))
    {
      switch (message)
      {
        case 0x255289B5: // "WonBot::Timer"
        {
          WonIface::KeepAlive();

          long t = time(NULL);
          tm *lt = localtime(&t);

          info->games.Check(L"Games", t, lt);
          info->logins.Check(L"Logins", t, lt);
          info->people.Check(L"People", t, lt);
          break;
        }

        case WonIface::Message::RetrievedServerList:
          printf("Logging in to WON\n");
          WonIface::LoginAccount(wonUserName.str, wonPassword.str);
          break;

        case WonIface::Message::LoggedIn:
          printf("Logged in to WON\n");
          break;

        case WonIface::Message::InitialRoomUpdate:
          printf("Searching for the lobby\n");
          break;

        case WonIface::Message::EnteredRoom:
          if (data)
          {
            CAST(WonIface::Message::Data::EnteredRoom *, enteredRoom, data)
            printf("Entered Room %s\n", Utils::Unicode2Ansi(enteredRoom->text));
          }
          break;

        case WonIface::Message::ConnectedRoom:
          printf("Connected to Room\n");
          break;

        case WonIface::Message::CreatedRoom:
          printf("Created Room\n");
          break;

        case WonIface::Message::RegisteredRoom:
          printf("Registered Room\n");
          break;

        case WonIface::Message::FirewallStatus:
          switch (WonIface::GetFirewallStatus())
          {
            case WonIface::Firewall::Unchecked:
              printf("Firewall Status: Unchecked\n");
              break;

            case WonIface::Firewall::Checking:
              printf("Firewall Status: Checking\n");
              break;

            case WonIface::Firewall::Behind:
              printf("Firewall Status: Behind\n");
              break;

            case WonIface::Firewall::None:
              printf("Firewall Status: Not Behind\n");
              break;
          }
          break;

        case WonIface::Message::Chat:
        {
          if (data)
          {
            // Message passed from WON dll
            CAST(WonIface::Message::Data::Chat *, chat, data)

            StrBuf<256> user;
            StrBuf<1024> text;

            if (chat->user)
            {
              Utils::Unicode2Ansi(user.str, user.GetSize(), chat->user);
            }
            if (chat->text)
            {
              Utils::Unicode2Ansi(text.str, text.GetSize(), chat->text);
            }

            switch (chat->id)
            {
              case WonIface::Message::Data::Chat::Private:
              {
                if (chat->user && chat->text)
                {
                  printf("prv: [%s] %s\n", user.str, text.str);

                  // Do we recognise this user ?
                  if (knownUsers.Exists(Crc::CalcStr(user.str)))
                  {
                    // Convert the text into arguments
                    const char *argv[10];
                    int argc = 0;

                    char *ptr = text.str;
                    while (*ptr && argc < 10)
                    {
                      // If there's a quote " then set the arg pointer to the
                      // next character and then contiue until we hit another "
                      if (*ptr == '"')
                      {
                        ptr++;
                        argv[argc] = ptr;

                        // Search for the closing quote or string terminator
                        while (*ptr && *ptr != '"')
                        {
                          ptr++;
                        }

                        if (*ptr)
                        {
                          *ptr = '\0';
                          ptr++;
                        }

                        // Skip over any spaces
                        while (*ptr && *ptr == ' ')
                        {
                          ptr++;
                        }
                      }
                      else
                      {
                        argv[argc] = ptr;

                        // Continue until we hit a delimiter ' '
                        while (*ptr && *ptr != ' ')
                        {
                          ptr++;
                        }
                        if (*ptr)
                        {
                          *ptr = '\0';
                          ptr++;
                        }
                      }

                      argc++;
                    }

                    StrBuf<1024, CH> reply;

                    switch (Crc::CalcStr(argv[0]))
                    {
                      case 0x0F3EBD98: // "Uptime"
                      {
                        U32 uptime = time(NULL) - startTime;
                        Utils::Sprintf
                        (
                          reply.str, reply.GetSize(), 
                          L"The statbot has been up for %s", niceTime(uptime)
                        );
                        WonIface::PrivateMessage(user.str, reply.str);
                        break;
                      }

                      case 0x0D33B8A8: // "Time"
                      {
                        U32 t = time(NULL);
                        Utils::Sprintf(reply.str, reply.GetSize(), L"Time here is %s", _wctime((S32*)&t));
                        if (Utils::Strchr(reply.str, L'\n'))
                        {
                          *Utils::Strchr(reply.str, L'\n') = L'\0';
                        }
                        WonIface::PrivateMessage(user.str, reply.str);
                        break;
                      }

                      case 0xB91518D4: // "Seen"
                      {
                        if (argc > 1)
                        {
                          const Info::User *u = GetUser(Crc::CalcStr(argv[1]));
                          if (u && (u->login || u->logout))
                          {
                            Utils::Sprintf(reply.str, reply.GetSize(), L"Saw '%s'", Utils::Ansi2Unicode(argv[1]));
                            WonIface::PrivateMessage(user.str, reply.str);

                            if (u->login)
                            {
                              Utils::Sprintf(reply.str, reply.GetSize(), L"login at %s", _wctime((S32*)&u->login));
                              if (Utils::Strchr(reply.str, L'\n'))
                              {
                                *Utils::Strchr(reply.str, L'\n') = L'\0';
                              }
                              WonIface::PrivateMessage(user.str, reply.str);
                            }
                            if (u->logout)
                            {
                              Utils::Sprintf(reply.str, reply.GetSize(), L"logout at %s", _wctime((S32*)&u->logout));
                              if (Utils::Strchr(reply.str, L'\n'))
                              {
                                *Utils::Strchr(reply.str, L'\n') = L'\0';
                              }
                              WonIface::PrivateMessage(user.str, reply.str);
                            }
                          }
                          else
                          {
                            Utils::Sprintf(reply.str, reply.GetSize(), L"Haven't seen '%s' before", Utils::Ansi2Unicode(argv[1]));
                            WonIface::PrivateMessage(user.str, reply.str);
                          }
                        }
                        else
                        {
                          WonIface::PrivateMessage(user.str, L"You sure you know what you're doing?");
                        }
                        break;
                      }

                      case 0x5AAFA976: // "Online"
                      {
                        if (argc > 1)
                        {
                          const Info::User *u = GetUser(Crc::CalcStr(argv[1]));
                          if (u && u->logout)
                          {
                            Utils::Sprintf(reply.str, reply.GetSize(), L"Online info for '%s'", Utils::Ansi2Unicode(argv[1]));
                            WonIface::PrivateMessage(user.str, reply.str);
                            Utils::Sprintf(
                              reply.str, 
                              reply.GetSize(), 
                              L"Samples %d, Time: Total %s, Last %ds, Max %ds, Avg %.1fs", 
                              u->online.samples,
                              niceTime(u->online.total),
                              u->online.lastSample,
                              u->online.maximum,
                              u->online.GetAvg());

                            WonIface::PrivateMessage(user.str, reply.str);
                          }
                          else
                          {
                            Utils::Sprintf(reply.str, reply.GetSize(), L"Haven't seen '%s' before", Utils::Ansi2Unicode(argv[1]));
                            WonIface::PrivateMessage(user.str, reply.str);
                          }
                        }
                        else
                        {
                          WonIface::PrivateMessage(user.str, L"You sure you know what you're doing?");
                        }
                        break;
                      }

                      case 0xB303ABED: // "TellLogins"
                        info->logins.Report(NULL, L"Logins", argc, argv);
                        break;

                      case 0xF1D065D5: // "Logins"
                        info->logins.Report(user.str, L"Logins", argc, argv);
                        break;

                      case 0x7E4967EE: // "TellPeople"
                        info->people.Report(NULL, L"People", argc, argv);
                        break;

                      case 0x3C9AA9D6: // "People"
                        info->people.Report(user.str, L"People", argc, argv);
                        break;

                      case 0x755E3ECF: // "TellGames"
                        info->games.Report(NULL, L"Games", argc, argv);
                        break;

                      case 0xFF5A6B37: // "Games"
                        info->games.Report(user.str, L"Games", argc, argv);
                        break;

                        WonIface::PrivateMessage(user.str, L"Can't find the damn manual anywhere.");
                        break;

                      case 0x1931653D: // "Hello"
                        WonIface::PrivateMessage(user.str, L"Hi");
                        break;

                      default:
                      case 0xFF52E6E7: // "Help"
                      case 0x0E40B855: // "Commands"
                        WonIface::PrivateMessage(user.str, L"Commands are; seen <username>, online <username>, [tell]logins <expand>, [tell]people <expand>, [tell]games <expand>");
                        break;
                    }
                  }
                  else
                  {
                    // If the text is the keyword then add them to the known users list
                    if (Crc::CalcStr(text.str) == 0x8F37ABC8) // "if dog!=cat then cat!=dog"
                    {
                      knownUsers.Add(Crc::CalcStr(user.str), NULL);
                      WonIface::PrivateMessage(user.str, L"Welcome to Statbot v0.1");
                    }
                  }
                }
                break;
              }

              case WonIface::Message::Data::Chat::Emote:
              {
                if (chat->user && chat->text)
                {
                  printf("emote: %s %s\n", user.str, text.str);
                }
                break;
              }

              case WonIface::Message::Data::Chat::Broadcast:
              {
                if (chat->user && chat->text)
                {
                  printf("msg: [%s] %s\n", user.str, text.str);
                }
                break;
              }

              case WonIface::Message::Data::Chat::PlayerEntered:
              {
                if (chat->user)
                {
                  UserLogin(user.str);
                  printf("enter: %s\n", user.str);
                }
                break;
              }

              case WonIface::Message::Data::Chat::PlayerLeft:
              {
                if (chat->user)
                {
                  UserLogout(user.str);
                  printf("exit: %s\n", user.str);
                }
                break;
              }

              case WonIface::Message::Data::Chat::GameCreated:
              {
                if (chat->user && chat->text)
                {
                  printf("created: %s by %s\n", text.str, user.str);

                  // Sample games
                  info->games.Sample();
                }
                break;
              }

              case WonIface::Message::Data::Chat::GameDestroyed:
              {
                if (chat->text)
                {
                  printf("ended: %s\n", text.str);
                }
                break;
              }
            }
            delete chat;
          }
          break;
        }

        case WonIface::Error::ReconnectFailure:
          printf("Lost connection to WON, reconnecting");
          WonIface::Connect();
          break;

      }
    }
    return (TRUE);
  }


  //
  // Find User
  //
  U32 FindUser(U32 crc)
  {
    U32 top = info->numUsers - 1;
    U32 bottom = 0;

    if (crc >= info->users[top].crc)
    {
      bottom = top;
    }
    else
    {
      while ((top - bottom) > 1)
      {
        U32 middle = (top + bottom) >> 1;

        /*
        printf("Searching for %08X Top %08X [%d] Middle %08X [%d] Bottom %08X [%d] %d\n",
          crc, 
          info->users[top].crc, top,
          info->users[middle].crc, middle,
          info->users[bottom].crc, bottom,
          info->numUsers);
          */

        if (crc < info->users[middle].crc)
        {
          top = middle;
        }
        else
        {
          bottom = middle;
        }
      }
    }
    // printf("Result %08X Bottom %08X [%d] %d\n", crc, info->users[bottom].crc, bottom);
    return (bottom);
  }


  //
  // Get a user
  //
  const Info::User * GetUser(U32 crc)
  {
    U32 index = FindUser(crc);

    if (info->users[index].crc == crc)
    {
      return (&info->users[index]);
    }
    else
    {
      return (NULL);
    }
  }


  //
  // User login 
  // 
  void UserLogin(const char *userName)
  {
    U32 crc = Crc::CalcStr(userName);

    // Sample total logins
    info->logins.Sample();

    Info::User *u;

    // If there are users already, search through them to find the user or
    if (info->numUsers)
    {
      U32 index = FindUser(crc);
      if (info->users[index].crc == crc)
      {
        u = &info->users[index];

        // Sample people logins
        info->people.Sample(1, u->login);
      }
      else
      {
        if (crc < info->users[index].crc)
        {
          //printf("Adding user %08X before %08X [%d/%d]\n", crc, info->users[index].crc, index, info->numUsers);

          if (index < info->numUsers)
          {
            Utils::Memmove
            (
              &info->users[index + 1], 
              &info->users[index], 
              (info->numUsers - index) * sizeof (Info::User)
            );
          }
          u = &info->users[index];
        }
        else
        {
          //printf("Adding user %08X after %08X [%d/%d]\n", crc, info->users[index].crc, index, info->numUsers);

          if (index < info->numUsers - 1)
          {
            Utils::Memmove
            (
              &info->users[index + 2], 
              &info->users[index + 1], 
              (info->numUsers - index - 1) * sizeof (Info::User)
            );
          }
          u = &info->users[index + 1];
        }

        // Add one to the number of users
        info->numUsers++;
        info->freeUsers--;

        // Set the pointer to the new user
        u->Reset(crc);

        // Sample people logins
        info->people.Sample();
      }
    }
    else
    {
      u = &info->users[0];
      u->Reset(crc);
      info->numUsers++;
      info->freeUsers--;

      // Sample people logins
      info->people.Sample();
    }

    // Set the last time they logged in
    u->login = time(NULL);

    // After users have been added check to see that all users are in order
    U32 prev = 0;
    for (U32 i = 0; i < info->numUsers; i++)
    {
      if (prev == info->users[i].crc)
      {
        printf("Duplicate user %08X\n", info->users[i].crc);
      }

      if (prev > info->users[i].crc)
      {
        printf("User is out of order %08X < %08X\n", info->users[i].crc, prev);
      }
      prev = info->users[i].crc;
    }
  }


  //
  // User logout
  //
  void UserLogout(const char *userName)
  {
    U32 crc = Crc::CalcStr(userName);

    if (info->numUsers)
    {
      U32 index = FindUser(crc);
      if (info->users[index].crc == crc)
      {
        Info::User *u = &info->users[index];

        U32 t = time(NULL);

        // Update the logout time
        u->logout = t;

        // Update the amount of time this user was online
        if (u->login)
        {
          u->online.Sample(u->logout - u->login);
          u->online.Roll();
        }

        printf("User Logout: '%s' Online: %d %d %d %d %d %f\n", 
          userName, 
          u->online.samples,
          u->online.total,
          u->online.sample,
          u->online.lastSample,
          u->online.maximum,
          u->online.GetAvg());
      }
    }
  }


  //
  // TimerProcess
  //
  U32 STDCALL TimerProcess(void *)
  {
    SafeQueue<WonIface::Event, 256> &eventQueue = WonIface::GetEventQueue();

    for (;;)
    {
      Sleep(30000);
      WonIface::Event *e = eventQueue.AddPre();
      e->message = 0x255289B5; // "WonBot::Timer"
      e->data = NULL;
      eventQueue.AddPost();
    }
  }

}


///////////////////////////////////////////////////////////////////////////////
//
// Pre C-runtime initialization
//
struct StaticInit
{
  StaticInit()
  {
    // Only absolutely essential (and safe) stuff to be initalized in here
    // Will be called before the C and C++ runtime libraries are inited, and
    // before virtual function tables are setup.
    Debug::PreIgnition();
    Clock::Time::Init();
    Log::Init();
  }

  ~StaticInit()
  {
    // Report memory leaks
    Debug::Memory::Check();

    // Shutdown logging
    Log::Done();

    // Shutdown Debug
    Debug::Done();
  }
};

#pragma warning(disable : 4074)
#pragma init_seg(compiler)
static StaticInit staticInit;
