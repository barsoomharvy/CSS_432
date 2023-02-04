#include "Timer.h"


Timer::Timer( ) 
{
    startTime.tv_sec = 0;
    startTime.tv_usec = 0;
    endTime.tv_sec = 0;
    endTime.tv_usec = 0;
}

void Timer::Start( ) 
{
    gettimeofday(&startTime, NULL);
}

long Timer::Lap( ) {
  gettimeofday( &endTime, NULL );
  long interval =
    ( endTime.tv_sec - startTime.tv_sec ) * 1000000 +
    ( endTime.tv_usec - startTime.tv_usec );
  return interval;
}


// Get the diff between the start and the current time 
long Timer::End() 
{
    gettimeofday(&endTime, NULL);
    long interval =
        (endTime.tv_sec - startTime.tv_sec) * 1000000 +
        (endTime.tv_usec - startTime.tv_usec);
    return interval;
}