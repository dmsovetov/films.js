//
//  LinearRegression.h
//  DataMining
//
//  Created by Советов Дмитрий on 25.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#ifndef __DataMining__LinearRegression__
#define __DataMining__LinearRegression__

#include "../Common.h"

// ** struct LinearSample
struct LinearSample {
    float           m_argument;
    float           m_value;

                    LinearSample( float argument = 0.0f, float value = 0.0f )
                        : m_argument( argument ), m_value( value ) {}
};

// ** class LinearSamples
class LinearSamples {
public:

    const LinearSamples&        operator += ( const LinearSample& sample );
    const LinearSample&         operator [] ( int index ) const;

    int                         count( void ) const;
    float                       argumentMean( void ) const;
    float                       valueMean( void ) const;

private:

    std::vector<LinearSample>   m_samples;
};


// ** class LinearRegression
class LinearRegression {
public:

    float                   operator()( float argument ) const;

    float                   sst( void ) const;
    float                   sse( void ) const;
    float                   ssr( void ) const;

    float                   slope( void ) const;
    float                   intercept( void ) const;

    static LinearRegression createFromSamples( const LinearSamples& samples );

    static void             selfTest( void );

private:

                            LinearRegression( float slope, float intercept, float error );

    void                    computeErrors( const LinearSamples& samples );

private:

    float                   m_slope;
    float                   m_intercept;
    float                   m_error;
    float                   m_sst, m_sse, m_ssr;
};

#endif /* defined(__DataMining__LinearRegression__) */
