/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "ring.h"

ring::ring(const rapidjson::Value& json_ring){
  // Set corners and bounding box.
  std::for_each(json_ring.Begin(),
                json_ring.End(),
                [&](const auto& c){
                  _corners.emplace_back(c[0].GetDouble(), c[1].GetDouble());
                  _bbox.extend(_corners.back());
                });

  // Stop if first and end corners are different.
  if (_corners[0] != _corners[_corners.size() - 1]){
    std::cout << "Invalid ring, first and last coordinates do not match."
              << std::endl;
    exit(0);
  }
}

// Uses the (l1l2)x(l1l0) cross product z-component to check the
// relative position between l0 and the oriented line defined by l1
// and l2.
//
// Return a positive (resp. negative) value if l0 is left
// (resp. right) of the oriented line through l1 and l2. 0 if l0 is on
// the line.
inline int64_t check_left(const osmium::Location& l0,
                          const osmium::Location& l1,
                          const osmium::Location& l2){
  // Use int32_t version of coordinates with member function x and y to
  // avoid floating point precision issues. Cast the values to int64_t
  // to avoid overflows for initial int32_t values.
  int64_t u_1 = static_cast<int64_t>(l2.x() - l1.x());
  int64_t u_2 = static_cast<int64_t>(l2.y() - l1.y());
  int64_t v_1 = static_cast<int64_t>(l0.x() - l1.x());
  int64_t v_2 = static_cast<int64_t>(l0.y() - l1.y());

  return u_1 * v_2 - u_2 * v_1;
}

bool ring::is_in_ring(const osmium::Location& loc) const{
  // Based on the winding number method, see
  // http://geomalgorithms.com/a03-_inclusion.html.
  int wn = 0;

  for(std::size_t i = 0; i < _corners.size() - 1; ++i){
    // Through all edges of the ring.
    if(_corners[i].y() <= loc.y()){
      if((_corners[i+1].y() > loc.y())
          and (check_left(loc, _corners[i], _corners[i+1]) > 0)){
        // Upward crossing with loc on the left of edge from corners i
        // to i + 1, gives a valid up intersect.
        ++wn;
      }
    }
    else{
      if((_corners[i+1].y() <= loc.y())
          and(check_left(loc, _corners[i], _corners[i+1]) < 0)){
        // Downward crossing with loc on the right of edge from
        // corners i to i + 1, gives a valid down intersect.
        --wn;
      }
    }
  }
  return (wn != 0);
}

bool ring::contains(const osmium::Location& loc) const{
  // First checking with the bounding box to quickly discard most
  // outsiders.
  bool contained = _bbox.contains(loc);
  if(contained){
    contained &= this->is_in_ring(loc);
  }
  return contained;
}

bool ring::contains(const osmium::Node& node) const{
  return this->contains(node.location());
}
