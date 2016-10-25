#include "spatialIndex.hpp"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

Nan::Persistent<v8::Function> SpatialIndex::constructor;

/*
 * Module initialization
 */
NAN_MODULE_INIT(SpatialIndex::init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("SpatialIndex").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "addBBox", addBBox);
  Nan::SetPrototypeMethod(tpl, "queryPoint", queryPoint);
  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("SpatialIndex").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

SpatialIndex::SpatialIndex() {

}

SpatialIndex::~SpatialIndex() {

}

NAN_METHOD(SpatialIndex::New) {
  if (info.IsConstructCall()) {
    SpatialIndex *obj = new SpatialIndex();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

/*
 * Adds a bounding box to the tree
 * addBox(id, [min_lat, min_lon, max_lat, max_lon])
 */
NAN_METHOD(SpatialIndex::addBBox) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());
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

  spi->rtree.insert(std::make_pair(*bbox, shape));
  spi->repository.insert(std::make_pair(id, shape));
}

/*
 * Gets all ids embedding the provided point coordinates
 * queryPoint(lat, lon)
 *
 * Returns an array of matching ids as strings
 */
NAN_METHOD(SpatialIndex::queryPoint) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());
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
  spi->rtree.query(bgi::covers(coordinates), std::back_inserter(found));

  for(std::vector<treeValue>::iterator it = found.begin(); it != found.end(); ++it) {
    if (it->second->covered(coordinates)) {
      Nan::Set(result, result->Length(), Nan::New(it->second->getId()).ToLocalChecked());
    }
  }

  info.GetReturnValue().Set(result);
}

NAN_MODULE_INIT(init) {
  SpatialIndex::init(target);
}

NODE_MODULE(BoostSpatialIndex, init)
