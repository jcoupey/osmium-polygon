/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "osm_parser.h"

struct PolygonHandler : public osmium::handler::Handler{
  uint64_t _all_nodes;
  uint64_t _nodes_in_polygon;
  uint32_t _all_ways;
  uint32_t _ways_in_polygon;
  const polygon& _polygon;
  index_pos_type& _index_pos;
  osmium::io::Writer _writer;

  PolygonHandler(const std::string &filename,
                 osmium::io::Header& header,
                 const polygon& polygon,
                 index_pos_type& index_pos):
    _all_nodes(0),
    _nodes_in_polygon(0),
    _all_ways(0),
    _ways_in_polygon(0),
    _polygon(polygon),
    _index_pos(index_pos),
    _writer(filename, header, osmium::io::overwrite::allow){}

  void node(osmium::Node& node){
    ++_all_nodes;
    if(_polygon.contains(node)){
      ++_nodes_in_polygon;
      _index_pos.set(static_cast<osmium::unsigned_object_id_type>(node.id()),
                     node.location());
      _writer(std::move(node));
    }
  }
};

int parse_file(std::string input_name,
               std::string output_name,
               const polygon& polygon){
  // Cache file for node locations.
  std::string nodes_file = "node_cache";
  int fd = open(nodes_file.c_str(), O_RDWR | O_CREAT, 0666);
  if (fd == -1){
    std::cerr << "Can not open node cache file '" << "': " << strerror(errno) << "\n";
    return 1;
  }

  index_pos_type index_pos{fd};

  // First pass: filtering nodes and remember relevant locations.
  std::cout << "Parsing nodes...\n";
  osmium::io::File infile(input_name);

  osmium::io::Reader reader_1(infile,
                              osmium::osm_entity_bits::node);

  osmium::io::Header header = reader_1.header();
  header.set("generator", "osmium-polygon");

  PolygonHandler polygon_handler(output_name, header, polygon, index_pos);
  osmium::apply(reader_1, polygon_handler);

  std::cout << "Done, kept "
            << polygon_handler._nodes_in_polygon
            << " nodes in "
            << polygon.get_name()
            << " out of "
            << polygon_handler._all_nodes
            << ".\n";
  reader_1.close();

  std::remove(nodes_file.c_str());

  return 0;
}

