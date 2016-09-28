/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "polygon.h"

polygon::polygon(const std::string& name,
                 const std::vector<osmium::Location>& corners):
  _name(name),
  _corners(corners)
{
  // Stop if first and end corners are different.
  if (_corners[0] != _corners[_corners.size() - 1]){
    std::cout << "Bad polygon "
              << name
              << ", first and last coordinates do not match."
              << std::endl;
    exit(0);
  }

  // Set bounding box.
  std::for_each(_corners.cbegin(), _corners.cend(),
                [&](const auto& n){_bbox.extend(n);});
}

std::string polygon::get_name() const{
  return _name;
}
