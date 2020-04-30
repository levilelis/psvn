/*
Copyright (C) 2011 by the PSVN Research Group, University of Alberta
*/

#ifndef _TIMER_HPP
#define _TIMER_HPP

#include <sys/time.h>

class Timer
{
public:
    Timer()
    {
        Start();
    }

    void Start()
    {
        gettimeofday(&m_start, NULL);
    }

    double Elapsed() const
    {
        struct timeval end;
        gettimeofday(&end, NULL);
        end.tv_sec -= m_start.tv_sec;
        end.tv_usec -= m_start.tv_usec;
        if( end.tv_usec < 0 ) {
            end.tv_usec += 1000000;
            --end.tv_sec;
        }
        return end.tv_sec + end.tv_usec / 1000000.0f;
    }

private:
    struct timeval m_start;
};

#endif
