/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <iostream>
#include <fstream>
#include "polygon.h"
#include "osm_parser.h"

void display_usage()
{
  std::string usage = "Usage : osmium-polygon [-o=OUT] FILE\n";
  usage += "Crop OSM data in FILE using polygon a write it to OUT.";
  std::cout << usage << std::endl;
  exit(0);
}

int main(int argc, char* argv[]){
  // File names.
  std::string input_name;
  std::string output_name;

  // Parsing command-line options
  const char* optString = "o:h?";

  int opt = getopt(argc, argv, optString);

  while(opt != -1){
    switch(opt){
    case 'o':
      output_name = optarg;
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

  // Define custom polygon.
  polygon my_poly("Strange shape in Berlin",
                  std::vector<osmium::Location>({{13.391647338867186,52.51705655410405},{13.394737243652344,52.549636074382285},{13.452072143554686,52.520190250694526},{13.400230407714844,52.506818254212604},{13.455848693847656,52.495741489296144},{13.381690979003906,52.48612543090344},{13.386840820312498,52.50640031375409},{13.330192565917967,52.49741363265356},{13.348731994628904,52.534811212925774},{13.391647338867186,52.51705655410405}}));

  return parse_file(input_name, output_name, my_poly);
}

