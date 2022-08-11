
#include "EnsightExternalData.h"
#include <string.h>
#include "Message.h"
#include "OS.h"
#include <cstring>
#include <algorithm>
#include <unordered_set>
#include <fstream>  

#include <chrono>
extern char *Name_Path;

extern struct EnsightCase Ensight_Case;

extern struct CurrentData Current;

EnsightExternalData::EnsightExternalData() {}

EnsightExternalData::~EnsightExternalData() {}

// Use always with LastTimeStep option!!! Only one <group-id> in each
// PrintExternal
// Example 			PrintExternal[ PointData {<post-quantity-id>}, OnElementsOf
// Region[{ <group-id>}], File "<folder-id>",Format ENSIGHT ,LastTimeStepOnly];
using std::chrono::duration;
//using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

void EnsightExternalData::write(std::string filename)
{
  //auto t1 = high_resolution_clock::now();
  groupParts();
  //auto t2 = high_resolution_clock::now();
  for(size_t i = 0; i < parts.size(); i++) { groupElementTypes(parts[i]); }
  //auto t3 = high_resolution_clock::now();
  addGeometry(); // This has to be done before any WriteGeometry because it
                 // stores geo data of each Print. Needed for the General .geo
                 // file.
  //auto t4 = high_resolution_clock::now();
  addVariable();
  //auto t5 = high_resolution_clock::now();
  // uncomment to get the Ensight ASCII format. Usefull for debugging.
  // writeGeometryASCII(filename);
  // writeVariableASCII(filename);

  writeVariableBinary(filename);
  //auto t6 = high_resolution_clock::now();
  writeGeometryBinary(filename);
  //auto t7 = high_resolution_clock::now();

  writeCaseFile(filename);
  //auto t8 = high_resolution_clock::now();

  //duration<double, std::milli> ms_groupParts = t2 - t1;
  //duration<double, std::milli> ms_groupElementTypes = t3 - t2;
  //duration<double, std::milli> ms_addGeometry = t4 - t3;
  //duration<double, std::milli> ms_addVariable = t5 - t4;
  //duration<double, std::milli> ms_writeVariableBinary = t6 - t5;
  //duration<double, std::milli> ms_writeGeometryBinary = t7 - t6;
  //duration<double, std::milli> ms_writeCaseFile = t8 - t7;

  /*Message::Info("ms_groupParts = %f", ms_groupParts);
  Message::Info("ms_groupElementTypes = %f", ms_groupElementTypes);
  Message::Info("ms_addGeometry = %f", ms_addGeometry);
  Message::Info("ms_addVariable = %f", ms_addVariable);
  Message::Info("ms_writeVariableBinary = %f", ms_writeVariableBinary);
  Message::Info("ms_writeGeometryBinary = %f", ms_writeGeometryBinary);
  Message::Info("ms_writeCaseFile = %f", ms_writeCaseFile);*/
}
int EnsightExternalData::getNextTimeStep(std::string fname)
{
  std::ifstream textFileStream;
  int nextStep = 0;
  textFileStream.open(fname.c_str());
  // Check, if the text file could be opened and continue only, of OK
  // Message::Info("Is open %d", textFileStream.is_open());
  if(textFileStream.is_open()) {
    std::string word;
    // look for line number of steps:
    std::string keyWord = "steps:";
    bool save_next_word = false;
    // read each word separated by spaces
    while(textFileStream >> word) {
      if(save_next_word) {
        nextStep = std::stoi(word);
        textFileStream.close();
        return nextStep;
      }
      if(word == keyWord) {
        save_next_word = true;
      }
    }
  }
 
  textFileStream.close();
  return nextStep;
}
std::vector<double> EnsightExternalData::getTimeValues(std::string fname)
{
  std::ifstream textFileStream;
  std::vector<double> timeValues;
  double time;
  textFileStream.open(fname.c_str());
  // Check, if the text file could be opened and continue only, of OK
  // Message::Info("Is open %d", textFileStream.is_open());
  if(textFileStream.is_open()) {
    // Message::Info("in file");
    std::string word;
    // look for line number of steps:
    std::string keyWord = "values:";
    bool save_next_word = false;
    // read each word separated by spaces
    while(textFileStream >> word) {

      if(save_next_word) {
        time = std::stod(word);
        timeValues.push_back(time);

      }
      if(word == keyWord || save_next_word) {
        save_next_word = true;

      }
    }
    textFileStream.close();
    return timeValues;
  }
  else {

    return timeValues;
  }
}
void EnsightExternalData::addGeometry()
{
  // add geometries for global .geo file
  bool step_stored = false;
  for(auto step : Ensight_Case.time_values) {
    if(data_sets[0].freq_value != 0) {
      if(step == data_sets[0].freq_value) { step_stored = true; }
    }
    else {
      if(step == data_sets[0].time_value) { step_stored = true; }
    }
  }
  // you should never simulate at freqency zero, in that case you would me more
  // interested in time
  if(!step_stored) {
 if(data_sets[0].freq_value != 0 && strcmp(Current.Step_Type, "Freq") == 0) {
      if(Ensight_Case.freq_values.empty()) {
        Ensight_Case.freq_values.push_back(data_sets[0].freq_value);
      }
      // check if this freq is already stored
      if(Ensight_Case.freq_values.back() != data_sets[0].freq_value) {
        Ensight_Case.freq_values.push_back(data_sets[0].freq_value);
      }

      // Message::Info("data_sets[0].freq_value = %f", data_sets[0].freq_value);
    }
    else {
      if(Ensight_Case.time_values.empty()) {
        Ensight_Case.time_values.push_back(data_sets[0].time_value);
      }
      // check if this time is already stored
      if(Ensight_Case.time_values.back() != data_sets[0].time_value) {
        Ensight_Case.time_values.push_back(data_sets[0].time_value);
      }
    }
  }
}
void EnsightExternalData::addVariable()
{
  // all done in Ensight_Case to store information after each Print_External
  for(auto part_item : region_elements) {
    Ensight_Case.vars[data_sets[0].point_data[0].name][part_item.first] =
      data_sets[0].point_data[0].data;

    Ensight_Case.region_name_map[part_item.first] =
      data_sets[0]
        .point_data[0]
        .groupName; // this maps the name to the int asociated for this name
  }
}
// binary
void EnsightExternalData::writeGeometryBinary(std::string fname)
{
  FILE *fd = nullptr;
  bool binary = true;
  std::string current_filename;
  char charBuffer[512];

  int step_for_naming;

  // define file path to find the next step, should be a resFile
  std::string resFile_filename;
  char resFile_charBuffer[512];
  snprintf(charBuffer, sizeof(resFile_charBuffer), "%s.%d.%s", "resFile", 0,
           "case");
  resFile_filename = Fix_RelativePath(charBuffer, Name_Path);
  resFile_filename = fname + charBuffer;

 

  // ugly thing for counting freqStep
  //if(data_sets[0].freq_value != 0) {
  //  step_for_naming = 0;
  //  for(auto step : Ensight_Case.time_values) {
  //    step_for_naming++;
  //    if(step == data_sets[0].freq_value) { break; }
  //  }
  //  step_for_naming--;
  //}
  //else {
  //  step_for_naming = data_sets[0].time_step - 1;
  //}
  step_for_naming = Ensight_Case.time_values.size() - 1;

  std::vector<double> timeValues = getTimeValues(resFile_filename);

  // check if simulation starts from previous simulation
  bool from_previous_sim = !strcmp(Current.From_Previous, "True");

    if(!timeValues.empty() && from_previous_sim) {
    step_for_naming = getNextTimeStep(resFile_filename);
    if(Ensight_Case.time_values.back() == Current.Time) { step_for_naming--; }
    Message::Info("From Previous");
  }
  
 
  snprintf(charBuffer, sizeof(charBuffer), "%sGlobal.%d.%05d.geo", "resFile", 0,
           step_for_naming);
  current_filename = Fix_RelativePath(charBuffer, Name_Path);
  current_filename = fname + charBuffer;
  if(!(fd = FOpen(current_filename.c_str(), binary ? "wb" : "w"))) {
    Message::Error("Unable to open file '%s', error %s",
                   current_filename.c_str(), strerror(errno));
    return;
  }
  

  WriteStringToFile("C Binary", fd);
  WriteStringToFile("Written by GetDP", fd);
  WriteStringToFile("description line 1", fd);
  WriteStringToFile("node id given", fd);
  WriteStringToFile("element id given", fd);

  std::map<int, int> NodeIdToOrder;
  int node_count = 0; // nodes are named sequentially in the same geo file
  int node_offset = 0; // used for referencing nodes of elements

  for(size_t i = 0; i < Ensight_Case.parts.size(); i++) {
    std::string namePart = "part_" + std::to_string(i);

    WriteStringToFile("part", fd);
    WriteIntToFile(Ensight_Case.parts[i].part, fd);
    // WriteStringToFile(namePart.c_str(), fd);
    WriteStringToFile(Ensight_Case.region_name_map[Ensight_Case.parts[i].part],
                      fd);
    WriteStringToFile("coordinates", fd);
    WriteIntToFile(Ensight_Case.parts[i].nodes.size(), fd);

    for(auto node : Ensight_Case.parts[i].nodes) {
      NodeIdToOrder[node] =
        ++node_count; // for each part, node starts at value 0 and we want
                      // univoque nodeIdToOrder for each part
      // node id
      WriteIntToFile(NodeIdToOrder[node], fd);
    }
    for(auto node : Ensight_Case.parts[i].nodes)
      WriteFloatToFile(Ensight_Case.parts[i].nodes_coordinates[node][0], fd);
    for(auto node : Ensight_Case.parts[i].nodes)
      WriteFloatToFile(Ensight_Case.parts[i].nodes_coordinates[node][1], fd);
    for(auto node : Ensight_Case.parts[i].nodes)
      WriteFloatToFile(Ensight_Case.parts[i].nodes_coordinates[node][2], fd);

    for(auto elementGroup : Ensight_Case.parts[i].type_el_Part) {
      // element type
      WriteStringToFile(ToEnsightElementType[elementGroup.first].c_str(), fd);
      // number of elements
      WriteIntToFile(elementGroup.second.size(), fd);

      int element_id_offset =
        0; // this works if elements are stores sequentially by regions
      while(Ensight_Case.parts[i].elements[element_id_offset].region !=
            Ensight_Case.parts[i].part) {
        element_id_offset++;
      }
      // print elements id
      for(auto el_index : elementGroup.second) {
        WriteIntToFile(
          Ensight_Case.parts[i].elements[el_index + element_id_offset].index,
          fd);
      }
      // print nodes of elements
      for(auto el_index : elementGroup.second) {
        for(auto n : Ensight_Case.parts[i]
                       .elements[el_index + element_id_offset]
                       .nodes) {
          WriteIntToFile(NodeIdToOrder[n] - node_offset, fd);
        }
      }
    }
    node_offset = node_count;
  }
  fclose(fd);
}
// binary
void EnsightExternalData::writeVariableBinary(std::string fname)
{
  FILE *fd = nullptr;
  bool binary = true;
  std::string current_filename;
  char charBuffer[512];
  size_t last_time_step = 0; // Use always with LastTimeStep option!!!
  int step_for_naming;
  // ugly thing for counting freqStep
 /* if(data_sets[0].freq_value != 0) {
    step_for_naming = 0;
    for(auto step : Ensight_Case.time_values) {
      step_for_naming++;
      if(step == data_sets[0].freq_value) { break; }
    }
    step_for_naming--;
  }
  else {
    step_for_naming = data_sets[0].time_step - 1;
  }*/
  // define file path to find the next step, should be a resFile
  std::string resFile_filename;
  char resFile_charBuffer[512];
  snprintf(charBuffer, sizeof(resFile_charBuffer), "%s.%d.%s", "resFile", 0,
           "case");
  resFile_filename = Fix_RelativePath(charBuffer, Name_Path);
  resFile_filename = fname + charBuffer;

  std::vector<double> timeValues = getTimeValues(resFile_filename);
  step_for_naming = Ensight_Case.time_values.size() - 1;
  // check if simulation starts from previous simulation
  bool from_previous_sim = !strcmp(Current.From_Previous, "True");
  if(!timeValues.empty() && from_previous_sim) {
    step_for_naming = getNextTimeStep(resFile_filename);
    if(Ensight_Case.time_values.back() == Current.Time) { step_for_naming--; }
    Message::Info("From Previous");
  }
  // UNCOMMENT THIS FOR DEBUGGING, THIS WRITES A .GEO FILE ONLY FOR THE CURRENT
  // POSTOPERATION int step_for_naming;

  // snprintf(charBuffer, sizeof(charBuffer), "%s.%d.%05d.geo", "resFile", 0,
  //         step_for_naming);
  // current_filename = Fix_RelativePath(charBuffer, Name_Path);
  // current_filename = fname + charBuffer;
  // if(!(fd = FOpen(current_filename.c_str(), binary ? "wb" : "w"))) {
  //  Message::Error("Unable to open file '%s', error %s",
  //                 current_filename.c_str(), strerror(errno));
  //  return;
  //}

  // WriteStringToFile("C Binary", fd);
  // WriteStringToFile("Written by GetDP", fd);
  // WriteStringToFile("description line 1", fd);
  // WriteStringToFile("node id given", fd);
  // WriteStringToFile("element id given", fd);

  // std::map<int, int> NodeIdToOrder;
  // int node_count = 0; // nodes are named sequentially
  // int node_offset = 0; // used for referencing nodes of elements

  // for(size_t i = 0; i < parts.size(); i++) {
  //  std::string namePart = "part_" + std::to_string(i);

  //  WriteStringToFile("part", fd);
  //  WriteIntToFile(parts[i].part, fd);
  //  WriteStringToFile(namePart.c_str(), fd);
  //  WriteStringToFile("coordinates", fd);
  //  WriteIntToFile(parts[i].nodes.size(), fd);

  //  for(auto node : parts[i].nodes) {
  //    // node id;
  //    NodeIdToOrder[node] = ++node_count;

  //    WriteIntToFile(NodeIdToOrder[node], fd);
  //  }
  //  // write cordinates
  //  for(auto node : parts[i].nodes)
  //    WriteFloatToFile(node_coordinates[node][0], fd);
  //  for(auto node : parts[i].nodes)
  //    WriteFloatToFile(node_coordinates[node][1], fd);
  //  for(auto node : parts[i].nodes)
  //    WriteFloatToFile(node_coordinates[node][2], fd);

  //  for(auto elementGroup : parts[i].type_el_Part) {
  //    WriteStringToFile(ToEnsightElementType[elementGroup.first].c_str(), fd);
  //    // number of elements
  //    WriteIntToFile(elementGroup.second.size(), fd);

  //    int element_id_offset =
  //      0; // this works if elements are stored sequentially by regions
  //    while(elements[element_id_offset].region != parts[i].part) {
  //      element_id_offset++;
  //    }
  //    // print elements id
  //    for(auto el_index : elementGroup.second) {
  //      WriteIntToFile(elements[el_index + element_id_offset].index, fd);
  //    }
  //    // print nodes of elements
  //    for(auto el_index : elementGroup.second) {
  //      for(auto n : elements[el_index + element_id_offset].nodes) {
  //        WriteIntToFile(NodeIdToOrder[n] - node_offset, fd);
  //      }
  //    }
  //  }
  //  node_offset = node_count;
  //}

  // write values for variable file , i.e field file

  snprintf(charBuffer, sizeof(charBuffer), "%s.%d.%05d_n.%s", "resFile", 0,
           step_for_naming, data_sets[last_time_step].point_data[0].name);
  current_filename = Fix_RelativePath(charBuffer, Name_Path);
  current_filename = fname + charBuffer;
  if(!(fd = FOpen(current_filename.c_str(), binary ? "wb" : "w"))) {
    Message::Error("Unable to open file '%s', error %s",
                   current_filename.c_str(), strerror(errno));
    return;
  }
 
 
  for(auto var : Ensight_Case.vars) {
    if(var.first == data_sets[last_time_step].point_data[0].name) {
      WriteStringToFile(var.first, fd); // variable name
      for(auto part : var.second) {
        WriteStringToFile("part", fd);
        WriteIntToFile(part.first, fd);
        WriteStringToFile("coordinates", fd);
        size_t point_size = part.second.size();
        size_t value_size = part.second[0].size();
        for(size_t iVal = 0; iVal < value_size; iVal++) {
          for(size_t iPoint = 0; iPoint < point_size; iPoint++) {
            WriteFloatToFile(part.second[iPoint][iVal], fd);
          }
        }
      }
    }
  }
  fflush(fd);

  // KEEP THIS AS A TEMPLATE IN CASE WE WANT TO INCLUDE MORE THAN ONE REGION FOR
  // A VARIABLE
  // 			PrintExternal[ PointData {afield}, OnElementsOf
  // Region[{ind,wp,air}], File "..\rawresults\dan\", Format ENSIGHT
  // ,LastTimeStepOnly];
  // WriteStringToFile(data_sets[last_time_step].point_data[0].name, fd);
  // for(size_t i = 0; i < parts.size(); i++) {
  //  WriteStringToFile("part", fd);
  //  WriteIntToFile(parts[i].part, fd);
  //  WriteStringToFile("coordinates", fd);
  //  size_t offset = (i == 0) ? 0 : parts[i - 1].nodes.size();

  //  for(size_t iData = 0; iData < data_sets[last_time_step].point_data.size();
  //      iData++) {
  //    for(int iVal = 0;
  //        iVal < data_sets[last_time_step].point_data[iData].data_size;
  //        iVal++) {
  //      for(size_t iPoint = offset; iPoint < (offset + parts[i].nodes.size());
  //          iPoint++) { // node_map.size()

  //        WriteFloatToFile(data_sets[last_time_step]
  //                           .point_data[iData]
  //                           .data[node_map[parts[i].nodes[iPoint]]][iVal],
  //                         fd);
  //      }
  //    }
  //  }
  //}
  bool name_in_case = false;
  for(auto name : Ensight_Case.postNames) {
    if(name == data_sets[0].point_data[0].name) { name_in_case = true; }
  }
  if(!name_in_case) {
    Ensight_Case.postNames.push_back(data_sets[0].point_data[0].name);
    Ensight_Case.valueTypes.push_back(data_sets[0].point_data[0].value_type);
  }
  fclose(fd);
}
// ascii
void EnsightExternalData::writeGeometryASCII(std::string fname)
{
  FILE *fd = nullptr;
  bool binary = false;
  std::string current_filename;
  char charBuffer[512];

  int step_for_naming;

  // ugly thing for counting freqStep
  if(data_sets[0].freq_value != 0) {
    step_for_naming = 0;
    for(auto step : Ensight_Case.time_values) {
      step_for_naming++;
      if(step == data_sets[0].freq_value) { break; }
    }
    step_for_naming--;
  }
  else {
    step_for_naming = data_sets[0].time_step - 1;
  }

  snprintf(charBuffer, sizeof(charBuffer), "%sGlobal.%d.%05d.geoASCII",
           "resFile", 0, step_for_naming);
  current_filename = Fix_RelativePath(charBuffer, Name_Path);
  current_filename = fname + charBuffer;
  if(!(fd = FOpen(current_filename.c_str(), binary ? "wb" : "w"))) {
    Message::Error("Unable to open file '%s', error %s",
                   current_filename.c_str(), strerror(errno));
    return;
  }

  fprintf(fd, "Written by GetDP Ensight writer\n");
  fprintf(fd, "No title specified\n");
  fprintf(fd, "node id given\n");
  fprintf(fd, "element id given\n");

  std::map<int, int> NodeIdToOrder;
  int node_count = 0; // nodes are named sequentially in the same geo file
  int node_offset = 0; // used for referencing nodes of elements

  for(size_t i = 0; i < Ensight_Case.parts.size(); i++) {
    fprintf(fd, "part\n");
    fprintf(fd, "%d\n", Ensight_Case.parts[i].part);
    fprintf(fd, "coordinates\n");
    fprintf(fd, "%lld\n", Ensight_Case.parts[i].nodes.size());
    for(auto node : Ensight_Case.parts[i].nodes) {
      NodeIdToOrder[node] =
        ++node_count; // for each part, node should not start at value 0. We
                      // want univoque nodeIdToOrder for each part
      // node id
      fprintf(fd, "%d\n", NodeIdToOrder[node]);
    }

    for(auto node : Ensight_Case.parts[i].nodes)
      fprintf(fd, "%.10e\n", Ensight_Case.parts[i].nodes_coordinates[node][0]);
    for(auto node : Ensight_Case.parts[i].nodes)
      fprintf(fd, "%.10e\n", Ensight_Case.parts[i].nodes_coordinates[node][1]);
    for(auto node : Ensight_Case.parts[i].nodes)
      fprintf(fd, "%.10e\n", Ensight_Case.parts[i].nodes_coordinates[node][2]);

    for(auto elementGroup : Ensight_Case.parts[i].type_el_Part) {
      // print element type
      fprintf(fd, "%s\n", ToEnsightElementType[elementGroup.first].c_str());
      // print number of elements of this type
      fprintf(fd, "%lld\n", elementGroup.second.size());

      int element_id_offset = 0; // this works if elements are stores
                                 // sequentially by regions, and they are
      while(Ensight_Case.parts[i].elements[element_id_offset].region !=
            Ensight_Case.parts[i].part) {
        element_id_offset++;
      }
      // print elements id
      for(auto el_index : elementGroup.second) {
        fprintf(
          fd, "%d\n",
          Ensight_Case.parts[i].elements[el_index + element_id_offset].index);
      }
      // print nodes of elements
      for(auto el_index : elementGroup.second) {
        for(auto n : Ensight_Case.parts[i]
                       .elements[el_index + element_id_offset]
                       .nodes) {
          fprintf(fd, "%d ", NodeIdToOrder[n] - node_offset);
        }
        fprintf(fd, "\n");
      }
    }
    node_offset = node_count;
  }
  fclose(fd);
}
// ascii
void EnsightExternalData::writeVariableASCII(std::string fname)
{
  FILE *fd = nullptr;
  bool binary = false;
  std::string current_filename;
  char charBuffer[512];
  size_t last_time_step = 0;
  int step_for_naming;

  // ugly thing for counting freqStep
  if(data_sets[0].freq_value != 0) {
    step_for_naming = 0;
    for(auto step : Ensight_Case.time_values) {
      step_for_naming++;
      if(step == data_sets[0].freq_value) { break; }
    }
    step_for_naming--;
  }
  else {
    step_for_naming = data_sets[0].time_step - 1;
  }

  // write .geo file

  // UNCOMMENT THIS FOR DEBUGGING, THIS WRITES A .GEO FILE ONLY FOR THE CURRENT
  // POSTOPERATION

  // snprintf(charBuffer, sizeof(charBuffer), "%s.%d.%05d.geoASCII", "resFile",
  // 0,
  //         data_sets[0].time_step - 1); // last time step data_sets.size()
  // current_filename = Fix_RelativePath(charBuffer, Name_Path);
  // current_filename = fname + charBuffer;
  // if(!(fd = FOpen(current_filename.c_str(), binary ? "wb" : "w"))) {
  //  Message::Error("Unable to open file '%s', error %s",
  //                 current_filename.c_str(), strerror(errno));
  //  return;
  //}

  // fprintf(fd, "Written by GetDP Ensight writer\n");
  // fprintf(fd, "No title specified\n");
  // fprintf(fd, "node id given\n");
  // fprintf(fd, "element id given\n");

  // std::map<int, int> NodeIdToOrder;
  // int node_count = 0; // nodes are named sequentially in the same geo file
  // int node_offset = 0; // used for referencing nodes of elements
  // for(size_t i = 0; i < parts.size(); i++) {
  //  fprintf(fd, "part\n");
  //  fprintf(fd, "%d\n", parts[i].part);
  //  fprintf(fd, "coordinates\n");
  //  fprintf(fd, "%lld\n", parts[i].nodes.size());

  //  for(auto node : parts[i].nodes) {
  //    NodeIdToOrder[node] =
  //      ++node_count; // for each part, node should not start at value 0. We
  //                    // want univoque nodeIdToOrder for each part
  //    // node id
  //    fprintf(fd, "%d\n", NodeIdToOrder[node]

  //    );
  //  }

  //  for(auto node : parts[i].nodes)
  //    fprintf(fd, "%.10e\n", node_coordinates[node][0]);
  //  for(auto node : parts[i].nodes)
  //    fprintf(fd, "%.10e\n", node_coordinates[node][1]);
  //  for(auto node : parts[i].nodes)
  //    fprintf(fd, "%.10e\n", node_coordinates[node][2]);

  //  for(auto elementGroup : parts[i].type_el_Part) {
  //    // print element type
  //    fprintf(fd, "%s\n", ToEnsightElementType[elementGroup.first].c_str());
  //    // print number of elements of this type
  //    fprintf(fd, "%lld\n", elementGroup.second.size());

  //    int element_id_offset =
  //      0; // this works if elements are stores sequentially by regions
  //    while(elements[element_id_offset].region != parts[i].part) {
  //      element_id_offset++;
  //    }
  //    // print elements id
  //    for(auto el_index : elementGroup.second) {
  //      // fprintf(fd, "el_index %d\n", el_index);
  //      fprintf(fd, "%d\n", elements[el_index + element_id_offset].index);
  //    }
  //    // print nodes of elements
  //    for(auto el_index : elementGroup.second) {
  //      for(auto n : elements[el_index + element_id_offset].nodes) {
  //        fprintf(fd, "%d ", NodeIdToOrder[n] - node_offset);
  //      }
  //      fprintf(fd, "\n");
  //    }
  //  }
  //  node_offset = node_count;
  //}

  // write values for variable file , i.e field file

  snprintf(charBuffer, sizeof(charBuffer), "%s.%d.%05d_n.%sASCII", "resFile", 0,
           step_for_naming, data_sets[last_time_step].point_data[0].name);
  current_filename = Fix_RelativePath(charBuffer, Name_Path);
  current_filename = fname + charBuffer;

  if(!(fd = FOpen(current_filename.c_str(), binary ? "wb" : "w"))) {
    Message::Error("Unable to open file '%s', error %s",
                   current_filename.c_str(), strerror(errno));
    return;
  }

  /*fprintf(fd, data_sets[last_time_step].point_data[0].name);
  fprintf(fd, "\n");*/

  for(auto var : Ensight_Case.vars) {
    fprintf(fd, var.first); // variable name
    fprintf(fd, "\n");
    for(auto part : var.second) {
      fprintf(fd, "part\n");
      fprintf(fd, "%d \n", part.first);
      fprintf(fd, "coordinates\n");
      size_t point_size = part.second.size();
      size_t value_size = part.second[0].size();
      for(size_t iVal = 0; iVal < value_size; iVal++) {
        for(size_t iPoint = 0; iPoint < point_size; iPoint++) {
          // auto point_data = part.second[iPoint][iVal];
          fprintf(fd, "%.10e\n", part.second[iPoint][iVal]);
        }
      }
    }
  }
  fflush(fd);

  // KEEP THIS AS A TEMPLATE IN CASE WE WANT TO INCLUDE MORE THAN ONE REGION FOR
  // A VARIABLE
  // 			PrintExternal[ PointData {afield},
  // OnElementsOfRegion[{ind,wp,air}], File "..\rawresults\dan\", Format ENSIGHT
  // ,LastTimeStepOnly]; for(size_t i = 0; i < parts.size(); i++) {
  //  fprintf(fd, "part\n");
  //  fprintf(fd, "%d\n", parts[i].part);
  //  fprintf(fd, "coordinates\n");
  //

  //  size_t offset = (i == 0) ? 0 : parts[i - 1].nodes.size();
  //  for(size_t iData = 0; iData < data_sets[last_time_step].point_data.size();
  //      iData++) {
  //    //fprintf(fd, "OLD\n");
  //    for(int iVal = 0;
  //        iVal < data_sets[last_time_step].point_data[iData].data_size;
  //        iVal++) {
  //      for(size_t iPoint = 0; iPoint < data_sets[last_time_step]
  //                                        .point_data[iData]
  //                                        .data_region[parts[i].part]
  //                                        .size();
  //          iPoint++) {
  //         fprintf(fd, "%.10e\n", //"%.10e\n"
  //                data_sets[last_time_step]
  //                  .point_data[iData]
  //                  .data[node_map[parts[i].nodes[iPoint]]][iVal]);
  //        //try {
  //        //  fprintf(fd, "%.10e \n", //"%.10e\n"
  //        //          data_sets[last_time_step]
  //        //            .point_data[iData]
  //        //            .data_region[parts[i].part][iPoint]
  //        //            .at(iVal));
  //        //} catch(const std::exception &ex) {
  //        //  Message::Info("iPoint %lld", iPoint);
  //        // Message::Info("data_sets[last_time_step].point_data[iData].data_"
  //        //                "region[parts[i].part][iPoint].size() %lld",
  //        //                data_sets[last_time_step]
  //        //                  .point_data[iData]
  //        //                  .data_region[parts[i].part][iPoint]
  //        //                  .size());
  //        //  Message::Info(ex.what());
  //        //}
  //      }
  //    }
  //  }
  //}
  fclose(fd);
}
// write .case file
void EnsightExternalData::writeCaseFile(std::string fname)
{
  FILE *fd = nullptr;
  std::string current_filename;
  char charBuffer[512];

  snprintf(charBuffer, sizeof(charBuffer), "%s.%d.%s", "resFile", 0, "case");
  current_filename = Fix_RelativePath(charBuffer, Name_Path);

  // define file path to find the next step, should be a resFile
  std::string resFile_filename;
  char resFile_charBuffer[512];
  snprintf(charBuffer, sizeof(resFile_charBuffer), "%s.%d.%s", "resFile", 0,
           "case");
  resFile_filename = Fix_RelativePath(charBuffer, Name_Path);
  resFile_filename = fname + charBuffer;

  /*int nextTimeStep = getNextTimeStep(resFile_filename);
  Message::Info("nextTimeStep = %d", nextTimeStep);
  int incrementTimeStep = getIncrementTimeStep(resFile_filename);
  Message::Info("incrementTimeStep = %d", nextTimeStep);*/
  std::vector<double> timeValues = getTimeValues(resFile_filename);
  // Message::Info("timeValues.size() = %d", timeValues.size());

  current_filename = fname + charBuffer;
  // Message::Info("next step %d", getNextTimeStep(current_filename));
  remove(current_filename.c_str());

  if(!(fd = FOpen(current_filename.c_str(), "w"))) {
    Message::Error("Unable to open file '%s', error %s",
                   current_filename.c_str(), strerror(errno));
    return;
  }

  fprintf(fd, "FORMAT\ntype: ensight gold \n \n \nGEOMETRY \nmodel: 1 "
              "resFileGlobal.0.*****.geo \n \nVARIABLE\n");
  for(size_t var = 0; var < Ensight_Case.postNames.size(); var++) {
    fprintf(fd, "%s per node: 1 ",
            ToEnsightValueType[Ensight_Case.valueTypes[var]].c_str());
    fprintf(fd, "%s resFile.0.*****_n.%s\n\n", Ensight_Case.postNames[var],
            Ensight_Case.postNames[var]);
  }
  // add last timevalue if it is not repeated before counting steps
 /* if(timeValues.size() != 0) {
    if(timeValues.back() < Ensight_Case.time_values.back()) {
      timeValues.push_back(Ensight_Case.time_values.back());
    }
  }*/

  fprintf(fd, "\nTIME \ntime set: 1 \nnumber of steps: %lld\n",
          timeValues.size());
  fprintf(fd, "filename start number: 00000 \nfilename increment: 00001\n");
  fprintf(fd, "time values:\n");

  // old with global variable, all time steps stored. It does not work if
  // starting simulation from previous step
  /*for(size_t time_step = 0; time_step < Ensight_Case.time_values.size();
      time_step++) {
    double time_step_aux = Ensight_Case.time_values[time_step];
    fprintf(fd, "%lf\n", time_step_aux);
  }*/

  // reading previous file, write previous time steps
  for(double time : timeValues) {
    fprintf(fd, "%lf\n", time);
  }
  // If there is no time values, we use old approach.
  if(timeValues.size() != 0) {
    // check if this time is already stored and if it is a next step
    if(timeValues.back() < Ensight_Case.time_values.back() - 0.0001 ) {
      fprintf(fd, "%lf\n", Ensight_Case.time_values.back());
    }
  }
  else {
    for(size_t time_step = 0; time_step < Ensight_Case.time_values.size();
        time_step++) {
      double time_step_aux = Ensight_Case.time_values[time_step];
      fprintf(fd, "%lf\n", time_step_aux);
    }
  } 
  fclose(fd);
}

void EnsightExternalData::groupParts()
{
  duration<double, std::milli> ms_groupParts_subLoop, ms_groupParts_subLoop_New;
  for(auto part_item : region_elements) {
    elementsInPart
      elementPart; // create a elementPart that will store information about
                   // type of elements in type_el_Part
    elementPart.part = part_item.first; // part_item.first is the region
    auto t1_1 = high_resolution_clock::now();
    // loop over all elements in that region
    //for(auto el_index : part_item.second) {
    //  PostExternalElementCopy elementCopy;
    //  // elements contains all elements and we deep copy only elements on this
    //  // region
    //  elementCopy.index = elements[el_index].index;
    //  elementCopy.nodes = elements[el_index].nodes;
    //  // elementCopy.nodes_coordinates = elements[el_index].nodes_coordinates;
    //  elementCopy.region = elements[el_index].region;
    //  elementCopy.type = elements[el_index].type;
    //  elementPart.nodes_coordinates = node_coordinates;
    //  //
    //  // elementPart.elements.push_back(elementCopy);//old one working
    //  // elementPart.elements = elements_in_region[part_item.first];
    //  // loop over all nodes for each element in that region
    //  for(auto n : elements[el_index].nodes) {
    //    // if the node is not is already pushed then push it
    //    if(std::find(elementPart.nodes.begin(), elementPart.nodes.end(), n) ==
    //       elementPart.nodes.end()) {
    //      elementPart.nodes.push_back(n);
    //    }
    //  }
    //}
    auto t1_2 = high_resolution_clock::now();
    ms_groupParts_subLoop = t1_2 - t1_1;
    //
    auto t1_3 = high_resolution_clock::now();
    elementPart.elements = elements_in_region[part_item.first];
    elementPart.nodes_coordinates = node_coordinates;

	// Copy node IDs for region from PostExternalData::node_map_region[region_id] unordered_set.
	auto &regionNodes = node_map_region[elementPart.part];
	elementPart.nodes.insert(elementPart.nodes.end(), regionNodes.begin(), regionNodes.end());

    auto t1_4 = high_resolution_clock::now();
    ms_groupParts_subLoop_New = t1_4 - t1_3;

    //Message::Info("ms_groupParts_subLoop = %f", ms_groupParts_subLoop);
    //Message::Info("ms_groupParts_subLoop_New = %f", ms_groupParts_subLoop_New);
    parts.push_back(elementPart);

    bool is_new_part = true;
    for(size_t j = 0; j < Ensight_Case.parts.size(); j++) {
      if(elementPart.part == Ensight_Case.parts[j].part) {
        is_new_part = false;
      }
    }
    if(is_new_part) {
      Ensight_Case.parts.push_back(elementPart);
      groupElementTypes(Ensight_Case.parts.back());
    }
  }
}
void EnsightExternalData::groupElementTypes(elementsInPart &el_part)
{
  for(size_t i = 0; i < el_part.elements.size(); i++) {
    if(!el_part.type_el_Part.count(el_part.elements[i].type)) {
      el_part.type_el_Part[el_part.elements[i].type] = std::vector<int>();
    }
    // to give an id sequentially
    el_part.type_el_Part[el_part.elements[i].type].push_back(i);
  }
  for(size_t j = 0; j < Ensight_Case.parts.size(); j++) {
    if(el_part.part == Ensight_Case.parts[j].part) {
      Ensight_Case.parts[j].type_el_Part = el_part.type_el_Part;
    }
  }
}

// functions to write in binary
void EnsightExternalData::WriteStringToFile(const char *cstring, FILE *file)
{
  char cbuffer[81];

  unsigned long cstringLength = static_cast<unsigned long>(strlen(cstring));
  // auto ts = vtkMath::Min(cstringLength, 80ul);
  memcpy(cbuffer, cstring, (80ul <= cstringLength ? 80ul : cstringLength));
  // memcpy(cbuffer, cstring, vtkMath::Min(cstringLength, 80ul));
  for(int i = cstringLength; i < 81; ++i) { cbuffer[i] = '\0'; }

  // Write a constant 80 bytes to the file.
  fwrite(cbuffer, sizeof(char), 80, file);
}
void EnsightExternalData::WriteIntToFile(const int i, FILE *file)
{
  fwrite(&i, sizeof(int), 1, file);
}
void EnsightExternalData::WriteFloatToFile(const float i, FILE *file)
{
  fwrite(&i, sizeof(float), 1, file);
}