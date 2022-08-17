#include "CopyStructures.h"
#include "Message.h"

CurrentData * CopyCurrentData(CurrentData * otherCurrentData)
{

Message::Warning("BOOM HEADSHOT");


    CurrentData *  newCurrentData = new CurrentData;


    //return newCurrentData;
    //copy all simple objects


    newCurrentData->Name = CharCopy(otherCurrentData->Name);


    newCurrentData->Time = otherCurrentData->Time;
    newCurrentData->TimeImag = otherCurrentData->TimeImag;
    newCurrentData->TimeStep = otherCurrentData->TimeStep;

    newCurrentData->NbrSystem = otherCurrentData->NbrSystem;
    newCurrentData->PostOpDataIndex = otherCurrentData->PostOpDataIndex;

    newCurrentData->NbrHar = otherCurrentData->NbrHar;
    newCurrentData->Region = otherCurrentData->Region;
    newCurrentData->SubRegion = otherCurrentData->SubRegion;
    newCurrentData->NumEntity = otherCurrentData->NumEntity;
    newCurrentData->NumEntityInElement = otherCurrentData->NumEntityInElement;
    //int NumEntities[NBR_MAX_BASISFUNCTIONS];

    for (int f=0; f<NBR_MAX_BASISFUNCTIONS;f++ )
    newCurrentData->NumEntities[f] = otherCurrentData->NumEntities[f];


    newCurrentData->IntegrationSupportIndex = otherCurrentData->IntegrationSupportIndex;


    newCurrentData->SourceIntegrationSupportIndex = otherCurrentData->SourceIntegrationSupportIndex;
    newCurrentData->TypeTime = otherCurrentData->TypeTime;
    newCurrentData->TypeAssembly = otherCurrentData->TypeAssembly;
    newCurrentData->SubTimeStep = otherCurrentData->SubTimeStep;
    newCurrentData->flagAssDiag = otherCurrentData->flagAssDiag;




    newCurrentData->x = otherCurrentData->x;
    newCurrentData->y = otherCurrentData->y;
    newCurrentData->z = otherCurrentData->z;

    newCurrentData->u = otherCurrentData->u;
    newCurrentData->v = otherCurrentData->v;
    newCurrentData->w = otherCurrentData->w;
	
    newCurrentData->xs = otherCurrentData->xs;
    newCurrentData->ys = otherCurrentData->ys;
    newCurrentData->zs = otherCurrentData->zs;
	
    newCurrentData->us = otherCurrentData->us;
    newCurrentData->vs = otherCurrentData->vs;
    newCurrentData->ws = otherCurrentData->ws;
	
    newCurrentData->a = otherCurrentData->a;
    newCurrentData->b = otherCurrentData->b;
    newCurrentData->c = otherCurrentData->c;
	
	
    newCurrentData->xp = otherCurrentData->xp;
    newCurrentData->yp = otherCurrentData->yp;
    newCurrentData->zp = otherCurrentData->zp;

    newCurrentData->ut = otherCurrentData->ut;
    newCurrentData->vt = otherCurrentData->vt;
    newCurrentData->wt = otherCurrentData->wt;


    for (int f=0; f<NBR_MAX_HARMONIC * MAX_DIM;f++ )	
    newCurrentData->Val[f] = otherCurrentData->Val[f];

    newCurrentData->QuadraturePointIndex = otherCurrentData->QuadraturePointIndex;

  // For TimeLoopTheta and TimeLoopNewmark



    newCurrentData->Time = otherCurrentData->Time;
    newCurrentData->TimeImag = otherCurrentData->TimeImag;
    newCurrentData->TimeStep = otherCurrentData->TimeStep;
    newCurrentData->DTime = otherCurrentData->DTime;
	
    newCurrentData->Theta = otherCurrentData->Theta;
    newCurrentData->Beta = otherCurrentData->Beta;
    newCurrentData->Gamma = otherCurrentData->Gamma;

  // For TimeLoopAdaptive
    newCurrentData->PredOrder = otherCurrentData->PredOrder;
    newCurrentData->CorrOrder = otherCurrentData->CorrOrder;
    
    for (int f=0; f<7;f++ )	
    newCurrentData->aPredCoeff[f] = otherCurrentData->aPredCoeff[f];
    
    for (int f=0; f<6;f++ )	
    newCurrentData->aCorrCoeff[f] = otherCurrentData->aCorrCoeff[f];
    
    newCurrentData->bCorrCoeff = otherCurrentData->bCorrCoeff;
    newCurrentData->PredErrorConst = otherCurrentData->PredErrorConst;
    newCurrentData->CorrErrorConst = otherCurrentData->CorrErrorConst;
    newCurrentData->Breakpoint = otherCurrentData->Breakpoint;
  
    // For TimeLoopAdaptive
    newCurrentData->Iteration = otherCurrentData->Iteration;
    newCurrentData->RelativeDifference = otherCurrentData->RelativeDifference;
    newCurrentData->RelativeDifferenceOld = otherCurrentData->RelativeDifferenceOld;
    newCurrentData->RelaxationFactor = otherCurrentData->RelaxationFactor;
    newCurrentData->Residual = otherCurrentData->Residual;
    newCurrentData->ResidualN = otherCurrentData->ResidualN;
    newCurrentData->Residual_Iter1 = otherCurrentData->Residual_Iter1;
    newCurrentData->RelaxFac = otherCurrentData->RelaxFac;
    newCurrentData->NbrTestedFac = otherCurrentData->NbrTestedFac;
    newCurrentData->SolveJacAdaptFailed = otherCurrentData->SolveJacAdaptFailed;

  // Iterative linear system solvers
    newCurrentData->KSPIterations = otherCurrentData->KSPIterations;
    newCurrentData->KSPIteration = otherCurrentData->KSPIteration;
    newCurrentData->KSPResidual = otherCurrentData->KSPResidual;
    newCurrentData->KSPSystemSize = otherCurrentData->KSPSystemSize;
	
    newCurrentData->Frequency = otherCurrentData->Frequency;
  
  
  
    //copy all list objects
     
     newCurrentData->PostOpData_L = nullptr;
     if (otherCurrentData->PostOpData_L != nullptr)
     {
      newCurrentData->PostOpData_L = List_Create(List_Nbr(otherCurrentData->PostOpData_L), 1, sizeof(struct PostOpSolutions));
      List_Copy(otherCurrentData->PostOpData_L, newCurrentData->PostOpData_L);
      }

     


    //copy all struct objects

    newCurrentData->DefineSystem_P = CopyDefineSystem(otherCurrentData->DefineSystem_P); //<- MUST HAVE !
    newCurrentData->DofData_P0 = CopyDofData(otherCurrentData->DofData_P0); //<- MUST HAVE !



    //sh*t hits the fan @here
    newCurrentData->GeoData = CopyGeoData(otherCurrentData->GeoData); //<- do not need
    //newCurrentData->DofData = CopyDofData(otherCurrentData->DofData); //<- do not need
    //newCurrentData->Element = CopyElement(otherCurrentData->Element); //<- do not need
    //newCurrentData->ElementSource = CopyElement(otherCurrentData->ElementSource); //<- do not need


  Message::Warning("BOOM HEADSHOT 2");
  
  
  return newCurrentData;
  
}



char * CharCopy(char * data)
{

char * rets = nullptr;

	if (data != nullptr)	
	{
	    const int size = strlen(data) + 1;
	    rets = new char[size];
	    strcpy(rets, data);
	}
        
return rets;

}


DofData * CopyDofData(DofData * otherDofData)
{

    DofData * newDofData = new DofData;
    newDofData->Num = otherDofData->Num;
    
  newDofData->ResolutionIndex = otherDofData->ResolutionIndex;
  newDofData->SystemIndex = otherDofData->SystemIndex;
  newDofData->GeoDataIndex = otherDofData->GeoDataIndex;


  newDofData->FunctionSpaceIndex = nullptr;
  if (otherDofData->FunctionSpaceIndex != nullptr)
  {
    newDofData->FunctionSpaceIndex = List_Create(List_Nbr(otherDofData->FunctionSpaceIndex), 1, sizeof( int ));
    List_Copy(otherDofData->FunctionSpaceIndex, newDofData->FunctionSpaceIndex);
   }



  newDofData->TimeFunctionIndex = nullptr;
  if (otherDofData->TimeFunctionIndex != nullptr)
  {
    newDofData->TimeFunctionIndex = List_Create(List_Nbr(otherDofData->TimeFunctionIndex), 1, sizeof( int ));
    List_Copy(otherDofData->TimeFunctionIndex, newDofData->TimeFunctionIndex);
   }




    //Current.DofData_P0->Solutions = nullptr; // <- MUST HAVE
  newDofData->Solutions = nullptr;
  if (otherDofData->Solutions != nullptr)
  {
    newDofData->Solutions = List_Create(List_Nbr(otherDofData->Solutions), 1, sizeof( struct Solution ));
    List_Copy(otherDofData->Solutions, newDofData->Solutions);
  }









  newDofData->Pulsation = nullptr;
  if (otherDofData->Pulsation != nullptr)
  {
    newDofData->Pulsation = List_Create(List_Nbr(otherDofData->Pulsation), 1, sizeof( double ));
    List_Copy(otherDofData->Pulsation, newDofData->Pulsation);
  }



    newDofData->Val_Pulsation = (double *)List_Pointer(newDofData->Pulsation, 0);



     newDofData->DofList = nullptr;
  if (otherDofData->DofList != nullptr)
  {
    newDofData->DofList = List_Create(List_Nbr(otherDofData->DofList), 1, sizeof( struct Dof ));
    List_Copy(otherDofData->DofList, newDofData->DofList);
  }




    newDofData->SolverDataFileName = CharCopy(otherDofData->SolverDataFileName);


    newDofData->NbrHar = otherDofData->NbrHar;
    newDofData->NbrAnyDof = otherDofData->NbrAnyDof;
    newDofData->NbrDof = otherDofData->NbrDof;
    
    newDofData->Flag_RHS = otherDofData->Flag_RHS; // only assemble RHS
    newDofData->Flag_ListOfRHS = otherDofData->Flag_ListOfRHS; // only assemble list of RHS

    for (int f=0; f<8;f++)
	newDofData->Flag_Init[f] = otherDofData->Flag_Init[f];

    newDofData->Flag_Only = otherDofData->Flag_Only;


    for (int f=0; f<3;f++)    
	newDofData->Flag_InitOnly[f] = otherDofData->Flag_InitOnly[f];


  newDofData->CounterOfRHS = otherDofData->CounterOfRHS;
  newDofData->TotalNumberOfRHS = otherDofData->TotalNumberOfRHS;




    return newDofData;

}



//Should be OK
DefineSystem * CopyDefineSystem(DefineSystem * otherDefineSystem)
{
	DefineSystem * newDefineSystem =  new DefineSystem;
	
	newDefineSystem->Type = otherDefineSystem->Type;
	
	// char* copying ????!!!!!
	
	newDefineSystem->Name = CharCopy(otherDefineSystem->Name);
	newDefineSystem->SolverDataFileName = CharCopy(otherDefineSystem->SolverDataFileName);
	
	
	newDefineSystem->MeshName = CharCopy(otherDefineSystem->MeshName);
	newDefineSystem->AdaptName = CharCopy(otherDefineSystem->AdaptName);
	newDefineSystem->DestinationSystemName = CharCopy(otherDefineSystem->DestinationSystemName);
	newDefineSystem->DestinationSystemIndex = otherDefineSystem->DestinationSystemIndex;
	

      //newCurrentData->PostOpData_L = List_Create(List_Nbr(otherCurrentData->PostOpData_L), 1, sizeof(struct PostOpSolutions));
      //List_Copy(otherCurrentData->PostOpData_L, newCurrentData->PostOpData_L);


	newDefineSystem->FormulationIndex = nullptr;
	if(otherDefineSystem->FormulationIndex != nullptr) 
	{
    	newDefineSystem->FormulationIndex = List_Create(List_Nbr(otherDefineSystem->FormulationIndex), 1, sizeof(struct Formulation));
	List_Copy(otherDefineSystem->FormulationIndex, newDefineSystem->FormulationIndex);
	}
	
	newDefineSystem->FrequencyValue = nullptr;
	if (otherDefineSystem->FrequencyValue != nullptr)
	{
	newDefineSystem->FrequencyValue = List_Create(List_Nbr(otherDefineSystem->FrequencyValue), 1, sizeof(double));
	List_Copy(otherDefineSystem->FrequencyValue, newDefineSystem->FrequencyValue);
	}
	
	newDefineSystem->OriginSystemIndex = nullptr;
	if (otherDefineSystem->OriginSystemIndex != nullptr)
	{
	newDefineSystem->OriginSystemIndex = List_Create(List_Nbr(otherDefineSystem->OriginSystemIndex), 1, sizeof(int));
	List_Copy(otherDefineSystem->OriginSystemIndex, newDefineSystem->OriginSystemIndex);
	}

	
	return newDefineSystem;
}




Grid Grid_Copy(Grid grid)
{



Grid new_grid;

new_grid.Init = grid.Init;


//Message::Warning("tree->size0 = []");




new_grid.Bricks = new List_T;


new_grid.Xmin = grid.Xmin;
new_grid.Xmax = grid.Xmax;
new_grid.Ymin = grid.Ymin;
new_grid.Ymax = grid.Ymax;
new_grid.Zmin = grid.Zmin;
new_grid.Zmax = grid.Zmax;


new_grid.Nx = grid.Nx;
new_grid.Ny = grid.Ny;
new_grid.Nz = grid.Nz;

return new_grid;
}




Tree_T * Tree_Copy(Tree_T * tree)
{

//Message::Warning("Tree_Copy f() START [%d]", tree->size);

int diff = 0;

Tree_T * new_tree = Tree_Create(tree->size, tree->root->compar);

  // šis cikls ir no avl_lookup funkcijas, kur tiek iziets cauri visām tree nodēm

    avl_tree * root = tree->root;
    avl_node * node = root->root;


//    void *ptr = Malloc(tree->size);

//    memcpy(ptr, &node, tree->size);
    
    
    
//    new_tree->root->root = ptr;


 // Message::Warning("tree->size = [%d]", tree->size);



    while (node != NIL(avl_node))
    {
        // šīs ir divas galvenās rindas. Kaut kā jākopē dati, iespējams vajag memcpy
        
        //void * key = node->key;
        //void * value = node->value;
        


        void * key = (void*) malloc (sizeof(node->key));
    		 memcpy(key, node->key, sizeof(node->key));
    		 
    	    
    	
        void * value = (void*) malloc (sizeof(node->value));
	        memcpy(value, node->value, sizeof(node->value));
        
        
        avl_insert(root, key, value);
        node = (diff < 0) ? node->left : node->right;
    }



//Message::Warning("Tree_Copy f() END [%d]", tree->size);

return new_tree;
}





/*
avl_node * avl_Tree_Copy( avl_node * t)
{
    if ( t != NULL )
    {
        avl_node* left = avl_Tree_Copy( t->left );
	avl_node* right = avl_Tree_Copy( t->right );
	void * key = t->key;
	void * value = t->value;
        return new avl_node(left, right, key, value, t->height);
    }
    else
    {
        return NULL;
    }
}
*/


GeoData * CopyGeoData(GeoData * otherGeoData)
{
	GeoData * newGeoData = new GeoData;
	newGeoData->Name = CharCopy(otherGeoData->Name);
	newGeoData->Num = otherGeoData->Num;
	newGeoData->NbrElementsWithEdges = otherGeoData->NbrElementsWithEdges;
	newGeoData->NbrElementsWithFacets = otherGeoData->NbrElementsWithFacets;
	newGeoData->NumCurrentEdge = otherGeoData->NumCurrentEdge;
	newGeoData->NumCurrentFacet = otherGeoData->NumCurrentFacet;
	


  ///double *H, *P;
  if (otherGeoData->Elements != nullptr)
  {
  
  if (otherGeoData->H != nullptr)
  {
  
    int size = List_Nbr(otherGeoData->Elements) + 2;    
    Message::Warning("List_Nbr =[%d]", size);
    newGeoData->H = new double [size];

    for (int f=0; f< size; f++ )
    newGeoData->H[f] = otherGeoData->H[f];
    
  }


  if (otherGeoData->P != nullptr )
  {
  
    int size = List_Nbr(otherGeoData->Elements) + 2;
    Message::Warning("List_Nbr =[%d]", size);
    
    newGeoData->P = new double [size];
    for (int f=0; f< size; f++ )
    newGeoData->P[f] = otherGeoData->P[f];
    
    }

    
  } else
  {
  
    newGeoData->H = nullptr;
    newGeoData->P = nullptr;
  }

	
	
	//newDefineSystem->FrequencyValue = nullptr;
	//if (otherDefineSystem->FrequencyValue != nullptr)
	//{
	//newDefineSystem->FrequencyValue = List_Create(List_Nbr(otherDefineSystem->FrequencyValue), 1, sizeof(double));
	//List_Copy(otherDefineSystem->FrequencyValue, newDefineSystem->FrequencyValue);
	//}

	
	
	
	newGeoData->Nodes = nullptr;
	if (otherGeoData->Nodes != nullptr)
	{
	newGeoData->Nodes = List_Create(List_Nbr(otherGeoData->Nodes), 1, sizeof(struct Geo_Node));
	List_Copy(otherGeoData->Nodes, newGeoData->Nodes);
	}
	
			
	newGeoData->Elements = nullptr;
	if (otherGeoData->Elements != nullptr)
	{
	newGeoData->Elements = List_Create(List_Nbr(otherGeoData->Elements), 1, sizeof(struct Geo_Element));
	List_Copy(otherGeoData->Elements, newGeoData->Elements);
	}
	
	
	newGeoData->GroupForPRE = nullptr;
	if (otherGeoData->GroupForPRE != nullptr)
	{
	newGeoData->GroupForPRE = List_Create(List_Nbr(otherGeoData->GroupForPRE), 1, sizeof(struct Group));
	List_Copy(otherGeoData->GroupForPRE, newGeoData->GroupForPRE);
	}
	
	newGeoData->PeriodicNodes = nullptr;
	if (otherGeoData->PeriodicNodes != nullptr)
	{
	newGeoData->PeriodicNodes = List_Create(List_Nbr(otherGeoData->PeriodicNodes), 1, sizeof(TwoInt));
	List_Copy(otherGeoData->PeriodicNodes, newGeoData->PeriodicNodes);
	}





/*
	if (otherGeoData->EdgesXNodes != nullptr)
	newGeoData->EdgesXNodes = Tree_Copy(otherGeoData->EdgesXNodes);
	
	
	if (otherGeoData->FacetsXEdges != nullptr)
	newGeoData->FacetsXEdges = Tree_Copy(otherGeoData->FacetsXEdges);
	
	
	if (otherGeoData->Normals != nullptr)
	newGeoData->Normals = Tree_Copy(otherGeoData->Normals);


	if (otherGeoData->NodesXElements != nullptr)
	newGeoData->NodesXElements = Tree_Copy(otherGeoData->NodesXElements);
*/	
  
	newGeoData->Xmin = otherGeoData->Xmin;
	newGeoData->Xmax = otherGeoData->Xmax;
	newGeoData->Ymin = otherGeoData->Ymin;
	newGeoData->Ymax = otherGeoData->Ymax;
	newGeoData->Zmin = otherGeoData->Zmin;
	newGeoData->Zmax = otherGeoData->Zmax;
	
	newGeoData->Dimension = otherGeoData->Dimension;
	newGeoData->CharacteristicLength = otherGeoData->CharacteristicLength;
	
    
  // TO DO copy 
  ///struct Grid Grid;
  
  
	newGeoData->Grid = Grid_Copy(otherGeoData->Grid);




  
    
// Message::Warning("List_Nbr 1 =[]");
  
  return newGeoData;
  
}

