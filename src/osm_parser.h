/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#ifndef OSM_PARSER_H
#define OSM_PARSER_H

#include <cstdio>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/builder/attr.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "polygon.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<float, 2, bg::cs::cartesian> point;
typedef bg::model::box<point> box;
typedef std::pair<box, unsigned> value;
typedef bgi::rtree<value, bgi::quadratic<16>> rtree_t;

typedef osmium::index::map::SparseMemArray<osmium::unsigned_object_id_type, osmium::Location> index_pos_type;

int parse_file(std::string input_name,
               std::string output_name,
               const std::vector<polygon>& polygons,
               const rtree_t& rtree);

#endif
