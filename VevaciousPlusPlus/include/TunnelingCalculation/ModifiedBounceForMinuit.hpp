/*
 * ModifiedBounceForMinuit.hpp
 *
 *  Created on: May 13, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef MODIFIEDBOUNCEFORMINUIT_HPP_
#define MODIFIEDBOUNCEFORMINUIT_HPP_

#include "../CommonIncludes.hpp"
#include "Minuit2/FCNBase.h"
#include "Minuit2/MnMigrad.h"
#include "Eigen/Dense"
#include "boost/numeric/odeint/integrate/integrate.hpp"
#include "../PotentialEvaluation.hpp"
#include "PathFromNodes.hpp"
#include "PathFieldsAndPotential.hpp"
#include "SplinePotential.hpp"
#include "BubbleProfile.hpp"

namespace VevaciousPlusPlus
{

  class ModifiedBounceForMinuit : public ROOT::Minuit2::FCNBase
  {
  public:
    ModifiedBounceForMinuit( PotentialFunction const& potentialFunction,
                             size_t const numberOfVaryingPathNodes,
                             size_t const numberOfSplinesInPotential,
                             PotentialMinimum const& falseVacuum,
                             PotentialMinimum const& trueVacuum,
                             double const dsbEvaporationTemperature,
                             size_t const undershootOvershootAttempts = 32,
                             size_t const maximumMultipleOfLongestLength = 16,
                           double const initialFractionOfShortestLength = 0.05,
                             double const minimumScaleSquared = 1.0,
                            double const shootingCloseEnoughThreshold = 0.01 );
    virtual
    ~ModifiedBounceForMinuit();


    // The bounce action is calculated by the following process:
    // 1) The path in field space from the false vacuum to the true vacuum is
    //    decoded from pathParameterization to give the field configuration f
    //    as a function of a path auxiliary variable p, giving f(p) and df/dp.
    //    This is obtained from pathFromNodes.
    // 2) The potential is fitted as a polynomial in p of degree
    //    numberOfSplinesInPotential, giving V(p).
    // 3) If the potential difference between the vacua is small enough, the
    //    thin-wall approximation is tried, but only returned if the resulting
    //    bubble radius turns out to be large enough compared to the wall
    //    thickness.
    // 4) If the thin-wall approximation is not returned, the one-dimensional
    //    bubble equation of motion along p is integrated to get p as a
    //    function of the radial variable r, which is the spatial radius for
    //    three-dimensional actions, or the length of the four-dimensional
    //    Euclidean vector. This gets the correct bubble profile only if the
    //    transverse equations of motion in field space are also satisfied, or
    //    for a modified potential which has additional terms that raise the
    //    energy barrier on the side of the path that has a lower energy
    //    barrier. Hence this bubble profile gives an upper bound on the bounce
    //    action for the unmodified potential, as the modified potential (which
    //    has the same true vacuum and false vacuum and is exactly the same
    //    along the path) cannot have a lower bounce action.
    //    [The p equation of motion has a strange term giving a force
    //    proportional to (dp/dr)^2 which is not a friction term as the sign
    //    is not proportional to dp/dr, rather to d^2f/dp^2. This is because p
    //    is not linear in the distance in field space, so actually this term
    //    behaves a bit like extra kinetic energy because it rolled further
    //    "down the hill" from the true vacuum initially because there is more
    //    field space distance covered by a small change in p if the derivative
    //    is that way, or like extra friction because the approach to the
    //    false vacuum is actually longer in field space distance than in p.
    //    The alternative is to divide up the path into pathResolution segments
    //    and take the path as a series of straight lines at a constant
    //    "velocity" in field space, meaning that the weird pseudo-friction
    //    force in the bubble wall equation of motion in p disappears. This
    //    would require some contortions with linked lists (probably) of
    //    segments to get f(p(r)) and df/dp.]
    // 5) The bounce action along p(r) is numerically integrated and returned.
    virtual double
    operator()( std::vector< double > const& pathParameterization ) const;

    // This implements Up() for FCNBase just to stick to a basic value.
    virtual double Up() const { return 1.0; }

    // This sets up initialParameterization and initialStepSizes to be a
    // straight path in field space with initial step sizes of stepSizeFraction
    // times the differences in the fields between true vacuum and false
    // vacuum. After the node parameters are added to initialParameterization,
    // startingTemperature is appended if it is > 0.0. There is no return value
    // optimization because we cannot be sure that poor physicist users will
    // have access to a C++11-compliant compiler.
    void
    SetUpStraightPathForMinuit( std::vector< double >& initialParameterization,
                                std::vector< double >& initialStepSizes,
                                double const startingTemperature,
                                double const stepSizeFraction = 0.2 ) const;


  protected:
    static double const radiusDifferenceThreshold;

    PotentialFunction const& potentialFunction;
    size_t const numberOfFields;
    size_t referenceFieldIndex;
    PathFromNodes pathFromNodes;
    size_t numberOfSplinesInPotential;
    PotentialMinimum const& falseVacuum;
    PotentialMinimum const& trueVacuum;
    // The vector in field space from falseVacuum to trueVacuum (which are
    // zero-temperature vacua) is given by zeroTemperatureStraightPath.
    std::vector< double > zeroTemperatureStraightPath;
    double zeroTemperatureStraightPathInverseLengthSquared;
    double const falseVacuumPotential;
    double const trueVacuumPotential;
    double const falseVacuumEvaporationTemperature;
    double const tunnelingScaleSquared;
    double shortestLength;
    double longestLength;
    size_t const undershootOvershootAttempts;
    double const initialFractionOfShortestLength;
    double const shootingThreshold;

    // This returns true if pathParameterization.size() is only
    // pathFromNodes.ParameterizationSize() (hence T = 0.0 is implicit) or if
    // pathParameterization.back() <= 0.0; otherwise false is returned, or an
    // exception is thrown if pathParameterization.size() is not
    // ( pathFromNodes.ParameterizationSize() + 1 ).
    bool ZeroTemperatureParameterization(
                     std::vector< double > const& pathParameterization ) const;

    // This turns pathParameterization into a PathFieldsAndPotential, by first
    // checking for a non-zero temperature, then setting up the straight path
    // in field space, and projecting the nodes extracted from
    // pathParameterization onto planes perpendicular to the straight path. A
    // few extra bits of information to do with the temperature are also
    // recorded in the PathFieldsAndPotential object returned.
    PathFieldsAndPotential DecodePathParameters(
                     std::vector< double > const& pathParameterization ) const;

    // This returns the effective bounce action [S_4 or
    // ((S_3(T)/T + ln(S_3(T)))] calculated under the thin-wall approximation.
    // Before returning, it sets thinWallIsGoodApproximation to be true or
    // false depending on whether the thin-wall approximation is a good
    // approximation or not.
    //double
    //ThinWallApproximation( PathFieldsAndPotential const pathFieldsAndPotential,
    //                       bool& thinWallIsGoodApproximation ) const;
    // Actually, I don't think that this is worth using.

    // This evaluates the bounce action density at the given point on the
    // bubble profile.
    double
    BounceActionDensity( PathFieldsAndPotential const& pathFieldsAndPotential,
                      BubbleRadialValueDescription const& profilePoint ) const;

    // This sets up the bubble profile, numerically integrates the bounce
    // action over it, and then returns effective bounce action
    // [S_4 or ((S_3(T)/T + ln(S_3(T)))].
    double EffectiveBounceAction(
                  PathFieldsAndPotential const& pathFieldsAndPotential ) const;

    // This puts a polynomial approximation of the potential along the path
    // given by pathFieldsAndPotential into pathFieldsAndPotential.
    void
    PotentialAlongPath( PathFieldsAndPotential& pathFieldsAndPotential ) const;
  };




  // This returns either S_4 or ( S_3(T)/T + ln(S_3(T)) ), where S_4 is the
  // dimensionless four-dimensional bounce action and S_3(T) the
  // three-dimensional bounce action in units of GeV for the given temperature
  // T, also in GeV. S_4 is returned if T is 0, otherwise the effective bounce
  // action ( S_3(T)/T + ln(S_3(T)) ) is returned. If no temperature is given,
  // T is assumed to be 0.
  // The bounce action is calculated by the following process:
  // 1) The path in field space from the false vacuum to the true vacuum is
  //    decoded from pathParameterization to give the field configuration f
  //    as a function of a path auxiliary variable p, giving f(p) and df/dp.
  //    See the comment above DecodePathParameters for the format of
  //    pathParameterization.
  // 2) The potential is fitted as a polynomial in p of degree
  //    numberOfSplinesInPotential, giving V(p).
  // 3) If the potential difference between the vacua is small enough, the
  //    thin-wall approximation is tried, but only returned if the resulting
  //    bubble radius turns out to be large enough compared to the wall
  //    thickness.
  // 4) If the thin-wall approximation is not returned, the one-dimensional
  //    bubble equation of motion along p is integrated to get p as a
  //    function of the radial variable r, which is the spatial radius for
  //    three-dimensional actions, or the length of the four-dimensional
  //    Euclidean vector. This gets the correct bubble profile only if the
  //    transverse equations of motion in field space are also satisfied, or
  //    for a modified potential which has additional terms that raise the
  //    energy barrier on the side of the path that has a lower energy
  //    barrier. Hence this bubble profile gives an upper bound on the bounce
  //    action for the unmodified potential, as the modified potential (which
  //    has the same true vacuum and false vacuum and is exactly the same
  //    along the path) cannot have a lower bounce action.
  //    [The p equation of motion has a strange term giving a force
  //    proportional to (dp/dr)^2 which is not a friction term as the sign
  //    is not proportional to dp/dr, rather to d^2f/dp^2. This is because p
  //    is not linear in the distance in field space, so actually this term
  //    behaves a bit like extra kinetic energy because it rolled further
  //    "down the hill" from the true vacuum initially because there is more
  //    field space distance covered by a small change in p if the derivative
  //    is that way, or like extra friction because the approach to the
  //    false vacuum is actually longer in field space distance than in p.
  //    The alternative is to divide up the path into pathResolution segments
  //    and take the path as a series of straight lines at a constant
  //    "velocity" in field space, meaning that the weird pseudo-friction
  //    force in the bubble wall equation of motion in p disappears. This
  //    would require some contortions with linked lists (probably) of
  //    segments to get f(p(r)) and df/dp.]
  // 5) The bounce action along p(r) is numerically integrated and returned.
  inline double ModifiedBounceForMinuit::operator()(
                      std::vector< double > const& pathParameterization ) const
  {
    PathFieldsAndPotential
    pathFieldsAndPotential( DecodePathParameters( pathParameterization ) );
    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "about to try to get potential as splines.";
    std::cout << std::endl;/**/
    PotentialAlongPath( pathFieldsAndPotential );
    // We return a thin-wall approximation if appropriate:
    /*bool thinWallIsGoodApproximation( false );
    double bounceAction( ThinWallApproximation( pathFieldsAndPotential,
                                               thinWallIsGoodApproximation ) );
    if( thinWallIsGoodApproximation )
    {
      // debugging:
      *//*std::cout << std::endl << "debugging:"
      << std::endl
      << "thin-wall bounce = " << bounceAction
      << "but not returning yet, to test whether shooting matches this.";
      std::cout << std::endl;*//*
      //return bounceAction;
    }*/
    // Actually, we don't bother.
    // If we didn't return bounceAction already, it means that the we go on to
    // try undershooting/overshooting.
    return EffectiveBounceAction( pathFieldsAndPotential );
  }

  // This evaluates the bounce action density at the given point on the
  // bubble profile.
  inline double ModifiedBounceForMinuit::BounceActionDensity(
                          PathFieldsAndPotential const& pathFieldsAndPotential,
                       BubbleRadialValueDescription const& profilePoint ) const
  {
    double const currentAuxiliary( profilePoint.auxiliaryValue );
    double kineticTerm( profilePoint.auxiliarySlope );
    kineticTerm *= ( 0.5 * kineticTerm
        * pathFieldsAndPotential.FieldDerivativesSquared( currentAuxiliary ) );
    return
    ( kineticTerm
      + pathFieldsAndPotential.PotentialApproximation( currentAuxiliary ) );
  }

  // This sets up initialParameterization and initialStepSizes to be a
  // straight path in field space with initial step sizes of stepSizeFraction
  // times the differences in the fields between true vacuum and false
  // vacuum. After the node parameters are added to initialParameterization,
  // startingTemperature is appended if it is > 0.0. There is no return value
  // optimization because we cannot be sure that poor physicist users will have
  // access to a C++11-compliant compiler.
  inline void ModifiedBounceForMinuit::SetUpStraightPathForMinuit(
                                std::vector< double >& initialParameterization,
                                       std::vector< double >& initialStepSizes,
                                              double const startingTemperature,
                                          double const stepSizeFraction ) const
  {
    size_t parameterizationSize( pathFromNodes.ParameterizationSize() );
    if( startingTemperature > 0.0 )
    {
      ++parameterizationSize;
    }
    initialParameterization.assign( parameterizationSize,
                                    0.0 );
    initialStepSizes.resize( initialParameterization.size() );
    if( startingTemperature > 0.0 )
    {
      initialParameterization.back() = startingTemperature;
      initialStepSizes.back() = ( stepSizeFraction * startingTemperature );
    }
    pathFromNodes.InitialStepsForMinuit( initialStepSizes,
                                         zeroTemperatureStraightPath,
                                         stepSizeFraction );
  }

  // This returns true if pathParameterization.size() is only
  // pathFromNodes.ParameterizationSize() (hence T = 0.0 is implicit) or if
  // pathParameterization.back() <= 0.0; otherwise false is returned, or an
  // exception is thrown if pathParameterization.size() is not
  // ( pathFromNodes.ParameterizationSize() + 1 ).
  inline bool ModifiedBounceForMinuit::ZeroTemperatureParameterization(
                      std::vector< double > const& pathParameterization ) const
  {
    if( pathParameterization.size() == pathFromNodes.ParameterizationSize() )
    {
      return true;
    }
    if( pathParameterization.size()
        == ( pathFromNodes.ParameterizationSize() + 1 ) )
    {
      return ( pathParameterization.back() <= 0.0 );
    }
    std::stringstream errorStream;
    errorStream << "ModifiedBounceForMinuit::operator() was given a wrong"
    << " number of parameters: " << pathParameterization.size()
    << "; it should have been given "
    << pathFromNodes.NumberOfVaryingPathNodes() << " nodes each of "
    << pathFromNodes.NumberOfParameterizationFields()
    << " fields, optionally 1 extra parameter for the temperature at the"
    << " end, so " << pathFromNodes.ParameterizationSize() << " or "
    << ( pathFromNodes.ParameterizationSize() + 1 ) << " parameters.";
    throw std::range_error( errorStream.str() );
  }

} /* namespace VevaciousPlusPlus */
#endif /* MODIFIEDBOUNCEFORMINUIT_HPP_ */
