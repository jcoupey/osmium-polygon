/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "polygon.h"

polygon::polygon(const std::string& name,
                 const rapidjson::Value& json_rings):
  _name(name),
  _outer_ring(json_rings[0])    // First ring as exterior ring.
{
  // Setting inner rings.
  std::for_each(json_rings.Begin() + 1,
                json_rings.End(),
                [&](const auto& r){
                  _inner_rings.emplace_back(r);
                });
}

std::string polygon::get_name() const{
  return _name;
}

bool polygon::contains(const osmium::Location& loc) const{
  bool contained = _outer_ring.contains(loc);
  for(const auto& r: _inner_rings){
    contained &= !r.contains(loc);
    if(!contained){
      break;
    }
  }
  return contained;
}

bool polygon::contains(const osmium::Node& node) const{
  return this->contains(node.location());
}

osmium::Box polygon::bbox() const{
  return _outer_ring.bbox();
}
