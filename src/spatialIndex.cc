#include "spatialIndex.hpp"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

/*
 * Converts a JS [lat, lon] pair into a boost point
 * boost coordinates are Long,Lat, not Lat,Long
 */
static point toPoint(Napi::Value value) {
  Napi::Object p = value.As<Napi::Object>();

  return point(
    p.Get(1u).ToNumber().DoubleValue(),
    p.Get(0u).ToNumber().DoubleValue());
}

static double toDouble(Napi::Value value) {
  return value.ToNumber().DoubleValue();
}

/*
 * Module initialization
 */
Napi::Object SpatialIndex::init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "SpatialIndex", {
    InstanceMethod("addBBox", &SpatialIndex::addBBox),
    InstanceMethod("addCircle", &SpatialIndex::addCircle),
    InstanceMethod("addAnnulus", &SpatialIndex::addAnnulus),
    InstanceMethod("addPolygon", &SpatialIndex::addPolygon),
    InstanceMethod("queryPoint", &SpatialIndex::queryPoint),
    InstanceMethod("queryIntersect", &SpatialIndex::queryIntersect),
    InstanceMethod("remove", &SpatialIndex::remove),
  });

  exports.Set("SpatialIndex", func);
  return exports;
}

SpatialIndex::SpatialIndex(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<SpatialIndex>(info) {
}

/*
 * Adds a bounding box to the tree
 * addBBox(id, min_lat, min_lon, max_lat, max_lon)
 */
Napi::Value SpatialIndex::addBBox(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // Checks the id parameter validity
  if (!info[0].IsString()) {
    return Napi::Boolean::New(env, false);
  }

  std::string id = info[0].As<Napi::String>().Utf8Value();

  // Checks the coordinates parameters validity
  for(size_t i = 1; i < 5; i++) {
    if (!info[i].IsNumber()) {
      return Napi::Boolean::New(env, false);
    }
  }

  // boost coordinates are Long,Lat, not Lat,Long
  box bbox(
    point(toDouble(info[2]), toDouble(info[1])),
    point(toDouble(info[4]), toDouble(info[3]))
  );

  std::shared_ptr<Shape> shape(new Shape(id, bbox));

  rtree.insert(std::make_pair(bbox, shape));
  repository.insert(std::make_pair(id, shape));

  return env.Undefined();
}

/*
 * Adds a circle to the tree
 * addCircle(id, lat, lon, radius)
 */
Napi::Value SpatialIndex::addCircle(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // Checks the id parameter validity
  if (!info[0].IsString()) {
    return Napi::Boolean::New(env, false);
  }

  std::string id = info[0].As<Napi::String>().Utf8Value();

  // Checks the coordinates parameters validity
  for(size_t i = 1; i < 4; i++) {
    if (!info[i].IsNumber()) {
      return Napi::Boolean::New(env, false);
    }
  }

  // boost coordinates are Long,Lat, not Lat,Long
  point p(toDouble(info[2]), toDouble(info[1]));

  std::shared_ptr<Shape> shape(new Shape(id, p, toDouble(info[3])));

  rtree.insert(std::make_pair(shape->getEnvelope(), shape));
  repository.insert(std::make_pair(id, shape));

  return env.Undefined();
}

/*
 * Adds an annulus to the tree
 * addCircle(id, lat, lon, outerRadius, innerRadius)
 */
Napi::Value SpatialIndex::addAnnulus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // Checks the id parameter validity
  if (!info[0].IsString()) {
    return Napi::Boolean::New(env, false);
  }

  std::string id = info[0].As<Napi::String>().Utf8Value();

  // Checks the coordinates parameters validity
  for(size_t i = 1; i < 5; i++) {
    if (!info[i].IsNumber()) {
      return Napi::Boolean::New(env, false);
    }
  }

  // boost coordinates are Long,Lat, not Lat,Long
  point p(toDouble(info[2]), toDouble(info[1]));

  std::shared_ptr<Shape> shape(new Shape(id, p, toDouble(info[3]), toDouble(info[4])));

  rtree.insert(std::make_pair(shape->getEnvelope(), shape));
  repository.insert(std::make_pair(id, shape));

  return env.Undefined();
}

/*
 * Adds a polygon to the tree
 * addPolygon(id, [[lat, lon], [lat, lon], [lat, lon], ...]])
 */
Napi::Value SpatialIndex::addPolygon(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  // Checks the id parameter validity
  if (!info[0].IsString()) {
    return Napi::Boolean::New(env, false);
  }

  std::string id = info[0].As<Napi::String>().Utf8Value();

  // Checks the coordinates parameters validity
  if (!info[1].IsArray()) {
    return Napi::Boolean::New(env, false);
  }

  Napi::Array points = info[1].As<Napi::Array>();
  polygon pl;

  for(uint32_t i = 0; i < points.Length(); i++) {
    pl.outer().push_back(toPoint(points.Get(i)));
  }

  std::shared_ptr<Shape> shape(new Shape(id, pl));

  rtree.insert(std::make_pair(shape->getEnvelope(), shape));
  repository.insert(std::make_pair(id, shape));

  return env.Undefined();
}

/*
 * Gets all ids embedding the provided point coordinates
 * queryPoint(lat, lon)
 *
 * Returns an array of matching ids as strings
 */
Napi::Value SpatialIndex::queryPoint(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Array result = Napi::Array::New(env);

  // Checks the point coordinates parameter validity
  if (!info[0].IsNumber() || !info[1].IsNumber()) {
    return result;
  }

  double lat = info[0].As<Napi::Number>().DoubleValue();
  double lon = info[1].As<Napi::Number>().DoubleValue();

  point coordinates(lon, lat);
  std::vector<treeValue> found;
  rtree.query(bgi::covers(coordinates), std::back_inserter(found));

  uint32_t count = 0;
  for(std::vector<treeValue>::iterator it = found.begin(); it != found.end(); ++it) {
    if (it->second->covered(coordinates)) {
      result.Set(count++, Napi::String::New(env, it->second->getId()));
    }
  }

  return result;
}

/*
 * Gets all ids embedding the provided point coordinates
 * queryIntersect([[lat, lon], [lat, lon], [lat, lon], ...]])
 *
 * Returns an array of matching ids as strings
 */
Napi::Value SpatialIndex::queryIntersect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Array result = Napi::Array::New(env);

  // Checks the coordinates parameters validity
  if (!info[0].IsArray()) {
    return result;
  }

  Napi::Array points = info[0].As<Napi::Array>();
  polygon queryPoly;

  // note: toPoint flips coordinates from lat,long to long,lat to abide by boost
  for(uint32_t i = 0; i < points.Length(); i++) {
    queryPoly.outer().push_back(toPoint(points.Get(i)));
  }

  std::vector<treeValue> found;
  //calling intersects here, pure inside polygon check would be covered_by
  rtree.query(bgi::intersects(queryPoly), std::back_inserter(found));

  uint32_t count = 0;
  for(std::vector<treeValue>::iterator it = found.begin(); it != found.end(); ++it) {
    result.Set(count++, Napi::String::New(env, it->second->getId()));
  }

  return result;
}

/*
 * Removes an object from the index
 * remove(id)
 *
 * Returns a boolean
 */
Napi::Value SpatialIndex::remove(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (!info[0].IsString()) {
    return Napi::Boolean::New(env, false);
  }

  std::string id = info[0].As<Napi::String>().Utf8Value();
  std::unordered_map<std::string, std::shared_ptr<Shape> >::const_iterator found = repository.find(id);

  if (found == repository.end()) {
    return Napi::Boolean::New(env, false);
  }

  /*
   The geographic R* tree occasionally fails to find an entry it holds when
   asked to remove it (remove() then returns 0): the shape would stay in the
   index and keep being returned by queries after its id is gone. When that
   happens, rebuild the tree without it. It is rare (a fraction of a percent
   of removals) and costs O(n log n) only then.
   */
  if (rtree.remove(std::make_pair(found->second->getEnvelope(), found->second)) == 0) {
    std::vector<treeValue> kept;
    kept.reserve(rtree.size());

    for (rtreeType::const_iterator it = rtree.begin(); it != rtree.end(); ++it) {
      if (it->second != found->second) {
        kept.push_back(*it);
      }
    }

    rtreeType rebuilt(kept.begin(), kept.end());
    rtree.swap(rebuilt);
  }

  repository.erase(id);

  return Napi::Boolean::New(env, true);
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  return SpatialIndex::init(env, exports);
}

NODE_API_MODULE(BoostSpatialIndex, init)
