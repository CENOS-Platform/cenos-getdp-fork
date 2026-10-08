#pragma once

#include "PostExternalData.h"
#include <list>

class EnsightExternalData : public PostExternalData {
public:
  static int g_pendingGeomStep;
  static bool g_deferCase;
  static std::string g_deferName;
  static void FlushDeferred();
private:
  // EnSight has no complete quad9/hexa27/penta18/pyramid14 cells: Order 2
  // exports the serendipity types (see Pos_PrintExternal.cpp)
  std::map<int, std::string> ToEnsightElementType = {
    {POINT_ELEMENT, "point"},       {LINE, "bar2"},
    {LINE_2, "bar3"},               {TRIANGLE, "tria3"},
    {TRIANGLE_2, "tria6"},          {QUADRANGLE, "quad4"},
    {QUADRANGLE_2_8N, "quad8"},     {TETRAHEDRON, "tetra4"},
    {TETRAHEDRON_2, "tetra10"},     {PYRAMID, "pyramid5"},
    {PYRAMID_2_13N, "pyramid13"},   {PRISM, "penta6"},
    {PRISM_2_15N, "penta15"},       {HEXAHEDRON, "hexa8"},
    {HEXAHEDRON_2_20N, "hexa20"}
    //,{not supported by GetDP, "nsided"},
    //{not supported by GetDP, "nfaced"}
  };
  std::map<int, std::string> ToEnsightValueType = {
    {SCALAR, "scalar"},
    {VECTOR, "vector"},
    {TENSOR, "tensor asymn"},
    {TENSOR_SYM, "tensor symn"},
    {TENSOR_DIAG, "tensor asymn"}};

  std::vector<elementsInPart>
    parts; // stores the parts for the current variable field, used to write
           // current geo file

  struct variableData {
    char *name;
    int value_type;
    int data_size;
  };
  std::vector<variableData> variables;

public:
  EnsightExternalData();
  ~EnsightExternalData();

  void write(std::string fname) override;

  int getNextTimeStep(std::string fname);
  std::vector<double> getTimeValues(std::string fname);

  void writeCaseFile(std::string fname);
  void writeVariableASCII(std::string fname);
  void writeVariableBinary(std::string fname);
  void writeGeometryASCII(std::string fname);
  void writeGeometryBinary(std::string fname, int stepOverride = -1);

  void groupParts();
  void groupElementTypes(elementsInPart &el_part);
  void addTime();
  void addVariable();

  void WriteStringToFile(const char *cstring, FILE *file);
  void WriteIntToFile(const int i, FILE *file);
  void WriteFloatToFile(const float i, FILE *file);
};
