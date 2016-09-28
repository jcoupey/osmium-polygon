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
  polygon my_poly("My polygon",
                  std::vector<osmium::Location>({{2.340087890625,48.87239311228893},{2.3448944091796875,48.87758662245016},{2.3531341552734375,48.878489786571116},{2.3566532135009766,48.87290119685805},{2.3545074462890625,48.86968324077576},{2.3478126525878906,48.86578754333881},{2.3445510864257812,48.862456199187356},{2.3411178588867188,48.8565834593617},{2.3362255096435547,48.861948008551956},{2.3325347900390625,48.86471476180277},{2.3239517211914062,48.87036072241377},{2.3253250122070312,48.87578024528628},{2.3327064514160156,48.87707858546583},{2.340087890625,48.87239311228893}}));

  return parse_file(input_name, output_name, my_poly);
}

