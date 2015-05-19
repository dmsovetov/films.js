//
//  LinearRegression.cpp
//  DataMining
//
//  Created by Советов Дмитрий on 25.12.14.
//  Copyright (c) 2014 data. All rights reserved.
//

#include "LinearRegression.h"

// ------------------------------------------------------ LinearSamples ----------------------------------------------------- //

// ** LinearSamples::operator +=
const LinearSamples& LinearSamples::operator += ( const LinearSample& sample )
{
    m_samples.push_back( sample );
    return *this;
}

// ** LinearSamples::operator []
const LinearSample& LinearSamples::operator [] ( int index ) const
{
    assert( index >= 0 && index < count() );
    return m_samples[index];
}

// ** LinearSamples::count
int LinearSamples::count( void ) const
{
    return ( int )m_samples.size();
}

// ** LinearSamples::argumentMean
float LinearSamples::argumentMean( void ) const
{
    float sum = 0.0f;

    for( int i = 0, n = count(); i < n; i++ ) {
        sum += m_samples[i].m_argument;
    }

    return sum / count();
}

// ** LinearSamples::valueMean
float LinearSamples::valueMean( void ) const
{
    float sum = 0.0f;

    for( int i = 0, n = count(); i < n; i++ ) {
        sum += m_samples[i].m_value;
    }

    return sum / count();
}


// ---------------------------------------------------- LinearRegression ---------------------------------------------------- //

// ** LinearRegression::LinearRegression
LinearRegression::LinearRegression( float slope, float intercept, float error ) : m_slope( slope ), m_intercept( intercept ), m_error( error ), m_sst( 0.0f ), m_sse( 0.0f ), m_ssr( 0.0f )
{

}

// ** LinearRegression::sst
float LinearRegression::sst( void ) const
{
    return m_sst;
}

// ** LinearRegression::sse
float LinearRegression::sse( void ) const
{
    return m_sse;
}

// ** LinearRegression::ssr
float LinearRegression::ssr( void ) const
{
    return m_ssr;
}

// ** LinearRegression::intercept
float LinearRegression::intercept( void ) const
{
    return m_intercept;
}

// ** LinearRegression::slope
float LinearRegression::slope( void ) const
{
    return m_slope;
}

// ** LinearRegression::createFromSamples
LinearRegression LinearRegression::createFromSamples( const LinearSamples& samples )
{
    float argumentMean  = samples.argumentMean();
    float valueMean     = samples.valueMean();

    float num = 0.0f;
    float den = 0.0f;

    // ** Evaluate slope
    for( int i = 0, n = samples.count(); i < n; i++ ) {
        const LinearSample& sample = samples[i];

        float da = sample.m_argument - argumentMean;
        float dv = sample.m_value    - valueMean;

        num += da * dv;
        den += da * da;
    }

    float slope     = num / den;

    // ** Evaluate intercept
    float intersept = valueMean - slope * argumentMean;

    // ** Create a regression & compute errors
    LinearRegression result = LinearRegression( slope, intersept, 0.0f );
    result.computeErrors( samples );

    return result;
}

// ** LinearRegression::computeErrors
void LinearRegression::computeErrors( const LinearSamples& samples )
{
    float mean = samples.valueMean();

    m_sst = m_sse = m_ssr = 0.0f;

    for( int i = 0, n = samples.count(); i < n; i++ ) {
        const LinearSample& sample = samples[i];

        float predicted = (*this)( sample.m_argument );

        m_sst += powf( sample.m_value - mean,       2.0f );
        m_ssr += powf( predicted      - mean,       2.0f );
        m_sse += powf( sample.m_value - predicted,  2.0f );
    }
}

// ** LinearRegression::operator()
float LinearRegression::operator()( float argument ) const
{
    return argument * m_slope + m_intercept + m_error;
}

// ** LinearRegression::selfTest
void LinearRegression::selfTest( void )
{
    LinearSamples samples;

    samples += LinearSample( 34,  5  );
    samples += LinearSample( 108, 17 );
    samples += LinearSample( 64,  11 );
    samples += LinearSample( 88,  8  );
    samples += LinearSample( 99,  14 );
    samples += LinearSample( 51,  5  );

    assert( samples.argumentMean() == 74.0f );
    assert( samples.valueMean() == 10.0f );

    LinearRegression lr = LinearRegression::createFromSamples( samples );

    assert( compareFloats( lr.slope(),      0.146219686f ) );
    assert( compareFloats( lr.intercept(), -0.820257187f ) );
    assert( compareFloats( lr.sst(),        120          ) );
    assert( compareFloats( lr.sse(),        30.074894f   ) );
    assert( compareFloats( lr.ssr(),        89.9251022   ) );

    assert( compareFloats( lr( 212 ), 30.18, 0.01f ) );
}