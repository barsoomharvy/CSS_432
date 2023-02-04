#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <iostream>
using namespace std;

class Timer 
{
    public:
        Timer();       
        void Start();        
        long Lap( );               // endTime - startTime
        long End();               		// endTime - startTime

    private:
        struct timeval startTime;  
        struct timeval endTime;    
};

#endif