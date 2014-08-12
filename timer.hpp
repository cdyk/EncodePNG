#pragma once
#include <sys/time.h>

class TimeStamp
{
public:
    TimeStamp();
    
    static
    double
    delta( const TimeStamp& start, const TimeStamp& stop );
    
    
protected:
    timespec m_time;
};
