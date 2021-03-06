/*
 * ThermalFunctions.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "PotentialEvaluation/ThermalFunctions.hpp"

namespace VevaciousPlusPlus
{
  std::string ThermalFunctions::JFunctionsAsPython()
  {
    // There are many terrible uses of numbers that are meant to be the sizes
    // of the arrays, all because we cannot be sure of C++11 compliant
    // compilers, so we cannot use nice static vector< double > const objects.
    std::stringstream stringBuilder;
    stringBuilder << std::setprecision( 12 ) << "# Thermal functions:\n"
    "bosonMinusOneToMinusTwelve = [ " << bosonMinusOneToMinusTwelve[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 111;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << bosonMinusOneToMinusTwelve[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "bosonZeroToMinusOne = [ " << bosonZeroToMinusOne[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 101;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << bosonZeroToMinusOne[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "bosonZeroToPlusOne = [ " << bosonZeroToPlusOne[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 101;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << bosonZeroToPlusOne[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "bosonPlusOneToPlusOneHundred = [ "
    << bosonPlusOneToPlusOneHundred[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 100;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << bosonPlusOneToPlusOneHundred[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "fermionMinusOneToMinusTwelve = [ "
    << fermionMinusOneToMinusTwelve[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 111;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << fermionMinusOneToMinusTwelve[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "fermionZeroToMinusOne = [ " << fermionZeroToMinusOne[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 101;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << fermionZeroToMinusOne[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "fermionZeroToPlusOne = [ " << fermionZeroToPlusOne[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 101;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << fermionZeroToPlusOne[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    << "fermionPlusOneToPlusOneHundred = [ "
    << fermionPlusOneToPlusOneHundred[ 0 ];
    for( unsigned int arrayIndex( 1 );
         arrayIndex < 100;
         ++arrayIndex )
    {
      stringBuilder
      << ", \n" << fermionPlusOneToPlusOneHundred[ arrayIndex ];
    }
    stringBuilder << " ]\n"
    "\n"
    << "# In these functions, r is the ratio of m^2/T^2.\n"
    "def BosonMinusOneToMinusTwelve( r ):\n"
    "    s = ( -10.0 * ( r + 1.0 ) )\n"
    "    n = int(s)\n"
    "    return ( bosonMinusOneToMinusTwelve[ n ]\n"
    "             + ( 0.1 * ( s - float( n ) )\n"
    "                     * ( bosonMinusOneToMinusTwelve[ n + 1 ]\n"
    "                         - bosonMinusOneToMinusTwelve[ n ] ) ) )\n"
    "\n"
    "def BosonZeroToMinusOne( r ):\n"
    "    s = ( -100.0 * r )\n"
    "    n = int(s)\n"
    "    return ( bosonZeroToMinusOne[ n ]\n"
    "             + ( 0.01 * ( s - float( n ) )\n"
    "                      * ( bosonZeroToMinusOne[ n + 1 ]\n"
    "                          - bosonZeroToMinusOne[ n ] ) ) )\n"
    "\n"
    "def BosonZeroToPlusOne( r ):\n"
    "    s = ( 100.0 * r )\n"
    "    n = int(s)\n"
    "    return ( bosonZeroToPlusOne[ n ]\n"
    "             + ( 0.01 * ( s - float( n ) )\n"
    "                      * ( bosonZeroToPlusOne[ n + 1 ]\n"
    "                          - bosonZeroToPlusOne[ n ] ) ) )\n"
    "\n"
    "def BosonPlusOneToPlusOneHundred( r ):\n"
    "    s = ( r - 1.0 )\n"
    "    n = int(s)\n"
    "    return ( bosonPlusOneToPlusOneHundred[ n ]\n"
    "             + ( ( s - float( n ) )\n"
    "                 * ( bosonPlusOneToPlusOneHundred[ n + 1 ]\n"
    "                     - bosonPlusOneToPlusOneHundred[ n ] ) ) )\n"
    "\n"
    "def FermionMinusOneToMinusTwelve( r ):\n"
    "    s = ( -10.0 * ( r + 1.0 ) )\n"
    "    n = int(s)\n"
    "    return ( fermionMinusOneToMinusTwelve[ n ]\n"
    "             + ( 0.1 * ( s - float( n ) )\n"
    "                     * ( fermionMinusOneToMinusTwelve[ n + 1 ]\n"
    "                         - fermionMinusOneToMinusTwelve[ n ] ) ) )\n"
    "\n"
    "def FermionZeroToMinusOne( r ):\n"
    "    s = ( -100.0 * r )\n"
    "    n = int(s)\n"
    "    return ( fermionZeroToMinusOne[ n ]\n"
    "             + ( 0.01 * ( s - float( n ) )\n"
    "                      * ( fermionZeroToMinusOne[ n + 1 ]\n"
    "                          - fermionZeroToMinusOne[ n ] ) ) )\n"
    "\n"
    "def FermionZeroToPlusOne( r ):\n"
    "    s = ( 100.0 * r )\n"
    "    n = int(s)\n"
    "    return ( fermionZeroToPlusOne[ n ]\n"
    "             + ( 0.01 * ( s - float( n ) )\n"
    "                      * ( fermionZeroToPlusOne[ n + 1 ]\n"
    "                          - fermionZeroToPlusOne[ n ] ) ) )\n"
    "\n"
    "def FermionPlusOneToPlusOneHundred( r ):\n"
    "    s = ( r - 1.0 )\n"
    "    n = int(s)\n"
    "    return ( fermionPlusOneToPlusOneHundred[ n ]\n"
    "             + ( ( s - float( n ) )\n"
    "                 * ( fermionPlusOneToPlusOneHundred[ n + 1 ]\n"
    "                     - fermionPlusOneToPlusOneHundred[ n ] ) ) )\n"
    "\n"
    "def BosonicJ( r ):\n"
    "    if ( r <= -12.0 ):\n"
    "        return 0.0\n"
    "    elif ( r <= -1.0 ):\n"
    "        return BosonMinusOneToMinusTwelve( r )\n"
    "    elif ( r < 0.0 ):\n"
    "        return BosonZeroToMinusOne( r )\n"
    "    elif ( r < 1.0 ):\n"
    "        return BosonZeroToPlusOne( r )\n"
    "    elif ( r < 100.0 ):\n"
    "        return BosonPlusOneToPlusOneHundred( r )\n"
    "    else:\n"
    "        return 0.0\n"
    "\n"
    "def FermionicJ( r ):\n"
    "    if ( r <= -12.0 ):\n"
    "        return 0.0\n"
    "    elif ( r <= -1.0 ):\n"
    "        return FermionMinusOneToMinusTwelve( r )\n"
    "    elif ( r < 0.0 ):\n"
    "        return FermionZeroToMinusOne( r )\n"
    "    elif ( r < 1.0 ):\n"
    "        return FermionZeroToPlusOne( r )\n"
    "    elif ( r < 100.0 ):\n"
    "        return FermionPlusOneToPlusOneHundred( r )\n"
    "    else:\n"
    "        return 0.0\n"
    "\n"
    "# End of thermal functions.\n";

    return stringBuilder.str();
  }

  double const ThermalFunctions::bosonMinusOneToMinusTwelve[ 111 ]
  = {-2.81842, -2.86878, -2.91708, -2.96442, -3.00669, -3.04769,
     -3.08684, -3.12507, -3.15996, -3.19119, -3.22226, -3.25209, -3.27792,
     -3.30286, -3.32645, -3.35409, -3.3679, -3.3844, -3.40226, -3.41439,
     -3.4273, -3.43844, -3.44689, -3.45837, -3.46216, -3.46837, -3.46686,
     -3.47267, -3.46792, -3.4689, -3.46423, -3.45791, -3.45226, -3.44186,
     -3.43267, -3.425, -3.41066, -3.40023, -3.38162, -3.36599, -3.35309,
     -3.33269, -3.30891, -3.29437, -3.26895, -3.24368, -3.21482, -3.18904,
     -3.16834, -3.1292, -3.09705, -3.06575, -3.03296, -3.01056, -2.97576,
     -2.93737, -2.89123, -2.85354, -2.81464, -2.77194, -2.72823, -2.68597,
     -2.65188, -2.59736, -2.56323, -2.51715, -2.4552, -2.43229, -2.36765,
     -2.3552, -2.2562, -2.24291, -2.15815, -2.09446, -2.0467, -2.00546,
     -1.92343, -1.89586, -1.80542, -1.74509, -1.69011, -1.62467, -1.5706,
     -1.488, -1.42991, -1.35777, -1.29835, -1.22394, -1.18179, -1.1054,
     -1.03546, -0.950629, -0.88303, -0.850134, -0.735434, -0.669491,
     -0.585089, -0.569669, -0.43443, -0.369764, -0.291595, -0.211436,
     -0.140016, -0.0571975,
     // We replace the last few entries with zeroes so that the function goes
     // smoothly to zero without going positive.
     0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
     /*0.00860118, 0.111432, 0.179408, 0.274796,
     0.360985, 0.431345, 0.525129*/};
  double const ThermalFunctions::bosonZeroToMinusOne[ 101 ]
  = {-2.16465, -2.17284, -2.18098, -2.18907, -2.19711, -2.20511,
     -2.21307, -2.221, -2.22886, -2.23668, -2.24449, -2.25225, -2.25994,
     -2.26762, -2.27525, -2.28287, -2.29042, -2.29798, -2.3055, -2.31291,
     -2.32039, -2.32776, -2.33512, -2.34248, -2.34981, -2.35699, -2.3642,
     -2.37139, -2.37853, -2.38571, -2.39276, -2.39991, -2.40689, -2.41385,
     -2.42079, -2.42774, -2.43471, -2.44173, -2.44841, -2.45517, -2.46238,
     -2.46881, -2.4755, -2.48242, -2.48879, -2.4954, -2.50205, -2.50875,
     -2.51514, -2.52166, -2.52818, -2.5346, -2.54121, -2.54742, -2.55413,
     -2.56024, -2.56653, -2.57283, -2.57938, -2.58523, -2.59171, -2.59778,
     -2.60379, -2.60989, -2.616, -2.62215, -2.62899, -2.63416, -2.64021,
     -2.64614, -2.6528, -2.65819, -2.66395, -2.66981, -2.67621, -2.68156,
     -2.68717, -2.69282, -2.69911, -2.70437, -2.71, -2.71578, -2.72122,
     -2.7269, -2.73247, -2.73802, -2.74412, -2.74906, -2.75444, -2.75989,
     -2.76543, -2.77093, -2.7763, -2.78247, -2.78708, -2.792, -2.7975,
     -2.80302, -2.80772, -2.81424, -2.81842};
  double const ThermalFunctions::bosonZeroToPlusOne[ 101 ]
  = {-2.16465, -2.15691, -2.14956, -2.14244, -2.13551, -2.12872,
     -2.12207, -2.11554, -2.10911, -2.10278, -2.09655, -2.0904, -2.08433,
     -2.07833, -2.07241, -2.06656, -2.06077, -2.05504, -2.04938, -2.04377,
     -2.03821, -2.03271, -2.02726, -2.02186, -2.01651, -2.01121, -2.00595,
     -2.00073, -1.99556, -1.99043, -1.98534, -1.98029, -1.97528, -1.9703,
     -1.96537, -1.96047, -1.9556, -1.95077, -1.94598, -1.94121, -1.93648,
     -1.93178, -1.92711, -1.92248, -1.91787, -1.91329, -1.90874, -1.90422,
     -1.89973, -1.89527, -1.89083, -1.88642, -1.88204, -1.87768, -1.87335,
     -1.86904, -1.86475, -1.8605, -1.85626, -1.85205, -1.84786, -1.8437,
     -1.83956, -1.83544, -1.83134, -1.82726, -1.82321, -1.81917, -1.81516,
     -1.81117, -1.8072, -1.80325, -1.79931, -1.7954, -1.79151, -1.78764,
     -1.78378, -1.77995, -1.77613, -1.77233, -1.76855, -1.76479, -1.76105,
     -1.75732, -1.75361, -1.74992, -1.74624, -1.74258, -1.73894, -1.73532,
     -1.73171, -1.72811, -1.72454, -1.72097, -1.71743, -1.7139, -1.71038,
     -1.70688, -1.7034, -1.69993, -1.69648};
  double const ThermalFunctions::bosonPlusOneToPlusOneHundred[ 100 ]
  = {-1.69648, -1.40887, -1.19768, -1.03324, -0.900881, -0.791934,
     -0.700785, -0.62356, -0.557465, -0.50042, -0.450833, -0.407467,
     -0.369339, -0.335656, -0.305776, -0.279169, -0.255394, -0.234085,
     -0.21493, -0.197667, -0.182069, -0.167945, -0.155128, -0.143474,
     -0.132858, -0.123171, -0.114315, -0.106208, -0.0987757, -0.0919514,
     -0.0856774, -0.0799019, -0.0745789, -0.0696671, -0.0651299,
     -0.0609341, -0.0570501, -0.0534512, -0.0501133, -0.0470147,
     -0.0441357, -0.0414585, -0.0389669, -0.0366461, -0.034483,
     -0.0324651, -0.0305816, -0.0288221, -0.0271774, -0.0256391,
     -0.0241993, -0.0228509, -0.0215874, -0.0204027, -0.0192913,
     -0.018248, -0.0172682, -0.0163475, -0.015482, -0.0146678, -0.0139017,
     -0.0131804, -0.0125009, -0.0118607, -0.011257, -0.0106877,
     -0.0101505, -0.00964338, -0.0091645, -0.00871209, -0.00828453,
     -0.0078803, -0.00749798, -0.00713625, -0.00679389, -0.00646974,
     -0.00616273, -0.00587185, -0.00559618, -0.00533482, -0.00508695,
     -0.00485182, -0.00462868, -0.00441687, -0.00421576, -0.00402474,
     -0.00384326, -0.0036708, -0.00350685, -0.00335097, -0.0032027,
     -0.00306165, -0.00292742, -0.00279966, -0.00267802, -0.00256218,
     -0.00245183, -0.0023467, -0.0022465, -0.002151};
  double const ThermalFunctions::fermionMinusOneToMinusTwelve[ 111 ]
  = {-2.38534, -2.43937, -2.49391, -2.54885, -2.60415, -2.65974,
     -2.71557, -2.77157, -2.82769, -2.88386, -2.94007, -2.99623, -3.0523,
     -3.10824, -3.164, -3.21955, -3.27479, -3.32972, -3.3843, -3.43847,
     -3.49218, -3.54544, -3.59812, -3.65023, -3.70172, -3.75259, -3.80263,
     -3.85201, -3.90058, -3.94833, -3.99511, -4.04102, -4.08599, -4.12992,
     -4.17278, -4.2146, -4.25516, -4.2947, -4.33282, -4.36972, -4.40532,
     -4.43936, -4.4721, -4.50334, -4.53305, -4.56108, -4.58754, -4.61221,
     -4.63537, -4.65638, -4.67556, -4.69285, -4.70818, -4.72162, -4.73259,
     -4.74141, -4.74809, -4.75217, -4.75406, -4.75332, -4.75, -4.74399,
     -4.73527, -4.72356, -4.70917, -4.69127, -4.67007, -4.64631, -4.61823,
     -4.58733, -4.55189, -4.51303, -4.46972, -4.42257, -4.37121, -4.31497,
     -4.25337, -4.18742, -4.11496, -4.03703, -3.95342, -3.8631, -3.76526,
     -3.65933, -3.54516, -3.42093, -3.28587, -3.13697, -2.97054, -2.77168,
     -2.55382, -2.33802, -2.12433, -1.91378, -1.70216, -1.49409, -1.2866,
     -1.08143, -0.877458, -0.675849, -0.475942, -0.276914, -0.0773436,
     // We replace the last few entries with zeroes so that the function goes
     // smoothly to zero without going positive.
     0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
     /*0.116269, 0.311153, 0.504606, 0.698892, 0.882082, 1.0687, 1.25597,
     1.43917*/};
  double const ThermalFunctions::fermionZeroToMinusOne[ 101 ]
  = {-1.89407, -1.8982, -1.90237, -1.90658, -1.91081, -1.91507, -1.91935,
     -1.92366, -1.928, -1.93235, -1.93673, -1.94113, -1.94555, -1.94999,
     -1.95445, -1.95893, -1.96343, -1.96794, -1.97248, -1.97703, -1.9816,
     -1.98619, -1.99079, -1.99541, -2.00004, -2.00469, -2.00936, -2.01404,
     -2.01874, -2.02345, -2.02817, -2.03291, -2.03767, -2.04243, -2.04721,
     -2.05201, -2.05681, -2.06164, -2.06647, -2.07131, -2.07617, -2.08104,
     -2.08592, -2.09082, -2.09572, -2.10064, -2.10557, -2.11051, -2.11546,
     -2.12043, -2.1254, -2.13038, -2.13538, -2.14039, -2.1454, -2.15043,
     -2.15546, -2.16051, -2.16557, -2.17063, -2.17571, -2.18079, -2.18588,
     -2.19099, -2.1961, -2.20122, -2.20635, -2.21149, -2.21664, -2.22179,
     -2.22696, -2.23213, -2.23731, -2.2425, -2.2477, -2.2529, -2.25812,
     -2.26334, -2.26857, -2.2738, -2.27904, -2.2843, -2.28955, -2.29482,
     -2.30009, -2.30537, -2.31066, -2.31595, -2.32125, -2.32656, -2.33187,
     -2.33719, -2.34251, -2.34785, -2.35318, -2.35853, -2.36388, -2.36924,
     -2.3746, -2.37997, -2.38534};
  double const ThermalFunctions::fermionZeroToPlusOne[ 101 ]
  = {-1.89407, -1.88998, -1.88592, -1.8819, -1.87791, -1.87394, -1.87001,
     -1.86609, -1.8622, -1.85833, -1.85449, -1.85066, -1.84686, -1.84308,
     -1.83932, -1.83557, -1.83185, -1.82815, -1.82446, -1.82079, -1.81714,
     -1.81351, -1.80989, -1.80629, -1.80271, -1.79915, -1.7956, -1.79206,
     -1.78854, -1.78504, -1.78155, -1.77808, -1.77462, -1.77118, -1.76775,
     -1.76433, -1.76093, -1.75755, -1.75417, -1.75081, -1.74747, -1.74414,
     -1.74082, -1.73751, -1.73422, -1.73094, -1.72767, -1.72441, -1.72117,
     -1.71794, -1.71472, -1.71152, -1.70832, -1.70514, -1.70197, -1.69881,
     -1.69566, -1.69253, -1.6894, -1.68629, -1.68319, -1.68009, -1.67701,
     -1.67394, -1.67089, -1.66784, -1.6648, -1.66177, -1.65876, -1.65575,
     -1.65276, -1.64977, -1.64679, -1.64383, -1.64087, -1.63793, -1.63499,
     -1.63207, -1.62915, -1.62624, -1.62335, -1.62046, -1.61758, -1.61471,
     -1.61185, -1.609, -1.60616, -1.60333, -1.60051, -1.59769, -1.59489,
     -1.59209, -1.58931, -1.58653, -1.58376, -1.581, -1.57824, -1.5755,
     -1.57277, -1.57004, -1.56732};
  double const ThermalFunctions::fermionPlusOneToPlusOneHundred[ 100 ]
  = {-1.56732, -1.33092, -1.14674, -0.998346, -0.876172, -0.773999,
     -0.687509, -0.613573, -0.549849, -0.494543, -0.446253, -0.403865,
     -0.366482, -0.333375, -0.303942, -0.277686, -0.254189, -0.2331,
     -0.214122, -0.197, -0.181517, -0.167486, -0.154746, -0.143154,
     -0.132589, -0.122944, -0.114124, -0.106047, -0.0986383, -0.0918345,
     -0.0855776, -0.0798166, -0.0745058, -0.0696044, -0.0650759,
     -0.0608876, -0.05701, -0.0534165, -0.0500833, -0.0469886, -0.044113,
     -0.0414388, -0.0389497, -0.0366311, -0.0344698, -0.0324536,
     -0.0305715, -0.0288132, -0.0271697, -0.0256323, -0.0241933,
     -0.0228456, -0.0215827, -0.0203986, -0.0192876, -0.0182448,
     -0.0172653, -0.016345, -0.0154797, -0.0146658, -0.0138999,
     -0.0131788, -0.0124995, -0.0118594, -0.0112559, -0.0106867,
     -0.0101496, -0.00964258, -0.00916378, -0.00871145, -0.00828395,
     -0.00787978, -0.00749751, -0.00713583, -0.00679351, -0.0064694,
     -0.00616242, -0.00587158, -0.00559593, -0.00533459, -0.00508675,
     -0.00485163, -0.00462852, -0.00441672, -0.00421562, -0.00402462,
     -0.00384315, -0.0036707, -0.00350676, -0.00335089, -0.00320263,
     -0.00306158, -0.00292736, -0.0027996, -0.00267797, -0.00256213,
     -0.00245179, -0.00234666, -0.00224647, -0.00215097};

} /* namespace VevaciousPlusPlus */
