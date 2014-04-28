/*
 * SlhaFunctionoid.hpp
 *
 *  Created on: Mar 3, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef SLHAFUNCTIONOID_HPP_
#define SLHAFUNCTIONOID_HPP_

#include "../../CommonIncludes.hpp"
#include "ParameterFunctionoid.hpp"
#include "Eigen/Dense"


namespace VevaciousPlusPlus
{

  class SlhaFunctionoid : public ParameterFunctionoid
  {
  public:
    SlhaFunctionoid( std::string const& indexString,
                     std::string const& creationString,
                     std::string const& pythonParameterName );
    virtual
    ~SlhaFunctionoid();


    void SetBlockPointer(
                LHPC::SLHA::SparseManyIndexedBlock< double >* slhaBlock )
    { this->slhaBlock = slhaBlock; }

    unsigned int NumberOfIndices() const{ return indexVector.size(); }

    // This returns the value of the functionoid for the given logarithm of the
    // scale.
    virtual double operator()( double const logarithmOfScale ) const;

    // This updates currentValue based on logarithmOfScale.
    virtual void
    UpdateForNewLogarithmOfScale( double const logarithmOfScale )
    { currentValue = (*this)( logarithmOfScale ); }

    // This re-calculates the coefficients of the polynomial of the logarithm
    // of the scale used in evaluating the functionoid.
    void UpdateForNewSlhaParameters();

    // This is mainly for debugging.
    virtual std::string AsString();

    // This is for creating a Python version of the potential.
    virtual std::string PythonParameterEvaluation() const;


  protected:
    LHPC::SLHA::SparseManyIndexedBlock< double >* slhaBlock;
    std::vector< int > const indexVector;
    std::vector< double > scaleLogarithmPowerCoefficients;
  };




  // This returns the value of the functionoid for the given logarithm of the
  // scale.
  inline double
  SlhaFunctionoid::operator()( double const logarithmOfScale ) const
  {
    // debugging:
    /*std::cout << std::endl << "debugging:"
    << std::endl
    << "[" << this->AsString()
    << "].UpdateForNewLogarithmOfScale( " << logarithmOfScale
    << " ) called. currentValue was " << currentValue;
    std::cout << std::endl;*/

    double returnValue( scaleLogarithmPowerCoefficients[ 0 ] );

    // debugging:
    /*std::cout << std::endl << "debugging:"
    << std::endl
    << "scaleLogarithmPowerCoefficients[ 0 ] = "
    << scaleLogarithmPowerCoefficients[ 0 ] << ", currentValue = "
    << currentValue;
    std::cout << std::endl;*/

    double termContribution( 0.0 );

    for( unsigned int whichPower( 1 );
         whichPower < scaleLogarithmPowerCoefficients.size();
         ++whichPower )
    {
      termContribution = scaleLogarithmPowerCoefficients[ whichPower ];

      // debugging:
      /*std::cout << std::endl << "debugging:"
      << std::endl
      << "scaleLogarithmPowerCoefficients[ " << whichPower << " ] = "
      << scaleLogarithmPowerCoefficients[ whichPower ]
      << ", termContribution = " << termContribution;
      std::cout << std::endl;*/

      for( unsigned int powerCount( 0 );
           powerCount < whichPower;
           ++powerCount )
      {
        termContribution *= logarithmOfScale;

        // debugging:
        /*std::cout << std::endl << "debugging:"
        << std::endl
        << "termContribution = " << termContribution;
        std::cout << std::endl;*/
      }
      returnValue += termContribution;
      // debugging:
      /*std::cout << std::endl << "debugging:"
      << std::endl
      << "currentValue = " << currentValue;
      std::cout << std::endl;*/
    }
    return returnValue;
  }

  // This is mainly for debugging.
  inline std::string SlhaFunctionoid::AsString()
  {
    std::stringstream returnStream;
    returnStream << slhaBlock->getName() << '[';
    for( std::vector< int >::const_iterator
         whichIndex( indexVector.begin() );
         whichIndex < indexVector.end();
         ++whichIndex )
    {
      returnStream << ' ' << *whichIndex;
    }
    returnStream << " ]";
    return returnStream.str();
  }

  // This is for creating a Python version of the potential.
  inline std::string SlhaFunctionoid::PythonParameterEvaluation() const
  {
    std::stringstream stringBuilder;
    stringBuilder << std::setprecision( 12 ) << pythonParameterName << " = ( "
    << scaleLogarithmPowerCoefficients[ 0 ];
    for( unsigned int whichPower( 1 );
         whichPower < scaleLogarithmPowerCoefficients.size();
         ++whichPower )
    {
      stringBuilder << " + ( " << scaleLogarithmPowerCoefficients[ whichPower ]
      << " ) * lnQ**" << whichPower;
    }
    stringBuilder << " )";
    return stringBuilder.str();
  }

} /* namespace VevaciousPlusPlus */
#endif /* SLHAFUNCTIONOID_HPP_ */