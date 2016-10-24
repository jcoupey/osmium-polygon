# About

This project is the result of playing around with
[libosmium](https://github.com/osmcode/libosmium) and polygon
inclusion.

So far, it allows to extract all nodes, ways and relations in an OSM
data file with regard to inclusion in a custom polygon.

- Nodes included in the polygon are kept.
- Ways containing at least a node in the polygon are kept and stay
  complete (even their nodes that are not in the polygon are
  included).
- Relations that have at least one node or a way member in the polygon
  are included with all their members (not reference-complete).

![berlin](/files/berlin_heart.png)

# Build

Clone the repo, create a `bin` folder and build.

```bash
git clone https://github.com/jcoupey/osmium-polygon.git
cd osmium-polygon/
mkdir bin
cd src/
make
```

# Usage

Use `-p` to describe which geojson file contains your polygon and run
on any OSM data file.

```bash
osmium-polygon -p files/berlin_heart.geojson berlin-latest.osm.pbf
```

# Tests

In the `src` folder, build and run using:

```bash
make test
../bin/osmium-polygon-tests
```
