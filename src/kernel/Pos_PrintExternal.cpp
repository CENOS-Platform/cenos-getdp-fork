#include <sstream>
#include <iostream>
#include "Pos_PrintExternal.h"
#include "Pos_Formulation.h"
#include "Message.h"
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

extern struct Problem Problem_S;
extern thread_local struct CurrentData Current;
extern char *Name_Path;
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
void Pos_PrintExternal(struct PostProcessing *PostProcessing_P, int Order,
                       struct DefineQuantity *DefineQuantity_P0,
                       struct QuantityStorage *QuantityStorage_P0,
                       struct PostSubOperation *PSO_P)
{
  PostExternalData *post_data;
  int Store = 0;
  if(PSO_P->Smoothing) { Store = 1; }

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

  /* Generate all PostElements */
  for(int iGeo = 0; iGeo < Geo_GetNbrGeoElements(); iGeo++) {
    List_T *PostElement_L = List_Create(10, 10, sizeof(struct PostElement *));
    Element.GeoElement = Geo_GetGeoElement(iGeo);
    if((Group_P->Type != ELEMENTLIST &&
        List_Search(Region_L, &Element.GeoElement->Region, fcmp_int)) ||
       (Group_P->Type == ELEMENTLIST &&
        Check_IsEntityInExtendedGroup(Group_P, Element.GeoElement->Num, 0))) {
      Fill_PostElement(Element.GeoElement, PostElement_L, iGeo, PSO_P->Depth,
                       PSO_P->Skin, 0, 0, PSO_P->Gauss);
    }

    for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
      struct PostElement *PE =
        *(struct PostElement **)List_Pointer(PostElement_L, iPost);
      post_data->addElement(PE, Group_P->Num);
      Destroy_PostElement(PE);
    }
    List_Delete(PostElement_L);
  }

  // outer time loop
  struct PostQuantity *PostQuantity_P;
  for(int iTime = 0; iTime < Pos_InitTimeSteps(PSO_P); iTime++) {
    Pos_InitAllSolutions(PSO_P->TimeStep_L, iTime);

    TimeStepData tdata;
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
    tdata.freq_value = Current.Frequency / frequencyScale;
    
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

      int NbrGeo = Geo_GetNbrGeoElements();
      List_T *PostElement_L =
        List_Create(Store ? NbrGeo / 10 : 10, Store ? NbrGeo / 10 : 10,
                    sizeof(struct PostElement *));

      for(int iGeo = 0; iGeo < Geo_GetNbrGeoElements(); iGeo++) {
        Element.GeoElement = Geo_GetGeoElement(iGeo);
        if((Group_P->Type != ELEMENTLIST &&
            List_Search(Region_L, &Element.GeoElement->Region, fcmp_int)) ||
           (Group_P->Type == ELEMENTLIST &&
            Check_IsEntityInExtendedGroup(Group_P, Element.GeoElement->Num,
                                          0))) {
          Fill_PostElement(Element.GeoElement, PostElement_L, iGeo,
                           PSO_P->Depth, PSO_P->Skin, 0, 0, PSO_P->Gauss);
        }

      } /* for iGeo */

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
                           QuantityStorage_P0, NULL, &Element, PE->u[iNode],
                           PE->v[iNode], PE->w[iNode], &PE->Value[iNode]);
          pd_set.setValueType(PE->Value[0].Type);
          std::vector<double> data_point(
            PE->Value[iNode].Val, PE->Value[iNode].Val + pd_set.data_size);
          pd_set.addData(post_data->node_map[PE->NumNodes[iNode]], data_point);
          pd_set.addDataRegion(post_data->node_map[PE->NumNodes[iNode]],
                               data_point, Group_P->Num);
        }
        if(!PSO_P->Smoothing) {
          Destroy_PostElement(PE);
        } // Do not destroy elements if you are going to SMOOTH the data
      }

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
          Destroy_PostElement(PE);
        }
        Tree_Delete(xyzv_T);
      } /* if Smoothing */

      List_Delete(PostElement_L);
      tdata.point_data.push_back(pd_set);
    }
    post_data->data_sets.push_back(tdata);
  }

  std::string filename = Fix_RelativePath(PSO_P->FileOut, Name_Path);
  post_data->write(filename);

  delete post_data;

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
