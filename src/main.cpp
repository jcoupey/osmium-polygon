/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <iostream>
#include <fstream>
#include "polygon.h"

int main(int argc, char* argv[]){
  // Define custom polygon.
  polygon my_poly("My polygon",
                  std::vector<osmium::Location>({{2.3414611816406246,48.88729478216038},{2.3613739013671875,48.907156769554426},{2.385406494140625,48.914828605652765},{2.41424560546875,48.90625412315376},{2.42523193359375,48.883231130667276},{2.410125732421875,48.86110101269274},{2.374420166015625,48.85251731276075},{2.35382080078125,48.84302835299516},{2.34283447265625,48.82494916931076},{2.3256683349609375,48.8073156833131},{2.3146820068359375,48.827661462789415},{2.303695678710937,48.844384028766356},{2.28240966796875,48.85884228699205},{2.2666168212890625,48.87871557505334},{2.267303466796875,48.89542109453028},{2.289276123046875,48.90805939965008},{2.3091888427734375,48.90625412315376},{2.3291015625,48.903997435817274},{2.3414611816406246,48.88729478216038}}));

  osmium::Location inside(2.34, 48.86);
  osmium::Location outside(2.18, 48.88);

  std::cout << my_poly.get_name()
            << " contains " << inside
            << ": "
            << my_poly.contains(inside)
            << std::endl;

  std::cout << my_poly.get_name()
            << " contains " << outside
            << ": "
            << my_poly.contains(outside)
            << std::endl;

  return 0;
}

