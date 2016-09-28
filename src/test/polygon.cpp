/*

This file is part of osmium-polygon.

Copyright (c) 2016, Julien Coupey.
All rights reserved (see LICENSE).

*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Polygon
#include <boost/test/unit_test.hpp>
#include "../polygon.h"

struct init_state_box{
  polygon p;
  init_state_box(): p("Box",
                      {{0.0, 0.0}, {20.0, 0.0}, {20.0, 50.0}, {0.0, 50.0}, {0.0, 0.0}}) {}
};

BOOST_FIXTURE_TEST_SUITE(box_checks, init_state_box)

BOOST_AUTO_TEST_CASE(box_name){
  BOOST_CHECK(p.get_name() == "Box");
}

BOOST_AUTO_TEST_CASE(box_contains_nodes){
  BOOST_CHECK(p.contains({0.0, 0.0}));
  // Upper horizontal and right-side boundaries are outside.
  BOOST_CHECK(!p.contains({0.0, 50.0}));
  BOOST_CHECK(!p.contains({20.0, 0.0}));
  BOOST_CHECK(!p.contains({20.0, 50.0}));
}

BOOST_AUTO_TEST_CASE(box_contains_limits){
  BOOST_CHECK(p.contains({10.0, 0.0}));
  BOOST_CHECK(p.contains({0.0, 43.0}));
  // Upper horizontal and right-side boundaries are outside.
  BOOST_CHECK(!p.contains({27.0, 0.0}));
  BOOST_CHECK(!p.contains({32.0, 50.0}));
}

BOOST_AUTO_TEST_CASE(box_contains_inside){
  for(double i = 1.0; i < 20.0; i += 2.0){
    for(double j = 1.0; j < 50.0; j += 5.0){
      BOOST_CHECK(p.contains({i, j}));
    }
  }
}

BOOST_AUTO_TEST_CASE(box_no_contains_outside){
  BOOST_CHECK(!p.contains({-1.0, 0.0}));
  BOOST_CHECK(!p.contains({21.0, 0.0}));
  BOOST_CHECK(!p.contains({32.0, 51.0}));
  BOOST_CHECK(!p.contains({4.0, -2.0}));
}

BOOST_AUTO_TEST_SUITE_END()

struct init_state_losange{
  polygon p;
  init_state_losange(): p("Losange",
                          {{20.0, 0.0}, {0.0, 40.0}, {-20.0, 0.0}, {0.0, -40.0}, {20.0, 0.0}}) {}
};

BOOST_FIXTURE_TEST_SUITE(losange_checks, init_state_losange)

BOOST_AUTO_TEST_CASE(losange_name){
  BOOST_CHECK(p.get_name() == "Losange");
}

BOOST_AUTO_TEST_CASE(losange_contains_nodes){
  BOOST_CHECK(p.contains({-20.0, 0.0}));
  // Right-side boundaries are outside.
  BOOST_CHECK(!p.contains({20.0, 0.0}));
  BOOST_CHECK(!p.contains({0.0, 40.0}));
  BOOST_CHECK(!p.contains({0.0, -40.0}));
}

BOOST_AUTO_TEST_CASE(losange_contains_limits){
  BOOST_CHECK(p.contains({-7.0, 26.0}));
  BOOST_CHECK(p.contains({-18.0, 4.0}));
  BOOST_CHECK(p.contains({-18.0, -4.0}));
  BOOST_CHECK(p.contains({-2.0, -36.0}));
  // Right-side boundaries are outside.
  BOOST_CHECK(!p.contains({10.0, 20.0}));
  BOOST_CHECK(!p.contains({8.0, 24.0}));
  BOOST_CHECK(!p.contains({3.0, -34.0}));
  BOOST_CHECK(!p.contains({10.0, -20.0}));
}

BOOST_AUTO_TEST_CASE(losange_contains_inside){
  BOOST_CHECK(p.contains({0.0, 0.0}));
  BOOST_CHECK(p.contains({12.0, 0.0}));
  BOOST_CHECK(p.contains({10.0, 3.0}));
  BOOST_CHECK(p.contains({0.0, 37.0}));
  BOOST_CHECK(p.contains({-1.0, 28.0}));
  BOOST_CHECK(p.contains({-19.0, 0.0}));
  BOOST_CHECK(p.contains({-5.0, -5.0}));
  BOOST_CHECK(p.contains({0.0, -22.0}));
  BOOST_CHECK(p.contains({4.0, -3.0}));
}

BOOST_AUTO_TEST_CASE(losange_no_contains_outside){
  // Outside bounding box, near the nodes.
  BOOST_CHECK(!p.contains({21.0, 0.0}));
  BOOST_CHECK(!p.contains({0.0, 41.0}));
  BOOST_CHECK(!p.contains({-41.0, 0.0}));
  BOOST_CHECK(!p.contains({0.0, -41.0}));

  // Outside the bounding box.
  BOOST_CHECK(!p.contains({27.0, 32.0}));
  BOOST_CHECK(!p.contains({-18.0, 49.0}));
  BOOST_CHECK(!p.contains({-40.0, -60.0}));
  BOOST_CHECK(!p.contains({2.0, -45.0}));

  // In the bouding box but not in the polygon!
  BOOST_CHECK(!p.contains({10.0, 21.0}));
  BOOST_CHECK(!p.contains({-1.0, 39.0}));
  BOOST_CHECK(!p.contains({-5.0, -31.0}));
  BOOST_CHECK(!p.contains({18.0,-5.0}));
}

BOOST_AUTO_TEST_SUITE_END()

struct init_state_poly_test{
  polygon p;
  init_state_poly_test(): p("Test",
                            {{2.30, 48.83},{2.25, 48.84},{2.25, 48.86},{2.28, 48.87},{2.31, 48.86},{2.29, 48.88},{2.33, 48.88},{2.36, 48.87},{2.37, 48.86},{2.35, 48.85},{2.38, 48.84},{2.38, 48.83},{2.345, 48.83},{2.34, 48.835},{2.33, 48.828},{2.325, 48.82},{2.30, 48.83}}) {}
};

BOOST_FIXTURE_TEST_SUITE(poly_test_checks, init_state_poly_test)

BOOST_AUTO_TEST_CASE(poly_test_name){
  BOOST_CHECK(p.get_name() == "Test");
}

BOOST_AUTO_TEST_CASE(poly_test_contains_nodes){
  BOOST_CHECK(p.contains({2.30, 48.83}));
  BOOST_CHECK(p.contains({2.25, 48.84}));
  BOOST_CHECK(p.contains({2.25, 48.86}));

  // Right-side boundary
  BOOST_CHECK(!p.contains({2.28, 48.87}));

  // Left-side boundary
  BOOST_CHECK(p.contains({2.31, 48.86}));

  // Upper-side boundary
  BOOST_CHECK(!p.contains({2.29, 48.88}));
  BOOST_CHECK(!p.contains({2.33, 48.88}));

  // Right-side boundary
  BOOST_CHECK(!p.contains({2.36, 48.87}));
  BOOST_CHECK(!p.contains({2.37, 48.86}));
  BOOST_CHECK(!p.contains({2.35, 48.85}));
  BOOST_CHECK(!p.contains({2.38, 48.84}));
  BOOST_CHECK(!p.contains({2.38, 48.83}));

  BOOST_CHECK(p.contains({2.345, 48.83}));
  BOOST_CHECK(p.contains({2.34, 48.835}));

  // Right-side boundary
  BOOST_CHECK(!p.contains({2.33, 48.828}));
  BOOST_CHECK(!p.contains({2.325, 48.82}));
}

BOOST_AUTO_TEST_CASE(poly_test_contains_limits){
  // Vertical edges, left and right.
  BOOST_CHECK(p.contains({2.25, 48.85}));
  BOOST_CHECK(!p.contains({2.38, 48.835}));
  // Horizontal upper edge.
  BOOST_CHECK(!p.contains({2.30, 48.88}));
  BOOST_CHECK(!p.contains({2.31, 48.88}));
  BOOST_CHECK(!p.contains({2.32, 48.88}));
}

BOOST_AUTO_TEST_CASE(poly_test_contains_inside){
  // Just before right edge.
  BOOST_CHECK(p.contains({2.37999, 48.835}));

  // Locations chosen uniformly inside.
  BOOST_CHECK(p.contains({2.3102188110351562, 48.87284474327185}));
  BOOST_CHECK(p.contains({2.3345947265625, 48.86990906900767}));
  BOOST_CHECK(p.contains({2.3521041870117188, 48.861778610526805}));
  BOOST_CHECK(p.contains({2.278289794921875, 48.85545400732256}));
  BOOST_CHECK(p.contains({2.3236083984375, 48.8615527456014}));
  BOOST_CHECK(p.contains({2.3232650756835938, 48.82291485286295}));
  BOOST_CHECK(p.contains({2.35107421875, 48.833763586380556}));
  BOOST_CHECK(p.contains({2.3692703247070312, 48.8360234435617}));
  BOOST_CHECK(p.contains({2.3294448852539062, 48.848902682969765}));
  BOOST_CHECK(p.contains({2.2580337524414062, 48.85387273165656}));
  BOOST_CHECK(p.contains({2.2652435302734375, 48.843254301505446}));
  BOOST_CHECK(p.contains({2.2813796997070312, 48.83805722786541}));
  BOOST_CHECK(p.contains({2.2964859008789062, 48.85613168160397}));
  BOOST_CHECK(p.contains({2.3023223876953125, 48.845965604118284}));
  BOOST_CHECK(p.contains({2.3201751708984375, 48.84031689136024}));
  BOOST_CHECK(p.contains({2.3009490966796875, 48.833763586380556}));
}

BOOST_AUTO_TEST_CASE(poly_test_no_contains_outside){
  // Outside the bounding box.
  BOOST_CHECK(!p.contains({2.3088455200195312, 48.891132372037255}));
  BOOST_CHECK(!p.contains({2.3582839965820312, 48.896549644565184}));
  BOOST_CHECK(!p.contains({2.4039459228515625, 48.87284474327185}));
  BOOST_CHECK(!p.contains({2.3912429809570312, 48.848902682969765}));
  BOOST_CHECK(!p.contains({2.4118423461914062, 48.82585328344824}));
  BOOST_CHECK(!p.contains({2.2484207153320312, 48.894518236158234}));
  BOOST_CHECK(!p.contains({2.2377777099609375, 48.860875144709475}));
  BOOST_CHECK(!p.contains({2.2113418579101562, 48.832181625698475}));
  BOOST_CHECK(!p.contains({2.2813796997070312, 48.8079940090084}));
  BOOST_CHECK(!p.contains({2.3280715942382812, 48.793294897853706}));
  BOOST_CHECK(!p.contains({-1.0986328125, 49.14578361775004}));
  BOOST_CHECK(!p.contains({2.373046875, 46.255846818480315}));
  BOOST_CHECK(!p.contains({-2.4609375, 52.45600939264076}));
  BOOST_CHECK(!p.contains({-102.12890625, 39.232253141714885}));
  BOOST_CHECK(!p.contains({-60.1171875, -15.623036831528252}));
  BOOST_CHECK(!p.contains({-35.5078125, 77.157162522661}));
  BOOST_CHECK(!p.contains({159.9609375, 66.08936427047088}));
  BOOST_CHECK(!p.contains({132.1875, -23.241346102386135}));
  BOOST_CHECK(!p.contains({22.8515625, 3.162455530237848}));
  BOOST_CHECK(!p.contains({69.9609375, 59.712097173322924}));

  // In the bouding box but not in the polygon!
  BOOST_CHECK(!p.contains({2.255115509033203, 48.8664086168748}));
  BOOST_CHECK(!p.contains({2.3024940490722656, 48.86403720372053}));
  BOOST_CHECK(!p.contains({2.28515625, 48.87431251581011}));
  BOOST_CHECK(!p.contains({2.267475128173828, 48.87419961175908}));
  BOOST_CHECK(!p.contains({2.3476409912109375, 48.876683442021886}));
  BOOST_CHECK(!p.contains({2.368927001953125, 48.87352218210199}));
  BOOST_CHECK(!p.contains({2.3696136474609375, 48.86584400488787}));
  BOOST_CHECK(!p.contains({2.358112335205078, 48.85037115779437}));
  BOOST_CHECK(!p.contains({2.3725318908691406, 48.85093594433627}));
  BOOST_CHECK(!p.contains({2.3397445678710933, 48.83252062147833}));
  BOOST_CHECK(!p.contains({2.335796356201172, 48.82472313822774}));
  BOOST_CHECK(!p.contains({2.3627471923828125, 48.82551424255817}));
  BOOST_CHECK(!p.contains({2.2544288635253906, 48.83591045312373}));
  BOOST_CHECK(!p.contains({2.2954559326171875, 48.827322434132746}));
  BOOST_CHECK(!p.contains({2.3021507263183594, 48.82268881260476}));
  BOOST_CHECK(!p.contains({2.264556884765625, 48.82415805606007}));
}

BOOST_AUTO_TEST_SUITE_END()
