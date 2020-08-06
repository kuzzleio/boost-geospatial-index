# boost-geospatial-index

Stores and indexes geospatial shapes and exposes query methods.  
Shapes can overlap.

This library is used by [Kuzzle](http://kuzzle.io)'s real-time engine and is focused on query performances:

* shapes are indexed using a [R* tree](https://en.wikipedia.org/wiki/R*_tree) ("slow" insertion, very fast query time)
* based on the C++ [boost::geometry](http://www.boost.org/doc/libs/1_62_0/libs/geometry/doc/html/geometry/spatial_indexes/introduction.html) library
* pure C++ module
* no dependencies

# Table of content

- [Install](#install)
- [API](#api)
  - [Creating a new index](#creating-a-new-index)
  - [Adding shapes to the index](#adding-shapes-to-the-index)
      - [addBoundingBox(id, min_lat, min_lon, max_lat, max_lon)](#addboundingboxid-min_lat-min_lon-max_lat-max_lon)
      - [addCircle(id, lat, lon, radius)](#addcircleid-lat-lon-radius)
      - [addAnnulus(id, lat, lon, outer, inner)](#addannulusid-lat-lon-outer-inner)
      - [addPolygon(id, points)](#addpolygonid-points)
- [Querying the index](#querying-the-index)
    - [queryPoint(lat, lon)](#querypointlat-lon)
    - [queryIntersect(points)](#queryintersecpoints)
- [Removing a shape from the index](#removing-a-shape-from-the-index)
    - [remove(id)](#removeid)
- [Example](#example)
- [TODO](#todo)
- [License](#license)


# Install

```
npm install --save boost-geospatial-index
```

# API

## Creating a new index

```js
var BoostSpatialIndex = require('boost-geospatial-index');

var bsi = new BoostSpatialIndex();
```

## Adding shapes to the index

##### addBoundingBox(id, min_lat, min_lon, max_lat, max_lon)

**Returns:** `boolean`

| Argument | Type | Description |
|----------|------|-------------|
| `id` | string | shape unique identifier |
| `min_lat` | number | bottom side of the bounding box |
| `min_lon` | number |  left side of the bounding box |
| `max_lat` | number |  top side of the bounding box |
| `max_lon` | number |  right side of the bounding box |



##### addCircle(id, lat, lon, radius)

**Returns:** `boolean`

| Argument | Type | Description |
|----------|------|-------------|
| `id` | string | shape unique identifier |
| `lat` | number | y coordinate of the circle's center |
| `lon` | number | x coordinate of the circle's center |
| `radius` | number | radius of the circle, in meters |

##### addAnnulus(id, lat, lon, outer, inner)

**Returns:** `boolean`

| Argument | Type | Description |
|----------|------|-------------|
| `id` | string | shape unique identifier |
| `lat` | number | y coordinate of the annulus' center |
| `lon` | number | x coordinate of the annulus' center |
| `outer` | number | outer radius of the annulus, in meters |
| `inner` | number | inner radius of the annulus, in meters |

##### addPolygon(id, points)

Creates an open polygon (automatically closed by the library).

**Returns:** `boolean`

| Argument | Type | Description |
|----------|------|-------------|
| `id` | string | shape unique identifier |
| `points` | array | array of arrays of [lat, lon] points |

# Querying the index

##### queryPoint(lat, lon)

Gets all shapes containing the provided point coordinates. Shapes matching the provided point exactly on their borders or corners are also returned.

If no shape match, an empty array is returned.

**Returns:** `array of shape ids`

| Argument | Type | Description |
|----------|------|-------------|
| `lat` | number | y coordinate |
| `lon` | number | x coordinate |

##### queryIntersect(points)

Gets all shapes intersecting the polygon created from the list of point coordinates provided. Shapes partially covered by the polygon also returned.

If no shape match, an empty array is returned.

**Returns:** `array of shape ids`

| Argument | Type | Description |
|----------|------|-------------|
| `points` | array | array of arrays of [lat, lon] points |

# Removing a shape from the index

##### remove(id)

**Returns:** `boolean`

| Argument | Type | Description |
|----------|------|-------------|
| `id` | string | shape unique identifier |

# Example

```js
var BoostSpatialIndex = require('boost-geospatial-index');

var bsi = new BoostSpatialIndex();

bsi.addBoundingBox('Montpellier, France', 43.5810609, 3.8433703, 43.6331979, 3.9282093);
bsi.addPolygon('Montpellier Millenaire', [
  [43.6021299, 3.8989713],
  [43.6057389, 3.8968173],
  [43.6092889, 3.8970423],
  [43.6100359, 3.9040853],
  [43.6069619, 3.9170343],
  [43.6076479, 3.9230133],
  [43.6038779, 3.9239153],
  [43.6019189, 3.9152403],
  [43.6036049, 3.9092313]
]);
bsi.addAnnulus('Around Kuzzle HQ', 43.6073913, 3.9109057, 1500, 1);
bsi.addCircle('Montpellier Airport', 43.5764455, 3.948711, 2000);

console.log('Querying Kuzzle HQ: ', bsi.queryPoint(43.6073913, 3.9109057));
console.log('Kuzzle team favorite pub:', bsi.queryPoint(43.6002203, 3.897105));
console.log('Hangout spots:', bsi.queryIntersect([
            [43.6072203, 3.9],
            [43.607,5.900],
            [44.609000, 5.90000],
            [44.605,3.9]
            ]));
```

Result:

```
Querying Kuzzle HQ:  [ 'Montpellier, France', 'Montpellier Millenaire' ]
Kuzzle team favorite pub: [ 'Montpellier, France', 'Around Kuzzle HQ' ]
Hangout spots: [ 'Montpellier, France',
  'Montpellier Millenaire',
  'Around Kuzzle HQ' ]
```

# TODO

Here are some features we might add in the future:

* add support for polygons with inner holes

# License

This library is published under [Apache 2 License](LICENSE.md).
