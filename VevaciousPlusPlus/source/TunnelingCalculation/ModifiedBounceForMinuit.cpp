/*
 * ModifiedBounceForMinuit.cpp
 *
 *  Created on: May 13, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "../../include/VevaciousPlusPlus.hpp"

namespace VevaciousPlusPlus
{
  double const ModifiedBounceForMinuit::radiusDifferenceThreshold( 0.01 );

  ModifiedBounceForMinuit::ModifiedBounceForMinuit(
                                    PotentialFunction const& potentialFunction,
                                         size_t const numberOfVaryingPathNodes,
                                       size_t const numberOfSplinesInPotential,
                                           PotentialMinimum const& falseVacuum,
                                            PotentialMinimum const& trueVacuum,
                                double const falseVacuumEvaporationTemperature,
                                      size_t const undershootOvershootAttempts,
                                   size_t const maximumMultipleOfLongestLength,
                                  double const initialFractionOfShortestLength,
                                              double const minimumScaleSquared,
                                  double const shootingCloseEnoughThreshold ) :
    ROOT::Minuit2::FCNBase(),
    potentialFunction( potentialFunction ),
    numberOfFields( potentialFunction.NumberOfFieldVariables() ),
    referenceFieldIndex( 0 ),
    pathFromNodes( numberOfFields,
                   referenceFieldIndex,
                   numberOfVaryingPathNodes ),
    numberOfSplinesInPotential( numberOfSplinesInPotential ),
    falseVacuum( falseVacuum ),
    trueVacuum( trueVacuum ),
    zeroTemperatureStraightPath( numberOfFields ),
    zeroTemperatureStraightPathInverseLengthSquared( 0.0 ),
    falseVacuumPotential( potentialFunction( falseVacuum.FieldConfiguration(),
                                             0.0 ) ),
    trueVacuumPotential( potentialFunction( trueVacuum.FieldConfiguration(),
                                            0.0 ) ),
    falseVacuumEvaporationTemperature( falseVacuumEvaporationTemperature ),
    tunnelingScaleSquared( std::max( minimumScaleSquared,
                            potentialFunction.ScaleSquaredRelevantToTunneling(
                                                                   falseVacuum,
                                                              trueVacuum ) ) ),
    shortestLength( 1.0 ),
    longestLength( 1.0 ),
    undershootOvershootAttempts( undershootOvershootAttempts ),
    initialFractionOfShortestLength( initialFractionOfShortestLength ),
    shootingThreshold( shootingCloseEnoughThreshold )
  {
    // We pick the field that has the largest difference between the vacua as
    // the reference field that goes linearly with the auxiliary variable.
    std::vector< double > const&
    falseFieldConfiguration( falseVacuum.FieldConfiguration() );
    std::vector< double > const&
    trueFieldConfiguration( trueVacuum.FieldConfiguration() );
    double greatestFieldDifference( trueFieldConfiguration.front()
                                    - falseFieldConfiguration.front() );
    double currentFieldDifference( greatestFieldDifference );
    zeroTemperatureStraightPath.front() = currentFieldDifference;
    zeroTemperatureStraightPathInverseLengthSquared
    += ( currentFieldDifference * currentFieldDifference );
    if( greatestFieldDifference < 0.0 )
    {
      greatestFieldDifference = -greatestFieldDifference;
    }
    for( size_t fieldIndex( 1 );
         fieldIndex < numberOfFields;
         ++fieldIndex )
    {
      currentFieldDifference = ( trueFieldConfiguration[ fieldIndex ]
                                 - falseFieldConfiguration[ fieldIndex ] );
      zeroTemperatureStraightPath[ fieldIndex ] = currentFieldDifference;
      zeroTemperatureStraightPathInverseLengthSquared
      += ( currentFieldDifference * currentFieldDifference );
      if( currentFieldDifference < 0.0 )
      {
        currentFieldDifference = -currentFieldDifference;
      }
      if( currentFieldDifference > greatestFieldDifference )
      {
        referenceFieldIndex = fieldIndex;
        greatestFieldDifference = currentFieldDifference;
      }
    }
    zeroTemperatureStraightPathInverseLengthSquared
    = ( 1.0 / zeroTemperatureStraightPathInverseLengthSquared );
    pathFromNodes.SetReferenceField( referenceFieldIndex );
    double lowestScaleSquared( tunnelingScaleSquared );
    double highestScaleSquared( tunnelingScaleSquared );
    double candidateScaleSquared( std::max( minimumScaleSquared,
                                            falseVacuum.LengthSquared() ) );
    if( candidateScaleSquared < lowestScaleSquared )
    {
      lowestScaleSquared = candidateScaleSquared;
    }
    if( candidateScaleSquared > highestScaleSquared )
    {
      highestScaleSquared = candidateScaleSquared;
    }
    candidateScaleSquared = std::max( minimumScaleSquared,
                                      trueVacuum.LengthSquared() );
    if( candidateScaleSquared < lowestScaleSquared )
    {
      lowestScaleSquared = candidateScaleSquared;
    }
    if( candidateScaleSquared > highestScaleSquared )
    {
      highestScaleSquared = candidateScaleSquared;
    }
    shortestLength = ( 1.0 / sqrt( highestScaleSquared ) );
    longestLength = ( 1.0 / sqrt( lowestScaleSquared ) );
    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "tunnelingScaleSquared = " << tunnelingScaleSquared;
    std::cout << std::endl;/**/
  }

  ModifiedBounceForMinuit::~ModifiedBounceForMinuit()
  {
    // This does nothing.
  }


  // This turns pathParameterization into a PathFieldsAndPotential, by first
  // checking for a non-zero temperature, then setting up the straight path
  // in field space, and projecting the nodes extracted from
  // pathParameterization onto planes perpendicular to the straight path. A
  // few extra bits of information to do with the temperature are also
  // recorded in the PathFieldsAndPotential object returned.
  PathFieldsAndPotential ModifiedBounceForMinuit::DecodePathParameters(
                      std::vector< double > const& pathParameterization ) const
  {
    if( ZeroTemperatureParameterization( pathParameterization ) )
    {
      return pathFromNodes( pathParameterization,
                            zeroTemperatureStraightPath,
                            zeroTemperatureStraightPathInverseLengthSquared,
                            falseVacuum.FieldConfiguration(),
                            falseVacuumPotential,
                            trueVacuumPotential,
                            0.0 );
    }
    double const givenTemperature( pathParameterization.back() );
    PotentialForMinuit potentialForMinuit( potentialFunction );
    potentialForMinuit.SetTemperature( givenTemperature );
    MinuitManager thermalDsbFinder( potentialForMinuit );

    MinuitMinimum
    thermalTrueVacuum( thermalDsbFinder( trueVacuum.FieldConfiguration() ) );
    double const thermalTrueVacuumPotential( thermalTrueVacuum.FunctionValue()
                                     + potentialForMinuit.FunctionAtOrigin() );
    // We undo the offset from potentialFromMinuit.

    double straightPathLengthSquared( 0.0 );
    std::vector< double >&
    straightPath( thermalTrueVacuum.VariableValues() );
    // Might as well put stuff straight into
    // thermalTrueVacuum.VariableValues() as it won't be used later anyway.
    bool const falseMinimumNotEvaporated( givenTemperature
                                         < falseVacuumEvaporationTemperature );
    if( falseMinimumNotEvaporated )
    {
      MinuitMinimum thermalFalseVacuum( thermalDsbFinder(
                                          falseVacuum.FieldConfiguration() ) );
      // We undo the offset from potentialFromMinuit.
      for( unsigned int fieldIndex( 0 );
           fieldIndex < numberOfFields;
           ++fieldIndex )
      {
        straightPath[ fieldIndex ]
        -= thermalFalseVacuum.VariableValues()[ fieldIndex ];
        straightPathLengthSquared += ( straightPath[ fieldIndex ]
                                       * straightPath[ fieldIndex ] );
      }
      return pathFromNodes( pathParameterization,
                            straightPath,
                            ( 1.0 / straightPathLengthSquared ),
                            thermalFalseVacuum.VariableValues(),
                            ( thermalFalseVacuum.FunctionValue()
                              + potentialForMinuit.FunctionAtOrigin() ),
                            thermalTrueVacuumPotential,
                            givenTemperature );
    }
    else
    {
      for( unsigned int fieldIndex( 0 );
           fieldIndex < numberOfFields;
           ++fieldIndex )
      {
        straightPathLengthSquared += ( straightPath[ fieldIndex ]
                                       * straightPath[ fieldIndex ] );
      }
      return pathFromNodes( pathParameterization,
                            straightPath,
                            ( 1.0 / straightPathLengthSquared ),
                            potentialFunction.FieldValuesOrigin(),
                      potentialFunction( potentialFunction.FieldValuesOrigin(),
                                         givenTemperature ),
                            thermalTrueVacuumPotential,
                            givenTemperature );
    }
  }

  // This returns a polynomial approximation of the potential along the path
  // given by splineCoefficients.
  void ModifiedBounceForMinuit::PotentialAlongPath(
                         PathFieldsAndPotential& pathFieldsAndPotential ) const
  {
    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "ModifiedBounceForMinuit::PotentialAlongPath(pathFieldsAndPotential="
    << pathFieldsAndPotential.AsDebuggingString()
    << ") called. numberOfSplinesInPotential = " << numberOfSplinesInPotential;
    std::cout << std::endl;/**/
    std::vector< double > fieldConfiguration( numberOfFields );
    // We choose to take the cos of a linear distribution of equally-spaced
    // points so that we have a finer resolution of the potential near the
    // minima
    double auxiliaryValue( 0.0 );
    for( size_t splinePoint( 1 );
         splinePoint < numberOfSplinesInPotential;
         ++splinePoint )
    {
      auxiliaryValue = ( 0.5 * ( 1.0 - cos( ( (double)splinePoint * M_PI )
                                    / (double)numberOfSplinesInPotential ) ) );
      // The above might run into numerical precision issues if too many spline
      // segments are asked for, but it's probably OK up to even 10^5 segments
      // (cos(0.0) - cos(10^(-5)) = 5E-11 which should still be well-resolved
      // by doubles).
      //auxiliaryValue
      //= ( (double)splinePoint /(double)numberOfSplinesInPotential );
      std::vector< double > const&
      fieldConfiguration( pathFieldsAndPotential.FieldConfiguration(
                                                            auxiliaryValue ) );
      pathFieldsAndPotential.PotentialApproximation().AddPoint( auxiliaryValue,
                                       ( potentialFunction( fieldConfiguration,
                                    pathFieldsAndPotential.GivenTemperature() )
                                         - falseVacuumPotential ) );
    }
    pathFieldsAndPotential.PotentialApproximation().SetSpline(
                                  trueVacuumPotential - falseVacuumPotential );
    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "ModifiedBounceForMinuit::PotentialAlongPath([pathFieldsAndPotential])"
    << " set pathFieldsAndPotential to be" << std::endl
    << pathFieldsAndPotential.AsDebuggingString();
    std::cout << std::endl;/**/
  }

  // This returns the effective bounce action [S_4 or ((S_3(T)/T + ln(S_3(T)))]
  // calculated under the thin-wall approximation. Before returning, it sets
  // thinWallIsGoodApproximation to be true or false depending on whether
  // the thin-wall approximation is a good approximation or not.
  /*double ModifiedBounceForMinuit::ThinWallApproximation(
                           PathFieldsAndPotential const pathFieldsAndPotential,
                                      bool& thinWallIsGoodApproximation ) const
  {
    // debugging:
    std::cout << std::endl << "debugging:"
    << std::endl
    << "Must remember to test thin-wall approximation!";
    std::cout << std::endl;

    double const potentialDifference( pathFieldsAndPotential.FalsePotential()
                                    - pathFieldsAndPotential.TruePotential() );
    if( potentialDifference
        > ( 1.0E-1 * tunnelingScaleSquared * tunnelingScaleSquared ) )
    {
      thinWallIsGoodApproximation = false;
      return NAN;
    }

    double const oneOverPotentialDifference( 1.0 / potentialDifference );

    // Following Coleman and adapting to thermal tunneling:
    // Quantum:
    // S_E = pi^2 * R^3 * S_1 - 0.5 * pi^2 * R^4 * epsilon
    // R = 3 S_1 / epsilon
    // S_E = -0.5 pi^2 R^3 S_1 = -13.5 pi^2 S_1^4 epsilon^-3
    // Thermal:
    // S_E = 2 * pi * R^2 * S_1 - 4/3 * pi * R^3 * epsilon
    // R = S_1 / epsilon
    // S_E = 2/3 pi R^2 S_1 = 2/3 S_1^3 epsilon^-2

    double integratedAction( 0.0 );
    double potentialValue( 0.0 );
    double const thinWallStep( 0.05 );
    // The start and end of the integral are neglected as they are proportional
    // to the derivatives of the fields with respect to the auxiliary variable,
    // which are zero at the ends of the integral.
    // We also integrate V(field[referenceFieldIndex])^(-1/2) over
    // field[referenceFieldIndex] to estimate the wall thickness, which is then
    // used for comparison to the bubble radius to validate the thin-wall
    // approximation. Because it will be compared to R / 100, accuracy doesn't
    // matter so much, so midpoint summation should suffice.
    double wallThickness( 0.0 );
    for( double thinWallAuxiliary( thinWallStep );
         thinWallAuxiliary < 1.0;
         thinWallAuxiliary += thinWallStep )
    {
      potentialValue
      = pathFieldsAndPotential.PotentialApproximation( thinWallAuxiliary );
      // debugging:
      std::cout << std::endl << "debugging:"
      << std::endl
      << "thinWallAuxiliary = " << thinWallAuxiliary
      << ", potentialValue = " << potentialValue;
      std::cout << std::endl;
      if( potentialValue > 0.0 )
      {
        integratedAction
        += sqrt( pathFieldsAndPotential.FieldDerivativesSquared(
                                        thinWallAuxiliary ) * potentialValue );
        wallThickness += ( thinWallStep / sqrt( potentialValue ) );
      }
    }
    integratedAction *= ( M_SQRT2 * thinWallStep );
    wallThickness *= pathFieldsAndPotential.FieldDerivatives()[
                             referenceFieldIndex ].CoefficientVector().front();
    // This last *= accounts for the change in integration variable to the
    // field linear in a.

    // debugging:
    std::cout << std::endl << "debugging:"
    << std::endl
    << "integratedAction = " << integratedAction
    << ", oneOverPotentialDifference = " << oneOverPotentialDifference
    << ", wallThickness = " << wallThickness;
    std::cout << std::endl;

    // We return the thin-wall approximation of the bounce action only if the
    // bubble radius is sufficiently large compared to the tunneling scale,
    // which is a good indicator for the validity of the approximation.
    if( ( integratedAction * oneOverPotentialDifference )
        < ( 1.0E+2 * wallThickness ) )
    {
      thinWallIsGoodApproximation = false;
      return NAN;
    }
    thinWallIsGoodApproximation = false;
    if( pathFieldsAndPotential.NonZeroTemperature() )
    {
      double const bounceAction( ( 2.0 * M_PI
                       * integratedAction * integratedAction * integratedAction
                    * oneOverPotentialDifference * oneOverPotentialDifference )
                    / pathFieldsAndPotential.GivenTemperature() );
      // This is at least 10^4 * 2 pi * ( S_1 / ( Q^2 T ) ), so S_3(T)/T is
      // likely to be at least 10^3, leading to a totally negligible
      // tunneling probability...
      return ( ( bounceAction / pathFieldsAndPotential.GivenTemperature() )
               + log( bounceAction ) );
    }
    else
    {
      return ( -13.5 * M_PI * M_PI
                     * integratedAction * integratedAction
                     * integratedAction * integratedAction
                     * oneOverPotentialDifference
                     * oneOverPotentialDifference
                     * oneOverPotentialDifference );
      // This is at least 3^4 * 10^4 * 13.5 pi^2 * ( S_1 / ( Q^3 ) ), so
      // S_4 is likely to be at least 10^6, leading to a totally negligible
      // tunneling probability...
    }
  }*/

  // This sets up the bubble profile, numerically integrates the bounce action
  // over it, and then returns effective bounce action
  // [S_4 or ((S_3(T)/T + ln(S_3(T)))].
  double ModifiedBounceForMinuit::EffectiveBounceAction(
                   PathFieldsAndPotential const& pathFieldsAndPotential ) const
  {
    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "ModifiedBounceForMinuit::EffectiveBounceAction("
    << " pathFieldsAndPotential =" << std::endl
    << pathFieldsAndPotential.AsDebuggingString() << " ) called.";
    std::cout << std::endl;/**/
    BubbleProfile bubbleProfile( pathFieldsAndPotential,
                          ( initialFractionOfShortestLength * shortestLength ),
                                 longestLength );
    std::vector< BubbleRadialValueDescription > const&
    auxiliaryProfile( bubbleProfile.DampedProfile( undershootOvershootAttempts,
                                                   shootingThreshold ) );

    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "Preparing to plot bubble profile.";
    std::cout << std::endl;
    BOL::TwoDimensionalDataPlotter bubblePlotter( "/opt/local/bin/gnuplot",
                                                  "BolBubbleProfile.eps" );
    std::vector< std::string > plotColors;
    plotColors.push_back( "red" );
    plotColors.push_back( "purple" );
    plotColors.push_back( "blue" );
    plotColors.push_back( "green" );
    plotColors.push_back( "orange" );
    size_t const numberOfPlottedFields( std::min( plotColors.size(),
                                                  numberOfFields ) );
    std::vector< BOL::TwoDimensionalDataPlotter::DoublePairVectorWithString >
    dataAndColors( numberOfPlottedFields );
    for( size_t fieldIndex( 0 );
         fieldIndex < numberOfPlottedFields;
         ++fieldIndex )
    {
      dataAndColors[ fieldIndex ].second.assign( plotColors[ fieldIndex ] );
    }
    std::vector< SimplePolynomial > const&
    fieldPath( pathFieldsAndPotential.FieldPath() );
    for( size_t radiusIndex( 0 );
         radiusIndex < auxiliaryProfile.size();
         ++radiusIndex )
    {
      double const
      radialValue( auxiliaryProfile[ radiusIndex ].radialValue );
      for( size_t fieldIndex( 0 );
           fieldIndex < numberOfPlottedFields;
           ++fieldIndex )
      {
        dataAndColors[ fieldIndex ].first.push_back(
                                                   std::make_pair( radialValue,
                                                       fieldPath[ fieldIndex ](
                          auxiliaryProfile[ radiusIndex ].auxiliaryValue ) ) );
      }
    }
    bubblePlotter.plotData( dataAndColors );/**/

    // We have a set of r_i, p(r_i), and dp/dr|_{r=r_i}, and can easily
    // evaluate a set of "bounce action densities" B_i = B(r_i). The numerical
    // integral is then the sum of B_i * [differential volume at r_i], which
    // normally would be
    // B_i * r_i^dampingFactor * [solid angle] * ( r_{i+1} - r_i ).
    // In an attempt to reduce the numerical error, one can instead take
    // 0.5 * (B_i + B_{i+1}) * r_i^dampingFactor
    //     * [solid angle] * ( r_{i+1} - r_i ), or other averages that are
    // formally identical as the difference in radius vanishes. We take the
    // average B(r) over each differential volume:
    // ( 0.5 * (B_i + B_{i+1}) ) * [differential volume]
    // where [differential volume] is
    // ( r_i^dampingFactor * [solid angle] * ( r_{i+1} - r_i ) )
    // if ( r_{i+1} - r_i ) is small compared to r_{i+1} and r_i, or we take
    // [solid angle] * ( r_{i+1}^d - r_i^d )/d where d = (dampingFactor+1) if
    // the difference in radius is not small enough.
    // Since then say B_4 is counted with 2 volumes (with a factor of 0.5 each
    // time), we actually sum up each B_i with the sum of its 2 adjacent
    // volumes, hence we sum up
    // 0.5 * [solid angle] * B_i * r_i^(d-1) * ( r_{i+1} - r_{i-1} )
    // or 0.5 * [solid angle] * B_i * ( r_{i+1}^d - r_{i-1}^d )/d.
    // (Actually, we leave the common factor of 0.5 * [solid angle] until after
    // the end of the loop.)
    // The first and last contributions have to be treated slightly
    // differently, because of the lack of radii at indices beyond the range of
    // auxiliaryProfile.

    double previousRadius( 0.0 );
    double previousVolume( 0.0 );
    double currentRadius( auxiliaryProfile.front().radialValue );
    double currentVolume( currentRadius * currentRadius * currentRadius );
    double nextRadius( auxiliaryProfile[ 1 ].radialValue );
    double nextVolume( nextRadius * nextRadius * nextRadius );
    if( pathFieldsAndPotential.NonZeroTemperature() )
    {
      currentVolume /= 3.0;
      nextVolume /= 3.0;
    }
    else
    {
      currentVolume *= ( 0.25 * currentRadius );
      nextVolume *= ( 0.25 * nextRadius );
    }
    // The bounce action up to the radius of auxiliaryProfile[ 1 ] is given by
    // B_{-1} (the bounce action density at r = 0.0, which has no kinetic
    // contribution because the bubble is smooth at its center by construction)
    // and B_0 as
    // ( 0.5 * ( B_{-1} + B_{0} ) * [volume from r = 0.0 to r_0] )
    // + ( 0.5 * ( B_{0} + B_{1} ) * [volume from r = r_0 to r_1] ).
    // The contribution from B_{1} is taken care of with the 1st iteration of
    // the loop, so the contribution up to the loop is given by
    // ( 0.5 * [solid angle] * B_{-1} * currentVolume )
    // + ( 0.5 * [solid angle] * B_{0} * nextVolume )
    // with the values currently in currentVolume and nextVolume, also with the
    // common factor of 0.5 * [solid angle] being left until after the loop.
    double
    bounceAction( ( currentVolume
                    * pathFieldsAndPotential.PotentialApproximation(
                                    bubbleProfile.AuxiliaryAtBubbleCenter() ) )
                  + ( nextVolume
                      * ( BounceActionDensity( pathFieldsAndPotential,
                                              auxiliaryProfile.front() ) ) ) );

    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "Before loop: currentRadius = " << currentRadius << ", currentVolume = "
    << currentVolume << ", nextRadius = " << nextRadius << ", nextVolume = "
    << nextVolume << ", p(0) = " << bubbleProfile.AuxiliaryAtBubbleCenter()
    << ", B_{-1} = " << pathFieldsAndPotential.PotentialApproximation(
                                     bubbleProfile.AuxiliaryAtBubbleCenter() )
    << ", B_{0} = " << BounceActionDensity( pathFieldsAndPotential,
                                            auxiliaryProfile.front() )
    << ", bounceAction = " << bounceAction;
    std::cout << std::endl;/**/

    for( size_t radiusIndex( 1 );
         radiusIndex < ( auxiliaryProfile.size() - 1 );
         ++radiusIndex )
    {
      previousRadius = currentRadius;
      previousVolume = currentVolume;
      currentRadius = nextRadius;
      currentVolume = nextVolume;
      nextRadius = auxiliaryProfile[ radiusIndex + 1 ].radialValue;
      nextVolume = ( nextRadius * nextRadius * nextRadius );
      if( pathFieldsAndPotential.NonZeroTemperature() )
      {
        nextVolume /= 3.0;
      }
      else
      {
        nextVolume *= ( 0.25 * nextRadius );
      }

      // debugging:
      /**/std::cout << std::endl << "debugging:"
      << std::endl
      << "previousRadius = " << previousRadius
      << ", previousVolume = " << previousVolume
      << ", currentRadius = " << currentRadius
      << ", currentVolume = " << currentVolume
      << ", nextRadius = " << nextRadius
      << ", nextVolume = " << nextVolume
      << ", B_" << radiusIndex << " = "
      << BounceActionDensity( pathFieldsAndPotential,
                              auxiliaryProfile[ radiusIndex ] );
      std::cout << std::endl;/**/

      // B_i * r_i^(d-1) * ( r_{i+1} - r_{i-1} )
      // or B_i * ( r_{i+1}^d - r_{i-1}^d )/d.
      if( ( nextRadius - previousRadius )
          > ( radiusDifferenceThreshold * currentRadius ) )
      {
        bounceAction
        += ( BounceActionDensity( pathFieldsAndPotential,
                                    auxiliaryProfile[ radiusIndex ] )
             * ( nextVolume - previousVolume ) );
      }
      else
      {
        double currentArea( currentRadius * currentRadius );
        if( !(pathFieldsAndPotential.NonZeroTemperature()) )
        {
          currentArea *= currentRadius;
        }
        bounceAction
        += ( BounceActionDensity( pathFieldsAndPotential,
                                    auxiliaryProfile[ radiusIndex ] )
             * ( nextRadius - previousRadius )
             * currentArea );
      }
      // The common factor of 0.5 * [solid angle] is being left until after the
      // loop.
      // debugging:
      /**/std::cout << std::endl << "debugging:"
      << std::endl
      << "bounceAction = " << bounceAction;
      std::cout << std::endl;/**/
    }
    // Now we add the last shell:
    double const currentAuxiliary( auxiliaryProfile.back().auxiliaryValue );
    double kineticTerm( auxiliaryProfile.back().auxiliarySlope );
    kineticTerm *= ( 0.5 * kineticTerm
        * pathFieldsAndPotential.FieldDerivativesSquared( currentAuxiliary ) );
    double const potentialTerm( pathFieldsAndPotential.PotentialApproximation(
                                                          currentAuxiliary ) );
    if( ( nextRadius - currentRadius )
        > ( radiusDifferenceThreshold * currentRadius ) )
    {
      bounceAction
      += ( ( kineticTerm + potentialTerm ) * ( nextVolume - currentVolume ) );
    }
    else
    {
      double currentArea( currentRadius * currentRadius );
      if( !(pathFieldsAndPotential.NonZeroTemperature()) )
      {
        currentArea *= currentRadius;
      }
      bounceAction += ( ( kineticTerm + potentialTerm )
                        * ( nextRadius - currentRadius ) * currentArea );
    }

    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "After last shell, bounceAction = " << bounceAction;
    std::cout << std::endl;/**/

    // If the bubble profile satisfies its equations of motion, at large r, we
    // can ignore the damping term and linearize the fields around the false
    // vacuum, which should be in the form of a quadratic near the minimum.
    // Thus we get that d^2f/dr^2 = d^2V/df^2 f for a field f, which is solved
    // by f being proportional to exp(-r). Therefore the kinetic term decays
    // like e^( 2 - ((2 r)/r_n) ) while the potential term decays as
    // e^( 1 - (r/r_n) ), with r_n being the largest r given by
    // auxiliaryProfile. The integrals given by Mathematica from r_n to
    // infinity are:
    // r^2 e(-r/r_n) -> 5 r_n^3 / e
    // r^3 e(-r/r_n) -> 16 r_n^4 / e
    // r^2 e(-(2r)/r_n) -> (5/4) r_n^3 / e^2
    // r^3 e(-(2r)/r_n) -> (19/8) r_n^4 / e^2
    // Hence the potential term integrates to 5 r_n^3 V(r_n)
    // and 16 r_n^4 V(r_n) for the thermal and quantum cases respectively, and
    // the kinetic term to (5/4) r_n^3 * [kinetic term at r_n] or
    // (19/8) r_n^4 * [kinetic term at r_n] respectively.
    if( pathFieldsAndPotential.NonZeroTemperature() )
    {
      bounceAction += ( 5.0 * nextRadius * nextRadius * nextRadius
                        * ( potentialTerm + ( 0.25 * kineticTerm ) ) );
    }
    else
    {
      bounceAction
      += ( nextRadius * nextRadius * nextRadius * nextRadius
           * ( ( 16.0 * potentialTerm ) + ( 2.375 * kineticTerm ) ) );
    }

    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "After exponents to infinity, bounceAction = " << bounceAction;
    std::cout << std::endl;/**/

    // The common factor of 1/2 is combined with the solid angle of
    // 2 pi^2 (quantum) or 4 pi (thermal):
    if( pathFieldsAndPotential.NonZeroTemperature() )
    {
      bounceAction *= ( 2.0 * M_PI );
      return ( ( bounceAction / pathFieldsAndPotential.GivenTemperature() )
               + log( bounceAction ) );
    }
    else
    {
      return ( bounceAction * ( M_PI * M_PI ) );
    }
  }

} /* namespace VevaciousPlusPlus */
