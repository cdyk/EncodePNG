#include <string>
#include <cstdio>
#include <iostream>
#include "timer.hpp"

TimeStamp::TimeStamp()
{
    clock_gettime( CLOCK_MONOTONIC, &m_time );
}


double
TimeStamp::delta( const TimeStamp& start, const TimeStamp& stop )
{
    timespec delta;
    if( (stop.m_time.tv_nsec-start.m_time.tv_nsec) < 0 ) {
        delta.tv_sec = stop.m_time.tv_sec - start.m_time.tv_sec - 1;
        delta.tv_nsec = 1000000000 + stop.m_time.tv_nsec - start.m_time.tv_nsec;
    }
    else {
        delta.tv_sec = stop.m_time.tv_sec - start.m_time.tv_sec;
        delta.tv_nsec = stop.m_time.tv_nsec - start.m_time.tv_nsec;
    }
    return delta.tv_sec + 1e-9*delta.tv_nsec;        
}
