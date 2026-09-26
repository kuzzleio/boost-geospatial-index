#ifndef _BOOSTSPATIALINDEX_SPATIALINDEX
#define _BOOSTSPATIALINDEX_SPATIALINDEX

#include <napi.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include "shape.hpp"

typedef std::pair<box, std::shared_ptr<Shape>> treeValue;
typedef boost::geometry::index::rtree< treeValue, boost::geometry::index::rstar<16> > rtreeType;

class SpatialIndex : public Napi::ObjectWrap<SpatialIndex> {
  public:
    static Napi::Object init(Napi::Env env, Napi::Object exports);
    explicit SpatialIndex(const Napi::CallbackInfo& info);

  private:
    // spatial index related methods
    Napi::Value addBBox(const Napi::CallbackInfo& info);
    Napi::Value addCircle(const Napi::CallbackInfo& info);
    Napi::Value addAnnulus(const Napi::CallbackInfo& info);
    Napi::Value addPolygon(const Napi::CallbackInfo& info);
    Napi::Value queryPoint(const Napi::CallbackInfo& info);
    Napi::Value queryIntersect(const Napi::CallbackInfo& info);
    Napi::Value remove(const Napi::CallbackInfo& info);


    // The spatial index containing MBRs (minimum bounding rectangles)
    rtreeType rtree;

    // Map geometry objects with their corresponding string id
    std::unordered_map<std::string, std::shared_ptr<Shape> > repository;
};

#endif
