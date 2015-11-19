/*
 * BounceAlongPathWithThreshold.hpp
 *
 *  Created on: Jul 1, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef BOUNCEALONGPATHWITHTHRESHOLD_HPP_
#define BOUNCEALONGPATHWITHTHRESHOLD_HPP_

#include "CommonIncludes.hpp"
#include "PotentialEvaluation/PotentialFunction.hpp"
#include "BounceActionEvaluation/PathParameterization/TunnelPath.hpp"
#include "BounceActionEvaluation/PathParameterization/LinearSplineThroughNodes.hpp"
#include "BounceActionEvaluation/BubbleProfile.hpp"
#include "BounceActionEvaluation/OneDimensionalPotentialAlongPath.hpp"
#include "../BounceActionTunneler.hpp"
#include "BounceActionEvaluation/BouncePathFinder.hpp"
#include "BounceActionEvaluation/BounceActionCalculator.hpp"

namespace VevaciousPlusPlus
{
  // This class takes memory management ownership of the components given to
  // the constructor as pointers! It'd be nice to use std::unique_ptrs, but we
  // are stubbornly sticking to allowing non-C++11-compliant compilers.
  class BounceAlongPathWithThreshold : public BounceActionTunneler
  {
  public:
    BounceAlongPathWithThreshold(
                           std::vector< BouncePathFinder* > const& pathFinders,
                                BounceActionCalculator* const actionCalculator,
                TunnelingCalculator::TunnelingStrategy const tunnelingStrategy,
                                  double const survivalProbabilityThreshold,
                               unsigned int const thermalIntegrationResolution,
                                  unsigned int const temperatureAccuracy,
                                  unsigned int const pathPotentialResolution,
                                  double const vacuumSeparationFraction );
    virtual ~BounceAlongPathWithThreshold();


  protected:
    std::vector< BouncePathFinder* > pathFinders;
    BounceActionCalculator* actionCalculator;
    unsigned int thermalIntegrationResolution;
    unsigned int const pathPotentialResolution;

    // This returns either the dimensionless bounce action integrated over four
    // dimensions (for zero temperature) or the dimensionful bounce action
    // integrated over three dimensions (for non-zero temperature) for
    // tunneling from falseVacuum to trueVacuum at temperature
    // tunnelingTemperature, or an upper bound if the upper bound drops below
    // actionThreshold during the course of the calculation. The vacua are
    // assumed to already be the minima at tunnelingTemperature.
    virtual double BounceAction( PotentialFunction& potentialFunction,
                                 PotentialMinimum const& falseVacuum,
                                 PotentialMinimum const& trueVacuum,
                                 double const tunnelingTemperature );

    // This sets thermalSurvivalProbability by numerically integrating up to
    // the critical temperature for tunneling to be possible from T = 0 unless
    // the integral already passes a threshold, and sets
    // dominantTemperatureInGigaElectronVolts to be the temperature with the
    // lowest survival probability.
    virtual void
    ContinueThermalTunneling( PotentialFunction& potentialFunction,
                              PotentialMinimum const& falseVacuum,
                              PotentialMinimum const& trueVacuum,
                             double const potentialAtOriginAtZeroTemperature );


    // This returns either the dimensionless bounce action integrated over four
    // dimensions (for zero temperature) or the dimensionful bounce action
    // integrated over three dimensions (for non-zero temperature) for
    // tunneling from falseVacuum to trueVacuum at temperature
    // tunnelingTemperature, or an upper bound if the upper bound drops below
    // actionThreshold during the course of the calculation. The vacua are
    // assumed to already be the minima at tunnelingTemperature.
    double BoundedBounceAction( PotentialFunction& potentialFunction,
                                PotentialMinimum const& falseVacuum,
                                PotentialMinimum const& trueVacuum,
                                double const tunnelingTemperature,
                                double const actionThreshold,
                                double const requiredVacuumSeparationSquared );
  };




  // This returns either the dimensionless bounce action integrated over four
  // dimensions (for zero temperature) or the dimensionful bounce action
  // integrated over three dimensions (for non-zero temperature) for
  // tunneling from falseVacuum to trueVacuum at temperature
  // tunnelingTemperature, or an upper bound if the upper bound drops below
  // actionThreshold during the course of the calculation. The vacua are
  // assumed to already be the minima at tunnelingTemperature.
  inline double BounceAlongPathWithThreshold::BounceAction(
                                          PotentialFunction& potentialFunction,
                                           PotentialMinimum const& falseVacuum,
                                            PotentialMinimum const& trueVacuum,
                                            double const tunnelingTemperature )
  {
    double actionThreshold( lnOfThermalIntegrationFactor );
    // We assume that the threshold should be the naive threshold for thermal
    // tunneling, and then take the T = 0 version if the temperature is not a
    // valid temperature (i.e. we treat T < 0 and T "= NaN" as T = 0.0).
    if( !(tunnelingTemperature > 0.0 ) )
    {
      double const squareRootOfSolitonicFactor(
                potentialFunction.ScaleSquaredRelevantToTunneling( falseVacuum,
                                                                trueVacuum ) );
      double const solitonicFactorTimesFourVolume( squareRootOfSolitonicFactor
                                                  * squareRootOfSolitonicFactor
                                    * fourVolumeOfKnownUniverseOverGevFourth );
      actionThreshold = ( log( -solitonicFactorTimesFourVolume
                               / log( survivalProbabilityThreshold ) ) );
    }
    return BoundedBounceAction( potentialFunction,
                                falseVacuum,
                                trueVacuum,
                                tunnelingTemperature,
                                actionThreshold,
                               ( vacuumSeparationFractionSquared
                              * falseVacuum.SquareDistanceTo( trueVacuum ) ) );
  }

} /* namespace VevaciousPlusPlus */
#endif /* BOUNCEALONGPATHWITHTHRESHOLD_HPP_ */
