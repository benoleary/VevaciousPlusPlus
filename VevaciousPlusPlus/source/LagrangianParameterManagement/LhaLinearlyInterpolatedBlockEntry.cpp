/*
 * LhaLinearlyInterpolatedBlockEntry.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "LagrangianParameterManagement/LhaLinearlyInterpolatedBlockEntry.hpp"

namespace VevaciousPlusPlus
{

  LhaLinearlyInterpolatedBlockEntry::LhaLinearlyInterpolatedBlockEntry(
                                              size_t const indexInValuesVector,
                                        LHPC::SimpleLhaParser const& lhaParser,
                                           std::string const& parameterName ) :
    LhaInterpolatedParameterFunctionoid( indexInValuesVector,
                                          lhaParser,
                                          parameterName ),
    logScalesWithValues(),
    lastIndex( 0 )
  {
    // This constructor is just an initialization list.
  }

  LhaLinearlyInterpolatedBlockEntry::LhaLinearlyInterpolatedBlockEntry(
                       LhaLinearlyInterpolatedBlockEntry const& copySource  ) :
    LhaInterpolatedParameterFunctionoid( copySource ),
    logScalesWithValues( copySource.logScalesWithValues ),
    lastIndex( copySource.lastIndex )
  {
    // This constructor is just an initialization list.
  }

  LhaLinearlyInterpolatedBlockEntry::~LhaLinearlyInterpolatedBlockEntry()
  {
    // This does nothing.
  }


  // This re-assigns the vector of values paired with logarithms of the
  // block's scale according to the current status of the block.
  void LhaLinearlyInterpolatedBlockEntry::UpdateForNewLhaParameters()
  {
    std::list< std::pair< std::string, double > > entriesAtScales;
    (*lhaParser)( parameterName,
                  entriesAtScales,
                  true );
    size_t const numberOfScales( entriesAtScales.size() );

    // First we guard against no block found (in which case the value is set
    // to a flat zero for all scales) or only 1 block found (in which case
    // the value is set to be the read value flat across all scales).
    if( !( numberOfScales > 1 ) )
    {
      logScalesWithValues.resize( 2 );
      lastIndex = 1;
      logScalesWithValues[ 0 ].first = 0.0;
      logScalesWithValues[ 1 ].first = 1.0;

      if( numberOfScales == 0 )
      {
        // If there were no entries with explicit scales, we check for entries
        // in blocks without scales, which will be assumed to be constant over
        // all scales.
        (*lhaParser)( parameterName,
                      entriesAtScales,
                      false );
        if( entriesAtScales.empty() )
        {
          logScalesWithValues[ 0 ].second = 0.0;
        }
        else
        {
          logScalesWithValues[ 0 ].second
          = LHPC::ParsingUtilities::StringToDouble(
                                                entriesAtScales.back().first );
        }
        logScalesWithValues[ 1 ].second = logScalesWithValues[ 0 ].second;
      }
      else
      {
        logScalesWithValues[ 0 ].second
        = LHPC::ParsingUtilities::StringToDouble(
                                               entriesAtScales.front().first );
        logScalesWithValues[ 1 ].second = logScalesWithValues[ 0 ].second;
      }
    }
    else
    {
      // The blocks are ordered as they were read from the SLHA file, which
      // may not necessarily be in ascending order with respect to the scale.
      entriesAtScales.sort( &(FirstPairDotSecondIsLower< std::string >) );
      logScalesWithValues.resize( numberOfScales );
      size_t scaleIndex( 0 );
      std::list< std::pair< std::string, double > >::const_iterator
      listIterator( entriesAtScales.begin() );
      while( scaleIndex < numberOfScales )
      {
        logScalesWithValues[ scaleIndex ].first = log( listIterator->second );
        logScalesWithValues[ scaleIndex++ ].second
        = LHPC::ParsingUtilities::StringToDouble( (listIterator++)->first );
        // Post-increments on last use of each variable. Yey terseness.
      }
      lastIndex = ( numberOfScales - 1 );
    }
  }

  // This is for creating a Python version of the potential.
  std::string LhaLinearlyInterpolatedBlockEntry::PythonParameterEvaluation(
                                            int const indentationSpaces ) const
  {
    std::stringstream stringBuilder;
    stringBuilder << std::setprecision( 12 )
    << PythonIndent( indentationSpaces );

    if( lastIndex == 1 )
    {
      stringBuilder
      << "parameterValues[ " << IndexInValuesVector() << " ] = ( "
      << logScalesWithValues[ 0 ].second << " )";
    }
    else
    {
      for( size_t whichIndex( 1 );
           whichIndex <= lastIndex;
           ++whichIndex )
      {
        // The first iteration should print "if (...):".
        // The subsequent iterations should have newlines before printing.
        // Iterations after the first but before the last should print
        // "elif(...):" after the newline.
        // The last iterations should print "else:" after the newline.
        if( whichIndex > 1 )
        {
          stringBuilder
          << PythonNewlineThenIndent( indentationSpaces ) << "el";
        }
        if( whichIndex < lastIndex )
        {
          stringBuilder
          << "if ( lnQ < " << logScalesWithValues[ whichIndex ].first << " ):";
        }
        else
        {
          stringBuilder << "se:";
        }

        stringBuilder
        << PythonNewlineThenIndent( indentationSpaces + 2 )
        << "parameterValues[ " << IndexInValuesVector() << " ] = ( "
        << logScalesWithValues[ whichIndex ].second << " + ( ( "
        << ( logScalesWithValues[ whichIndex ].second
             - logScalesWithValues[ whichIndex - 1 ].second )
        << " * ( lnQ - " << logScalesWithValues[ whichIndex ].first
        << " ) ) / "
        << ( logScalesWithValues[ whichIndex ].first
             - logScalesWithValues[ whichIndex - 1 ].first )
        << " ) )";
        // The above should print
        // [el]if ( lnQ < [lnQi] ): [or just "else" for the last iteration]
        //   parameterValues[ n ]
        //   = ( [Vi]
        //       + ( ( [Vi-Vbefore] * ( lnQ - lnQi ) )
        //           / [lnQi-lnQbefore] ) )
      }
    }
    return stringBuilder.str();
  }

} /* namespace VevaciousPlusPlus */
