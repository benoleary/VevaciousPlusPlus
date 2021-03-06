/*
 * VevaciousPlusPlus.cpp
 *
 *  Created on: Feb 25, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "VevaciousPlusPlus.hpp"

namespace VevaciousPlusPlus
{
  // This is the constructor for those who know what they are doing, to allow
  // the main function of the program to decide the components and pass them
  // in to the constructor, allowing for custom components without having to
  // edit the VevaciousPlusPlus files. Those wishing to just use the default
  // possibilities can use the other constructor, which takes the name of an
  // initialization file, and then creates the components based on the data
  // in that file. Since this constructor leaves ownedPotentialFunction,
  // ownedPotentialMinimizer, and ownedPotentialMinimizer all as
  // NULL, and no other function sets them, there should be no problem with
  // the destructor calling delete on these pointers, as they do not get set to
  // point at the addresses of the given components.
  VevaciousPlusPlus::VevaciousPlusPlus( PotentialMinimizer& potentialMinimizer,
                                   TunnelingCalculator& tunnelingCalculator ) :
    lagrangianParameterManager( &(potentialMinimizer.GetPotentialFunction(
                                          ).GetLagrangianParameterManager()) ),
    ownedLagrangianParameterManager( NULL ),
    ownedPotentialFunction( NULL ),
    potentialMinimizer( &potentialMinimizer ),
    ownedPotentialMinimizer( NULL ),
    tunnelingCalculator( &tunnelingCalculator ),
    ownedTunnelingCalculator( NULL ),
    warningMessagesFromConstructor(),
    resultsFromLastRunAsXml( "<!-- No results yet. -->" ),
    warningMessagesFromLastRun()
  {
    // This constructor is just an initialization list.
  }

  // This is the constructor that we expect to be used in normal use: it reads
  // in an initialization file in XML with name given by initializationFileName
  // and then assembles the appropriate components for the objects. The body of
  // the constructor is just a lot of statements reading in XML elements and
  // creating new instances of components. It'd be great to be able to use
  // std::unique_ptrs but we're sticking to allowing non-C++11-compliant
  // compilers.
  VevaciousPlusPlus::VevaciousPlusPlus(
                                  std::string const& initializationFileName ) :
    lagrangianParameterManager( NULL ),
    ownedLagrangianParameterManager( NULL ),
    ownedPotentialFunction( NULL ),
    potentialMinimizer( NULL ),
    ownedPotentialMinimizer( NULL ),
    tunnelingCalculator( NULL ),
    ownedTunnelingCalculator( NULL ),
    warningMessagesFromConstructor(),
    resultsFromLastRunAsXml( "<!-- No results yet. -->" ),
    warningMessagesFromLastRun()
  {
    WarningLogger::SetWarningRecord( &warningMessagesFromConstructor );
    std::string potentialFunctionInitializationFilename( "error" );
    std::string potentialMinimizerInitializationFilename( "error" );
    std::string tunnelingCalculatorInitializationFilename( "error" );
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.OpenRootElementOfFile( initializationFileName );
    while( xmlParser.ReadNextElement() )
    {
      if( xmlParser.CurrentName() == "PotentialFunctionInitializationFile" )
      {
        potentialFunctionInitializationFilename
        = xmlParser.TrimmedCurrentBody();
      }
      else if( xmlParser.CurrentName()
               == "PotentialMinimizerInitializationFile" )
      {
        potentialMinimizerInitializationFilename
        = xmlParser.TrimmedCurrentBody();
      }
      else if( xmlParser.CurrentName()
               == "TunnelingCalculatorInitializationFile" )
      {
        tunnelingCalculatorInitializationFilename
        = xmlParser.TrimmedCurrentBody();
      }
    }

    // Now we know the overall picture of what components to set up.
    // ALL SUB-COMPONENTS FOR potentialMinimizer AND tunnelingCalculator ARE
    // MEMORY-MANAGED BY THE COMPONENTS THEMSELVES! The VevaciousPlusPlus
    // destructor only deletes ownedTunnelingCalculator,
    // ownedPotentialMinimizer, ownedPotentialFunction, and
    // ownedLagrangianParameterManager, while this constructor is allocating
    // much more memory than that (through the various functions called within
    // functions). Everything would be clear if we restricted ourselves to
    // requiring a C++11-compliant compiler, as then we could have the
    // constructors use std::unique_ptrs, but, alas, we're sticking to C++98.
    FullPotentialDescription
    fullPotentialDescription( CreateFullPotentialDescription(
                                   potentialFunctionInitializationFilename ) );
    lagrangianParameterManager = ownedLagrangianParameterManager
    = fullPotentialDescription.first;
    ownedPotentialFunction = fullPotentialDescription.second;
    potentialMinimizer = ownedPotentialMinimizer
    = CreatePotentialMinimizer( *ownedPotentialFunction,
                                potentialMinimizerInitializationFilename );
    tunnelingCalculator = ownedTunnelingCalculator
    = CreateTunnelingCalculator( tunnelingCalculatorInitializationFilename );
    WarningLogger::SetWarningRecord( NULL );
  }

  VevaciousPlusPlus::~VevaciousPlusPlus()
  {
    delete ownedTunnelingCalculator;
    delete ownedPotentialMinimizer;
    delete ownedPotentialFunction;
    delete ownedLagrangianParameterManager;
  }

    //This reads in a Slha block and passes it over to LagrangianParameterManager updating 
    //the given parameters in the blockset object. The parameter values are given in a vector 
    // and the dimension is given, with 1 is given the block is read as a list, if n is given
    // the vector is read as a n x n matrix in sequential order row by row. 
    
  void VevaciousPlusPlus::ReadLhaBlock( std::string const& uppercaseBlockName, 
  										double const scale, 
  										std::vector<std::pair<int,double>> const& parameters, 
  										int const dimension )
  {
   lagrangianParameterManager->NewLhaBlock( uppercaseBlockName, scale, parameters, dimension  );
  }



  // This runs the point parameterized by newInput, which for the default
  // case gives the name of a file with the input parameters, but could in
  // principle itself contain all the necessary parameters.
  void VevaciousPlusPlus::RunPoint( std::string const& newInput )
  {
    warningMessagesFromLastRun.clear();
    WarningLogger::SetWarningRecord( &warningMessagesFromLastRun );
    time_t runStartTime;
    time_t runEndTime;
    time_t stageStartTime;
    time_t stageEndTime;
    time( &runStartTime );
    std::cout
    << std::endl
    << "Running \"" << newInput << "\" starting at "
    << ctime( &runStartTime );
    std::cout << std::endl;

    time( &stageStartTime );
    lagrangianParameterManager->NewParameterPoint( newInput );
    potentialMinimizer->FindMinima( 0.0 );
    time( &stageEndTime );
    std::cout << std::endl
    << "Minimization of potential took " << difftime( stageEndTime,
                                                      stageStartTime )
    << " seconds, finished at " << ctime( &stageEndTime );
    std::cout << std::endl;

    if( potentialMinimizer->DsbVacuumIsMetastable() )
    {
      time( &stageStartTime );
      tunnelingCalculator->CalculateTunneling(
                                    potentialMinimizer->GetPotentialFunction(),
                                               potentialMinimizer->DsbVacuum(),
                                           potentialMinimizer->PanicVacuum() );
      time( &stageEndTime );
      std::cout << std::endl
      << "Tunneling calculation took " << difftime( stageEndTime,
                                                    stageStartTime )
      << " seconds, finished at " << ctime( &stageEndTime );
      std::cout << std::endl;
      std::cout << std::endl;
    }

    WarningLogger::SetWarningRecord( NULL );
    PrepareResultsAsXml();
    std::cout
    << std::endl
    << "Result:" << std::endl << resultsFromLastRunAsXml;
    std::cout << std::endl;

    time( &runEndTime );
    std::cout << std::endl
    << "Total running time was " << difftime( runEndTime,
                                              runStartTime )
    << " seconds, finished at " << ctime( &runEndTime );
    std::cout << std::endl;
    if( newInput == "internal" ){lagrangianParameterManager->ClearParameterPoint(); }
  }
  
  // This writes the results as an SLHA file.
  void
  VevaciousPlusPlus::AppendResultsToLhaFile( std::string const& lhaFilename,
                                             bool const writeWarnings )
  {
    std::fstream outputFile( lhaFilename.c_str() );
    if( !(outputFile.good()) )
    {
      std::stringstream errorBuilder;
      errorBuilder
      << "Could not open \"" << lhaFilename << "\" to append results.";
      throw std::runtime_error( errorBuilder.str() );
    }
    long endPosition( outputFile.seekg( 0,
                                        std::ios::end ).tellg() );
    while( static_cast< char >( outputFile.seekg( --endPosition ).peek() )
           == '\n' )
    {
      // this loop just brings the get pointer back to the char before the
      // last '\n'.
    }
    outputFile.seekp( ++endPosition );
    // the put pointer is now about to overwrite the 1st '\n' of the sequence
    // of '\n' characters ending the file.
    std::time_t currentTime( time( NULL ) );
    outputFile << "\n"
    "BLOCK VEVACIOUSSTABILITY # Results from VevaciousPlusPlus\n"
    "# version " << VersionInformation::CurrentVersion() << ", documented in "
    << VersionInformation::CurrentCitation()
    << "\n"
    "# Results written " << std::string( ctime( &currentTime ) )
    << "# [index] [verdict int]\n"
    "  1  ";
    if( potentialMinimizer->DsbVacuumIsStable() )
    {
      outputFile << "1  # Stable DSB vacuum\n";
    }
    else
    {
      outputFile << "0  # Metastable DSB vacuum\n";
    }
    outputFile << "BLOCK VEVACIOUSZEROTEMPERATURE # Results at T = 0\n"
    "# [index] [verdict float]\n";
    if( tunnelingCalculator->QuantumSurvivalProbability() >= 0.0 )
    {
      outputFile <<  "  1  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                            tunnelingCalculator->QuantumSurvivalProbability() )
      << "  # Probability of DSB vacuum surviving 4.3E17 seconds\n";
      outputFile << "  2  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                              tunnelingCalculator->QuantumLifetimeInSeconds() )
      << "  # Tunneling time out of DSB vacuum in seconds\n"
      "  3  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                     tunnelingCalculator->LogOfMinusLogOfQuantumProbability() )
      << "  # L = ln(-ln(P)), => P = e^(-e^L)\n";
    }
    else
    {
      outputFile << "  1  " << LHPC::ParsingUtilities::FormatNumberForSlha(
          tunnelingCalculator->QuantumSurvivalProbability() )
      << "  # Not calculated: ignore this number\n"
      "  2  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                              tunnelingCalculator->QuantumLifetimeInSeconds() )
      << "  # Not calculated: ignore this number\n"
      "  3  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                     tunnelingCalculator->LogOfMinusLogOfQuantumProbability() )
      << "  # Not calculated: ignore this number\n";
    }
    outputFile << "BLOCK VEVACIOUSNONZEROTEMPERATURE # Results at T != 0\n"
    "# [index] [verdict float]\n";
    if( tunnelingCalculator->ThermalSurvivalProbability() >= 0.0 )
    {
      outputFile <<  "  1  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                            tunnelingCalculator->ThermalSurvivalProbability() )
      << "  # Probability of DSB vacuum surviving thermal tunneling\n";
      outputFile << "  2  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                tunnelingCalculator->DominantTemperatureInGigaElectronVolts() )
      << "  # Dominant tunneling temperature in GeV\n"
      "  3  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                     tunnelingCalculator->LogOfMinusLogOfThermalProbability() )
      << "  # L = ln(-ln(P)), => P = e^(-e^L)\n";
    }
    else
    {
      outputFile << "  1  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                            tunnelingCalculator->ThermalSurvivalProbability() )
      << "  # Not calculated: ignore this number\n"
      "  2  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                 tunnelingCalculator->DominantTemperatureInGigaElectronVolts() )
      << "  # Not calculated: ignore this number\n"
      "  3  " << LHPC::ParsingUtilities::FormatNumberForSlha(
                     tunnelingCalculator->LogOfMinusLogOfThermalProbability() )
      << "  # Not calculated: ignore this number\n";
    }
    outputFile
    << "BLOCK VEVACIOUSFIELDNAMES # Field names for each index\n"
    "# [index] [field name in \"\"]\n";
    std::vector< std::string > const&
    fieldNames( potentialMinimizer->GetPotentialFunction().FieldNames() );
    for( size_t fieldIndex( 0 );
         fieldIndex < fieldNames.size();
         ++fieldIndex )
    {
      outputFile << ' ' << std::setw( 2 ) << fieldIndex << "  \""
      << fieldNames[ fieldIndex ] << "\"\n";
    }
    outputFile << "BLOCK VEVACIOUSDSBVACUUM # VEVs for DSB vacuum in GeV\n"
    "# [index] [field VEV in GeV]\n";
    std::vector< double > const&
    dsbFields( potentialMinimizer->DsbVacuum().FieldConfiguration() );
    for( size_t fieldIndex( 0 );
         fieldIndex < fieldNames.size();
         ++fieldIndex )
    {
      outputFile << ' ' << std::setw( 2 ) << fieldIndex << "  "
      << LHPC::ParsingUtilities::FormatNumberForSlha( dsbFields[ fieldIndex ] )
      << "  # " << fieldNames[ fieldIndex ] << "\n";
    }
    outputFile << "BLOCK VEVACIOUSPANICVACUUM # ";
    if( potentialMinimizer->DsbVacuumIsMetastable() )
    {
      outputFile << "VEVs for panic vacuum in GeV\n";
    }
    else
    {
      outputFile << "Stable DSB vacuum => repeating DSB VEVs\n";
    }
    outputFile << "# [index] [field VEV in GeV]\n";
    std::vector< double > const*
    panicFields( &(potentialMinimizer->DsbVacuum().FieldConfiguration()) );
    if( potentialMinimizer->DsbVacuumIsMetastable() )
    {
      panicFields = &(potentialMinimizer->PanicVacuum().FieldConfiguration());
    }
    for( size_t fieldIndex( 0 );
         fieldIndex < fieldNames.size();
         ++fieldIndex )
    {
      outputFile << ' ' << std::setw( 2 ) << fieldIndex << "  "
      << LHPC::ParsingUtilities::FormatNumberForSlha(
                                                 (*panicFields)[ fieldIndex ] )
      << "  # " << fieldNames[ fieldIndex ] << "\n";
    }
    outputFile << "BLOCK VEVACIOUSWARNINGS # ";
    std::vector< std::string > const
    warningMessagesToReport( WarningMessagesToReport() );
    if( warningMessagesToReport.empty() )
    {
      outputFile << "   0  No warnings\n";
    }
    else
    {
      for( size_t messageIndex( 0 );
           messageIndex < warningMessagesToReport.size();
           ++messageIndex )
      {
        outputFile << "  " << std::setw(2) << messageIndex << "  "
        << warningMessagesToReport[ messageIndex ] << '#' << "\n";
      }
    }
    std::cout << std::endl << "Wrote results in SLHA format at end of file \""
    << lhaFilename << "\"." << std::endl;
  }

  // This creates a new LagrangianParameterManager and a new
  // PotentialFunction according to the XML elements in the file given by
  // potentialFunctionInitializationFilename and returns pointers to them.
  VevaciousPlusPlus::FullPotentialDescription
  VevaciousPlusPlus::CreateFullPotentialDescription(
                   std::string const& potentialFunctionInitializationFilename )
  {
    LesHouchesAccordBlockEntryManager*
    createdLagrangianParameterManager( NULL );
    PotentialFromPolynomialWithMasses* createdPotentialFunction( NULL );
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.OpenRootElementOfFile( potentialFunctionInitializationFilename );
    std::string lagrangianParameterManagerClass( "error" );
    std::string lagrangianParameterManagerArguments( "error" );
    std::string potentialFunctionClass( "error" );
    std::string potentialFunctionArguments( "error" );

    // The root element of this file should have child elements
    // <LagrangianParameterManagerClass> and <PotentialFunctionClass>.
    while( xmlParser.ReadNextElement() )
    {
      ReadClassAndArguments( xmlParser,
                             "LagrangianParameterManagerClass",
                             lagrangianParameterManagerClass,
                             lagrangianParameterManagerArguments );
      ReadClassAndArguments( xmlParser,
                             "PotentialFunctionClass",
                             potentialFunctionClass,
                             potentialFunctionArguments );
    }
    createdLagrangianParameterManager
    = CreateLagrangianParameterManager( lagrangianParameterManagerClass,
                                        lagrangianParameterManagerArguments );
    createdPotentialFunction = CreatePotentialFunction( potentialFunctionClass,
                                                    potentialFunctionArguments,
                                          *createdLagrangianParameterManager );
    return FullPotentialDescription( createdLagrangianParameterManager,
                                     createdPotentialFunction );
  }

  // This creates a new LagrangianParameterManager based on the given
  // arguments and returns a pointer to it.
  LesHouchesAccordBlockEntryManager*
  VevaciousPlusPlus::CreateLagrangianParameterManager(
                                                std::string const& classChoice,
                                      std::string const& constructorArguments )
  {
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    std::string scaleAndBlockFilename( "error" );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "ScaleAndBlockFile",
                                     scaleAndBlockFilename );
    }
    if( classChoice == "SlhaCompatibleWithSarahManager" )
    {
      return new SlhaCompatibleWithSarahManager( scaleAndBlockFilename );
    }
    else if( classChoice == "SlhaBlocksWithSpecialCasesManager" )
    {
      return new SlhaBlocksWithSpecialCasesManager( scaleAndBlockFilename );
    }
    else if( classChoice == "LesHouchesAccordBlockEntryManager" )
    {
      return new LesHouchesAccordBlockEntryManager( scaleAndBlockFilename );
    }
	else if( classChoice == "SARAHManager" )
    {
      return new SARAHManager( scaleAndBlockFilename );
    }
    else
    {
      std::stringstream errorBuilder;
      errorBuilder << "Read \"" << classChoice
      << "\" as a Lagrangian parameter manager class, but this is not one of"
      << " the known types (\"LesHouchesAccordBlockEntryManager\" or"
      << " \"SlhaBlocksWithSpecialCasesManager\" or"
      << " \"SlhaCompatibleWithSarahManager\" or \"SARAHManager\").";
      throw std::runtime_error( errorBuilder.str() );
    }
  }

  // This creates a new PotentialFunction based on the given arguments and
  // returns a pointer to it.
  PotentialFromPolynomialWithMasses*
  VevaciousPlusPlus::CreatePotentialFunction( std::string const& classChoice,
                                       std::string const& constructorArguments,
                       LagrangianParameterManager& lagrangianParameterManager )
  {
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    std::string modelFilename( "error" );
    double assumedPositiveOrNegativeTolerance( 1.0 );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "ModelFile",
                                     modelFilename );
      InterpretElementIfNameMatches( xmlParser,
                                     "AssumedPositiveOrNegativeTolerance",
                                     assumedPositiveOrNegativeTolerance );
    }
    if( classChoice == "FixedScaleOneLoopPotential" )
    {
      return new FixedScaleOneLoopPotential( modelFilename,
                                            assumedPositiveOrNegativeTolerance,
                                             lagrangianParameterManager );
    }
    else if( classChoice == "RgeImprovedOneLoopPotential" )
    {
      return new RgeImprovedOneLoopPotential( modelFilename,
                                            assumedPositiveOrNegativeTolerance,
                                              lagrangianParameterManager );
    }
    else
    {
      std::stringstream errorStream;
      errorStream
      << "<PotentialClass> was not a recognized class! The only options"
      << " currently valid are \"FixedScaleOneLoopPotential\" and"
      << " \"RgeImprovedOneLoopPotential\".";
      throw std::runtime_error( errorStream.str() );
    }
  }

  // This creates a new GradientFromStartingPoints based on the given
  // arguments and returns a pointer to it.
  GradientFromStartingPoints*
  VevaciousPlusPlus::CreateGradientFromStartingPoints(
                          PotentialFromPolynomialWithMasses& potentialFunction,
                                      std::string const& constructorArguments )
  {
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    std::string startingPointFinderClass( "error" );
    std::string startingPointFinderArguments( "error" );
    std::string gradientMinimizerClass( "error" );
    std::string gradientMinimizerArguments( "error" );
    double extremumSeparationThresholdFraction( 0.05 );
    double nonDsbRollingToDsbScalingFactor( 4.0 );
    // The <ConstructorArguments> for this class should have child elements
    // <StartingPointFinderClass> and <GradientMinimizerClass>, and
    // optionally <ExtremumSeparationThresholdFraction> and
    // <NonDsbRollingToDsbScalingFactor>.
    while( xmlParser.ReadNextElement() )
    {
      ReadClassAndArguments( xmlParser,
                             "StartingPointFinderClass",
                             startingPointFinderClass,
                             startingPointFinderArguments );
      ReadClassAndArguments( xmlParser,
                             "GradientMinimizerClass",
                             gradientMinimizerClass,
                             gradientMinimizerArguments );
      InterpretElementIfNameMatches( xmlParser,
                                     "ExtremumSeparationThresholdFraction",
                                     extremumSeparationThresholdFraction );
      InterpretElementIfNameMatches( xmlParser,
                                     "NonDsbRollingToDsbScalingFactor",
                                     nonDsbRollingToDsbScalingFactor );
    }
    StartingPointFinder*
    startingPointFinder( CreateStartingPointFinder( potentialFunction,
                                                    startingPointFinderClass,
                                              startingPointFinderArguments ) );
    GradientMinimizer*
    gradientMinimizer( CreateGradientMinimizer( potentialFunction,
                                                gradientMinimizerClass,
                                                gradientMinimizerArguments ) );
    return new GradientFromStartingPoints( potentialFunction,
                                           startingPointFinder,
                                           gradientMinimizer,
                                           extremumSeparationThresholdFraction,
                                           nonDsbRollingToDsbScalingFactor );
  }

  // This creates a new PolynomialAtFixedScalesSolver based on the given
  // arguments and returns a pointer to it.
  PolynomialAtFixedScalesSolver*
  VevaciousPlusPlus::CreatePolynomialAtFixedScalesSolver(
                    PotentialFromPolynomialWithMasses const& potentialFunction,
                                      std::string const& constructorArguments )
  {
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    unsigned int numberOfScales( 10 );
    bool returnOnlyPolynomialMinima( false );
    std::string polynomialSystemSolverClass( "error" );
    std::string polynomialSystemSolverArguments( "error" );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "NumberOfScales",
                                     numberOfScales );
      InterpretElementIfNameMatches( xmlParser,
                                     "ReturnOnlyPolynomialMinima",
                                     returnOnlyPolynomialMinima );
      ReadClassAndArguments( xmlParser,
                             "PolynomialSystemSolver",
                             polynomialSystemSolverClass,
                             polynomialSystemSolverArguments );
    }
    PolynomialSystemSolver*
    polynomialSystemSolver( CreatePolynomialSystemSolver(
                                                 polynomialSystemSolverClass,
                                         polynomialSystemSolverArguments ) );
    return new PolynomialAtFixedScalesSolver(
                                   potentialFunction.PolynomialApproximation(),
                             potentialFunction.GetLagrangianParameterManager(),
                                              polynomialSystemSolver,
                                              numberOfScales,
                                              returnOnlyPolynomialMinima,
                                  potentialFunction.NumberOfFieldVariables() );
  }

  // This puts the content of the current element of xmlParser into
  // contentDestination, interpreted as a bool represented by
  // case-insensitive "yes/no" or "y/n" or "true/false" or "t/f" or "0/1",
  // if the element's name matches elementName. If the element content
  // doesn't match any valid input, contentDestination is left untouched.
  void VevaciousPlusPlus::InterpretElementIfNameMatches(
                                    LHPC::RestrictedXmlParser const& xmlParser,
                                                std::string const& elementName,
                                                     bool& contentDestination )
  {
    if( xmlParser.CurrentName() == elementName )
    {
      std::string contentString( xmlParser.TrimmedCurrentBody() );
      LHPC::ParsingUtilities::TransformToLowercase( contentString );
      if( ( contentString == "yes" )
          ||
          ( contentString == "y" )
          ||
          ( contentString == "true" )
          ||
          ( contentString == "t" )
          ||
          ( contentString == "1" ) )
      {
        contentDestination = true;
      }
      else if( ( contentString == "no" )
               ||
               ( contentString == "n" )
               ||
               ( contentString == "false" )
               ||
               ( contentString == "f" )
               ||
               ( contentString == "0" ) )
      {
        contentDestination = false;
      }
    }
  }

  // This creates a new MinuitPotentialMinimizer based on the given arguments
  // and returns a pointer to it.
  MinuitPotentialMinimizer* VevaciousPlusPlus::CreateMinuitPotentialMinimizer(
                                    PotentialFunction const& potentialFunction,
                                      std::string const& constructorArguments )
  {
    double errorFraction( 0.1 );
    double errorMinimum( 1.0 );
    unsigned int minuitStrategy( 1 );
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "InitialStepSizeFraction",
                                     errorFraction );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinimumInitialStepSize",
                                     errorMinimum );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinuitStrategy",
                                     minuitStrategy );
    }
    return new MinuitPotentialMinimizer( potentialFunction,
                                         errorFraction,
                                         errorMinimum,
                                         minuitStrategy );
  }

  // This creates a new CosmoTransitionsRunner based on the given arguments
  // and returns a pointer to it.
  CosmoTransitionsRunner* VevaciousPlusPlus::CreateCosmoTransitionsRunner(
                                      std::string const& constructorArguments )
  {
    // The <ConstructorArguments> for this class should have child elements
    // <TunnelingStrategy>, <SurvivalProbabilityThreshold>,
    // <CriticalTemperatureAccuracy>, <EvaporationBarrierResolution>,
    // <PathToCosmotransitions>, <PathResolution>, <MaxInnerLoops>, and
    // <MaxOuterLoops>.
    std::string tunnelingStrategy( "ThermalThenQuantum" );
    double survivalProbabilityThreshold( 0.1 );
    unsigned int thermalStraightPathFitResolution( 5 );
    unsigned int temperatureAccuracy( 7 );
    std::string pathToCosmotransitions( "./cosmoTransitions/" );
    unsigned int resolutionOfDsbVacuum( 20 );
    double vacuumSeparationFraction( 0.2 );
    unsigned int maxInnerLoops( 10 );
    unsigned int maxOuterLoops( 10 );
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "TunnelingStrategy",
                                     tunnelingStrategy );
      InterpretElementIfNameMatches( xmlParser,
                                     "SurvivalProbabilityThreshold",
                                     survivalProbabilityThreshold );
      InterpretElementIfNameMatches( xmlParser,
                                     "ThermalActionResolution",
                                     thermalStraightPathFitResolution );
      InterpretElementIfNameMatches( xmlParser,
                                     "CriticalTemperatureAccuracy",
                                     temperatureAccuracy );
      InterpretElementIfNameMatches( xmlParser,
                                     "PathToCosmotransitions",
                                     pathToCosmotransitions );
      InterpretElementIfNameMatches( xmlParser,
                                     "PathResolution",
                                     resolutionOfDsbVacuum );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinimumVacuumSeparationFraction",
                                     vacuumSeparationFraction );
      InterpretElementIfNameMatches( xmlParser,
                                     "MaxInnerLoops",
                                     maxInnerLoops );
      InterpretElementIfNameMatches( xmlParser,
                                     "MaxOuterLoops",
                                     maxOuterLoops );
    }
    CheckSurvivalProbabilityThreshold( survivalProbabilityThreshold );

    return new CosmoTransitionsRunner(
                               InterpretTunnelingStrategy( tunnelingStrategy ),
                                       survivalProbabilityThreshold,
                                       temperatureAccuracy,
                                       pathToCosmotransitions,
                                       resolutionOfDsbVacuum,
                                       maxInnerLoops,
                                       maxOuterLoops,
                                       thermalStraightPathFitResolution,
                                       vacuumSeparationFraction );
  }

  // This interprets the given string as the appropriate element of the
  // TunnelingCalculator::TunnelingStrategy enum.
  TunnelingCalculator::TunnelingStrategy
  VevaciousPlusPlus::InterpretTunnelingStrategy(
                                               std::string& tunnelingStrategy )
  {
    if( ( tunnelingStrategy == "DefaultTunneling" )
        ||
        ( tunnelingStrategy == "ThermalThenQuantum" ) )
    {
      return TunnelingCalculator::ThermalThenQuantum;
    }
    else if( tunnelingStrategy == "QuantumThenThermal" )
    {
      return TunnelingCalculator::QuantumThenThermal;
    }
    else if( tunnelingStrategy == "JustThermal" )
    {
      return TunnelingCalculator::JustThermal;
    }
    else if( tunnelingStrategy == "JustQuantum" )
    {
      return TunnelingCalculator::JustQuantum;
    }
    else if( tunnelingStrategy == "NoTunneling" )
    {
      return TunnelingCalculator::NoTunneling;
    }
    std::stringstream errorBuilder;
    errorBuilder << "\"" << tunnelingStrategy << "\" is not a valid tunneling"
    << " strategy. Valid options are \"NoTunneling\", \"JustQuantum\","
    << " \"JustThermal\", \"ThermalThenQuantum\", or \"QuantumThenThermal\".";
    throw std::runtime_error( errorBuilder.str() );
  }

  // This creates a new CosmoTransitionsRunner based on the given arguments
  // and returns a pointer to it.
  BounceAlongPathWithThreshold*
  VevaciousPlusPlus::CreateBounceAlongPathWithThreshold(
                                      std::string const& constructorArguments )
  {
    std::string tunnelPathFinders( "" );
    std::string bouncePotentialFitClass( "BubbleShootingOnSpline" );
    std::string bouncePotentialFitArguments( "" );
    std::string tunnelingStrategy( "ThermalThenQuantum" );
    double survivalProbabilityThreshold( 0.1 );
    unsigned int thermalIntegrationResolution( 5 );
    unsigned int temperatureAccuracy( 7 );
    unsigned int resolutionOfPathPotential( 100 );
    double vacuumSeparationFraction( 0.2 );

    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "TunnelingStrategy",
                                     tunnelingStrategy );
      InterpretElementIfNameMatches( xmlParser,
                                     "SurvivalProbabilityThreshold",
                                     survivalProbabilityThreshold );
      InterpretElementIfNameMatches( xmlParser,
                                     "ThermalActionResolution",
                                     thermalIntegrationResolution );
      InterpretElementIfNameMatches( xmlParser,
                                     "CriticalTemperatureAccuracy",
                                     temperatureAccuracy );
      InterpretElementIfNameMatches( xmlParser,
                                     "PathResolution",
                                     resolutionOfPathPotential );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinimumVacuumSeparationFraction",
                                     vacuumSeparationFraction );
      ReadClassAndArguments( xmlParser,
                             "BouncePotentialFit",
                             bouncePotentialFitClass,
                             bouncePotentialFitArguments );
      InterpretElementIfNameMatches( xmlParser,
                                     "TunnelPathFinders",
                                     tunnelPathFinders );
    }

    CheckSurvivalProbabilityThreshold( survivalProbabilityThreshold );

    std::vector< BouncePathFinder* >
    pathFinders( CreateBouncePathFinders( tunnelPathFinders ) );
    if( pathFinders.empty() )
    {
      std::stringstream errorStream;
      errorStream
      << "<TunnelPathFinders> produced no valid tunnel-path-finding"
      << " objects!";
      throw std::runtime_error( errorStream.str() );
    }

    BounceActionCalculator* bounceActionCalculator(
                       CreateBounceActionCalculator( bouncePotentialFitClass,
                                               bouncePotentialFitArguments ) );

    return new BounceAlongPathWithThreshold( pathFinders,
                                             bounceActionCalculator,
                               InterpretTunnelingStrategy( tunnelingStrategy ),
                                             survivalProbabilityThreshold,
                                             thermalIntegrationResolution,
                                             temperatureAccuracy,
                                             resolutionOfPathPotential,
                                             vacuumSeparationFraction );
  }

  // This parses the XMl of tunnelPathFinders to construct a set of
  // BouncePathFinder instances, filling pathFinders with pointers to them.
  std::vector< BouncePathFinder* > VevaciousPlusPlus::CreateBouncePathFinders(
                                         std::string const& tunnelPathFinders )
  {
    std::vector< BouncePathFinder* > pathFinders;
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( tunnelPathFinders );
    std::string classChoice( "" );
    std::string constructorArguments( "" );
    while( xmlParser.ReadNextElement() )
    {
      classChoice.clear();
      constructorArguments.clear();
      ReadClassAndArguments( xmlParser,
                             "PathFinder",
                             classChoice,
                             constructorArguments );
      if( !( classChoice.empty()
             ||
             constructorArguments.empty() ) )
      {
        if( classChoice == "MinuitOnPotentialOnParallelPlanes" )
        {
          pathFinders.push_back( CreateMinuitOnPotentialOnParallelPlanes(
                                                      constructorArguments ) );
        }
        else if( classChoice == "MinuitOnPotentialPerpendicularToPath" )
        {
          pathFinders.push_back( CreateMinuitOnPotentialPerpendicularToPath(
                                                      constructorArguments ) );
        }
        else
        {
          std::stringstream errorStream;
          errorStream
          << "<PathFinder> was not a recognized class! The only options"
          << " currently valid are \"MinuitOnPotentialOnParallelPlanes\" or"
          << " \"MinuitOnPotentialPerpendicularToPath\".";
          throw std::runtime_error( errorStream.str() );
        }
      }
    }
    return pathFinders;
  }

  // This parses arguments from constructorArguments and uses them to
  // construct a MinuitOnPotentialPerpendicularToPath instance to use to try
  // to extremize the bounce action.
  MinuitOnPotentialPerpendicularToPath*
  VevaciousPlusPlus::CreateMinuitOnPotentialPerpendicularToPath(
                                      std::string const& constructorArguments )
  {
    // The <ConstructorArguments> for this class should have child elements
    // <NumberOfPathSegments>, <MinuitStrategy> and <MinuitTolerance>.
    unsigned int numberOfPathSegments( 100 );
    unsigned int numberOfAllowedWorsenings( 3 );
    double convergenceThresholdFraction( 0.05 );
    double minuitDampingFraction( 0.75 );
    std::string neighborDisplacementWeightsString( "0.5, 0.25" );
    unsigned int minuitStrategy( 1 );
    double minuitToleranceFraction( 0.5 );
    LHPC::RestrictedXmlParser xmlParser;
    xmlParser.LoadString( constructorArguments );
    while( xmlParser.ReadNextElement() )
    {
      InterpretElementIfNameMatches( xmlParser,
                                     "NumberOfPathSegments",
                                     numberOfPathSegments );
      InterpretElementIfNameMatches( xmlParser,
                                     "NumberOfAllowedWorsenings",
                                     numberOfAllowedWorsenings );
      InterpretElementIfNameMatches( xmlParser,
                                     "ConvergenceThresholdFraction",
                                     convergenceThresholdFraction );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinuitDampingFraction",
                                     minuitDampingFraction );
      InterpretElementIfNameMatches( xmlParser,
                                     "NeighborDisplacementWeights",
                                     neighborDisplacementWeightsString );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinuitStrategy",
                                     minuitStrategy );
      InterpretElementIfNameMatches( xmlParser,
                                     "MinuitTolerance",
                                     minuitToleranceFraction );
    }
    std::vector< std::string >
    weightsStrings( LHPC::ParsingUtilities::SplitBySubstrings(
                                             neighborDisplacementWeightsString,
                                                               " \t\n,;" ) );
    std::vector< double > neighborDisplacementWeights;
    for( std::vector< std::string >::const_iterator
         weightString( weightsStrings.begin() );
         weightString != weightsStrings.end();
         ++weightString )
    {
      neighborDisplacementWeights.push_back(
                     LHPC::ParsingUtilities::StringToDouble( *weightString ) );
    }

    return new MinuitOnPotentialPerpendicularToPath( numberOfPathSegments,
                                                     numberOfAllowedWorsenings,
                                                  convergenceThresholdFraction,
                                                     minuitDampingFraction,
                                                   neighborDisplacementWeights,
                                                     minuitStrategy,
                                                     minuitToleranceFraction );
  }

  // This prepares the results in XML format, stored in resultsAsXml;
  void VevaciousPlusPlus::PrepareResultsAsXml()
  {
    std::stringstream xmlBuilder;
    xmlBuilder << "  <StableOrMetastable>\n"
    << "    ";
    if( potentialMinimizer->DsbVacuumIsMetastable() )
    {
      xmlBuilder << "meta";
    }
    std::vector< std::string > const&
    fieldNames( potentialMinimizer->GetPotentialFunction().FieldNames() );
    xmlBuilder << "stable\n"
    << "  </StableOrMetastable>\n"
    << potentialMinimizer->DsbVacuum().AsVevaciousXmlElement( "DsbVacuum",
                                                              fieldNames )
    << "\n";
    if( potentialMinimizer->DsbVacuumIsMetastable() )
    {
      xmlBuilder
      << potentialMinimizer->PanicVacuum().AsVevaciousXmlElement(
                                                                 "PanicVacuum",
                                                                  fieldNames )
      << "\n";
      if( tunnelingCalculator->QuantumSurvivalProbability() >= 0.0 )
      {
        xmlBuilder << "  <ZeroTemperatureDsbSurvival>\n"
        << "    <DsbSurvivalProbability>\n"
        << "      " << tunnelingCalculator->QuantumSurvivalProbability()
        << "\n"
        << "    </DsbSurvivalProbability>\n"
        << "    <LogOfMinusLogOfDsbSurvival>\n"
        << "      " << tunnelingCalculator->LogOfMinusLogOfQuantumProbability()
        << " <!-- this = ln(-ln(P)), so P = e^(-e^this)) -->\n"
        << "    </LogOfMinusLogOfDsbSurvival>\n"
        << "    <DsbLifetime>\n"
        << "      " << tunnelingCalculator->QuantumLifetimeInSeconds()
        << " <!-- in seconds; age of observed Universe is 4.3E+17s -->\n"
        << "    </DsbLifetime>\n"
        "  </ZeroTemperatureDsbSurvival>\n";
      }
      else
      {
        xmlBuilder << "  <!-- Survival probability at zero temperature not"
        << " calculated. -->\n";
      }
      if( tunnelingCalculator->ThermalSurvivalProbability() >= 0.0 )
      {
        xmlBuilder << "  <NonZeroTemperatureDsbSurvival>\n"
        << "    <DsbSurvivalProbability>\n"
        << "      " << tunnelingCalculator->ThermalSurvivalProbability()
        << "\n"
        << "    </DsbSurvivalProbability>\n"
        << "    <LogOfMinusLogOfDsbSurvival>\n"
        << "      " << tunnelingCalculator->LogOfMinusLogOfThermalProbability()
        << " <!-- this = ln(-ln(P)), so P = e^(-e^this)) --> \n"
        << "    </LogOfMinusLogOfDsbSurvival>\n"
        << "    <DominantTunnelingTemperature>\n"
        << "      "
        << tunnelingCalculator->DominantTemperatureInGigaElectronVolts()
        << " <!-- in GeV -->\n"
        << "    </DominantTunnelingTemperature>\n"
        << "  </NonZeroTemperatureDsbSurvival>\n";
      }
      else
      {
        xmlBuilder << "  <!-- Survival probability at non-zero temperatures"
        << " not calculated. -->\n";
      }
    }
    xmlBuilder << "  <WarningMessages>";
    std::vector< std::string > const
    warningMessagesToReport( WarningMessagesToReport() );
    if( warningMessagesToReport.empty() )
    {
      xmlBuilder << "<!-- No warnings. -->";
    }
    else
    {
      for( std::vector< std::string >::const_iterator
           warningMessage( warningMessagesToReport.begin() );
           warningMessage != warningMessagesToReport.end();
           ++warningMessage )
      {
        xmlBuilder << "\n"
        << "    " << "<WarningMessage>" << "\n"
        << "      " << *warningMessage << "\n"
        << "    " << "</WarningMessage>";
      }
      xmlBuilder << "\n  ";
    }
    xmlBuilder << "</WarningMessages>";
    resultsFromLastRunAsXml.assign( xmlBuilder.str() );
  }

} /* namespace VevaciousPlusPlus */
