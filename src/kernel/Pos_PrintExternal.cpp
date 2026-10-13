#include <sstream>
#include <iostream>
#include "Pos_PrintExternal.h"
#include <string.h>
#include "Pos_Formulation.h"
#include "Message.h"
#include "OS.h"
#include "Get_DofOfElement.h"
#include "DofData.h"
#include "GeoData.h"
#include "ExtendedGroup.h"
#include "Get_Geometry.h"
#include "Cal_PostQuantity.h"
#include "Cal_Value.h"
#include <type_traits>
#include "VTUExternalData.h"
#include "EnsightExternalData.h"
#include "ProDefine.h"
#include <fstream>
#include <algorithm>
#include <unordered_map>

extern struct Problem Problem_S;
extern struct CurrentData Current;
extern char *Name_Path;
extern struct EnsightCase Ensight_Case;

/* ------------------------------------------------------------------------ */
/*  P o s _ P r i n t V T U                                       */
/* ------------------------------------------------------------------------ */

struct xyzv {
  double x, y, z;
  struct Value v;
  /*int nbvals; for time domain -> malloc Value *v... */
  int nboccurences;
};

static int fcmp_xyzv(const void *a, const void *b)
{
  struct xyzv *p1, *p2;
  double TOL = Current.GeoData->CharacteristicLength * 1.e-8;
  p1 = (struct xyzv *)a;
  p2 = (struct xyzv *)b;
  if(p1->x - p2->x > TOL) return 1;
  if(p1->x - p2->x < -TOL) return -1;
  if(p1->y - p2->y > TOL) return 1;
  if(p1->y - p2->y < -TOL) return -1;
  if(p1->z - p2->z > TOL) return 1;
  if(p1->z - p2->z < -TOL) return -1;
  return 0;
}

/* ------------------------------------------------------------------------ */
/*  Second-order PostElements (PrintExternal Order 2)                        */
/* ------------------------------------------------------------------------ */

// Gmsh second-order node order: vertices, then one node per edge in this
// order. GetDP's Den_* edge tables differ from it for triangles, quadrangles
// and tetrahedra, so they cannot be used here.
static const int Edges2_Line[][2] = {{0, 1}};
static const int Edges2_Triangle[][2] = {{0, 1}, {1, 2}, {2, 0}};
static const int Edges2_Quadrangle[][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
static const int Edges2_Tetrahedron[][2] = {{0, 1}, {1, 2}, {2, 0},
                                            {3, 0}, {3, 2}, {3, 1}};
static const int Edges2_Hexahedron[][2] = {{0, 1}, {0, 3}, {0, 4}, {1, 2},
                                           {1, 5}, {2, 3}, {2, 6}, {3, 7},
                                           {4, 5}, {4, 7}, {5, 6}, {6, 7}};
static const int Edges2_Prism[][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 4},
                                      {2, 5}, {3, 4}, {3, 5}, {4, 5}};
static const int Edges2_Pyramid[][2] = {{0, 1}, {0, 3}, {0, 4}, {1, 2},
                                        {1, 4}, {2, 3}, {2, 4}, {3, 4}};

// Complete second-order types (quad9, hex27, prism18) are reduced to their
// serendipity subset, the only quadratic cells EnSight Gold has.
static int Get_Order2Type(int Type, int *BaseType, const int (**Edges)[2],
                          int *NbrEdges)
{
  switch(Type) {
  case POINT_ELEMENT:
    *BaseType = POINT_ELEMENT;
    *NbrEdges = 0;
    return POINT_ELEMENT;
  case LINE:
  case LINE_2:
    *BaseType = LINE;
    *Edges = Edges2_Line;
    *NbrEdges = 1;
    return LINE_2;
  case TRIANGLE:
  case TRIANGLE_2:
    *BaseType = TRIANGLE;
    *Edges = Edges2_Triangle;
    *NbrEdges = 3;
    return TRIANGLE_2;
  case QUADRANGLE:
  case QUADRANGLE_2:
  case QUADRANGLE_2_8N:
    *BaseType = QUADRANGLE;
    *Edges = Edges2_Quadrangle;
    *NbrEdges = 4;
    return QUADRANGLE_2_8N;
  case TETRAHEDRON:
  case TETRAHEDRON_2:
    *BaseType = TETRAHEDRON;
    *Edges = Edges2_Tetrahedron;
    *NbrEdges = 6;
    return TETRAHEDRON_2;
  case HEXAHEDRON:
  case HEXAHEDRON_2:
  case HEXAHEDRON_2_20N:
    *BaseType = HEXAHEDRON;
    *Edges = Edges2_Hexahedron;
    *NbrEdges = 12;
    return HEXAHEDRON_2_20N;
  case PRISM:
  case PRISM_2:
  case PRISM_2_15N:
    *BaseType = PRISM;
    *Edges = Edges2_Prism;
    *NbrEdges = 9;
    return PRISM_2_15N;
  // not PYRAMID_2: GetDP counts 13 nodes for it while Gmsh writes 14
  case PYRAMID:
  case PYRAMID_2_13N:
    *BaseType = PYRAMID;
    *Edges = Edges2_Pyramid;
    *NbrEdges = 8;
    return PYRAMID_2_13N;
  default: return -1;
  }
}

static struct PostElement *
Create_Order2PostElement(struct Geo_Element *GE, int Index,
                         std::unordered_map<long long, int> &EdgeNodeNum)
{
  int BaseType, NbrEdges = 0, NbrVertices;
  const int(*Edges)[2] = NULL;
  int Type = Get_Order2Type(GE->Type, &BaseType, &Edges, &NbrEdges);
  if(Type < 0) {
    Message::Error("PrintExternal Order 2 does not support elements of type %s",
                   Get_StringForDefine(Element_Type, GE->Type));
    return NULL;
  }
  double *uvw = Geo_GetNodes_uvw(BaseType, &NbrVertices);

  struct Element E;
  Get_InitDofOfElement(&E);
  E.GeoElement = GE;
  E.Num = GE->Num;
  E.Type = GE->Type;
  E.Region = GE->Region;
  Get_NodesCoordinatesOfElement(&E);

  struct PostElement *PE =
    Create_PostElement(Index, Type, NbrVertices + NbrEdges, 1);
  for(int i = 0; i < NbrVertices; i++) {
    PE->NumNodes[i] = GE->NumNodes[i];
    PE->u[i] = uvw[3 * i];
    PE->v[i] = uvw[3 * i + 1];
    PE->w[i] = uvw[3 * i + 2];
    PE->x[i] = E.x[i];
    PE->y[i] = E.y[i];
    PE->z[i] = E.z[i];
  }
  for(int k = 0; k < NbrEdges; k++) {
    int a = Edges[k][0], b = Edges[k][1], i = NbrVertices + k;
    long long lo = std::min(GE->NumNodes[a], GE->NumNodes[b]);
    long long hi = std::max(GE->NumNodes[a], GE->NumNodes[b]);
    // mesh nodes are > 0 and Cut_PostElement uses -1, so mid-edge nodes are
    // numbered -2, -3, ... to stay distinct in node_map
    PE->NumNodes[i] =
      EdgeNodeNum.emplace((lo << 32) | hi, -2 - (int)EdgeNodeNum.size())
        .first->second;
    PE->u[i] = 0.5 * (uvw[3 * a] + uvw[3 * b]);
    PE->v[i] = 0.5 * (uvw[3 * a + 1] + uvw[3 * b + 1]);
    PE->w[i] = 0.5 * (uvw[3 * a + 2] + uvw[3 * b + 2]);
    // through the geometric mapping, so a curved second-order mesh gets its
    // own mid-edge node
    Get_BFGeoElement(&E, PE->u[i], PE->v[i], PE->w[i]);
    PE->x[i] = PE->y[i] = PE->z[i] = 0.;
    for(int j = 0; j < GE->NbrNodes; j++) {
      PE->x[i] += E.x[j] * E.n[j];
      PE->y[i] += E.y[j] * E.n[j];
      PE->z[i] += E.z[j] * E.n[j];
    }
  }
  return PE;
}

void Pos_PrintExternal(struct PostProcessing *PostProcessing_P, int Order,
                       struct DefineQuantity *DefineQuantity_P0,
                       struct QuantityStorage *QuantityStorage_P0,
                       struct PostSubOperation *PSO_P)
{
  PostExternalData *post_data;
  int Store = 0;
  if(PSO_P->Smoothing) { Store = 1; }

  if(PSO_P->ElementOrder == 2) {
    if(PSO_P->Format != FORMAT_ENSIGHT) {
      Message::Error("PrintExternal Order 2 is only available with Format ENSIGHT");
      return;
    }
    if(PSO_P->Depth != 1 || PSO_P->Skin || PSO_P->Gauss) {
      Message::Error("PrintExternal Order 2 cannot be combined with Depth, Skin or Gauss");
      return;
    }
  }

  if(PSO_P->Format == FORMAT_VTU)
    post_data = new VTUExternalData();
  else if(PSO_P->Format == FORMAT_ENSIGHT)
    post_data = new EnsightExternalData();
  else {
    Message::Error(
      "Unknown format ",
      PSO_P->Format); // is this needed? Should be checked at parsing?!
    return;
  }

  if(PSO_P->Binary == 1)
    post_data->wtype = PostExternalData::WriteType::BINARY;
  else
    post_data->wtype = PostExternalData::WriteType::ASCII;

  // Fill post_data with elements and nodes
  struct Group *Group_P = (struct Group *)List_Pointer(
    Problem_S.Group, PSO_P->Case.OnRegion.RegionIndex);
  List_T *Region_L = Group_P->InitialList;
  struct Element Element;
  Get_InitDofOfElement(&Element);

  /* Create the list of PostElements */

  /* One PostElement list per call, shared by the geometry registration and by
   * every quantity (previously rebuilt per quantity: N+1 mesh scans for N). */
  int NbrGeoAll = Geo_GetNbrGeoElements();
  List_T *PostElement_L =
    List_Create(NbrGeoAll / 10 + 10, NbrGeoAll / 10 + 10,
                sizeof(struct PostElement *));
  std::unordered_map<long long, int> EdgeNodeNum;
  for(int iGeo = 0; iGeo < NbrGeoAll; iGeo++) {
    Element.GeoElement = Geo_GetGeoElement(iGeo);
    if((Group_P->Type != ELEMENTLIST &&
        List_Search(Region_L, &Element.GeoElement->Region, fcmp_int)) ||
       (Group_P->Type == ELEMENTLIST &&
        Check_IsEntityInExtendedGroup(Group_P, Element.GeoElement->Num, 0))) {
      if(PSO_P->ElementOrder == 2) {
        struct PostElement *PE =
          Create_Order2PostElement(Element.GeoElement, iGeo, EdgeNodeNum);
        if(!PE) {
          for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
            Destroy_PostElement(
              *(struct PostElement **)List_Pointer(PostElement_L, iPost));
          }
          List_Delete(PostElement_L);
          delete post_data;
          return;
        }
        List_Add(PostElement_L, &PE);
      }
      else {
        Fill_PostElement(Element.GeoElement, PostElement_L, iGeo,
                         PSO_P->Depth, PSO_P->Skin, 0, 0, PSO_P->Gauss);
      }
    }
  }
  for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
    struct PostElement *PE =
      *(struct PostElement **)List_Pointer(PostElement_L, iPost);
    post_data->addElement(PE, Group_P->Num);
  }

  // outer time loop
  struct PostQuantity *PostQuantity_P;
  for(int iTime = 0; iTime < Pos_InitTimeSteps(PSO_P); iTime++) {
    Pos_InitAllSolutions(PSO_P->TimeStep_L, iTime);

    TimeStepData tdata;
	if (Current.TypeTime == TIME_STATIC)	
		tdata.time_step = 1;
	else
		tdata.time_step = Current.TimeStep;
    tdata.time_value = Current.Time;
    
    /*Message::Info("------ PSO_P->SetFrequencyScale  ");
    if(PSO_P->SetFrequencyScale == NULL) {
      Message::Info("------ PSO_P->SetFrequencyScale  NULL");
    }
    else {
      Message::Info("------ PSO_P->SetFrequencyScale  NOT NULL %s",
                    PSO_P->SetFrequencyScale);
    }*/
    double frequencyScale;
    sscanf(PSO_P->SetFrequencyScale, "%lf", &frequencyScale);
    //Message::Info("------ frequencyMultiplier = %lf", SetFrequencyScale);
	if (strcmp(Current.Step_Type, "Freq") == 0)
		tdata.freq_value = Current.Frequency / frequencyScale;
	else
		tdata.freq_value = 0;
	
  
    for(int ipq = 0; ipq < List_Nbr(PSO_P->PointQuantities); ipq++) {
      PostQuantity_P = (struct PostQuantity *)List_Pointer(
        PostProcessing_P->PostQuantity,
        *(int *)List_Pointer(PSO_P->PointQuantities, ipq));
      PointDataSet pd_set(PostQuantity_P->Name);
      pd_set.groupName =
        (PSO_P->PartName != NULL) ?
          PSO_P->PartName :
          Group_P->Name; // gives as default name the one defined in Group,
                         // optional name the one defined in PrintExternal with
                         // option: PartName "include_your_name"

      for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
        struct PostElement *PE =
          *(struct PostElement **)List_Pointer(PostElement_L, iPost);
        Element.GeoElement = Geo_GetGeoElement(
          PE->Index); // this is important if we have PostElement_L for all
                      // elements, not only for each iGeo
        Element.Num = Element.GeoElement->Num;
        Element.Type = Element.GeoElement->Type;
        Current.Region = Element.Region = Element.GeoElement->Region;
        Get_NodesCoordinatesOfElement(&Element);
        for(int iNode = 0; iNode < PE->NbrNodes; iNode++) {
          Current.x = PE->x[iNode];
          Current.y = PE->y[iNode];
          Current.z = PE->z[iNode];
          Cal_PostQuantity(PostQuantity_P, DefineQuantity_P0,
                           QuantityStorage_P0, PSO_P->Distributed, NULL,
                           &Element, PE->u[iNode], PE->v[iNode],
                           PE->w[iNode], &PE->Value[iNode]);
          pd_set.setValueType(PE->Value[0].Type);
          std::vector<double> data_point(
            PE->Value[iNode].Val, PE->Value[iNode].Val + pd_set.data_size);
          pd_set.addData(post_data->node_map[PE->NumNodes[iNode]], data_point);
          pd_set.addDataRegion(post_data->node_map[PE->NumNodes[iNode]],
                               data_point, Group_P->Num);
        }
      } /* PEs reused across quantities, freed at end of call */

      /* Perform Smoothing */ // Smoothing taken from Pos_Print

      if(PSO_P->Smoothing) {
        Tree_T *xyzv_T;
        double x1, x2;
        struct xyzv xyzv, *xyzv_P;

        xyzv_T = Tree_Create(sizeof(struct xyzv), fcmp_xyzv);
        //Message::Info("Smoothing (phase 1)");
        for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
          struct PostElement *PE =
            *(struct PostElement **)List_Pointer(PostElement_L, iPost);
          for(int iNode = 0; iNode < PE->NbrNodes; iNode++) {
            xyzv.x = PE->x[iNode];
            xyzv.y = PE->y[iNode];
            xyzv.z = PE->z[iNode];
            if((xyzv_P = (struct xyzv *)Tree_PQuery(xyzv_T, &xyzv))) {
              x1 = (double)(xyzv_P->nboccurences) /
                   (double)(xyzv_P->nboccurences + 1.);
              x2 = 1. / (double)(xyzv_P->nboccurences + 1);
              Cal_AddMultValue2(&xyzv_P->v, x1, &PE->Value[iNode], x2);
              xyzv_P->nboccurences++;
            }
            else {
              Cal_CopyValue(&PE->Value[iNode], &xyzv.v);
              xyzv.nboccurences = 1;
              Tree_Add(xyzv_T, &xyzv);
            }
          }
        }

        //Message::Info("Smoothing (phase 2)");

        for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
          struct PostElement *PE =
            *(struct PostElement **)List_Pointer(PostElement_L, iPost);
          for(int iNode = 0; iNode < PE->NbrNodes; iNode++) {
            xyzv.x = PE->x[iNode];
            xyzv.y = PE->y[iNode];
            xyzv.z = PE->z[iNode];
            if((xyzv_P = (struct xyzv *)Tree_PQuery(xyzv_T, &xyzv))) {
              Cal_CopyValue(&xyzv_P->v, &PE->Value[iNode]);
            }
            else {
              Message::Warning("Node (%g,%g,%g) not found", xyzv.x, xyzv.y,
                               xyzv.z);
            }
            std::vector<double> data_point(
              PE->Value[iNode].Val, PE->Value[iNode].Val + pd_set.data_size);
            pd_set.addData(post_data->node_map[PE->NumNodes[iNode]],
                           data_point);
            pd_set.addDataRegion(post_data->node_map[PE->NumNodes[iNode]],
                                 data_point, Element.GeoElement->Region);
          }
        }
        Tree_Delete(xyzv_T);
      } /* if Smoothing */

      tdata.point_data.push_back(pd_set);
    }
    post_data->data_sets.push_back(tdata);
  }

  for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
    Destroy_PostElement(
      *(struct PostElement **)List_Pointer(PostElement_L, iPost));
  }
  List_Delete(PostElement_L);

  std::string filename = Fix_RelativePath(PSO_P->FileOut, Name_Path);
  // read Current on this thread; the writer must not touch it
  post_data->stepTypeIsFreq = (strcmp(Current.Step_Type, "Freq") == 0);
  PostExternalData::QueueWrite(post_data, filename); // takes ownership

  /*
      for (int iTime = 0 ; iTime < NbrTimeStep ; iTime++)
      {
          int  TimeStepIndex, Num_Solution;
          List_Read(PSO_P->TimeStep_L, iTime, &TimeStepIndex);

          for(int k = 0 ; k < Current.NbrSystem ; k++)

          if( (Num_Solution =
     std::min(List_Nbr((Current.DofData_P0+k)->Solutions)-1, TimeStepIndex)) >=0
     ) (Current.DofData_P0+k)->CurrentSolution = (struct
     Solution*)List_Pointer((Current.DofData_P0+k)->Solutions, Num_Solution) ;

          if(TimeStepIndex >= 0 && TimeStepIndex <
     List_Nbr(Current.DofData->Solutions))
          {
              Solution *Solution_P = ((struct
     Solution*)List_Pointer(Current.DofData->Solutions, TimeStepIndex));
              Current.TimeStep = Solution_P->TimeStep ;
              Current.Time = Solution_P->Time ;
              Current.TimeImag = Solution_P->TimeImag ;
          }
          else
          { // Warning: this can be wrong
              Current.TimeStep = TimeStepIndex;
              if(Current.DofData->CurrentSolution)
              {
                  Current.Time = Current.DofData->CurrentSolution->Time;
                  Current.TimeImag = Current.DofData->CurrentSolution->TimeImag;
              }
          }
          std::cout << Current.Time << std::endl;
      }
          */
}
/* ------------------------------------------------------------------------ */
/*  P o s _ O p t i o n s                                                   */
/* ------------------------------------------------------------------------ */

void Pos_PrintExternalStepType(struct PostSubOperation *PSO_P)
{
  PostExternalData::JoinWrites(); // mutates Ensight_Case below
  int NbrTimeStep, iTime;
  struct Value Value;
  char *str = PSO_P->Case.Expression.String;
  char *str2 = PSO_P->Case.Expression.String2;
  List_T *expr = PSO_P->Case.Expression.Expressions;

  if((!str || !strlen(str)) && (!str2 || !strlen(str2)) && !List_Nbr(expr))
    return; // no options to set
  Current.Step_Type = str;
}

void Pos_PrintExternalFromPrevious(struct PostSubOperation *PSO_P)
{
  PostExternalData::JoinWrites(); // mutates Ensight_Case below
  // if multiple From Previous command is called again, check if there already
  // are values before adding to Ensight Case
  // This IF ensures that start index and time values are read only once
  if (!Ensight_Case.time_values.empty()) 
     return;	  

  if (!PSO_P->FileIn)
  {
      Ensight_Case.From_Previous = false;
	  return;
  }

  std::string filename = Fix_RelativePath(PSO_P->FileIn, Name_Path);

  Ensight_Case.From_Previous = true;

  // now read file and find next step
  // C stdio, not std::ifstream: with -mcrtdll=ucrt the inlined fstream layout
  // (8-byte mbstate_t) does not match the msvcrt-built libstdc++.a
  int next_step = 0;
  double time;
  std::vector<double> time_values;
  FILE *fp = FOpen(filename.c_str(), "r");
  if(fp) {
    char word[1024];
    while(fscanf(fp, "%1023s", word) == 1) {
      if(!strcmp(word, "steps:")) {
        if(fscanf(fp, "%d", &next_step) != 1) next_step = 0;
      }
      if(!strcmp(word, "values:")) {
        // in ensight format nr of steps is before time data
        for(int i = 0; i < next_step; i++) {
          if(fscanf(fp, "%lf", &time) != 1) break;
          time_values.push_back(time);
        }
      }
    }
    fclose(fp);
  }

  Ensight_Case.time_values = time_values;
  Ensight_Case.start_index = next_step;

  Message::Info("PrintExternal_FromPrevious: %d time step(s) in '%s'", next_step,
                filename.c_str());
  
}
