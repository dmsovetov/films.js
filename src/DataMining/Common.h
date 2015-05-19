//
//  Common.h
//  DataMining
//
//  Created by Советов Дмитрий on 05.01.15.
//  Copyright (c) 2015 data. All rights reserved.
//

#ifndef DataMining_Common_h
#define DataMining_Common_h

#include <stdlib.h>
#include <math.h>
#include <string>
#include <float.h>
#include <assert.h>
#include <limits.h>

#include <pthread.h>

#include <map>
#include <vector>
#include <list>
#include <memory>
#include <set>
#include <chrono>
#include <locale>
#include <algorithm>

// ** struct TimeMeasure
struct TimeMeasure {
                        TimeMeasure( const std::string& name ) : m_name( name ) { reset(); }
                        ~TimeMeasure( void ) { printf( "%s time elapsed %2.2f seconds\n", m_name.c_str(), reset() ); }

    float reset( void ) {
        float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - m_clock ).count() * 0.001f;
        m_clock = std::chrono::high_resolution_clock::now();
        return elapsed;
    }

    static int minutes( float seconds ) { return seconds / 60; }
    static int hours( float seconds ) { return minutes( seconds ) / 60; }
    static int days( float seconds ) { return hours( seconds ) / 24; }

    std::string formatTimeLeft( float seconds ) const {
        if( days( seconds ) ) return std::to_string( days( seconds ) ) + " days left";
        if( hours( seconds ) ) return std::to_string( hours( seconds ) ) + " hours left";
        if( minutes( seconds ) ) return std::to_string( minutes( seconds ) ) + " minutes left";

        return std::to_string( ( int )seconds ) + " seconds left";
    }

    std::string                                     m_name;
    std::chrono::high_resolution_clock::time_point  m_clock;
};

// ** compareFloats
inline bool compareFloats( float a, float b, float eps = 0.00001f )
{
    return fabs( a - b ) <= eps;
}

typedef std::vector<int>    IntegerArray;

#endif
