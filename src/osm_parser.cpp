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
  uint32_t _all_relations;
  uint32_t _relations_in_polygon;
  const polygon& _polygon;
  index_pos_type& _index_pos;
  std::unordered_set<osmium::object_id_type>& _outside_nodes;
  std::unordered_set<osmium::object_id_type>& _inside_ways;
  osmium::io::Writer& _writer;

  PolygonHandler(const polygon& polygon,
                 index_pos_type& index_pos,
                 std::unordered_set<osmium::object_id_type>& outside_nodes,
                 std::unordered_set<osmium::object_id_type>& inside_ways,
                 osmium::io::Writer& writer):
    _all_nodes(0),
    _nodes_in_polygon(0),
    _all_ways(0),
    _ways_in_polygon(0),
    _all_relations(0),
    _relations_in_polygon(0),
    _polygon(polygon),
    _index_pos(index_pos),
    _outside_nodes(outside_nodes),
    _inside_ways(inside_ways),
    _writer(writer){}

  void node(osmium::Node& node){
    ++_all_nodes;
    if(_polygon.contains(node)){
      // Remember node id for further checking of the ways and write node.
      ++_nodes_in_polygon;
      _index_pos.set(static_cast<osmium::unsigned_object_id_type>(node.id()),
                     node.location());
      _writer(std::move(node));
    }
  }

  void way(osmium::Way& way){
    ++_all_ways;
    // Only keep ways which have a node in the polygon.
    bool keep_way = false;
    for (auto& node_ref : way.nodes()){
      try{
        _index_pos.get(static_cast<osmium::unsigned_object_id_type>(node_ref.ref()));
        // No exception means this way contains a node in the polygon
        // (node inclusion has already been tested during the first
        // "node" pass).
        keep_way = true;
        break;
      }
      catch (osmium::not_found&){}
    }

    if(keep_way){
      ++_ways_in_polygon;
      // Remember outside nodes needed to keep the way complete.
      for (auto& node_ref : way.nodes()){
        try{
          _index_pos.get(static_cast<osmium::unsigned_object_id_type>(node_ref.ref()));
        }
        catch(osmium::not_found&){
          _outside_nodes.insert(node_ref.ref());
        }
      }
      _inside_ways.insert(way.id());
      _writer(std::move(way));
    }
  }

  void relation(osmium::Relation& relation){
    ++_all_relations;
    // Keep relations which have a node in the polygon.
    bool keep_relation = false;
    for (auto& rm : relation.members()){
      if(rm.type() == osmium::item_type::node){
        // Keep relations that have a node member in the polygon.
        try{
          _index_pos.get(static_cast<osmium::unsigned_object_id_type>(rm.ref()));
          // No exception means this node is a member of the relation
          // that has already been tested as included in the polygon.
          keep_relation = true;
          break;
        }
        catch (osmium::not_found&){}
      }
      if(rm.type() == osmium::item_type::way){
        // Also keep relations that have a way member in the polygon.
        if(_inside_ways.find(static_cast<osmium::unsigned_object_id_type>(rm.ref()))
           != _inside_ways.end()){
          keep_relation = true;
          break;
        }
      }
    }

    if(keep_relation){
      ++_relations_in_polygon;
      _writer(std::move(relation));
    }
  }
};

struct OutsideNodesHandler : public osmium::handler::Handler{
  uint64_t _nodes_outside;
  const std::unordered_set<osmium::object_id_type>& _outside_nodes;
  osmium::io::Writer& _writer;

  OutsideNodesHandler(std::unordered_set<osmium::object_id_type>& outside_nodes,
                      osmium::io::Writer& writer):
    _nodes_outside(0),
    _outside_nodes(outside_nodes),
    _writer(writer){}

  void node(osmium::Node& node){
    if(_outside_nodes.find(node.id()) != _outside_nodes.end()){
      ++_nodes_outside;
      _writer(std::move(node));
    }
  }
};

int parse_file(std::string input_name,
               std::string output_name,
               const polygon& polygon){
  // Cache file for node that are inside the polygon.
  std::string nodes_file = "node_cache";
  int fd_inside = open(nodes_file.c_str(), O_RDWR | O_CREAT, 0666);
  if (fd_inside == -1){
    std::cerr << "Can not open node cache file '" << "': " << strerror(errno) << "\n";
    return 1;
  }
  index_pos_type index_pos{fd_inside};

  // Used to store nodes that are outside the polygon BUT in a way
  // that is kept (it has another node inside the polygon).
  std::unordered_set<osmium::object_id_type> outside_nodes;

  // Used to keep track of kept ways for further relation filtering.
  std::unordered_set<osmium::object_id_type> inside_ways;

  // A pass through nodes: filtering nodes and remembering relevant
  // locations.
  osmium::io::File infile(input_name);

  osmium::io::Reader reader_1(infile, osmium::osm_entity_bits::node);

  osmium::io::Header header = reader_1.header();
  header.set("generator", "osmium-polygon");

  osmium::io::Writer writer(output_name,
                            header,
                            osmium::io::overwrite::allow);

  PolygonHandler polygon_handler(polygon,
                                 index_pos,
                                 outside_nodes,
                                 inside_ways,
                                 writer);

  std::cout << "Parsing nodes in " << input_name << "...\n";

  osmium::apply(reader_1, polygon_handler);

  std::cout << "Done, kept "
            << polygon_handler._nodes_in_polygon
            << " nodes out of "
            << polygon_handler._all_nodes
            << " in "
            << polygon.get_name()
            << ".\n";
  reader_1.close();

  // A pass to filter ways.
  osmium::io::Reader reader_2(infile, osmium::osm_entity_bits::way);

  std::cout << "Parsing ways in " << input_name << "...\n";
  osmium::apply(reader_2, polygon_handler);

  std::cout << "Done, kept "
            << polygon_handler._ways_in_polygon
            << " ways in "
            << polygon.get_name()
            << " out of "
            << polygon_handler._all_ways
            << ".\n";

  // Another pass through nodes to make sure ways are complete.
  OutsideNodesHandler outside_nodes_handler(outside_nodes, writer);

  osmium::io::Reader reader_3(infile, osmium::osm_entity_bits::node);

  std::cout << "Making sure all ways are complete...\n";

  osmium::apply(reader_3, outside_nodes_handler);

  std::cout << "Done, added "
            << outside_nodes_handler._nodes_outside
            << " nodes outside "
            << polygon.get_name()
            << " to complete all ways.\n";

  // A pass to filter relations.
  osmium::io::Reader reader_4(infile, osmium::osm_entity_bits::relation);

  std::cout << "Parsing relations in " << input_name << "...\n";
  osmium::apply(reader_4, polygon_handler);

  std::cout << "Done, kept "
            << polygon_handler._relations_in_polygon
            << " relations in "
            << polygon.get_name()
            << " out of "
            << polygon_handler._all_relations
            << ".\n";

  std::remove(nodes_file.c_str());

  return 0;
}

