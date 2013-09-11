// Switched Reluctance Motor Gmsh geometry file (2D)

Include "srm_data.geo" ;

Mesh.Algorithm = 1 ;
Mesh.CharacteristicLengthFactor = 1. ;

// Some general settings
Lc = 7.5e-4/2; // base characteristic length

//------------------------------------------------------------
//------------------------------------------------------------

// Create origin
pAxe = newp ; Point(pAxe) = { 0. , 0. , 0., 3*Lc} ;


Include "srm_stator.geo";
Include "srm_rotor.geo";

//-------------------------------------------------------------------------------
// For nice visualization
//-------------------------------------------------------------------------------

Hide { Point{ Point '*' }; }
Hide { Line{ Line '*' }; }
Show { Line{ linStator[], linRotor[] }; }

Physical Line(NICEPOS) = {linStator[],linRotor[] } ;

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
