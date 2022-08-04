#include "CopyStructures.h"

CurrentData CopyCurrentData(CurrentData otherCurrentData)
{
	CurrentData newCurrentData;
	
	//copy all simple objects
    newCurrentData.Name = otherCurrentData.Name;
    newCurrentData.NbrSystem = otherCurrentData.NbrSystem;
    newCurrentData.PostOpDataIndex = otherCurrentData.PostOpDataIndex;
	
    newCurrentData.NbrHar = otherCurrentData.NbrHar;
    newCurrentData.Region = otherCurrentData.Region;
    newCurrentData.SubRegion = otherCurrentData.SubRegion;
    newCurrentData.NumEntity = otherCurrentData.NumEntity;
    newCurrentData.NumEntityInElement = otherCurrentData.NumEntityInElement;
	//int NumEntities[NBR_MAX_BASISFUNCTIONS];
    newCurrentData.NumEntities[0] = otherCurrentData.NumEntities[0];
	
	
    newCurrentData.SourceIntegrationSupportIndex = otherCurrentData.SourceIntegrationSupportIndex;
    newCurrentData.TypeTime = otherCurrentData.TypeTime;
    newCurrentData.TypeAssembly = otherCurrentData.TypeAssembly;
    newCurrentData.SubTimeStep = otherCurrentData.SubTimeStep;
    newCurrentData.flagAssDiag = otherCurrentData.flagAssDiag;
	
	
    newCurrentData.x = otherCurrentData.x;
    newCurrentData.y = otherCurrentData.y;
    newCurrentData.z = otherCurrentData.z;

    newCurrentData.u = otherCurrentData.u;
    newCurrentData.v = otherCurrentData.v;
    newCurrentData.w = otherCurrentData.w;
	
    newCurrentData.xs = otherCurrentData.xs;
    newCurrentData.ys = otherCurrentData.ys;
    newCurrentData.zs = otherCurrentData.zs;
	
    newCurrentData.us = otherCurrentData.us;
    newCurrentData.vs = otherCurrentData.vs;
    newCurrentData.ws = otherCurrentData.ws;
	
    newCurrentData.a = otherCurrentData.a;
    newCurrentData.b = otherCurrentData.b;
    newCurrentData.c = otherCurrentData.c;
	
	
    newCurrentData.xp = otherCurrentData.xp;
    newCurrentData.yp = otherCurrentData.yp;
    newCurrentData.zp = otherCurrentData.zp;

    newCurrentData.ut = otherCurrentData.ut;
    newCurrentData.vt = otherCurrentData.vt;
    newCurrentData.wt = otherCurrentData.wt;
	
    newCurrentData.Val[0] = otherCurrentData.Val[0];

    newCurrentData.QuadraturePointIndex = otherCurrentData.QuadraturePointIndex;

  // For TimeLoopTheta and TimeLoopNewmark

    newCurrentData.Time = otherCurrentData.Time;
    newCurrentData.TimeImag = otherCurrentData.TimeImag;
    newCurrentData.TimeStep = otherCurrentData.TimeStep;
    newCurrentData.DTime = otherCurrentData.DTime;
	
    newCurrentData.Theta = otherCurrentData.Theta;
    newCurrentData.Beta = otherCurrentData.Beta;
    newCurrentData.Gamma = otherCurrentData.Gamma;

  // For TimeLoopAdaptive
    newCurrentData.PredOrder = otherCurrentData.PredOrder;
    newCurrentData.CorrOrder = otherCurrentData.CorrOrder;
    newCurrentData.aPredCoeff[0] = otherCurrentData.aPredCoeff[0];
    newCurrentData.aCorrCoeff[0] = otherCurrentData.aCorrCoeff[0];
    newCurrentData.bCorrCoeff = otherCurrentData.bCorrCoeff;
    newCurrentData.PredErrorConst = otherCurrentData.PredErrorConst;
    newCurrentData.CorrErrorConst = otherCurrentData.CorrErrorConst;
    newCurrentData.Breakpoint = otherCurrentData.Breakpoint;
  
    // For TimeLoopAdaptive
    newCurrentData.Iteration = otherCurrentData.Iteration;
    newCurrentData.RelativeDifference = otherCurrentData.RelativeDifference;
    newCurrentData.RelativeDifferenceOld = otherCurrentData.RelativeDifferenceOld;
    newCurrentData.RelaxationFactor = otherCurrentData.RelaxationFactor;
    newCurrentData.Residual = otherCurrentData.Residual;
    newCurrentData.ResidualN = otherCurrentData.ResidualN;
    newCurrentData.Residual_Iter1 = otherCurrentData.Residual_Iter1;
    newCurrentData.RelaxFac = otherCurrentData.RelaxFac;
    newCurrentData.NbrTestedFac = otherCurrentData.NbrTestedFac;
    newCurrentData.SolveJacAdaptFailed = otherCurrentData.SolveJacAdaptFailed;

  // Iterative linear system solvers
    newCurrentData.KSPIterations = otherCurrentData.KSPIterations;
    newCurrentData.KSPIteration = otherCurrentData.KSPIteration;
    newCurrentData.KSPResidual = otherCurrentData.KSPResidual;
    newCurrentData.KSPSystemSize = otherCurrentData.KSPSystemSize;
	
    newCurrentData.Frequency = otherCurrentData.Frequency;
  
	//copy all list objects
	newCurrentData.PostOpData_L = List_Copy(otherCurrentData.PostOpData_L);



	//copy all struct objects

	newCurrentData.DefineSystem_P = CopyDefineSystem(otherCurrentData.DefineSystem_P);
	
	
// Still to copy!
 /* struct DofData *DofData_P0;

  struct DofData *DofData;
  struct GeoData *GeoData;

  struct Element *Element;*/
}

DefineSystem CopyDefineSystem(DefineSystem otherDefineSystem)
{
	DefineSystem newDefineSystem;
	
	newDefineSystem.Type = otherDefineSystem.Type;
	
	// char* copying ????!!!!!
	newDefineSystem.SolverDataFileName = otherDefineSystem.SolverDataFileName;
	newDefineSystem.MeshName = otherDefineSystem.MeshName;
	newDefineSystem.AdaptName = otherDefineSystem.AdaptName;
	newDefineSystem.DestinationSystemName = otherDefineSystem.DestinationSystemName;
	newDefineSystem.DestinationSystemIndex = otherDefineSystem.DestinationSystemIndex;
	
	newDefineSystem.FormulationIndex = List_Copy(otherDefineSystem.FormulationIndex);
			
	newDefineSystem.FrequencyValue = List_Copy(otherDefineSystem.FrequencyValue);
			
	newDefineSystem.OriginSystemIndex = List_Copy(otherDefineSystem.OriginSystemIndex);
}

GeoData CopyGeoData(GeoData otherGeoData)
{
	GeoData newGeoData;
	newGeoData.Name = otherGeoData.Name;
	newGeoData.Num = otherGeoData.Num;
	newGeoData.NbrElementsWithEdges = otherGeoData.NbrElementsWithEdges;
	newGeoData.NbrElementsWithFacets = otherGeoData.NbrElementsWithFacets;
	newGeoData.NumCurrentEdge = otherGeoData.NbrElementsWithFacets;
	newGeoData.NumCurrentFacet = otherGeoData.NumCurrentFacet;
	
	newGeoData.Nodes = List_Copy(otherGeoData.Nodes);
			
	newGeoData.Elements = List_Copy(otherGeoData.Elements);
	
	newGeoData.GroupForPRE = List_Copy(otherGeoData.GroupForPRE);
	newGeoData.PeriodicNodes = List_Copy(otherGeoData.PeriodicNodes);
			
		
  // TO DO Copy Tree_T objects!!
  ///Tree_T *EdgesXNodes, *FacetsXEdges;
  ///
  ///Tree_T *NodesXElements;
  ///Tree_T *Normals;
  
	newGeoData.Xmin = otherGeoData.Xmin;
	newGeoData.Xmax = otherGeoData.Xmax;
	newGeoData.Ymin = otherGeoData.Ymin;
	newGeoData.Ymax = otherGeoData.Ymax;
	newGeoData.Zmin = otherGeoData.Zmin;
	newGeoData.Zmax = otherGeoData.Zmax;
	
	newGeoData.Dimension = otherGeoData.Dimension;
	newGeoData.CharacteristicLength = otherGeoData.CharacteristicLength;
    
  // TO DO copy 
  ///struct Grid Grid;

  ///double *H, *P;
  
}
