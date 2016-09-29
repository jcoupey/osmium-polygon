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
#include "../include/rapidjson/document.h"

class polygon{
private:
  std::string _name;
  std::vector<osmium::Location> _corners;
  osmium::Box _bbox;

  bool is_in_polygon(const osmium::Location& loc) const;

public:
  polygon(const std::string& name,
          const std::vector<osmium::Location>& nodes);

  polygon(const rapidjson::Value& feature);

  std::string get_name() const;

  bool contains(const osmium::Location& loc) const;

  bool contains(const osmium::Node& node) const;
};

#endif
