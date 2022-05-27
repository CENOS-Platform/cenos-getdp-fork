
#include "PostExternalData.h"
#include "Message.h"
#include "GeoData.h"
#include "Get_Geometry.h"

extern struct CurrentData Current;
extern struct EnsightCase Ensight_Case;

PointDataSet::PointDataSet(char *n) : name(n)
{
  value_type = -1; // -1 means not set
}

void PointDataSet::setValueType(int vt)
{
  value_type = vt;
  if(value_type != -1) {
    switch(vt) {
    case SCALAR: data_size = 1; break;
    case VECTOR: data_size = 3; break;
    case TENSOR_DIAG: data_size = 3; break;
    case TENSOR_SYM: data_size = 6; break;
    case TENSOR: data_size = 9; break;
    default: Message::Error("Unsupported value type for %s", name);
    }
  }
}

void PointDataSet::addData(int i, std::vector<double> d) { data[i] = d; }
void PointDataSet::addDataRegion(int i, std::vector<double> d, int region)
{
  data_region[region][i] = d;
}
// Class PostExternalData definition

PostExternalData::PostExternalData() {}

PostExternalData::~PostExternalData() {}

void PostExternalData::addElement(PostElement *PE, int region_nr)
{
  struct Element e;
  e.GeoElement = Geo_GetGeoElement(PE->Index);
  e.Num = e.GeoElement->Num;
  e.Type = e.GeoElement->Type;
  Current.Region = e.Region = e.GeoElement->Region;
  Get_NodesCoordinatesOfElement(&e);

  PostExternalElement post_el;
  post_el.type = e.Type;
  post_el.index = PE->Index;
  post_el.region = region_nr; // e.GeoElement->ElementaryRegion;

  for(int iNode = 0; iNode < e.GeoElement->NbrNodes; iNode++) {
    if(node_map.count(PE->NumNodes[iNode]) == 0) {
      node_map[PE->NumNodes[iNode]] = node_map.size();
      node_coordinates.push_back(
        std::vector<double>({PE->x[iNode], PE->y[iNode], PE->z[iNode]}));
      
    }
    post_el.nodes.push_back(node_map[PE->NumNodes[iNode]]);
    post_el.nodes_coordinates.push_back(
      std::vector<double>({PE->x[iNode], PE->y[iNode], PE->z[iNode]}));

    // node_map_region[post_el.region] = node_map[PE->NumNodes[iNode]];
  }

  if(!region_elements.count(post_el.region)) {
    region_elements[post_el.region] = std::vector<int>();
  }
  region_elements[post_el.region].push_back(elements.size());

  elements.push_back(post_el);
  // deep copy of post_el
  PostExternalElementCopy elementCopy;
  elementCopy.index = post_el.index;
  elementCopy.nodes = post_el.nodes;
  elementCopy.region = post_el.region;
  elementCopy.type = post_el.type;
  elements_in_region[post_el.region].push_back(elementCopy);
  // deep copy nodes of each element
  node_map_region[post_el.region].insert(post_el.nodes.begin(), post_el.nodes.end());
}
