/*
 * SimplePolynomial.hpp
 *
 *  Created on: May 15, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef SIMPLEPOLYNOMIAL_HPP_
#define SIMPLEPOLYNOMIAL_HPP_

#include <cstddef>
#include <vector>
#include "Eigen/Dense"
#include <string>
#include <cmath>
#include <sstream>

namespace VevaciousPlusPlus
{

  // This is a class to just hold a bunch of coefficients of a polynomial and
  // return the value of the polynomial for a given input.
  class SimplePolynomial
  {
  public:
    SimplePolynomial() : coefficientVector(),
                         leadingPower( 0 ) {}

    SimplePolynomial( size_t const reserveSize,
                      unsigned int const leadingPower = 0 ) :
      coefficientVector( reserveSize,
                         0.0 ),
      leadingPower( leadingPower ) {}

    SimplePolynomial( std::vector< double > const& coefficientVector,
                      unsigned int const leadingPower = 0 ) :
      coefficientVector( coefficientVector ),
      leadingPower( leadingPower ) {}

    SimplePolynomial( Eigen::VectorXd const& eigenVector,
                      unsigned int const leadingPower = 0,
                      size_t extraEmptyEntriesAtConstruction = 0 ) :
      coefficientVector(),
      leadingPower( leadingPower )
    { CopyFromEigen( eigenVector,
                     leadingPower,
                     extraEmptyEntriesAtConstruction ); }

    SimplePolynomial( SimplePolynomial const& copySource ) :
      coefficientVector( copySource.coefficientVector ),
      leadingPower( copySource.leadingPower ) {}

    virtual ~SimplePolynomial() {}


    // This returns the sum of coefficientVector[ p ] * inputValue^p over p.
    double operator()( double const inputValue ) const;

    // This sets coefficientVector to have elements equal to those of
    // eigenVector.
    void CopyFromEigen( Eigen::VectorXd const& eigenVector,
                        unsigned int const leadingPower = 0,
                        size_t extraEmptyEntriesAtConstruction = 0 );

    // This sets this SimplePolynomial to be the first derivative of
    // polynomialToDifferentiate.
    void BecomeFirstDerivativeOf(
                           SimplePolynomial const& polynomialToDifferentiate );

    // This returns the first derivative of this SimplePolynomial with respect
    // to its variable, as a SimplePolynomial.
    SimplePolynomial FirstDerivative() const;

    std::vector< double > const& CoefficientVector() const
    { return coefficientVector; }

    std::vector< double >& CoefficientVector(){ return coefficientVector; }

    unsigned int const& LeadingPower() const{ return leadingPower; }

    unsigned int& LeadingPower(){ return leadingPower; }

    // This is for debugging.
    std::string AsDebuggingString() const;


  protected:
    std::vector< double > coefficientVector;
    unsigned int leadingPower;
  };



  // This returns the sum of coefficientVector[ p ] * inputValue^p over p.
  inline double SimplePolynomial::operator()( double const inputValue ) const
  {
    double returnValue( 0.0 );
    size_t const maximumPowerPlusOne( coefficientVector.size() );
    for( size_t whichPower( static_cast< size_t >( leadingPower ) );
         whichPower < maximumPowerPlusOne;
         ++whichPower )
    {
      returnValue += ( coefficientVector[ whichPower ] * pow( inputValue,
                                                              whichPower ) );
    }
    return returnValue;
  }

  // This sets coefficientVector to have elements equal to those of
  // eigenVector.
  inline void
  SimplePolynomial::CopyFromEigen( Eigen::VectorXd const& eigenVector,
                                   unsigned int const leadingPower,
                                   size_t extraEmptyEntriesAtConstruction )
  {
    this->leadingPower = leadingPower;
    coefficientVector = std::vector< double >( ( eigenVector.rows()
                                                 + leadingPower
                                           + extraEmptyEntriesAtConstruction ),
                             0.0 );
    size_t const numberOfRows( static_cast< size_t >( eigenVector.rows() ) );
    for( size_t whichIndex( 0 );
         whichIndex < numberOfRows;
         ++whichIndex )
    {
      coefficientVector[ whichIndex + leadingPower ]
      = eigenVector( whichIndex );
    }
  }


  // This sets this SimplePolynomial to be the first derivative of
  // polynomialToDifferentiate.
  inline void SimplePolynomial::BecomeFirstDerivativeOf(
                            SimplePolynomial const& polynomialToDifferentiate )
  {
    std::vector< double > const&
    integralCoefficients( polynomialToDifferentiate.CoefficientVector() );
    coefficientVector.resize( integralCoefficients.size() - 1 );
    if( polynomialToDifferentiate.leadingPower == 0 )
    {
      leadingPower = 0;
    }
    else
    {
      leadingPower = ( polynomialToDifferentiate.leadingPower - 1 );
    }
    size_t const maximumPowerPlusOne( coefficientVector.size() );
    for( size_t whichPower( 1 );
         whichPower <= maximumPowerPlusOne;
         ++whichPower )
    {
      coefficientVector[ whichPower - 1 ]
      = ( whichPower * integralCoefficients[ whichPower ] );
    }
  }

  // This returns the first derivative of this SimplePolynomial with respect
  // to its variable, as a SimplePolynomial.
  inline SimplePolynomial SimplePolynomial::FirstDerivative() const
  {
    SimplePolynomial firstDerivative;
    firstDerivative.BecomeFirstDerivativeOf( *this );
    return firstDerivative;
  }

  // This is for debugging.
  inline std::string SimplePolynomial::AsDebuggingString() const
  {
    std::stringstream returnStream;
    double coefficientMagnitude( 0.0 );
    for( size_t whichPower( leadingPower );
         whichPower < coefficientVector.size();
         ++whichPower )
    {
      coefficientMagnitude = coefficientVector[ whichPower ];
      if( whichPower == leadingPower )
      {
        returnStream << coefficientMagnitude;
      }
      else
      {
        if( coefficientMagnitude < 0.0 )
        {
          coefficientMagnitude = -coefficientMagnitude;
          returnStream << " - " << coefficientMagnitude;
        }
        else
        {
          returnStream << " + " << coefficientMagnitude;
        }
      }
      returnStream << " x^" << whichPower;
    }
    return returnStream.str();
  }

} /* namespace VevaciousPlusPlus */
#endif /* SIMPLEPOLYNOMIAL_HPP_ */
