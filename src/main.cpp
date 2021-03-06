/*

This file is part of osmium-polygon.

Copyright (c) 2016-2017, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <iostream>
#include <fstream>
#include "polygon.h"
#include "osm_parser.h"
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/error/en.h"

void display_usage(){
  std::string usage = "Usage : osmium-polygon -p GEOJSON_FILE [-o=OUT] OSM_FILE\n";
  usage += "Crop OSM data in FILE using (multi)-polygons in GEOJSON_FILE and write it to OUT.\n";
  usage += "\t-p GEOJSON_FILE\t geojson file containing the polygon\n";
  usage += "\t-o OUTPUT\t output file name\n";
  std::cout << usage;
  exit(0);
}

int main(int argc, char* argv[]){
  // File names.
  std::string input_name;
  std::string output_name;
  std::string poly_name;

  // Parsing command-line options
  const char* optString = "o:p:h?";

  int opt = getopt(argc, argv, optString);

  while(opt != -1){
    switch(opt){
    case 'o':
      output_name = optarg;
      break;
    case 'p':
      poly_name = optarg;
      break;
    default:
      // Shouldn't be used.
      break;
    }
    opt = getopt(argc, argv, optString);
  }

  // Getting input file from command-line.
  if(argc == optind){
    // No input file given!
    display_usage();
  }
  input_name = argv[optind];

  if(output_name.empty()){
    // Default output name.
    output_name = "polygon_" + input_name;
  }

  // Parsing input file for polygons.
  if(poly_name.empty()){
    display_usage();
  }
  std::cout << "[info] Parsing geojson file, searching for polygons...\n";

  rapidjson::Document json_input;
  std::string error_msg;

  std::ifstream ifs (poly_name);
  std::stringstream buffer;
  buffer << ifs.rdbuf();

  if(json_input.Parse(buffer.str().c_str()).HasParseError()){
      std::string error_msg = std::string(rapidjson::GetParseError_En(json_input.GetParseError()))
        + " (offset: "
        + std::to_string(json_input.GetErrorOffset())
        + ")";
      std::cout << error_msg << std::endl;
      exit(1);
  }

  if(!json_input.HasMember("features")
     or !json_input["features"].IsArray()){
    std::cout << "[error] Invalid \"features\" key.\n";
    exit(1);
  }

  std::vector<std::string> name_keys({"name", "id", "ID"});

  std::vector<polygon> polygons;

  // Finding the first polygon feature in the json file.
  for(rapidjson::SizeType i = 0; i < json_input["features"].Size(); ++i){
    auto& feature = json_input["features"][i];
    if(!feature.HasMember("geometry")
       or !feature["geometry"].HasMember("type")
       or !feature["geometry"]["type"].IsString()
       or (feature["geometry"]["type"] != "Polygon"
           and feature["geometry"]["type"] != "MultiPolygon")
       or !feature["geometry"].HasMember("coordinates")
       or !feature["geometry"]["coordinates"].IsArray()){
      continue;
    }
    std::string current_name;
    if(feature.HasMember("properties")){
      for(const auto& name: name_keys){
        if(feature["properties"].HasMember(name.c_str())
           and feature["properties"][name.c_str()].IsString()){
          // Using property name.
          current_name = feature["properties"][name.c_str()].GetString();
          break;
        }
      }
    }
    if(current_name.empty()){
      // Default to feature index.
      current_name = "feature_" + std::to_string(i);
      }

    if(feature["geometry"]["type"] == "Polygon"){
      polygons.emplace_back(current_name,
                            feature["geometry"]["coordinates"]);
    }
    if(feature["geometry"]["type"] == "MultiPolygon"){
      auto& coordinates = feature["geometry"]["coordinates"];
      for(rapidjson::SizeType i = 0; i < coordinates.Size(); ++i){
        polygons.emplace_back(current_name + "_" + std::to_string(i),
                              coordinates[i]);
      }
    }
  }

  if(polygons.empty()){
    std::cout << "[info] No polygon feature found in file: "
              << poly_name << "!\n";
    return 0;
  }
  else{
    std::cout << "[info] Found "
              << polygons.size()
              << " polygon feature(s).\n";

    std::cout << "[info] Building R-tree...\n";

    rtree_t rtree;
    for(unsigned i = 0 ; i < polygons.size() ; ++i){
      const osmium::Box osmium_box = polygons[i].bbox();
      const auto bl = osmium_box.bottom_left();
      const auto tr = osmium_box.top_right();

      box b(point(bl.lon(), bl.lat()), point(tr.lon(), tr.lat()));

      rtree.insert(std::make_pair(b, i));
    }

    return parse_file(input_name, output_name, polygons, rtree);
  }
}

