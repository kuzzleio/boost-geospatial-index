#ifndef _BOOSTSPATIALINDEX_SPATIALINDEX
#define _BOOSTSPATIALINDEX_SPATIALINDEX

#include <v8.h>
#include <node.h>
#include <nan.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include "shape.hpp"

typedef std::pair<box, std::shared_ptr<Shape>> treeValue;

const std::string toString(v8::Isolate *isolate, v8::Local<v8::Value> value) {
#if NODE_MODULE_VERSION < NODE_12_0_MODULE_VERSION
    v8::String::Utf8Value utf8(value);
#else
    v8::String::Utf8Value utf8(isolate, value);
#endif

    return *utf8;
}

class SpatialIndex : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(init);

  private:
    explicit SpatialIndex();
    ~SpatialIndex();

    // Node bridge handling
    static Nan::Persistent<v8::Function> constructor;
    static NAN_METHOD(New);

    // spatial index related methods
    static NAN_METHOD(addBBox);
    static NAN_METHOD(addCircle);
    static NAN_METHOD(addAnnulus);
    static NAN_METHOD(addPolygon);
    static NAN_METHOD(queryPoint);
    static NAN_METHOD(queryIntersect);
    static NAN_METHOD(remove);


    // The spatial index containing MBRs (minimum bounding rectangles)
    boost::geometry::index::rtree< treeValue, boost::geometry::index::rstar<16> > rtree;

    // Map geometry objects with their corresponding string id
    std::unordered_map<std::string, std::shared_ptr<Shape> > repository;
};

#endif
