#include <v8.h>
#include <node.h>
#include <nan.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "shape.hpp"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// shortcuts
typedef std::pair<box, std::shared_ptr<Shape>> treeValue;

// The spatial index containing MBRs (minimum bounding rectangles)
bgi::rtree< treeValue, bgi::rstar<16> > rtree;

// Map geometry objects with their corresponding string id
std::unordered_map< std::shared_ptr<std::string>, std::shared_ptr<Shape> > repository;

/*
 * Adds a bounding box to the tree
 * addBox(id, [min_lat, min_lon, max_lat, max_lon])
 */
NAN_METHOD(addBBox) {
  Nan::HandleScope scope;
  v8::Local<v8::Array> coords;

  // Checks the id parameter validity
  if (info[0]->IsUndefined() || !info[0]->IsString()) {
    info.GetReturnValue().Set(false);
    return;
  }

  v8::String::Utf8Value argId(info[0]);
  std::shared_ptr<std::string> id(new std::string(*argId));

  // Checks the box coordinates parameter validity
  if (info[1]->IsUndefined() || !info[1]->IsArray()) {
    info.GetReturnValue().Set(false);
    return;
  }

  coords = info[1].As<v8::Array>();

  if (coords->Length() != 4) {
    info.GetReturnValue().Set(false);
    return;
  }

  std::shared_ptr<box> bbox(new box(
    point(coords->Get(0)->ToNumber()->Value(), coords->Get(1)->ToNumber()->Value()),
    point(coords->Get(2)->ToNumber()->Value(), coords->Get(3)->ToNumber()->Value())
  ));

  std::shared_ptr<Shape> shape(new Shape(id, bbox));

  rtree.insert(std::make_pair(*bbox, shape));
  repository.insert(std::make_pair(id, shape));
}

/*
 * Gets all ids embedding the provided point coordinates
 * queryPoint(lat, lon)
 *
 * Returns an array of matching ids as strings
 */
NAN_METHOD(queryPoint) {
  Nan::HandleScope scope;
  v8::Local<v8::Array> result = Nan::New<v8::Array>();
  v8::Local<v8::Number> lat, lon;

  // Checks the point coordinates parameter validity
  if (info[0]->IsUndefined() || info[1]->IsUndefined() || !info[0]->IsNumber() || !info[1]->IsNumber()) {
    info.GetReturnValue().Set(result);
    return;
  }

  lat = info[0].As<v8::Number>();
  lon = info[1].As<v8::Number>();

  point coordinates(lat->Value(), lon->Value());
  std::vector<treeValue> found;
  rtree.query(bgi::covers(coordinates), std::back_inserter(found));

  for(std::vector<treeValue>::iterator it = found.begin(); it != found.end(); ++it) {
    if (it->second->covered(coordinates)) {
      Nan::Set(result, result->Length(), Nan::New(it->second->getId()).ToLocalChecked());
    }
  }

  info.GetReturnValue().Set(result);
}

NAN_MODULE_INIT(init) {
  Nan::Set(target, Nan::New("addBBox").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(addBBox)).ToLocalChecked());
  Nan::Set(target, Nan::New("queryPoint").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(queryPoint)).ToLocalChecked());
}

NODE_MODULE(BoostSpatialIndex, init)
