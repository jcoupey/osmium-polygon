/*

This file is part of osmium-polygon.

Copyright (c) 2016-2017, Julien Coupey.
All rights reserved (see LICENSE).

*/

#ifndef RING_H
#define RING_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <osmium/osm/box.hpp>
#include <osmium/osm/node.hpp>
#include "../include/rapidjson/document.h"

class ring{
private:
  std::vector<osmium::Location> _corners;
  osmium::Box _bbox;

  bool is_in_ring(const osmium::Location& loc) const;

public:
  ring(const rapidjson::Value& json_ring);

  bool contains(const osmium::Location& loc) const;

  bool contains(const osmium::Node& node) const;

  osmium::Box bbox() const;
};

#endif
