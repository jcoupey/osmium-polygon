# About

This project is the result of playing around with
[libosmium](https://github.com/osmcode/libosmium) and polygon
inclusion.

So far, it allows to extract all nodes and ways in an OSM data file
with regard to inclusion in a custom polygon.

![berlin](/files/berlin_heart.png)

**Notes**

- Nodes included in the polygon are kept.
- Ways that have at least a node in the polygon are kept (with all
their nodes).
- Relations are not handled at all so far!

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
