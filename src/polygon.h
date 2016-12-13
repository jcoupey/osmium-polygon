/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include <string>
#include <osmium/osm/box.hpp>
#include <osmium/osm/node.hpp>
#include "../include/rapidjson/document.h"
#include "ring.h"

class polygon{
private:
  std::string _name;
  ring _outer_ring;
  std::vector<ring> _inner_rings;

public:
  polygon(const std::string& name,
          const rapidjson::Value& json_rings);

  std::string get_name() const;

  bool contains(const osmium::Location& loc) const;

  bool contains(const osmium::Node& node) const;
};

#endif
