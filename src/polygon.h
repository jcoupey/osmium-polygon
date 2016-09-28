/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <algorithm>
#include <osmium/osm/box.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>

class polygon{
private:
  std::string _name;
  std::vector<osmium::Location> _corners;
  osmium::Box _bbox;

public:
  polygon(const std::string& name,
          const std::vector<osmium::Location>& nodes);

  std::string get_name() const;
};

#endif
