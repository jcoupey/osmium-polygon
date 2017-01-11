# Changelog

## [v0.2] - 2017-01-11

### Added

- Use a Rtree to avoid most inclusion checks (#5), results in huge
  speedup when using many polygons in input.

### Changed

- Update `libosmium` to v2.10.3.

## [v0.1] - 2016-12-17

### Added

- Extract OSM data (nodes, ways and relations) with regard to
inclusion in polygons.
- Use geojson as polygon(s) input format.
- Handle holes in polygons (#1).
- Handle multi-polygons (#2).

### Fixed

- Respect standard output block order (#3).
- Respect output objects ordering (#4).
