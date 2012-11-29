/***************************************************************************
 * blitz/Timer.h        Timer class, for use in benchmarking
 *
 * $Id: timer.h 1414 2005-11-01 22:04:59Z cookedm $
 *
 * Copyright (C) 1997-2001 Todd Veldhuizen <tveldhui@oonumerics.org>
 *
 * This code was relicensed under the modified BSD license for use in SciPy
 * by Todd Veldhuizen (see LICENSE.txt in the weave directory).
 *
 *
 * Suggestions:          blitz-dev@oonumerics.org
 * Bugs:                 blitz-bugs@oonumerics.org
 *
 * For more information, please see the Blitz++ Home Page:
 *    http://oonumerics.org/blitz/
 *
 ***************************************************************************/

// This class is not portable to non System V platforms.
// It will need to be rewritten for Windows, NT, Mac.
// NEEDS_WORK

#ifndef BZ_TIMER_H
#define BZ_TIMER_H

#include <time.h>


class Timer {

public:
    Timer() 
    { 
        state_ = uninitialized;
    }

    void start()
    { 
        state_ = running;
        t1_ = systemTime();
    }

    void stop()
    {
        t2_ = systemTime();
        //if (state_ != running) error;
        state_ = stopped;
    }
    
    long double elapsedSeconds()
    {
        //if (state_ != stopped) error;
        return t2_ - t1_;
    }

private:
    Timer(Timer&) { }
    void operator=(Timer&) { }

    long double systemTime()
    {
        return clock() / (long double) CLOCKS_PER_SEC;
    }

    enum { uninitialized, running, stopped } state_;

    long double t1_, t2_;
};

#endif // BZ_TIMER_H

