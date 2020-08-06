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
  Nan::SetPrototypeMethod(tpl, "addCircle", addCircle);
  Nan::SetPrototypeMethod(tpl, "addAnnulus", addAnnulus);
  Nan::SetPrototypeMethod(tpl, "addPolygon", addPolygon);
  Nan::SetPrototypeMethod(tpl, "queryPoint", queryPoint);
  Nan::SetPrototypeMethod(tpl, "queryIntersect", queryIntersect);
  Nan::SetPrototypeMethod(tpl, "remove", remove);

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
    info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
  }
}

/*
 * Adds a bounding box to the tree
 * addBBox(id, min_lat, min_lon, max_lat, max_lon)
 */
NAN_METHOD(SpatialIndex::addBBox) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());

  // Checks the id parameter validity
  if (info[0]->IsUndefined() || !info[0]->IsString()) {
    info.GetReturnValue().Set(false);
    return;
  }

  std::string id = toString(info.GetIsolate(), info[0]);

  // Checks the coordinates parameters validity
  for(int i = 1; i < 5; i++) {
    if (info[i]->IsUndefined() || !info[i]->IsNumber()) {
      info.GetReturnValue().Set(false);
      return;
    }
  }

  // boost coordinates are Long,Lat, not Lat,Long
  box bbox(
    point(Nan::To<double>(info[2]).FromJust(), Nan::To<double>(info[1]).FromJust()),
    point(Nan::To<double>(info[4]).FromJust(), Nan::To<double>(info[3]).FromJust())
  );

  std::shared_ptr<Shape> shape(new Shape(id, bbox));

  spi->rtree.insert(std::make_pair(bbox, shape));
  spi->repository.insert(std::make_pair(id, shape));
}

/*
 * Adds a circle to the tree
 * addCircle(id, lat, lon, radius)
 */
NAN_METHOD(SpatialIndex::addCircle) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());

  // Checks the id parameter validity
  if (info[0]->IsUndefined() || !info[0]->IsString()) {
    info.GetReturnValue().Set(false);
    return;
  }

  std::string id = toString(info.GetIsolate(), info[0]);

  // Checks the coordinates parameters validity
  for(int i = 1; i < 4; i++) {
    if (info[i]->IsUndefined() || !info[i]->IsNumber()) {
      info.GetReturnValue().Set(false);
      return;
    }
  }

  // boost coordinates are Long,Lat, not Lat,Long
  point p(Nan::To<double>(info[2]).FromJust(), Nan::To<double>(info[1]).FromJust());

  std::shared_ptr<Shape> shape(new Shape(id, p, Nan::To<double>(info[3]).FromJust()));

  spi->rtree.insert(std::make_pair(shape->getEnvelope(), shape));
  spi->repository.insert(std::make_pair(id, shape));
}

/*
 * Adds an annulus to the tree
 * addCircle(id, lat, lon, outerRadius, innerRadius)
 */
NAN_METHOD(SpatialIndex::addAnnulus) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());

  // Checks the id parameter validity
  if (info[0]->IsUndefined() || !info[0]->IsString()) {
    info.GetReturnValue().Set(false);
    return;
  }

  std::string id = toString(info.GetIsolate(), info[0]);

  // Checks the coordinates parameters validity
  for(int i = 1; i < 5; i++) {
    if (info[i]->IsUndefined() || !info[i]->IsNumber()) {
      info.GetReturnValue().Set(false);
      return;
    }
  }

  // boost coordinates are Long,Lat, not Lat,Long
  point p(Nan::To<double>(info[2]).FromJust(), Nan::To<double>(info[1]).FromJust());

  std::shared_ptr<Shape> shape(new Shape(id, p, Nan::To<double>(info[3]).FromJust(), Nan::To<double>(info[4]).FromJust()));

  spi->rtree.insert(std::make_pair(shape->getEnvelope(), shape));
  spi->repository.insert(std::make_pair(id, shape));
}

/*
 * Adds a polygon to the tree
 * addPolygon(id, [[lat, lon], [lat, lon], [lat, lon], ...]])
 */
NAN_METHOD(SpatialIndex::addPolygon) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());

  // Checks the id parameter validity
  if (info[0]->IsUndefined() || !info[0]->IsString()) {
    info.GetReturnValue().Set(false);
    return;
  }

  std::string id = toString(info.GetIsolate(), info[0]);

  // Checks the coordinates parameters validity
  if (info[1]->IsUndefined() || !info[1]->IsArray()) {
    info.GetReturnValue().Set(false);
    return;
  }

  v8::Local<v8::Array> points = info[1].As<v8::Array>();
  polygon pl;

  // boost coordinates are Long,Lat, not Lat,Long
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  for(unsigned int i = 0; i < points->Length(); i++) {
    v8::Local<v8::Array> p = points->Get(context, i)
      .ToLocalChecked()
      .As<v8::Array>();

    pl.outer().push_back(
      point(
        Nan::To<double>(p->Get(context, 1).ToLocalChecked()).FromJust(),
        Nan::To<double>(p->Get(context, 0).ToLocalChecked()).FromJust()));
  }

  std::shared_ptr<Shape> shape(new Shape(id, pl));

  spi->rtree.insert(std::make_pair(shape->getEnvelope(), shape));
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

  point coordinates(lon->Value(), lat->Value());
  std::vector<treeValue> found;
  spi->rtree.query(bgi::covers(coordinates), std::back_inserter(found));

  for(std::vector<treeValue>::iterator it = found.begin(); it != found.end(); ++it) {
    if (it->second->covered(coordinates)) {
      Nan::Set(result, result->Length(), Nan::New(it->second->getId()).ToLocalChecked());
    }
  }

  info.GetReturnValue().Set(result);
}

/*
 * Gets all ids embedding the provided point coordinates
 * queryIntersect([[lat, lon], [lat, lon], [lat, lon], ...]])
 *
 * Returns an array of matching ids as strings
 */
NAN_METHOD(SpatialIndex::queryIntersect) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());

  v8::Local<v8::Array> result = Nan::New<v8::Array>();
  
  // Checks the coordinates parameters validity
  if (info[0]->IsUndefined() || !info[0]->IsArray()) {
    info.GetReturnValue().Set(result);
    return;
  }

  v8::Local<v8::Array> points = info[0].As<v8::Array>();
  polygon queryPoly;

  // note: flip coordinates from lat,long to long,lat to abide by boost
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
  for(unsigned int i = 0; i < points->Length(); i++) {
    v8::Local<v8::Array> p = points->Get(context, i)
      .ToLocalChecked()
      .As<v8::Array>();

    queryPoly.outer().push_back(
      point(
        Nan::To<double>(p->Get(context, 1).ToLocalChecked()).FromJust(),
        Nan::To<double>(p->Get(context, 0).ToLocalChecked()).FromJust()));
  }

  std::vector<treeValue> found;
  //calling intersects here, pure inside polygon check would be covered_by
  spi->rtree.query(bgi::intersects(queryPoly), std::back_inserter(found));

  for(std::vector<treeValue>::iterator it = found.begin(); it != found.end(); ++it) {
      Nan::Set(result, result->Length(), Nan::New(it->second->getId()).ToLocalChecked());
  }

  info.GetReturnValue().Set(result);
}

/*
 * Removes an object from the index
 * remove(id)
 *
 * Returns a boolean
 */
NAN_METHOD(SpatialIndex::remove) {
  Nan::HandleScope scope;
  SpatialIndex *spi = Nan::ObjectWrap::Unwrap<SpatialIndex>(info.This());

  if (info[0]->IsUndefined() || !info[0]->IsString()) {
    info.GetReturnValue().Set(false);
    return;
  }

  std::string id = toString(info.GetIsolate(), info[0]);
  std::unordered_map<std::string, std::shared_ptr<Shape> >::const_iterator found = spi->repository.find(id);

  if (found == spi->repository.end()) {
    info.GetReturnValue().Set(false);
    return;
  }

  spi->rtree.remove(std::make_pair(found->second->getEnvelope(), found->second));
  spi->repository.erase(id);

  info.GetReturnValue().Set(true);
}

NAN_MODULE_INIT(init) {
  SpatialIndex::init(target);
}

NODE_MODULE(BoostSpatialIndex, init)
