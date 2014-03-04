/*
 * ConstantFunctionoid.hpp
 *
 *  Created on: Mar 4, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef CONSTANTFUNCTIONOID_HPP_
#define CONSTANTFUNCTIONOID_HPP_

#include "../StandardIncludes.hpp"
#include "ParameterFunctionoid.hpp"

namespace VevaciousPlusPlus
{

  class ConstantFunctionoid : public ParameterFunctionoid
  {
  public:
    ConstantFunctionoid( double const constantValue );
    virtual
    ~ConstantFunctionoid();


    // This does nothing, actually.
    virtual void
    UpdateForNewLogarithmOfScale( double const logarithmOfScale ){}

    // This is mainly for debugging.
    virtual std::string AsString();
  };




  // This is mainly for debugging.
  inline std::string ConstantFunctionoid::AsString()
  {
    std::stringstream returnStream;
    returnStream << "[CONSTANT " << currentValue << "]";
    return std::string( returnStream.str() );
  }

} /* namespace VevaciousPlusPlus */
#endif /* CONSTANTFUNCTIONOID_HPP_ */