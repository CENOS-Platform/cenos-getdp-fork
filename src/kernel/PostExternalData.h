#pragma once

#include <vector>
#include "ProData.h"
#include "Pos_Element.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>

class PointDataSet {
public:
  PointDataSet(char *n);
  void setValueType(int vt);
  void addData(int i, std::vector<double> d);
  void addDataRegion(int i, std::vector<double> d, int region);
  char *name;
  char *groupName;
  int value_type;
  int data_size;

  std::unordered_map<int, std::vector<double> > data;
  std::unordered_map<int, std::unordered_map<int, std::vector<double> > >
    data_region;
};

struct PostExternalElement {
  int type;
  int index;
  int region;
  std::vector<int> nodes;
  std::vector<std::vector<double> > nodes_coordinates;
};

struct TimeStepData {
  int time_step;
  double time_value;
  int freq_step;
  double freq_value;
  std::vector<PointDataSet> point_data;
};

class PostExternalData {
public:
  enum WriteType { ASCII, BINARY };

  WriteType wtype;

  // unordered map showed about 15% better performance in tests than map
  std::unordered_map<int, int> node_map; // map to store new node ids
  std::unordered_map<int, std::vector<int> >
    node_map_region; // map to store new node ids for each region, first int is region, second int is node
  std::unordered_map<int, std::unordered_set<int>> node_set_region;
  
  std::vector<std::vector<double> > node_coordinates;
  std::unordered_map<int, std::vector<int> >
    region_elements; // int is a region, vector<int> is id for elements in this region

  std::vector<TimeStepData> data_sets;
  std::vector<PostExternalElement> elements;//contains all elements
  std::unordered_map<int, std::vector<PostExternalElementCopy> >
    elements_in_region; // 

  PostExternalData();
  virtual ~PostExternalData();

  void addElement(PostElement *PE, int region_nr);

  virtual void write(std::string fname) = 0;
};

