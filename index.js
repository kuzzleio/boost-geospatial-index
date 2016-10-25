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
 * @param {Array<Number>} coordinates - [min_lat, min_lon, max_lat, max_lon]
 * @return {boolean}
 */
BoostSpatialIndex.prototype.addBoundingBox = function addBoundingBox(id, coordinates) {
  var idx;

  if (!id) {
    throw new Error('Missing "id" parameter');
  }

  if (!coordinates) {
    throw new Error('Missing "coordinates" parameter');
  }

  if (typeof id !== 'string' || id.length === 0) {
    throw new Error('Invalid "id" parameter: must be a non-empty string');
  }

  idx = coordinates.findIndex(v => typeof v !== 'number');

  if (typeof coordinates !== 'object' || !Array.isArray(coordinates) || coordinates.length !== 4 || idx !== -1) {
    throw new Error('Invalid "coordinates" parameter: must be an array containing numbers');
  }

  return this.spatialIndex.addBBox(id, coordinates);
}

/**
 * Gets all shapes identifiers covering the provided coordinates
 *
 * @param {Number} lat
 * @param {Number} lon
 * @return {Array<string>}
 */
BoostSpatialIndex.prototype.queryPoint = function queryPoint(lat, lon) {
  if (!lat) {
    throw new Error('Missing lat parameter');
  }

  if (!lon) {
    throw new Error('Missing lon parameter');
  }

  if (typeof lat !== 'number' || typeof lon !== 'number') {
    throw new Error('lat and lon parameters must be numbers');
  }

  return this.spatialIndex.queryPoint(lat, lon);
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
