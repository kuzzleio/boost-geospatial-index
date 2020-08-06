var BSI = require('bindings')('BoostSpatialIndex');

/**
 * @constructor
 */
function BoostSpatialIndex() {
  this.spatialIndex = new BSI.SpatialIndex();
  return this;
}

/**
 * Adds a bounding box shape to the index
 *
 * @param {string} id - bounding box identifier
 * @param {Number} min_lat
 * @param {Number} min_lon
 * @param {Number} max_lat
 * @param {Number} max_lon
 * @return {boolean}
 */
BoostSpatialIndex.prototype.addBoundingBox = function addBoundingBox(id, min_lat, min_lon, max_lat, max_lon) {
  if (!id || typeof id !== 'string' || id.length === 0) {
    throw new Error('Invalid "id" parameter: must be a non-empty string');
  }

  if ([min_lat, min_lon, max_lat, max_lon].findIndex(v => typeof v !== 'number') !== -1) {
    throw new Error('Invalid coordinates parameter');
  }

  if (min_lat >= max_lat) {
    throw new Error('min_lat argument cannot be greater or equal max_lat')
  }

  if (min_lon >= max_lon) {
    throw new Error('min_lon argument cannot be greater or equal max_lon')
  }

  return this.spatialIndex.addBBox(id, min_lat, min_lon, max_lat, max_lon);
}

/**
 * Adds a circle shape to the index
 *
 * @param {string} id - circle identifier
 * @param {Number} lat
 * @param {Number} lon
 * @param {Number} radius in meters
 * @return {boolean}
 */
BoostSpatialIndex.prototype.addCircle = function addCircle(id, lat, lon, radius) {
  if (!id || typeof id !== 'string' || id.length === 0) {
    throw new Error('Invalid "id" parameter: must be a non-empty string');
  }

  if ([lat, lon, radius].findIndex(v => typeof v !== 'number') !== -1) {
    throw new Error('Invalid coordinates parameter');
  }

  return this.spatialIndex.addCircle(id, lat, lon, radius);
}

/**
 * Adds an annulus shape to the index
 *
 * @param {string} id - annulus identifier
 * @param {Number} lat
 * @param {Number} lon
 * @param {Number} outerRadius in meters
 * @param {Number} innerRadius in meters
 * @return {boolean}
 */
BoostSpatialIndex.prototype.addAnnulus = function addAnnulus(id, lat, lon, outer, inner) {
  if (!id || typeof id !== 'string' || id.length === 0) {
    throw new Error('Invalid "id" parameter: must be a non-empty string');
  }

  if ([lat, lon, outer, inner].findIndex(v => typeof v !== 'number') !== -1) {
    throw new Error('Invalid coordinates parameter');
  }

  if (outer <= inner)  {
    throw new Error('Outer radius must be strictly greater than the inner one');
  }

  return this.spatialIndex.addAnnulus(id, lat, lon, outer, inner);
}

/**
 * Adds a polygon shape to the index
 * The polygon is open (automatically closed)
 *
 * The "points" argument is an array of array of coordinates.
 * Example:
 *   [[lat, lon], [lat, lon], [lat, lon], ...]
 *
 * @param {string} id - polygon identifier
 * @param {Array<Array<Number>>} points
 * @return {boolean}
 */
BoostSpatialIndex.prototype.addPolygon = function addPolygon(id, points) {
  var idx;

  if (!id || typeof id !== 'string' || id.length === 0) {
    throw new Error('Invalid "id" parameter: must be a non-empty string');
  }

  if (!points || typeof points !== 'object' || !Array.isArray(points)) {
    throw new Error('Invalid points arguments');
  }

  idx = points.findIndex(v => {
    return !v ||
      typeof v !== 'object' ||
      !Array.isArray(v) ||
      v.length !== 2 ||
      v.find(coord => typeof coord !== 'number');
  });

  if (idx !== -1) {
    throw new Error('points arguments must only contain arrays, each one being a point coordinates');
  }

  return this.spatialIndex.addPolygon(id, points);
}

/**
 * Gets all shapes identifiers covering the provided coordinates
 *
 * @param {Number} lat
 * @param {Number} lon
 * @return {Array<string>}
 */
BoostSpatialIndex.prototype.queryPoint = function queryPoint(lat, lon) {
  if (typeof lat !== 'number') {
    throw new Error('Invalid lat parameter');
  }

  if (typeof lon !== 'number') {
    throw new Error('Invalid lon parameter');
  }

  return this.spatialIndex.queryPoint(lat, lon);
}

/**
 * Gets all shape identifiers intersecting the polygon
 * created using the "points" argument
 * The polygon is open (automatically closed)
 *
 * The "points" argument is an array of array of coordinates.
 * Example:
 *   [[lat, lon], [lat, lon], [lat, lon], ...]
 *
 * @param {Array<Array<Number>>} points
 * @return {Array<string>}
 */
BoostSpatialIndex.prototype.queryIntersect = function queryIntersect(points) {

  if (!points || typeof points !== 'object' || !Array.isArray(points)) {
    throw new Error('Invalid points arguments');
  }

  idx = points.findIndex(v => {
    return !v ||
      typeof v !== 'object' ||
      !Array.isArray(v) ||
      v.length !== 2 ||
      v.find(coord => typeof coord !== 'number');
  });

  if (idx !== -1) {
    throw new Error('points arguments must only contain arrays, each one being point coordinates');
  }

  return this.spatialIndex.queryIntersect(points);
}

/**
 * Removes an id from the spatial index
 *
 * @param {string} id - shape identifier to remove
 * @return {boolean}
 */
BoostSpatialIndex.prototype.remove = function remove(id) {
  if (!id || typeof id !== 'string' || id.length === 0) {
    throw new Error('Invalid id parameter');
  }

  return this.spatialIndex.remove(id);
}

module.exports = BoostSpatialIndex;
