#ifndef OUTPUTQUEUE_H
#define OUTPUTQUEUE_H

#include <iostream>
#include <list>
#include <string>
#if defined(_LINUX) && defined(_USE_CURSES)
#include <curses.h>
#undef erase
#endif // _LINUX && _USE_CURSES

#if defined(_LINUX) && defined(_USE_CURSES)
# define SAMPLE_PRINTF printw
#else
# define SAMPLE_PRINTF printf
#endif // _LINUX && _USE_CURSES

class OutputQueue : public std::list<std::string> {
public:
	~OutputQueue() { Flush(); }
	void Flush()
	{
		iterator aQueueItr = begin();
		for ( ; aQueueItr != end(); )
		{
			SAMPLE_PRINTF("%s\n", aQueueItr->c_str());
			aQueueItr = erase(aQueueItr);
		}
#if defined(_LINUX) && defined(_USE_CURSES)
		refresh();
#else
		std::cout << std::flush; 
#endif // _LINUX && _USE_CURSES
	}
};

#endif // OUTPUTQUEUE_H
