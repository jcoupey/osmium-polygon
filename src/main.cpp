/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <iostream>
#include <fstream>
#include "polygon.h"
#include "osm_parser.h"

int main(int argc, char* argv[]){
  // File names.
  std::string input_name = argv[1];
  std::string output_name = "polygon_" + input_name;

  // Define custom polygon.
  polygon my_poly("Strange shape in Berlin",
                  std::vector<osmium::Location>({{13.391647338867186,52.51705655410405},{13.394737243652344,52.549636074382285},{13.452072143554686,52.520190250694526},{13.400230407714844,52.506818254212604},{13.455848693847656,52.495741489296144},{13.381690979003906,52.48612543090344},{13.386840820312498,52.50640031375409},{13.330192565917967,52.49741363265356},{13.348731994628904,52.534811212925774},{13.391647338867186,52.51705655410405}}));

  return parse_file(input_name, output_name, my_poly);
}

