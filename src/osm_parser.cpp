/*

This file is part of osmium-polygon.

Copyright (c) 2016-2017, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "osm_parser.h"

struct polygon_check_handler : public osmium::handler::Handler{
  uint64_t _all_nodes;
  uint32_t _all_ways;
  uint32_t _all_relations;
  const std::vector<polygon>& _polygons;
  const rtree_t& _rtree;
  std::unordered_set<osmium::object_id_type>& _inside_nodes;
  std::unordered_set<osmium::object_id_type>& _outside_nodes;
  std::unordered_set<osmium::object_id_type>& _inside_ways;
  std::unordered_set<osmium::object_id_type>& _inside_relations;

  polygon_check_handler(const std::vector<polygon>& polygons,
                        const rtree_t& rtree,
                        std::unordered_set<osmium::object_id_type>& inside_nodes,
                        std::unordered_set<osmium::object_id_type>& outside_nodes,
                        std::unordered_set<osmium::object_id_type>& inside_ways,
                        std::unordered_set<osmium::object_id_type>& inside_relations):
    _all_nodes(0),
    _all_ways(0),
    _all_relations(0),
    _polygons(polygons),
    _rtree(rtree),
    _inside_nodes(inside_nodes),
    _outside_nodes(outside_nodes),
    _inside_ways(inside_ways),
    _inside_relations(inside_relations){}

  void node(osmium::Node& node){
    ++_all_nodes;
    // Query Rtree to limit checking to a few polygons.
    std::vector<value> query_result;
    _rtree.query(bgi::contains(point(node.location().lon(),
                                     node.location().lat())),
                 std::back_inserter(query_result));

    auto container = std::find_if(query_result.begin(),
                                  query_result.end(),
                                  [&](const auto& v){
                                    return _polygons[v.second].contains(node);
                                  });

    if(container != query_result.end()){
      // One of the polygons contains this node. Remember node id for
      // further checking of the ways.
      _inside_nodes.insert(node.id());
    }
  }

  void way(osmium::Way& way){
    ++_all_ways;
    // Only keep ways which have a node in the polygons.
    bool keep_way = false;
    for (auto& node_ref: way.nodes()){
      if(_inside_nodes.find(node_ref.ref()) != _inside_nodes.end()){
        // This way contains a node in the polygons (node inclusion
        // has already been tested during the first "node" pass).
        keep_way = true;
        break;
      }
    }

    if(keep_way){
      _inside_ways.insert(way.id());

      // Remember outside nodes needed to keep the way complete.
      for (auto& node_ref : way.nodes()){
        if(_inside_nodes.find(node_ref.ref()) == _inside_nodes.end()){
          _outside_nodes.insert(node_ref.ref());
        }
      }
    }
  }

  void relation(osmium::Relation& relation){
    ++_all_relations;
    // Keep relations which have a node in the polygons.
    for (auto& rm: relation.members()){
      if((rm.type() == osmium::item_type::node
          and _inside_nodes.find(static_cast<osmium::unsigned_object_id_type>(rm.ref()))
          != _inside_nodes.end())
         // Keep relations that have a node member in the polygons.
         or (rm.type() == osmium::item_type::way
             and _inside_ways.find(static_cast<osmium::unsigned_object_id_type>(rm.ref()))
             != _inside_ways.end())
         // Also keep relations that have a way member in the polygons.
         ){
        _inside_relations.insert(relation.id());
        break;
      }
    }
  }
};

struct filter_handler : public osmium::handler::Handler{
  const std::unordered_set<osmium::object_id_type>& _inside_nodes;
  const std::unordered_set<osmium::object_id_type>& _outside_nodes;
  const std::unordered_set<osmium::object_id_type>& _inside_ways;
  const std::unordered_set<osmium::object_id_type>& _inside_relations;
  osmium::io::Writer& _writer;

  filter_handler(const std::unordered_set<osmium::object_id_type>& inside_nodes,
                 const std::unordered_set<osmium::object_id_type>& outside_nodes,
                 const std::unordered_set<osmium::object_id_type>& inside_ways,
                 const std::unordered_set<osmium::object_id_type>& inside_relations,
                 osmium::io::Writer& writer):
    _inside_nodes(inside_nodes),
    _outside_nodes(outside_nodes),
    _inside_ways(inside_ways),
    _inside_relations(inside_relations),
    _writer(writer){}

  void node(osmium::Node& node){
    // Inside nodes could be written during the inclusion check pass,
    // but writing all nodes at once avoids messing the ordering.
    if((_inside_nodes.find(node.id()) != _inside_nodes.end())
       or (_outside_nodes.find(node.id()) != _outside_nodes.end())){
      _writer(std::move(node));
    }
  }

  void way(osmium::Way& way){
    if(_inside_ways.find(way.id()) != _inside_ways.end()){
      _writer(std::move(way));
    }
  }

  void relation(osmium::Relation& relation){
    if(_inside_relations.find(relation.id()) != _inside_relations.end()){
      _writer(std::move(relation));
    }
  }
};

int parse_file(std::string input_name,
               std::string output_name,
               const std::vector<polygon>& polygons,
               const rtree_t& rtree){
  // Used to keep track of nodes that are inside the polygons.
  std::unordered_set<osmium::object_id_type> inside_nodes;

  // Used to keep track of nodes that are outside the polygons BUT in
  // an inside way (with another node inside the polygons).
  std::unordered_set<osmium::object_id_type> outside_nodes;

  // Used to keep track of inside ways.
  std::unordered_set<osmium::object_id_type> inside_ways;

  // Used to keep track of inside relations.
  std::unordered_set<osmium::object_id_type> inside_relations;

  // A pass through nodes to check for inclusion.
  osmium::io::File infile(input_name);

  osmium::io::Reader reader_1(infile,
                              osmium::osm_entity_bits::node
                              | osmium::osm_entity_bits::way
                              | osmium::osm_entity_bits::relation);

  osmium::io::Header header = reader_1.header();
  header.set("generator", "osmium-polygon");

  osmium::io::Writer writer(output_name,
                            header,
                            osmium::io::overwrite::allow);

  polygon_check_handler polygon_handler(polygons,
                                        rtree,
                                        inside_nodes,
                                        outside_nodes,
                                        inside_ways,
                                        inside_relations);

  std::cout << "[info] Checking inclusion for polygons in "
            << input_name
            << "..."
            << std::endl;

  osmium::apply(reader_1, polygon_handler);
  reader_1.close();

  std::cout << "* "
            << inside_nodes.size()
            << " nodes out of "
            << polygon_handler._all_nodes
            << " are inside."
            << std::endl;

  std::cout << "* "
            << inside_ways.size()
            << " ways out of "
            << polygon_handler._all_ways
            << " are inside."
            << std::endl;

  std::cout << "* "
            << inside_relations.size()
            << " relations out of "
            << polygon_handler._all_relations
            << " are inside."
            << std::endl;

  std::cout << "* To ensure way completeness, "
            << outside_nodes.size()
            << " nodes outside polygon(s) should be added."
            << std::endl;

  // Now writing everything with filtering based on the previous
  // inclusion checks.
  filter_handler filter(inside_nodes,
                        outside_nodes,
                        inside_ways,
                        inside_relations,
                        writer);

  osmium::io::Reader reader_2(infile,
                              osmium::osm_entity_bits::node
                              | osmium::osm_entity_bits::way
                              | osmium::osm_entity_bits::relation);

  std::cout << "[info] writing down extract in "
            << output_name
            << std::endl;

  osmium::apply(reader_2, filter);

  return 0;
}
