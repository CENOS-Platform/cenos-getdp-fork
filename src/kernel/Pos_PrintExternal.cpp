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
extern struct CurrentData Current;
extern char *Name_Path;
/* ------------------------------------------------------------------------ */
/*  P o s _ P r i n t V T U                                       */
/* ------------------------------------------------------------------------ */
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
  
  int Store = 0;
   if(PSO_P->Smoothing ) Store = 1;

  if(PSO_P->Binary == 1)
    post_data->wtype = PostExternalData::WriteType::BINARY;
  else
    post_data->wtype = PostExternalData::WriteType::ASCII;
  ///

  // Smoothing taken from Pos_Print
  Tree_T *xyzv_T;
  struct PostElement *PE;
  struct xyzv xyzv, *xyzv_P;
  double Dummy[5], x1, x2;
  List_T *PostElement_L;
  int NbrGeo = Geo_GetNbrGeoElements();
  PostElement_L =
    List_Create(Store ? NbrGeo / 10 : 10, Store ? NbrGeo / 10 : 10,
                sizeof(struct PostElement *));
  if(PSO_P->Smoothing) {
    Message::Info("Smoothing (phase 1) External");

    xyzv_T = Tree_Create(sizeof(struct xyzv), fcmp_xyzv);

    for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
      PE = *(struct PostElement **)List_Pointer(PostElement_L, iPost);
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

    Message::Info("Smoothing (phase 2) External");

    for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
      PE = *(struct PostElement **)List_Pointer(PostElement_L, iPost);
      for(int iNode = 0; iNode < PE->NbrNodes; iNode++) {
        xyzv.x = PE->x[iNode];
        xyzv.y = PE->y[iNode];
        xyzv.z = PE->z[iNode];
        if((xyzv_P = (struct xyzv *)Tree_PQuery(xyzv_T, &xyzv))) {
          Cal_CopyValue(&xyzv_P->v, &PE->Value[iNode]);
        }
        else
          Message::Warning("Node (%g,%g,%g) not found", xyzv.x, xyzv.y, xyzv.z);
      }
    }

    Tree_Delete(xyzv_T);
  }

  // Store

  if(Store) {
    /* Sort the elements */

    /*switch(PSO_P->Sort) {
    case SORT_BY_POSITION: List_Sort(PostElement_L, fcmp_PostElement); break;
    case SORT_BY_CONNECTIVITY:
      Sort_PostElement_Connectivity(PostElement_L);
      break;
    }*/

    Dummy[0] = Dummy[1] = Dummy[2] = Dummy[3] = Dummy[4] = 0.;

    for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
      PE = *(struct PostElement **)List_Pointer(PostElement_L, iPost);

      /* Get the values from adaption */
      /*if(PSO_P->Adapt) {
        Element.GeoElement = Geo_GetGeoElement(PE->Index);

        Dummy[0] = Element.GeoElement->Num;
        Dummy[1] = Error[PE->Index + 1];
        Dummy[2] = Current.GeoData->H[PE->Index + 1];
        Dummy[3] = Current.GeoData->P[PE->Index + 1];
        Dummy[4] = iPost ? 0 : NbrPost;

        for(iNode = 0; iNode < PE->NbrNodes; iNode++) {
          PE->Value[iNode].Type = SCALAR;
          if(PSO_P->Adapt == ADAPT_H1 || PSO_P->Adapt == ADAPT_H2)
            PE->Value[iNode].Val[0] = Dummy[2];
          else
            PE->Value[iNode].Val[0] = Dummy[3];
        }
      }*/

      /* Compute curvilinear coord if connection sort */
      /*if(PSO_P->Sort == SORT_BY_CONNECTIVITY) {
        Dummy[0] = Dummy[1];
        Dummy[1] =
          Dummy[0] + sqrt(SQU(PE->x[1] - PE->x[0]) + SQU(PE->y[1] - PE->y[0]) +
                          SQU(PE->z[1] - PE->z[0]));
        Dummy[2] = PE->v[0];
        Dummy[3] = -1.;
      }*/

      /*Format_PostElement(PSO_P, PSO_P->Iso, 1, Current.Time, 0, 1,
                         Current.NbrHar, PSO_P->HarmonicToTime, Dummy, PE);*/
      List_Add(PostElement_L, &PE);
    }
  }

  // Format_PostFooter(PSO_P, Store);

  if(Store)
    for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
      PE = *(struct PostElement **)List_Pointer(PostElement_L, iPost);
      //post_data->addElement(PE);
      Destroy_PostElement(PE);
    }

  //List_Delete(PostElement_L);

  ///

  // Fill post_data with elements and nodes
  struct Group *Group_P = (struct Group *)List_Pointer(
    Problem_S.Group, PSO_P->Case.OnRegion.RegionIndex);
  List_T *Region_L = Group_P->InitialList;
  struct Element Element;
  Get_InitDofOfElement(&Element);
  for(int iGeo = 0; iGeo < Geo_GetNbrGeoElements(); iGeo++) {
    //List_T *PostElement_L = List_Create(10, 10, sizeof(struct PostElement *));

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
      post_data->addElement(PE);
      Destroy_PostElement(PE);
    }
  }

  // outer time loop
  struct PostQuantity *PostQuantity_P;
  Message::Info("outer time loop");
  for(int iTime = 0; iTime < Pos_InitTimeSteps(PSO_P); iTime++) {
    Pos_InitAllSolutions(PSO_P->TimeStep_L, iTime);

    TimeStepData tdata;
    tdata.time_step = Current.TimeStep;
    tdata.time_value = Current.Time;
    tdata.freq_value = Current.Frequency;

    for(int ipq = 0; ipq < List_Nbr(PSO_P->PointQuantities); ipq++) {
      PostQuantity_P = (struct PostQuantity *)List_Pointer(
        PostProcessing_P->PostQuantity,
        *(int *)List_Pointer(PSO_P->PointQuantities, ipq));
      PointDataSet pd_set(PostQuantity_P->Name);

      for(int iGeo = 0; iGeo < Geo_GetNbrGeoElements(); iGeo++) {
        List_T *PostElement_L =
          List_Create(10, 10, sizeof(struct PostElement *));

        Element.GeoElement = Geo_GetGeoElement(iGeo);
        if((Group_P->Type != ELEMENTLIST &&
            List_Search(Region_L, &Element.GeoElement->Region, fcmp_int)) ||
           (Group_P->Type == ELEMENTLIST &&
            Check_IsEntityInExtendedGroup(Group_P, Element.GeoElement->Num,
                                          0))) {
          Fill_PostElement(Element.GeoElement, PostElement_L, iGeo,
                           PSO_P->Depth, PSO_P->Skin, 0, 0, PSO_P->Gauss);
        }

        for(int iPost = 0; iPost < List_Nbr(PostElement_L); iPost++) {
          struct PostElement *PE =
            *(struct PostElement **)List_Pointer(PostElement_L, iPost);

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
            pd_set.addData(post_data->node_map[PE->NumNodes[iNode]],
                           data_point);
            pd_set.addDataRegion(post_data->node_map[PE->NumNodes[iNode]],
                                 data_point, Element.GeoElement->Region);
          }

          Destroy_PostElement(PE);
        }

        List_Delete(PostElement_L);
      } /* for iGeo */

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
