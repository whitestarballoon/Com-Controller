#ifndef TimeKeeper_h
#define TimeKeeper_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <string.h>

class TimeKeeper
{

public:
	static TimeKeeper& getInstance() 
        {
          static TimeKeeper instance;
          return instance;
        }
        void setInitialSeconds(unsigned long ulSeconds);
        void setLastMillis(unsigned long ulMillis);
        boolean update();
        boolean update(unsigned long ulMillis);
	String getFormattedTime();
	static String getFormattedTime(unsigned long lSec);
        
private:
	TimeKeeper(): ulSecondsTick(0ul), ulLastMillis(0ul) {};  // Private so you can't call it
        //TimeKeeper(TimeKeeper const&);     // copy constructor
        //void operator=(TimeKeeper const&); // assignment operator 
        
	//static TimeKeeper* oTimeKeeper;  // Only instance

        unsigned long ulSecondsTick;
        unsigned long ulLastMillis;
};

#endif



