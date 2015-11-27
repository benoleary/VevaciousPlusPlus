/*
 * LesHouchesAccordBlockEntryManager.hpp
 *
 *  Created on: Oct 29, 2015
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef LESHOUCHESACCORDBLOCKENTRYMANAGER_HPP_
#define LESHOUCHESACCORDBLOCKENTRYMANAGER_HPP_

#include "CommonIncludes.hpp"
#include "LHPC/ParsingUtilities.hpp"
#include "LagrangianParameterManager.hpp"
#include "LhaInterpolatedParameterFunctionoid.hpp"
#include "LhaLinearlyInterpolatedBlockEntry.hpp"
#include "LhaPolynomialFitBlockEntry.hpp"

namespace VevaciousPlusPlus
{

  class LesHouchesAccordBlockEntryManager : public LagrangianParameterManager
  {
  public:
    static std::string const blockNameSeparationCharacters;

    LesHouchesAccordBlockEntryManager( std::string const& validBlocksString,
                                       std::string const& minimumScaleType,
                                       std::string const& minimumScaleArgument,
                                       std::string const& fixedScaleType,
                                       std::string const& fixedScaleArgument,
                                       std::string const& maximumScaleType,
                                     std::string const& maximumScaleArgument );
    LesHouchesAccordBlockEntryManager(
                                 std::set< std::string > const& validBlocksSet,
                                       std::string const& minimumScaleType,
                                       std::string const& minimumScaleArgument,
                                       std::string const& fixedScaleType,
                                       std::string const& fixedScaleArgument,
                                       std::string const& maximumScaleType,
                                     std::string const& maximumScaleArgument );
    LesHouchesAccordBlockEntryManager( std::string const& xmlFileName );
    virtual ~LesHouchesAccordBlockEntryManager();


    // This returns the value of the requested parameter at the requested scale
    // (exp( logarithmOfScale )) as an alternative to adding it to the list of
    // parameters evaluated by ParameterValues. It is almost certain to
    // be much slower (as it searches for the appropriate block and then makes
    // a new functionoid) if used to obtain parameters repeatedly for
    // different scales at the same parameter point, but is more efficient for
    // the rest of the execution of Vevacious, which depends on the speed of
    // evaluation of the potential, if there are some parameters which do not
    // need to be evaluated for the potential but still depend on Lagrangian
    // parameters, for example when evaluating the VEVs of the DSB vacuum for
    // the parameter point.
    virtual double OnceOffParameter( std::string const& parameterName,
                                     double const logarithmOfScale ) const
    { return OnceOffBlockEntry( parameterName,
                                logarithmOfScale ); }

    // This checks to see if the parameter name has already been registered,
    // and if so, returns true paired with the index to its functionoid. If
    // not, then it returns the result of RegisterUnregisteredParameter.
    virtual std::pair< bool, size_t >
    RegisterParameter( std::string const& parameterName );

    // This fills the given vector with the values of the Lagrangian parameters
    // in activeInterpolatedParameters evaluated at the given scale, ordered so
    // that the indices given out by RegisterParameter correctly match the
    // parameter with its element in the vector. There may be gaps if a derived
    // class has registered parameters other than those in
    // activeInterpolatedParameters.
    virtual void ParameterValues( double const logarithmOfScale,
                              std::vector< double >& destinationVector ) const;

    // This should return the minimum scale which is appropriate for evaluating
    // the Lagrangian parameters at the current parameter point.
    virtual double MinimumEvaluationScale() const
    { return GetScale( minimumScaleType,
                       minimumScaleArgument ); }

    // This returns the lowest scale above zero (not including zero) which was
    // in the set of scales read for fixedScaleDefiningBlock last read in.
    virtual double AppropriateSingleFixedScale() const
    { return GetScale( fixedScaleType,
                       fixedScaleArgument ); }

    // This should return the maximum scale which is appropriate for evaluating
    // the Lagrangian parameters at the current parameter point.
    virtual double MaximumEvaluationScale() const
    { return GetScale( maximumScaleType,
                       maximumScaleArgument ); }

    // This puts all variables with index brackets into a consistent form,
    // putting all those which are a valid block name followed by index
    // brackets into uppercase, to account for SLHA block name case
    // insensitivity.
    virtual std::string
    FormatVariable( std::string const& variableToFormat ) const;

    // This writes a function in the form
    // def LagrangianParameters( lnQ ): return ...
    // to return an array of the values of the Lagrangian parameters evaluated
    // at the scale exp(lnQ) (i.e. the logarithm of the scale is given as the
    // argument), in the order in which a call to ParametersAtScale would
    // return them internal to this C++ code.
    virtual std::string ParametersAsPython() const;

    // This is mainly for debugging.
    virtual std::string AsDebuggingString() const;


  protected:
    // It is probably better to use a set of SlhaLinearlyInterpolatedBlockEntry
    // objects rather than a set of SlhaPolynomialFitBlockEntry objects.
    typedef LhaLinearlyInterpolatedBlockEntry LhaBlockEntryInterpolator;

    // It is necessary to keep track of the number of active parameters
    // separately from the size of activeParametersToIndices because several
    // strings could point to the same parameter functionoid, and separately
    // from the size of activeInterpolatedParameters because derived classes
    // might have other functionoids stored elsewhere, which could already have
    // been registered in activeParametersToIndices.
    size_t numberOfDistinctActiveParameters;
    std::map< std::string, size_t > activeParametersToIndices;
    std::vector< LhaBlockEntryInterpolator* > referenceSafeActiveParameters;
    std::vector< LhaBlockEntryInterpolator > referenceUnsafeActiveParameters;
    std::set< std::string > validBlocks;
    LHPC::SlhaSimplisticInterpreter lhaParser;
    std::string minimumScaleType;
    std::string minimumScaleArgument;
    std::string fixedScaleType;
    std::string fixedScaleArgument;
    std::string maximumScaleType;
    std::string maximumScaleArgument;


    // This parses validBlocksString into a set of valid block names and
    // inserts them into validBlocks.
    void ParseValidBlocks( std::string const& validBlocksString );

    // This sets parameterName to map to newParameter, increments
    // numberOfDistinctActiveParameters, and returns true paired with the index
    // given by newParameter.
    std::pair< bool, size_t >
    RegisterNewParameter( LhaSourcedParameterFunctionoid const& newParameter,
                          std::string const& parameterName );

    // This updates the SLHA file parser with the file with name given by
    // newInput and then, once the blocks have been updated, tells each
    // parameter in referenceSafeActiveParameters to update itself, and sets up
    // referenceUnsafeActiveParameters as a contiguous array of
    // LhaBlockEntryInterpolator objects copied from the objects pointed at by
    // the pointers in referenceSafeActiveParameters.
    virtual void PrepareNewParameterPoint( std::string const& newInput );

    // This finds the block name part of parameterName, returns true if the
    // block name is found in validBlocks.
    bool RefersToValidBlock( std::string const& parameterName ) const
    { return ( validBlocks.find( BlockNamePart( parameterName ) )
               != validBlocks.end() ); }

    // This adds a new LhaBlockEntryInterpolator for the given parameter
    // to activeInterpolatedParameters and activeParametersToIndices, and
    // returns a reference to it.
    LhaInterpolatedParameterFunctionoid const&
    CreateNewBlockEntry( std::string const& parameterName );

    // This checks to see if the parameter name corresponds to an entry in a
    // valid LHA block, and if so, adds an interpolation functionoid to the set
    // of functionoids and returns true paired with the index of the new
    // functionoid. Otherwise false is returned, paired with -1, rolling over
    // to the maximum value of size_t as it is unsigned. It can be over-ridden
    // to allow for checks for special cases first, for example.
    virtual std::pair< bool, size_t >
    RegisterUnregisteredParameter( std::string const& parameterName )
    { return ( RefersToValidBlock( parameterName ) ?
               RegisterNewParameter( CreateNewBlockEntry( parameterName ),
                                     parameterName ) :
               std::pair< bool, size_t >( false,
                                          -1 ) ); }

    // This creates a functionoid and evaluates it at the scale given by
    // logarithmOfScale and returns its result.
    double OnceOffBlockEntry( std::string const& parameterName,
                              double const logarithmOfScale ) const;

    // This returns a string which is the concatenated set of strings from
    // parameter functionoids giving their Python evaluations.
    virtual std::string
    ParametersInPythonFunction( unsigned int const indentationSpaces ) const;

    // This ensures that the given parameter exists as a block interpolator
    // functionoid in activeInterpolatedParameters and
    // activeParametersToIndices, and returns its index. It also updates
    // validBlocks based on the block name and throws an exception if the name
    // was not in the correct format of block name followed by square brackets
    // containing nothing, an index, or several indices.
    size_t RegisterBlockEntry( std::string const& parameterName );

    // This returns the appropriate scale for the given type of evaluation and
    // its argument.
    double GetScale( std::string const& evaluationType,
                     std::string const& evaluationArgument ) const;

    // This returns the substring of parameterName up to '[', converted
    // completely to uppercase, or an empty string if there was no '['.
    std::string BlockNamePart( std::string const& parameterName ) const;
  };





  // This checks to see if the parameter name has already been registered,
  // and if so, returns true paired with the index to its functionoid. If
  // not, then it returns the result of RegisterUnregisteredParameter.
  inline std::pair< bool, size_t >
  LesHouchesAccordBlockEntryManager::RegisterParameter(
                                             std::string const& parameterName )
  {
    std::string const nameAfterFormat( FormatVariable( parameterName ) );
    std::map< std::string, size_t >::const_iterator
    alreadyExistsResult( activeParametersToIndices.find( nameAfterFormat ) );
    if( alreadyExistsResult != activeParametersToIndices.end() )
    {
      return std::pair< bool, size_t >( true, alreadyExistsResult->second );
    }
    else
    {
      return RegisterUnregisteredParameter( nameAfterFormat );
    }
  }

  // This fills the given vector with the values of the Lagrangian parameters
  // in activeInterpolatedParameters evaluated at the given scale, ordered so
  // that the indices given out by RegisterParameter correctly match the
  // parameter with its element in the vector. There may be gaps if a derived
  // class has registered parameters other than those in
  // activeInterpolatedParameters.
  inline void LesHouchesAccordBlockEntryManager::ParameterValues(
                                                 double const logarithmOfScale,
                               std::vector< double >& destinationVector ) const
  {
    destinationVector.resize( numberOfDistinctActiveParameters );
    for( std::vector< LhaBlockEntryInterpolator >::const_iterator
         parameterInterpolator( referenceUnsafeActiveParameters.begin() );
         parameterInterpolator != referenceUnsafeActiveParameters.end();
         ++parameterInterpolator )
    {
      destinationVector[ parameterInterpolator->IndexInValuesVector() ]
      = (*parameterInterpolator)( logarithmOfScale );
    }
  }

  // This writes a function in the form
  // def LagrangianParameters( lnQ ): return ...
  // to return an array of the values of the Lagrangian parameters evaluated
  // at the scale exp(lnQ) (i.e. the logarithm of the scale is given as the
  // argument), in the order in which a call to ParametersAtScale would
  // return them internal to this C++ code.
  inline std::string
  LesHouchesAccordBlockEntryManager::ParametersAsPython() const
  {
    std::stringstream stringBuilder;
    stringBuilder
    << "def FirstIfNonzeroOtherwiseSecond( firstValue, secondValue ):\n"
    << "  if ( firstValue != 0.0 ):\n"
    << "    return firstValue\n"
    << "  else:\n"
    << "    return secondValue\n"
    << "\n"
    << "def LagrangianParameters( lnQ ):\n"
    << "  parameterValues = [ 0.0 ] * " << numberOfDistinctActiveParameters
    << "\n";
    stringBuilder << ParametersInPythonFunction( 2 );
    stringBuilder << "  return parameterValues\n\n";
    return stringBuilder.str();
  }

  // This parses validBlocksString into a set of valid block names and
  // inserts them into validBlocks.
  inline void LesHouchesAccordBlockEntryManager::ParseValidBlocks(
                                         std::string const& validBlocksString )
  {
    size_t wordStart( validBlocksString.find_first_not_of(
                                             blockNameSeparationCharacters ) );
    size_t wordEnd( 0 );
    // If there are any more chars in validBlocks that are not in
    // blockSeparators, we have at least one substring to add.
    while( wordStart != std::string::npos )
    {
      wordEnd
      = validBlocksString.find_first_of( blockNameSeparationCharacters,
                                         wordStart );
      validBlocks.insert( validBlocksString.substr( wordStart,
                                                   ( wordEnd - wordStart ) ) );
      wordStart
      = validBlocksString.find_first_not_of( blockNameSeparationCharacters,
                                             wordEnd );
    }
  }

  // This sets parameterName to map to newParameter, increments
  // numberOfDistinctActiveParameters, and returns true paired with the index
  // given by newParameter.
  inline std::pair< bool, size_t >
  LesHouchesAccordBlockEntryManager::RegisterNewParameter(
                           LhaSourcedParameterFunctionoid const& newParameter,
                                             std::string const& parameterName )
  {
    ++numberOfDistinctActiveParameters;
    activeParametersToIndices[ parameterName ]
    = newParameter.IndexInValuesVector();
    return std::pair< bool, size_t >( true,
                                      newParameter.IndexInValuesVector() );
  }

  // This updates the SLHA file parser with the file with name given by
  // newInput and then, once the blocks have been updated, tells each
  // parameter in referenceSafeActiveParameters to update itself, and sets up
  // referenceUnsafeActiveParameters as a contiguous array of
  // LhaBlockEntryInterpolator objects copied from the objects pointed at by
  // the pointers in referenceSafeActiveParameters.
  inline void LesHouchesAccordBlockEntryManager::PrepareNewParameterPoint(
                                                  std::string const& newInput )
  {
    lhaParser.readFile( newInput );
    size_t const
    numberOfActiveInterpolators( referenceSafeActiveParameters.size() );
    referenceSafeActiveParameters[ 0 ]->UpdateForNewLhaParameters();
    referenceUnsafeActiveParameters.resize( numberOfActiveInterpolators,
                                       *(referenceSafeActiveParameters[ 0 ]) );

    for( size_t parameterIndex( 1 );
         parameterIndex < numberOfActiveInterpolators;
         ++parameterIndex )
    {
      referenceSafeActiveParameters[
                                parameterIndex ]->UpdateForNewLhaParameters();
      referenceUnsafeActiveParameters[ parameterIndex ]
      = *(referenceSafeActiveParameters[ parameterIndex ]);
    }
  }

  // This adds a new LhaBlockEntryInterpolator for the given parameter
  // to activeInterpolatedParameters and activeParametersToIndices, and
  // returns a reference to it.
  inline LhaInterpolatedParameterFunctionoid const&
  LesHouchesAccordBlockEntryManager::CreateNewBlockEntry(
                                             std::string const& parameterName )
  {
    referenceSafeActiveParameters.push_back( new LhaBlockEntryInterpolator(
                                              numberOfDistinctActiveParameters,
                                                                     lhaParser,
                                                             parameterName ) );
    return *(referenceSafeActiveParameters.back());
  }

  // This creates a functionoid and evaluates it at the scale given by
  // logarithmOfScale and returns its result.
  inline double LesHouchesAccordBlockEntryManager::OnceOffBlockEntry(
                                              std::string const& parameterName,
                                          double const logarithmOfScale ) const
  {
    if( !(RefersToValidBlock( parameterName )) )
    {
      std::stringstream errorBuilder;
      errorBuilder
      << "LesHouchesAccordBlockEntryManager::OnceOffParameter could not"
      << " find a block corresponding to that in \"" << parameterName << "\".";
      throw std::runtime_error( errorBuilder.str() );
    }
    LhaBlockEntryInterpolator parameterInterpolator( 0,
                                                     lhaParser,
                                                     parameterName );
    parameterInterpolator.UpdateForNewLhaParameters();
    return parameterInterpolator( logarithmOfScale );
  }

  // This returns a string which is the concatenated set of strings from
  // parameter functionoids giving their Python evaluations.
  inline std::string
  LesHouchesAccordBlockEntryManager::ParametersInPythonFunction(
                                   unsigned int const indentationSpaces ) const
  {
    std::stringstream stringBuilder;
    for( std::vector< LhaBlockEntryInterpolator >::const_iterator
         activeParameter( referenceUnsafeActiveParameters.begin() );
         activeParameter < referenceUnsafeActiveParameters.end();
         ++activeParameter )
    {
      stringBuilder
      << activeParameter->PythonParameterEvaluation( indentationSpaces )
      << "\n";
    }
    return stringBuilder.str();
  }

  // This ensures that the given parameter exists as a block interpolator
  // functionoid in activeInterpolatedParameters and activeParametersToIndices,
  // and returns its index. It also updates validBlocks based on the block name
  // and throws an exception if the name was not in the correct format of block
  // name followed by square brackets containing nothing, an index, or several
  // indices.
  inline size_t
  LesHouchesAccordBlockEntryManager::RegisterBlockEntry(
                                             std::string const& parameterName )
  {
    std::string const nameAfterFormat( FormatVariable( parameterName ) );
    std::map< std::string, size_t >::const_iterator
    alreadyExistsResult( activeParametersToIndices.find( nameAfterFormat ) );
    if( alreadyExistsResult != activeParametersToIndices.end() )
    {
      return alreadyExistsResult->second;
    }
    else
    {
      if( nameAfterFormat.find( '[' ) == std::string::npos )
      {
        throw std::runtime_error( parameterName
                  + " is not in the format of block name followed by [...]!" );
      }
      return RegisterNewParameter( CreateNewBlockEntry( nameAfterFormat ),
                                   nameAfterFormat ).second;
    }
  }

  // This returns the appropriate scale for the given type of evaluation and
  // its argument.
  inline double LesHouchesAccordBlockEntryManager::GetScale(
                                             std::string const& evaluationType,
                                  std::string const& evaluationArgument ) const
  {
    if( evaluationType == "FixedNumber" )
    {
      return atof( evaluationArgument.c_str() );
    }
    else if( evaluationType == "BlockLowestScale" )
    {
      return lhaParser.getLowestScale( evaluationArgument );
    }
    else if( evaluationType == "BlockEntry" )
    {
      return LHPC::ParsingUtilities::StringToDouble(
                                             lhaParser( evaluationArgument ) );
    }
    else if( evaluationType == "SqrtAbs" )
    {
      return std::sqrt( std::abs( LHPC::ParsingUtilities::StringToDouble(
                                         lhaParser( evaluationArgument ) ) ) );
    }
    else
    {
      throw std::runtime_error( evaluationType
                                  + " is not a valid scale evaluation type!" );
    }
  }

  // This returns the substring of parameterName up to '[', converted
  // completely to uppercase, or an empty string if there was no '['.
  inline std::string LesHouchesAccordBlockEntryManager::BlockNamePart(
                                       std::string const& parameterName ) const
  {
    size_t openBracket( parameterName.find( '[' ) );
    if( openBracket == std::string::npos )
    {
      return "";
    }
    std::string blockName( parameterName.substr( 0,
                                                 openBracket ) );
    LHPC::ParsingUtilities::TransformToUppercase( blockName );
    return blockName;
  }

} /* namespace VevaciousPlusPlus */

#endif /* LESHOUCHESACCORDBLOCKENTRYMANAGER_HPP_ */
