# About

This project is the result of playing around with
[libosmium](https://github.com/osmcode/libosmium) and polygon
inclusion.

It allows to extract all nodes, ways and relations in an OSM data file
with regard to inclusion in a set of (multi-)polygons defined in a
simple `geojson` file.

- Nodes included in the polygons are kept.
- Ways containing at least a node in the polygons are kept and stay
  complete (even their nodes that are not in the polygons are
  included).
- Relations that have at least one node or a way member in the
  polygons are included with all their members (not
  reference-complete).

# Examples

## Single polygon

Using [this geojson file](/files/berlin_heart.geojson).

![berlin heart](/files/berlin_heart.png)

## Polygon with a hole

Using [this geojson file](/files/berlin_ring.geojson).

![berlin ring](/files/berlin_ring.png)

## Several (multi-)polygons

Using [this geojson file](/files/berlin_flower.geojson).

![berlin flower](/files/berlin_flower.png)

# Build

Make sure `boost` is installed on your system, clone the repo and
build from the `src` folder.

```bash
git clone https://github.com/jcoupey/osmium-polygon.git
cd osmium-polygon/src/
make
cd ..
```

# Usage

Use `-p` to describe which geojson file contains your (multi-)polygons
and run on any OSM data file.

```bash
./osmium-polygon -p files/berlin_heart.geojson berlin-latest.osm.pbf
```

# Tests

In the `src` folder, build and run using:

```bash
make test
cd ../
./osmium-polygon-tests
```
